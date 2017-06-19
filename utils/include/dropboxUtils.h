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
#include <libgen.h>
#include <time.h>
#include <dirent.h>
#include "./support.h"

#define SUCCESS 1
#define LISTSUCCESS 0
#define ERROR -1
#define MAXNAME 40
#define MAXFILES 50
#define BUFFERSIZE 4096
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
 time_t lastModification;
 pthread_mutex_t downloadUploadMutex;
 pthread_mutex_t fileListMutex;
} ClientInfo;

typedef struct auxclient {
 int socketId; // numeros de dispositivos de usuario
 char userId[MAXNAME]; //metadados de cada arquivo que o cliente possui no servidor
} clientThread;

void createDirectory(char *argv, int server);
char* getUserDirectory(char *userId);
void initializeList(PFILA2 list);
int searchForFile(char *fileName, PFILA2 fileList);
int addFileToUser(char *name, char *extension, char *lastModified, int size, PFILA2 fileList);
int removeFileFromUser(char *fileName, PFILA2 fileList, char *userId, int isServer);
int send_(int socket, char *filename);
int receive_(int socket, char path[255]);
struct stat getAttributes(char* pathFile);
void getFilesFromUser(char* userId, PFILA2 filesList, int server);
int isRegularFile(struct dirent *file);
/*
    se o printing for
    2 = printa sem pular linhas entre os receives
    1 = printa pulando linhas entre os receives
    0 = não printa

*/
char *receiveMessage (int socket, char *conditionToStop, int printing);
int cleanList(PFILA2 fileList, char *fileName);
char *getListFilesFromUser(char *buffer, PFILA2 fila, int isServer);
