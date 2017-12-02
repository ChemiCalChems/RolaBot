FROM ubuntu:latest

RUN  apt-get clean && apt-get update && \
DEBIAN_FRONTEND=noninteractive apt-get install -y libmysql++3v5 mysql-server-5.7

RUN useradd --create-home -s /bin/bash user
WORKDIR /home/user
USER user
RUN mkdir /home/user/mysql-files

VOLUME /home/user/mysql-files

COPY build /test
COPY start.sh /home/user/start.sh
CMD ["/bin/bash", "./start.sh"]

