##### MAKEFILE FOR UTIL #####
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "utils"
#

CC=gcc
CFLAGS=-c -pthread
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/


all: clean util

util: $(SRC_DIR)dropboxUtils.c
	$(CC) $(CFLAGS) $(SRC_DIR)dropboxUtils.c -Wall -o $(BIN_DIR)dropboxUtils.o

clean:
	find $(BIN_DIR) -type f ! -name '*.c' ! -name 'Makefile' -delete