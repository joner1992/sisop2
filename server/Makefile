##### MAKEFILE FOR SERVER ####
#

# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "server"
# 

CC=gcc
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/
MAIN_DIR=./main/
UTILS_SRC=../utils/src/
UTILS_BIN=../utils/bin/
CFLAGS=-c
OBJS=dropboxUtils.o dropboxServer.o
MOUNTED_OBJS=$(UTILS_BIN)dropboxUtils.o $(BIN_DIR)dropboxServer.o


all: server

server: clean $(OBJS)
	$(CC) $(MAIN_DIR)server.c -Wall -o server $(MOUNTED_OBJS)
    
dropboxServer.o: $(SRC_DIR)dropboxServer.c
	$(CC) $(CFLAGS) $(SRC_DIR)dropboxServer.c -Wall -o $(BIN_DIR)dropboxServer.o

dropboxUtils.o: $(UTILS_SRC)dropboxUtils.c
	$(CC) $(CFLAGS) $(UTILS_SRC)dropboxUtils.c -Wall -o $(UTILS_BIN)dropboxUtils.o

clean:
	find $(BIN_DIR) -type f ! -name '*.c' ! -name 'Makefile' -delete