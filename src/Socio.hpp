#pragma once

#include <mysql++/mysql++.h>
#include "Conn.hpp"

struct Socio {
	int id_socio = 0;
	
	Socio(int id_socio) : id_socio(id_socio) {};

	void sancionar(int dias) {
		auto query = conn.query();
		query << "update socios set sancionado_hasta = if(sancionado_hasta, sancionado_hasta + interval %0q day, curdate() + interval %0q day) where id = %1q";
		query.parse();
		query.execute(dias, id_socio );
	}
	
	bool sancionado() {
		auto query = conn.query();
		query << "select sancionado_hasta from socios where id = %0q;";
		query.parse();
		auto queryResult = query.store(id_socio);
		if (queryResult.num_rows() > 0) {
			if (queryResult[0]["sancionado_hasta"] != "NULL") {
				return true;
			}
		}
		return false;
	}

	bool tienePrestamosVencidos() {
		auto query = conn.query();
		query << "select 1 from prestamos where id_socio = %0q and datediff(curdate(), fecha_devolucion_max) > 0";
		query.parse();
		if (!query.store(id_socio).empty()) return true;
		return false;
	}
};

