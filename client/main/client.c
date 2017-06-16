#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxClient.h"
#include <stdio.h>

int sockfd, n, server_port;
char userId[MAXNAME];
struct sockaddr_in serv_addr;
struct hostent *server;

int aux_sockfd, m;
struct sockaddr_in aux_serv_addr;
struct hostent *aux_server;



FILE *file;

void setHost(char *argv) {
  server = gethostbyname(argv);
  
  if (server == NULL) {
    perror("ERROR, no such host\n");
    exit(ERROR);
  }
  
}

void setAuxHost(char *argv){
  aux_server = gethostbyname(argv);
  
  if (aux_server == NULL) {
    perror("ERROR, no such host for aux_socket");
    exit(ERROR);
  }
}

int getPort(char *argv) {
  char *endptr;
  int port = strtoimax(argv, &endptr,10);
  if(port <= 0) {
    return ERROR;
  }
  return port;

}

void setUserId(char *argv){
  if(strlen(argv) > MAXNAME) {
    perror("ERROR username exceeds maximum length");
    exit(ERROR);
  }
  strcpy(userId, argv);
}

void createSocket() {
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
      perror("ERROR opening socket\n");
      exit(ERROR);
    } 

}

void createAuxSocket(){
  if((aux_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
    perror("ERROR opening aux socket\n");
    exit(ERROR);
  }
}

void connectSocket() {
    char buffer[BUFFERSIZE];
    int n;
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_port = htons(server_port);    
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8); 

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
      perror("ERROR connecting to server\n");
      exit(ERROR);
    }
    bzero(buffer, BUFFERSIZE);
    strcpy(buffer, userId);
    n = write(sockfd, buffer, strlen(buffer));
    if (n == ERROR) {
      perror("ERROR writing to socket\n");
      exit(ERROR);
    }

}

void connectAuxSocket() {
    char buffer[BUFFERSIZE];
    int n;
    aux_serv_addr.sin_family = AF_INET;  
    aux_serv_addr.sin_port = htons(server_port);    
    aux_serv_addr.sin_addr = *((struct in_addr *)aux_server->h_addr);
    bzero(&(serv_addr.sin_zero), 8); 

    if (connect(aux_sockfd,(struct sockaddr *) &aux_serv_addr,sizeof(aux_serv_addr)) < 0) 
    {
      perror("ERROR connecting to server\n");
      exit(ERROR);
    }
    
    bzero(buffer, BUFFERSIZE);
    strcat(buffer, "aux_");
    strcat(buffer, userId);
    
    n = write(aux_sockfd, buffer, strlen(buffer));
    if (n == ERROR) {
      perror("ERROR writing to socket\n");
      exit(ERROR);
    }

}

char *adaptEntry(char *cmd) {
  char aux[BUFFERSIZE];
  strcpy(aux, cmd);
  int i;
  for(i = 0; i<sizeof(aux); i++) {
    if(aux[i] == ' ' || aux[i] == '\n'){
      aux[i] = '#';
    }
  }
  return aux;
}

void *auxSocketFunctions() {
  char buffer[BUFFERSIZE];
  char auxBuffer[BUFFERSIZE];
  while(1) {
    bzero(buffer, BUFFERSIZE);
    printf(">> ");
    fgets(buffer, BUFFERSIZE, stdin);
    //coloca # entre os espaços e no final
    strcpy(buffer, adaptEntry(buffer));

    n = write(aux_sockfd, buffer, strlen(buffer));
    if (n == ERROR) {
      perror("ERROR writing to socket\n");
      exit(ERROR);
    }
  }
}

int main(int argc, char *argv[]) {

  if(validateClientArguments(argc, argv) != ERROR) {
    
    setHost(argv[2]);
    setUserId(argv[6]);
    createSocket();
    
    
    
    setAuxHost(argv[2]);
    setAuxHost(argv[2]);
    createAuxSocket();

    if((server_port = getPort(argv[4])) == ERROR ) {
      perror("ERROR bad port configuration\n");
      exit(ERROR);
    }
    
    connectSocket();
    connectAuxSocket();


    //AUX socket para comandos, upload e download
    pthread_t auxSocketThread;
    pthread_attr_t attributesAuxSocketThread;
    pthread_attr_init(&attributesAuxSocketThread);
    pthread_create(&auxSocketThread,&attributesAuxSocketThread,auxSocketFunctions,NULL);    


    //Normal thread para sync

    //pthread_t readWriteThread;
    //pthread_attr_t attributesReadWriteThread;
    //pthread_attr_init(&attributesReadWriteThread);
    //pthread_create(&readWriteThread,&attributesReadWriteThread,readWriteUser,NULL);
    


    pthread_join(auxSocketThread, NULL);
        
    //close(sockfd);

  } else {
    return ERROR;
  }

  return 0;
}
   
    