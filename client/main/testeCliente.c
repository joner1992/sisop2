#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxClient.h"
#include <stdio.h>
#include "../../utils/include/file.h"


int sockfd, n, server_port;
char userId[MAXNAME];
char buffer[BUFFERSIZE];
struct sockaddr_in serv_addr;
struct hostent *server;


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
    bzero(buffer, BUFFERSIZE);
    strcpy(buffer, userId);
    n = write(sockfd, buffer, strlen(buffer));
    if (n == ERROR) {
      perror("ERROR writing to socket\n");
      exit(ERROR);
    }

}

int main(int argc, char *argv[]) {
    
    server = getHost(argv[2]);
    createSocket();
    sockfd = createSocket();
    
    printf("\nFile to transfer %s", argv[5]);

    if((server_port = getPort(argv[4])) == ERROR ) {
      perror("ERROR bad port configuration\n");
      exit(ERROR);
    }
    
    connectSocket(serv_addr, server, sockfd);
    
    send_(sockfd, argv[5]);
    
    close(sockfd);
    
}