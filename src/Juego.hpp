#pragma once

#include "Socio.hpp"
#include "Conn.hpp"

struct Juego {
	int id_juego = 0;

	Juego(int id_juego) : id_juego(id_juego) {};

	/*
	void reservar(int id_socio, std::string fecha) {
		if (Socio(id_socio).sancionado()) return; //Un socio sancionado no puede reservar juegos

		auto query = conn.query();
		// Un juego no puede ser reservado si una reserva activa para este acaba menos de una semana antes de que la nueva reserva comience.
		query << "select * from reservas where id_juego = %0q";
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
	*/
	
	void prestar(int id_socio) {
		auto query = conn.query();

		Socio socio {id_socio};
		
		//Si el usuario esta sancionado o tiene prestamos activos vencidos, no prestarle el juego
		if(socio.sancionado() && socio.tienePrestamosVencidos()) return;
	
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
	
		query << "select 1 from prestamos where id_juego = %0q and fecha_devolucion = NULL";
		query.parse();
		auto prestamos = query.store(id_juego);
		
		if (!prestamos.empty()) {
			std::cout << "juego esta prestado" << std::endl;
		}

		query.reset();

		query << "select es_libro from juegos where id = %0q";
		query.parse();
		auto es_libro = (bool)query.store(id_juego)[0][0];
		query.reset();
		
		query << "insert into prestamos (id_juego, id_socio, fecha_prestamo, fecha_devolucion_max) values (%0q, %1q, curdate(), curdate() + interval %2q day);";
		query.parse();

		//Los libros se prestan una semana (7 dias) extra.
		query.execute(id_juego, id_socio, 7 + 7*es_libro);
	}

	void devolver() {
		//Comprobar si el juego está prestado, y a qué socio
		auto query = conn.query();
		query << "select id_socio from prestamos where id_juego = %0q;";
		query.parse();
		auto results = query.store(id_juego);
		
		if (results.num_rows() != 0) { //Si lo está, comprobar si ha sido devuelto con retraso
			auto id_socio = results[0]["id_socio"];
			query.reset();
			
			query << "select datediff(curdate(), fecha_devolucion_max) as retraso from prestamos where id_juego=%0q";
			query.parse();
			auto retraso = query.store(id_juego)[0]["retraso"];
			query.reset();
			
			if ((int)retraso > 0) {
				std::cout << "El juego se devolvio con un retraso de " << retraso << " dias" << '\n';
				Socio{id_socio}.sancionar(retraso);
			}

			//No borramos el prestamo porque queremos mantener un registro de los prestamos antiguos
			//Simplemente añadimos la fecha de devolucion y ya
			query << "update prestamos set fecha_devolucion = curdate() where id_juego = %0q";
			query.parse();
			query.execute(id_juego);
			
			std::cout << "Game returned correctly" << std::endl;
		}
		else {
			std::cerr << "Game not loaned!" << std::endl;
		}
	}
};
