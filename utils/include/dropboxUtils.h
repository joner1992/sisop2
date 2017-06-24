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
#define MAXFILES 100
#define BUFFERSIZE 1024
#define DEFAULTPORT 12001
#define DISCONNECTED "BYE!"
#define SERVER 1
#define CLIENT 0
#define MAXDEVICES 2
#define DISCONNECTEXISTEDBEFORE 1
#define DISCONNECT 0

typedef struct	file_info	{
    char name[MAXNAME]; // refere-se ao nome do arquivo
    char extension[MAXNAME]; // refere-se ao tipo de extensao do arquivo
    char last_modified[MAXNAME]; // refere-se a data da ultima mofidicacao no arquivo
    int size; // tamanho do arquivo em bytes.
    pthread_mutex_t fileMutex; 
} UserFile; //TODO UserFiles -> UserFile

typedef struct client	{
    int devices[2]; // dispositivos de usuario
    char userId[MAXNAME]; // id do usuario no servidor, que devera ser unico. Informado pela linha de comando
    struct chain_list* filesList; //TODO ALTERADO PARA UMA LISTA
    int logged_in; // cliente esta logado ou nao
    pthread_mutex_t loginMutex;
} Client_Info;
// END OF IMPORT

typedef struct chain_list {
  struct chain_node* header;
  int size;
  pthread_mutex_t* lock;
} chain_list;

typedef struct chain_node {
  Client_Info* client;
  UserFile* file;
  struct chain_node* next;
} chain_node;


typedef struct auxclient {
 int socketId; // numeros de dispositivos de usuario
 char userId[MAXNAME]; //metadados de cada arquivo que o cliente possui no servidor
} clientThread;


struct chain_list* chain_create_list(); //Cria uma lista de arquivos vazia
struct chain_node* chain_create_client_node(char* userId);
struct chain_node* chain_create_file_node(char* name, char* extension, char* last_modified, int size);
int chain_add(chain_list* list, chain_node* node); //Adiciona um arquivo a lista
int chain_remove(chain_list* list, char* chainname); //Remove o arquivo com o nome chainname da lista.
struct chain_node* chain_find(chain_list* list, char *chainname); //Busca um arquivo pelo nome (chainname). Se não encontrar retorna NULL.
int chain_clear(chain_list* list); //Limpa toda a lista
int chain_print(chain_list* list); //Imprime toda a lista para fins de debug

void createDirectory(char *argv, int server);
char* getUserDirectory(char *userId);
void initializeList(PFILA2 list);
int searchForFile(char *fileName, PFILA2 fileList);
int addFileToUser(char *name, char *extension, char *lastModified, int size, PFILA2 fileList);
int removeFileFromUser(char *fileName, PFILA2 fileList);
int send_(int socket, char *filename);
int receive_(int socket, char path[255]);
struct stat getAttributes(char* pathFile);
void getFilesFromUser(char* userId, PFILA2 filesList, int isServer);
int isRegularFile(struct dirent *file);
/*
    se o printing for
    2 = printa sem pular linhas entre os receives
    1 = printa pulando linhas entre os receives
    0 = não printa

*/
char *receiveMessage (int socket, char *conditionToStop, int printing);