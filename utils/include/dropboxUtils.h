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

#define SUCCESS 1
#define LISTSUCCESS 0
#define ERROR -1
#define MAXNAME 40
#define MAXFILES 50
#define BUFFERSIZE 4096
#define DEFAULTPORT 12001
#define SERVER 1
#define CLIENT 0
#define TIMESIZE 20
#define MAXDEVICES 2
#define TRUE 1
#define FALSE 0

typedef struct	file_info	{
    char name[MAXNAME]; // refere-se ao nome do arquivo
    char lastModified[MAXNAME]; // refere-se a data da ultima mofidicacao no arquivo
    int size; // tamanho do arquivo em bytes.
    pthread_mutex_t fileMutex; 
} userFile; //TODO UserFiles -> UserFile

typedef struct client	{
    int numDevices; // dispositivos de usuario
    char userId[MAXNAME]; // id do usuario no servidor, que devera ser unico. Informado pela linha de comando
    char lastModification[TIMESIZE]; //datetime of the last modification
    struct chain_list* fileList; //TODO ALTERADO PARA UMA LISTA
    pthread_mutex_t loginMutex; 
    int loggedIn; // cliente esta logado ou nao
} clientInfo;

typedef struct chain_list {
  struct chain_node* header;
  int size;
  pthread_mutex_t lock;
} chain_list;

typedef struct chain_node {
    clientInfo* client;
    userFile* file;
    struct chain_node* next;
} chain_node;


typedef struct socketsClient {
    int commandsSocket; // numeros de dispositivos de usuario
    int syncSocket; // numeros de dispositivos de usuario
    char userId[MAXNAME]; //metadados de cada arquivo que o cliente possui no servidor
} socketsStruct;


struct chain_list* chain_create_list(); //Cria uma lista de arquivos vazia
struct chain_node* chain_create_client_node(char* userId);
struct chain_node* chain_create_file_node(char* name, char* lastModified, int size);
int chain_add(chain_list* list, chain_node* node); //Adiciona um arquivo a lista
int chain_remove(chain_list* list, char* chainname); //Remove o arquivo com o nome chainname da lista.
struct chain_node* chain_find(chain_list* list, char *chainname); //Busca um arquivo pelo nome (chainname). Se não encontrar retorna NULL.
int chain_clear(chain_list* list); //Limpa toda a lista
int chain_print(chain_list* list); //Imprime toda a lista para fins de debug

void createDirectory(char *userId, int isServer);
char* getUserDirectory(char *userId);
int send_(int socket, char *filename);
int receive_(int socket, char path[255]);
struct stat getAttributes(char* pathFile);
int isRegularFile(struct dirent *file);
char *receiveMessage (int socket, char *condition, int isCondition);
void sendMessage (int socket, char *buffer);
void updateLocalTime(char *newDate);
char* fileListToArray(chain_list* list);
void getFilesFromUser(char* userId, chain_list* list, int isServer, char *lastModification);