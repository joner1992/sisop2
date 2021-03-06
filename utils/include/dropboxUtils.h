#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include "./support.h"

#define SUCCESS 1
#define LISTSUCCESS 0
#define ERROR -1
#define MAXNAME 24
#define MAXFILES 100
#define BUFFERSIZE 1024
#define DEFAULTPORT 12001
#define DISCONNECTED "BYE!"