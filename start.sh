service mysql start

if ! mysql -uroot -prootpass -e 'use roladata' > /dev/null 2>&1; then
	echo "Could not log in to database roladata, or root password was not set... setting up the database..." 

	mysqladmin -u root password rootpass
	
	mysql -uroot -prootpass -e "create database if not exists roladata; grant all privileges on roladata.* to 'rolauser'@'localhost' identified by 'passwd'; flush privileges; 
	create table roladata.juegos ( 
		   id int not null auto_increment, 
		   nombre text not null, 
		   tipo_juego int not null,
		   defectos text, 
		   primary key(id) 
	); 
	create table roladata.prestamos (
		   num_prestamo int not null auto_increment,  
		   id_juego int not null, 
		   id_socio int not null,
		   fecha_prestamo date not null, 
		   fecha_devolucion date,
		   fecha_devolucion_max date not null, # 7 dias + fecha_prestamo
		   primary key(num_prestamo) 
	); 
	create table roladata.reservas (  
		   id_juego int not null, 
		   id_socio int not null, 
		   fecha_inicio date not null,
		   fecha_final date not null 
	);  
	create table roladata.socios ( 
		   id int not null auto_increment, 
		   fecha date not null, 
		   nombre text not null,
		   email text not null , 
		   dni text not null, 
		   fisicas_o_no bool not null, 
		   carne_expedido bool not null, 
		   cuentas bool not null, 
		   primary key (id) 
	);"	
fi

/test/gestion


