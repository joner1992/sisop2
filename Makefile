include Makefile.compile

all: server client 

run-cliente:
	./cliente -ip localhost -p 12001 -U USUARIO

run-servidor:
	./servidor -p 12001

run:
	./servidor -p 12001 & ./cliente -ip localhost -p 12001 -U USUARIO

runTests: testeCompile
	./servidor -p 12001 & ./cliente -ip localhost -p 12001 ./files/in/sockets.pdf

show:
	ps aux | grep ./cliente
	ps aux | grep ./servidor
