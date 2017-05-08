#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxClient.h"

int main(int argc, char *argv[]) {

  if(validateClientArguments(argc, argv) != ERROR)
  {
    // VARIABLES FOR SOCKET

    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFERSIZE];
    
    server = gethostbyname(argv[2]);
    if (server == NULL) 
    {
      perror("ERROR, no such host\n");
      return ERROR;
    }
    // CREATE SOCKET
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
    {
      perror("ERROR opening socket\n");
      return ERROR;
    } 
    // CONNECT SOCKET
    serv_addr.sin_family = AF_INET;     
    serv_addr.sin_port = htons(DEFAULTPORT);    
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8); 

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
      perror("ERROR connecting to server\n");
    }

    // READ/WRITE
     printf("Enter the message: ");
    bzero(buffer, BUFFERSIZE);
    fgets(buffer, BUFFERSIZE, stdin);
    
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

  return SUCCESS;
}
    
	

   
    