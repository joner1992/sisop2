include Makefile.compile

all: client server

run-cliente:
	./cliente -ip localhost -p 12001 -U rodrigo

run-servidor:
	./servidor -p 12001

run:
	pkill servidor & ./servidor -p 12001 & ./cliente -ip localhost -p 12001 -U rodrigo
