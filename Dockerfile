FROM ubuntu:latest

RUN  apt-get clean && apt-get update && \
DEBIAN_FRONTEND=noninteractive apt-get install -y libmysql++3v5 mysql-server-5.7

VOLUME /var/lib/mysql

COPY build /test
COPY start.sh /
CMD ["/bin/bash", "./start.sh"]

