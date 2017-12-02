#include <iostream>
#include <ctime>
#include <mysql++/mysql++.h>


mysqlpp::Connection conn;

struct Fecha {
	int dia, mes, ano;
	std::string to_string() {
		return std::to_string(ano) + "-" + std::to_string(mes) + "-" + std::to_string(dia);
	}
};
	
enum TipoJuego {
	verde = 0, amarillo = 1, rojo = 2 
};

void nuevoSocio() {
	auto query = conn.query();
	try {
		query << "insert into socios (sancionado_hasta) values ('2017-11-20');";
		query.execute();
	}
	catch(...) {
		std::cerr << query.error() << std::endl;
	}
}

void nuevoJuego(std::string nombre, int tipoJuego) {
	auto query = conn.query();
	try {
		query << "insert into juegos (nombre, tipo_juego) values ( %0q , %1q );";
		query.parse();
		query.execute(nombre, tipoJuego);
	}
	catch(...) {
		std::cerr << query.error() << std::endl; 
	}
}

void nuevoPrestamo(int id_juego, int id_socio) {
	auto query = conn.query();
	try {
		//Ver si el usuario esta o no sancionado
		query << "select sancionado_hasta from socios where id = %0q;";
		query.parse();
		auto sancion = query.store(id_socio)[0][0];
		if (sancion != "NULL") {
			std::cout << "El usuario esta sancionado hasta el dia " << sancion << std::endl;
		}

		
		//Si hay una reserva activa sobre el juego que 
		query << "select id_socio, datediff (fecha_final, curdate()) from reservas where id_juego = %0q";
		query.parse();
		auto reservas = query.store(id_juego);
		
		if (reservas.num_rows() > 0) {
			if(reservas[0]["id_socio"] != std::to_string(id_socio)) {
				int proximaReserva = reservas[0][1];
				if (proximaReserva <= 7) {
					std::cout << "juego ya reservado mientras se prestaba" << std::endl;
					return; //quejarse en el futuro
				}
			}
		}

		query.reset();
		
		query << "select 1 from prestamos where id_juego = %0q";
		query.parse();
		auto prestamos = query.store(id_juego);
		
		if (prestamos.num_rows() > 0) {
			std::cout << "juego esta prestado" << std::endl;
		}

		query.reset();
		
		query << "insert into prestamos (id_juego, id_socio, fecha_prestamo, fecha_devolucion_max) values (%0q, %1q, curdate(), curdate() + interval 7 day);";
		query.parse();
		query.execute(id_juego, id_socio);
	}
	catch(...) {
		std::cerr << query.error() << std::endl;
	}
}

void finalizarPrestamo(int id) {
	auto query = conn.query();
	try {
		query << "select 1 from prestamos where id_juego = %0q;";
		query.parse();
		
		if (query.store(id).num_rows() != 0) {
			query.reset();
			query << "delete from prestamos where id_juego = %0q;";
			query.parse();
			query.execute(id);
			std::cout << "Game returned correctly" << std::endl;
		}
		else {
			std::cerr << "Game not loaned!" << std::endl;
		}
	}
	catch(...) {
		std::cerr << query.error() << std::endl;
	}
}

void nuevaReserva(int id_juego, int id_socio, Fecha fecha_inicio) {
	auto query = conn.query();
	try {
		// Un juego no puede ser reservado si una reserva activa para este acaba menos de una semana antes de que la nueva reserva comience.
		query << "select id_socio, datediff (fecha_final, %1q) as deltaT from reservas where id_juego = %0q";
		query.parse();
		auto reservas = query.store(id_juego, fecha_inicio.to_string());

		//Si hay reservas activas para el juego, hemos de comprobar que la nueva reserva pudiese ser tramitada.
		if (reservas.num_rows() > 0) {
			int proximaReserva = reservas[0]["deltaT"];
			for (auto row : reservas) {
				if (row["id_socio"] == std::to_string(id_socio)) {
					//Un usuario no puede reservar un mismo juego mas de una vez al mismo tiempo
					std::cout << "el socio ya ha reservado el juego" << std::endl;
					return;
				}

				//Solo hemos de tener en cuenta la reserva que comience antes que cualquier otra para comprobar si los plazos de la nueva reserva se solapan o no con otra.
				proximaReserva = (int)row["deltaT"] < proximaReserva ? row["deltaT"] : proximaReserva; 
			}
			
			if (proximaReserva <= 7) {
				std::cout << "juego ya reservado durante reserva" <<  proximaReserva <<std::endl;
				return;
			}
		}
		
		query.reset();

		//Un juego tampoco puede ser reservado si el prestamo activo acaba despues de que empiece la reserva
		query << "select datediff (fecha_devolucion_max, %1q) from prestamos where id_juego = %0q";
		query.parse();
		auto prestamos = query.store(id_juego, fecha_inicio.to_string());
		
		if (prestamos.num_rows() > 0) {
			int proximoPrestamo = prestamos[0][0];
			if (proximoPrestamo <= 7){
				std::cout << "juego va a estar prestado durante la reserva" << std::endl;
				return;
			}
		}
		
		query.reset();

		query << "insert into reservas (id_juego, id_socio, fecha_inicio, fecha_final) values (%0q, %1q, %2q, fecha_inicio + interval 7 day);";
		query.parse();
		query.execute(id_juego, id_socio, fecha_inicio.to_string());
	}
	catch(...) {
		std::cerr << query.error() << std::endl;
	}
}

void finalizarReserva(int id_juego, int id_socio) {
	auto query = conn.query();
	try {
		query << "select 1 from reservas where id_juego = %0q;";
		query.parse();
		
		if (query.store(id_juego).num_rows() != 0) {
			query.reset();
			query << "delete from reservas where id_juego = %0q, id_socio = %1q;";
			query.parse();
			query.execute(id_juego, id_socio);
			std::cout << "Reserva finalizada del juego" << std::endl;
		}
		else {
			std::cerr << "Game not loaned!" << std::endl;
		}
	}
	catch(...) {
		std::cerr << query.error() << std::endl;
	}
}

int main(int argc, char** argv) {
	if (conn.connect("roladata", "127.0.0.1:3306", "rolauser", "passwd")) std::cout << "Connected!" << std::endl;

	nuevoSocio();
	nuevoJuego("top kek", TipoJuego::verde);
	nuevoJuego("allahu akbar", TipoJuego::rojo);
	
	auto r = conn.query("select * from reservas").store();
	for (auto row: r) {
		for (auto field : row) std::cout << std::left << std::setw(20) << field;
		std::cout << std::endl;
	}
	
	nuevoPrestamo(1, 1);
	
	/*auto result = conn.query("select * from juegos;").store();

	for (auto row : result) {
		for (auto field : row) std::cout << std::left << std::setw(20) << field;
		std::cout << '\n';
	}
	std::cout << "done" << std::endl;
	*/
}
