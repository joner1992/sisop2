#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxClient.h"
#include <stdio.h>
#include "../../utils/include/file.h"

int sockfd, n, server_port;
char userId[MAXNAME];
char buffer[BUFFERSIZE];
struct sockaddr_in serv_addr;
struct hostent *server;

FILE *file;

struct hostent* getHost(char *argv) {
  struct hostent *server = gethostbyname(argv);
  
  if (server == NULL) {
    perror("ERROR, no such host\n");
    exit(ERROR);
  }
  return server;
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


int createSocket() {
    int  socketNumber;
    printf("\n Opening socket");
    if ((socketNumber = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
      perror("ERROR opening socket\n");
      exit(ERROR);
    }
    return socketNumber;
}

void connectSocket(struct sockaddr_in serv_addr, struct hostent *server, int socketNumber) {
    serv_addr.sin_family = AF_INET;     
    serv_addr.sin_port = htons(server_port);    
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8); 
    
    printf("\n Connecting to server");
    if (connect(socketNumber,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
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

void *readWriteUser() {

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
    
    server = getHost(argv[2]);
    getUserId(argv[6]);
    createSocket();
    printf("Socket created");
    sockfd = createSocket();
    

    if((server_port = getPort(argv[4])) == ERROR ) {
      perror("ERROR bad port configuration\n");
      exit(ERROR);
    }
    
    connectSocket(serv_addr, server, sockfd);
    
    //CREATE READING/WRITING THREAD
   /* pthread_t readWriteThread;
    pthread_attr_t attributesReadWriteThread;
    pthread_attr_init(&attributesReadWriteThread);
    pthread_create(&readWriteThread,&attributesReadWriteThread,readWriteUser,NULL);

    pthread_join(readWriteThread, NULL);
     */   
     
    file = fopen("./files/in/test.txt", "r");
    if(file) send_(sockfd, file);
    
    close(sockfd);
    
    fclose(file);

  } else {
    return ERROR;
  }

  return 0;
}
    
	

   
    