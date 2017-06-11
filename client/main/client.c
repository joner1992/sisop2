#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxClient.h"
#include <stdio.h>

int sockfd, n, server_port;
char userId[MAXNAME];
struct sockaddr_in serv_addr;
struct hostent *server;

FILE *file;

void getHost(char *argv) {
  server = gethostbyname(argv);
  
  if (server == NULL) {
    perror("ERROR, no such host\n");
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

void getUserId(char *argv){
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

void _send(FILE *file) {
    char buffer[BUFFERSIZE];
    
    bzero(buffer, BUFFERSIZE);
    
    fgets(buffer, BUFFERSIZE, file);
    
    if (write(sockfd, buffer, strlen(buffer)))
    {
      perror("ERROR writing to socket\n");
      exit(ERROR);
    } 
}

void _read() {
    char buffer[BUFFERSIZE];
    bzero(buffer,BUFFERSIZE);
    
    if (read(sockfd, buffer, BUFFERSIZE)) {
      perror("ERROR reading from socket\n");
      exit(ERROR);
    } 

    printf("%s\n",buffer);
}

void *readUser() {

  int n;  
  char buffer[BUFFERSIZE];

  //RECEIVE FILE AND SYNC

  while(1){
    //READ FROM THE USER
    bzero(buffer, BUFFERSIZE);
    n = read(sockfd, buffer, BUFFERSIZE);
    if (n == ERROR) {
      printf("ERROR reading from socket");
      exit(ERROR);
    } 

    if(strcmp(buffer, DISCONNECTED) == 0){
      printf("BYE!\n");
      close(sockfd);
      exit(SUCCESS);
    }

    if(n != 0){
      printf("RECEBEU: %s\n", buffer);
    } 
  }
}

void *writeUser(){

  int n;
  char buffer[BUFFERSIZE];

  //SEND FILE AND SYNC

  while(1){
    //WRITE TO THE USER
    bzero(buffer, BUFFERSIZE);

    //READ SOMETHING FROM THE INPUT
    printf("Cliente manda: ");
    fgets(buffer,BUFFERSIZE,stdin);

    n = write(sockfd, buffer, strlen(buffer));
    if (n == ERROR) {
      perror("ERROR writing to socket\n");
      exit(ERROR);
    }

    printf("ENVIOU: %s\n", buffer);
  }
}

int main(int argc, char *argv[]) {

  if(validateClientArguments(argc, argv) != ERROR) {
    
    getHost(argv[2]);
    getUserId(argv[6]);
    createSocket();

    if((server_port = getPort(argv[4])) == ERROR ) {
      perror("ERROR bad port configuration\n");
      exit(ERROR);
    }
    
    connectSocket();

    //CREATE READING THREAD
    pthread_t readThread;
    pthread_attr_t attributesReadThread;
    pthread_attr_init(&attributesReadThread);
    pthread_create(&readThread,&attributesReadThread,readUser,NULL);
    
    //CREATE WRITING THREAD
    pthread_t writeThread;
    pthread_attr_t attributesWriteThread;
    pthread_attr_init(&attributesWriteThread);
    pthread_create(&writeThread,&attributesReadThread,writeUser,NULL);

    //  printf("Opening file ");
    //  file = fopen("./test.txt", "r");
   
    // if(file) {
    //   _send(file);
    // }
    
    // _read();

    pthread_join(writeThread, NULL);
    pthread_join(readThread, NULL);
        
    //close(sockfd);

  } else {
    return ERROR;
  }

  return 0;
}
    
	

   
    