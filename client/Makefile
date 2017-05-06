##### MAKEFILE FOR CLIENT #####
#

# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "client"
# 

CC=gcc
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/
MAIN_DIR=./main/
UTILS_SRC=../utils/src/
UTILS_BIN=../utils/bin/
CFLAGS=-c
OBJS=dropboxUtils.o dropboxClient.o
MOUNTED_OBJS=$(UTILS_BIN)dropboxUtils.o $(BIN_DIR)dropboxClient.o


all: client

client: clean $(OBJS)
	$(CC) $(MAIN_DIR)client.c -Wall -o client $(MOUNTED_OBJS)
    
dropboxClient.o: $(SRC_DIR)dropboxClient.c
	$(CC) $(CFLAGS) $(SRC_DIR)dropboxClient.c -Wall -o $(BIN_DIR)dropboxClient.o

dropboxUtils.o: $(UTILS_SRC)dropboxUtils.c
	$(CC) $(CFLAGS) $(UTILS_SRC)dropboxUtils.c -Wall -o $(UTILS_BIN)dropboxUtils.o

clean:
	find $(BIN_DIR) -type f ! -name '*.c' ! -name 'Makefile' -delete