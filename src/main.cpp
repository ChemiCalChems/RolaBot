#include <iostream>
#include <ctime>
#include <mysql++/mysql++.h>
#include "Conn.hpp"
#include "Juego.hpp"

enum TipoJuego {
	verde = 0, amarillo = 1, rojo = 2 
};


void nuevoSocio() {
	auto query = conn.query();
	try {
		query << "insert into socios () values ();";
		query.execute();
	}
	catch(...) {
		std::cerr << query.error() << std::endl;
	}
}

void nuevoJuego(std::string nombre, int tipoJuego, bool es_libro) {
	auto query = conn.query();
	try {
		query << "insert into juegos (nombre, color_juego, es_libro) values ( %0q , %1q, %2q );";
		query.parse();
		query.execute(nombre, tipoJuego, es_libro);
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
	nuevoJuego("top kek", TipoJuego::verde, true);
	nuevoJuego("allahu akbar", TipoJuego::rojo, false);

	std::cout << "Prestando juego" << std::endl;

	Juego{1}.prestar(1);
	
	Juego{2}.prestar(1);

	auto result = conn.query("select fecha_devolucion_max from prestamos;").store();

	for (auto row : result) {
		for (auto field : row) std::cout << std::left << std::setw(20) << field;
		std::cout << '\n';
	}
	std::cout << "done" << std::endl;
	
}
