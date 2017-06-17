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
#include <sys/stat.h>
#include "./support.h"

#define SUCCESS 1
#define LISTSUCCESS 0
#define ERROR -1
#define MAXNAME 24
#define MAXFILES 100
#define BUFFERSIZE 1024
#define DEFAULTPORT 12001
#define DISCONNECTED "BYE!"
#define SERVER 1
#define CLIENT 0
#define MAXDEVICES 2
#define DISCONNECTEXISTEDBEFORE 1
#define DISCONNECT 0

// Estruturas estarao dispostas no servidor na forma de uma lista encadeada de clientes
typedef struct	file_info	{
  char name[MAXNAME]; // refere-se ao nome do arquivo
  char extension[MAXNAME]; // refere-se ao tipo de extensao do arquivo
  char last_modified[MAXNAME]; // refere-se a data da ultima mofidicacao no arquivo
  int size; // tamanho do arquivo em bytes.
  pthread_mutex_t fileMutex;
} UserFiles;

typedef struct client	{
 int numDevices; // numeros de dispositivos de usuario
 char userId[MAXNAME]; // id do usuario no servidor, que devera ser unico. Informado pela linha de comando
 FILA2 filesList; //metadados de cada arquivo que o cliente possui no servidor
 int logged_in; // cliente esta logado ou nao
 pthread_mutex_t loginMutex;
} ClientInfo;

typedef struct auxclient {
 int socketId; // numeros de dispositivos de usuario
 char userId[MAXNAME]; //metadados de cada arquivo que o cliente possui no servidor
} clientThread;

void createDirectory(char *argv, int server);
void initializeList(PFILA2 list);
int searchForFile(char *fileName, PFILA2 fileList);
int addFileToUser(char *name, char *extension, char *lastModified, int size, FILA2 fileList);
int removeFileFromUser(char *fileName, FILA2 fileList);
