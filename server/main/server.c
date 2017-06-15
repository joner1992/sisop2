#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxServer.h"


//Variables for socket
int sockfd;
struct sockaddr_in serv_addr;
int server_port;
// Queue for clients structs
FILA2 clientList;
FILA2 auxSocketsList;
pthread_mutex_t userVerificationMutex;


void initializeList(PFILA2 list){
  if(CreateFila2(list) != LISTSUCCESS) { // 0 = linked list initialized successfully
    perror("ERROR initializing linked list");
    exit(ERROR);
  }
}

int verifyAuxSocket(char *buffer) {
  char buffercmp[BUFFERSIZE];
  strcpy(buffercmp, buffer);
  if(buffercmp[0] == 'a' && buffercmp[1] == 'u' && buffercmp[2] == 'x'){
    return SUCCESS;        
  }
  return ERROR;
}

void verifyUserAuthentication(char *buffer, int newsockfd) {
  
  if(searchForUserId(&clientList, buffer) == SUCCESS) {
    //cria uma thread pro SYNC e soma 1 no num_devices do user
    //
    
    
  }
  else {
    Client_Info *firstTimeUser = (Client_Info*) malloc(sizeof(Client_Info));
    strcpy(firstTimeUser->userId, buffer);
    firstTimeUser->numDevices = 1;
    initializeList(&(firstTimeUser->filesList));    
    createDirectory(buffer, SERVER);
  }
  
}

int getPort(char *argv) {
  char *endptr;
  
  if(strtoimax(argv, &endptr,10) <= 0) {
    return ERROR;
  }
  return strtoimax(argv, &endptr,10);

}

void createServerSocket(){
   // CREATE SOCKET
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
    perror("ERROR: Failed in socket creation");
    exit(ERROR);
  }
}

void bindServerSocket() {
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(server_port); //CHANGE LINE!!!
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(serv_addr.sin_zero), 8);     
  
  // BIND SOCKET
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == ERROR) {
    perror("ERROR on binding server socket");
    exit(ERROR);
  }
}

void *readUser(void* arg) {
  
  int *newsockfd_ptr = (int *) arg; 
  int newsockfd = *newsockfd_ptr; 
  int n;
  char buffer[BUFFERSIZE];

  //RECEIVE FILE AND SYNC
  while(1){
   
   
  }
}

void *writeUser(void* arg){

  int *newsockfd_ptr = (int *) arg; 
  int newsockfd = *newsockfd_ptr; 
  int n;
  char buffer[BUFFERSIZE];

  //SEND FILE AND SYNC

  while(1){
    //WRITE TO THE USER
    bzero(buffer, BUFFERSIZE);

     //void void
  }
}

void *auxClientThread(void* auxThread){
  int n;
  char buffer[BUFFERSIZE];
  clientThread *newAuxThread = auxThread;

  while(1){ 
    bzero(buffer, BUFFERSIZE);
    n = read(newAuxThread->socketId, buffer, BUFFERSIZE);
    if (n == ERROR) {
      printf("ERROR reading from socket");
    }
    else {
      printf("%s", buffer);
    }

    
    char *forIterator;
    char *subString;
    char *fileName;
    char *fileContent;
    char command[BUFFERSIZE];
    int numCommands;
    
    for (forIterator = strtok_r(buffer,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)){
       printf("ENTROU FOR");
       if (numCommands == 0){
			    strcpy(command, forIterator);
		    }
		    else if (numCommands == 1){
			    strcpy(fileName, forIterator);
		    }
		    else{ //se usarmos
			    strcpy(fileContent, forIterator);
		    }
		    numCommands++;
    }
    
    if(strcmp(command, "list") == 0) {
      printf("chamou list\n");
    } else if(strcmp(command, "exit") == 0) {
      printf("chamou exit\n");
    } else if(strcmp(command, "upload") == 0) {
      printf("chamou upload com fileName = %s\n", fileName);
    } else if(strcmp(command, "download") == 0) {
      printf("chamou download com fileName = %s\n", fileName);
    } 
  }
}

void *acceptClient() {
  while(1) {
    int newsockfd, n;
    socklen_t client;
    struct sockaddr_in cli_addr;
    char buffer[BUFFERSIZE];

    client = sizeof(struct sockaddr_in);
    if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &client)) == ERROR)
    {
      perror("ERROR on accept client");
      exit(ERROR);
    }  
    
    pthread_mutex_lock(&userVerificationMutex);

    // READ/WRITE (WILL NEED THREADS)
    bzero(buffer, BUFFERSIZE);
    n = read(newsockfd, buffer, BUFFERSIZE);
    if (n == ERROR) {
      printf("ERROR reading from socket");
    }    
    
    if(verifyAuxSocket(buffer) == SUCCESS){

      clientThread *auxSocket = (clientThread*) malloc(sizeof(clientThread));
      strcpy(auxSocket->userId, cropUserId(buffer));
      auxSocket->socketId = newsockfd;

      AppendFila2(&auxSocketsList, (void *) auxSocket);
      
      pthread_mutex_unlock(&userVerificationMutex);
        
      //cria thread auxiliar para download/upload/comandos
      pthread_t auxThread;
      pthread_attr_t attributesAuxThread;
      pthread_attr_init(&attributesAuxThread);
      pthread_create(&auxThread,&attributesAuxThread, auxClientThread, (void *) auxSocket);
      
      //criar uma thread e passa newsockfd e userID
      //download/upload/comandos
    }
    else {
      verifyUserAuthentication(buffer, newsockfd);
      
      pthread_mutex_unlock(&userVerificationMutex);
      
      //Criar struct Client_Info e gerar diretório
      // sync
    }
    
    // close(newsockfd);
  }
}


int main(int argc, char *argv[]) 
{ 
  if(validateServerArguments(argc, argv) != ERROR) 
  {
    initializeList(&clientList);
    initializeList(&auxSocketsList);
    if((server_port = getPort(argv[2])) == ERROR) {
      printf ("ERROR on attributing the port");
      return ERROR;
    }

    createServerSocket();
    bindServerSocket();
    // LISTEN

    listen(sockfd, 5);
    printf("Server is listening at: %s:%d\n", inet_ntoa(serv_addr.sin_addr), (int) ntohs(serv_addr.sin_port));

    //ACCEPT CLIENT
    pthread_t acceptThread;
    pthread_attr_t attributesAcceptThread;
    pthread_attr_init(&attributesAcceptThread);
    pthread_create(&acceptThread,&attributesAcceptThread,acceptClient,NULL);

    pthread_join(acceptThread, NULL);
    
    
    
    //close(sockfd);
  }
  else 
  {
      return ERROR;
  }        
  return 0;
}



