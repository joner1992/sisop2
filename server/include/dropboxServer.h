/************************************ 
********* HEADER PARA SERVER ********
************************************/

pthread_mutex_t acceptingConnection;
pthread_mutex_t userVerificationMutex;
pthread_mutex_t disconnectMutex;
pthread_mutex_t clientListMutex;
pthread_mutex_t auxSocketsListMutex;
pthread_mutex_t syncSocketsListMutex;

/*  Sincroniza o servidor com o diretorio 
    “sync_dir_<nomeusuario>” com o cliente*/
void syncClientServer(int isServer, int socketId, char *userId, PFILA2 fileList);

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
    se isAux
    0 = é sync
    1 = é aux
*/
void disconnectClientFromServer(int socket, char *userId, PFILA2 auxSocketsList, PFILA2 syncSocketList, int isAux);
void disconnectClient(int newsockfd);
char *listFiles(PFILA2 clientList, char *userId, int socket);
char *getFiles(char *buffer, PFILA2 fila, int socket);
char *getServerFileNames(PFILA2 fileList);
char *compareDatesFromFileList(PFILA2 fileList, char *fileName, char *dateOfFile, int isServer);