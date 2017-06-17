/************************************ 
********* HEADER PARA SERVER ********
************************************/

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

/*  Sincroniza o servidor com o diretorio 
    “sync_dir_<nomeusuario>” com o cliente*/
void sync_server();

/*  Recebe um arquivo file do cliente.
    Deverá ser executada quando for realizar upload de um arquivo.
    file – path/filename.ext do arquivo a ser recebido */
void receive_file(char *file);

/*  Envia o arquivo file para o usuário.
    Deverá ser executada quando for realizar download de um arquivo.
    file – filename.ext */
void send_file(char *file);


/* ACIMA ESTÃO AS FUNCIONALIDADES MÍNIMAS PARA O SERVIDOR */

int validateServerArguments(int argc, char *argv[]);
//procura usuario na fila de clients
int searchForUserId(PFILA2 fila, char *userId);
//procura usuario, adiciona 1 a lista de devices conectados, logged_in = 1 
int secondLogin(PFILA2 fila, char *userId);
//recorta UserId do login auxiliar
char *cropUserId(char *auxSocketName);
/*
    desconecta os sockets do client

    se existedBefore
    0 = não existia o usuário no sistema, portanto não precisa retirar da lista
    1 = existia antes, portanto precisamos retirar ele da lista

    clientList = lista de clientes do servidor atualmente
    auxSocketsList = lista de sockets auxiliares no servidor atualmente
    syncSocketList = lista de sockets de sincronização no servidor atualmente
*/
void disconnectClientFromServer(int auxSocket, int syncSocket, char *userId, PFILA2 clientList, PFILA2 auxSocketsList, PFILA2 syncSocketList, int existedBefore);
void disconnectClient(int newsockfd);