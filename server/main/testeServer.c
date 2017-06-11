#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxServer.h"
#include <stdio.h>
#include "../../utils/include/file.h"


int sockfd, newsockfd;
struct sockaddr_in serv_addr;
int server_port;
char *endptr;
socklen_t client;
struct sockaddr_in serv_addr, cli_addr;


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
}

void bindServerSocket() {
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(server_port); //CHANGE LINE!!!
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(serv_addr.sin_zero), 8);     
  
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == ERROR) {
    perror("ERROR on binding server socket");
    exit(ERROR);
  }
}

void createServerSocket(){
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
    perror("ERROR: Failed in socket creation");
    exit(ERROR);
  }
}

int main(int argc, char *argv[]) {
    
   if((server_port = getPort(argv[2])) == ERROR) {
      printf ("ERROR on attributing the port");
      return ERROR;
    }

    createServerSocket();
    bindServerSocket();

    // LISTEN
    listen(sockfd, 5);
    printf("Server is listening at: %s:%d\n", inet_ntoa(serv_addr.sin_addr), (int) ntohs(serv_addr.sin_port));
    
    client = sizeof(struct sockaddr_in);
	  if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &client)) == -1) 
		  printf("ERROR on accept");
	
    
    receive_(newsockfd);
    
    close(sockfd);
    close(newsockfd);
    
    
}