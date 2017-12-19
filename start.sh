echo This is a test

ls -al /home/user/mysql-files

mkdir /home/user/mysql-files/data

echo Initializing database...

mysqld --no-defaults --initialize-insecure --datadir=/home/user/mysql-files/data

echo Starting database...

mysqld --no-defaults --datadir=/home/user/mysql-files/data --secure-file-priv="" --socket="" --port=3306 &

sleep 2

echo Praying to god...

if ! mysql -h 127.0.0.1 --port=3306 -uroot -prootpass -e 'use roladata' > /dev/null 2>&1; then
	echo "Could not log in to database roladata, or root password was not set... setting up the database..." 

	mysqladmin -h 127.0.0.1 --port=3306 -uroot password rootpass
	
	mysql -h 127.0.0.1 --port=3306 -uroot -prootpass -e "create database if not exists roladata; grant all privileges on roladata.* to 'rolauser'@'localhost' identified by 'passwd'; flush privileges; 
	create table roladata.juegos ( 
		   id int not null auto_increment, 
		   nombre text not null, 
		   color_juego int not null,
		   es_libro bool not null,
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
		   # fecha date not null, 
		   # nombre text not null,
		   # email text not null , 
		   # dni text not null, 
		   # fisicas_o_no bool not null, 
		   # carne_expedido bool not null, 
		   # cuentas bool not null,
		   sancionado_hasta date,
		   primary key (id) 
	);"	
fi

/test/gestion
kill $(pgrep mysqld)
sleep 5
echo Done!

