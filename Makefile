include Makefile.compile

all: client server

run-cliente:
	./cliente -ip localhost -p PORTA -U USUARIO

run-servidor:
	./servidor -p PORTA

run:
	./servidor -p PORTA & ./cliente -ip localhost -p PORTA -U USUARIO
