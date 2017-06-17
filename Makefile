include Makefile.compile

all: client server

run-cliente: client
	./cliente -ip localhost -p 12001 -U rodrigo

run-servidor: server
	./servidor -p 12001

run:
	pkill servidor & ./servidor -p 12001 & ./cliente -ip localhost -p 12001 -U rodrigo
