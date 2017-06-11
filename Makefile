include Makefile.compile

all: server client 

run-cliente:
	./cliente -ip localhost -p 12001 -U USUARIO

run-servidor:
	./servidor -p 12001

run:
	./servidor -p PORTA & ./cliente -ip localhost -p PORTA -U USUARIO

show:
	ps aux | grep ./cliente
	ps aux | grep ./servidor
