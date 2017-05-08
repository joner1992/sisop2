#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#define SUCCESS 1
#define ERROR -1
#define MAXNAME 12
#define MAXFILES 100
#define BUFFERSIZE 1024
#define DEFAULTPORT 12001
