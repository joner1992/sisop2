#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxClient.h"
#include <stdio.h>

int sockfd, n;
struct sockaddr_in serv_addr;
struct hostent *server;
char buffer[BUFFERSIZE];

FILE *file;

void getHost(char *argv) {
  server = gethostbyname(argv);
  
  if (server == NULL) {
    perror("ERROR, no such host\n");
    exit(ERROR);
  }
}

void createSocket() {
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
      perror("ERROR opening socket\n");
      exit(ERROR);
    } 
}

void connectSocket() {
    serv_addr.sin_family = AF_INET;     
    serv_addr.sin_port = htons(DEFAULTPORT);    
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8); 

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
      perror("ERROR connecting to server\n");
      exit(ERROR);
    }
}

int main(int argc, char *argv[]) {

  if(validateClientArguments(argc, argv) != ERROR) {
    
    getHost(argv[2]);
    
    createSocket();
    
    connectSocket();

     printf("Opening file ");
     file = fopen("./test.txt", "r");
     
    bzero(buffer, BUFFERSIZE);
    fgets(buffer, BUFFERSIZE, file);
    
	/* write in the socket */
    n = write(sockfd, buffer, strlen(buffer));
    if (n == ERROR)
    {
      perror("ERROR writing to socket\n");
      return ERROR;
    } 

    bzero(buffer,BUFFERSIZE);
    
    /* read from the socket */
    n = read(sockfd, buffer, BUFFERSIZE);
    if (n == ERROR)
    {
      perror("ERROR reading from socket\n");
      return ERROR;
    } 
    printf("%s\n",buffer);

    // CLOSE
    close(sockfd);

  }
  else
  {
    return ERROR;
  }

  return 0;
}
    
	

   
    