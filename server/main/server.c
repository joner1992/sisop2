#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxServer.h"

int main(int argc, char *argv[]) 
{

 
  //VARIABLES FOR SOCKET 
  int sockfd, newsockfd, n;
  socklen_t client;
  char buffer[BUFFERSIZE];
  struct sockaddr_in serv_addr, cli_addr;
  
  if(validateServerArguments(argc, argv) != ERROR) 
  {

    
    // CREATE SOCKET
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("Failed in socket creation");
        return ERROR;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(DEFAULTPORT); //CHANGE LINE!!!
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8);     
  
    // BIND SOCKET
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == ERROR)
    {
      perror("ERROR on binding");
      return ERROR;
    }
    // LISTEN
    listen(sockfd, 5);
    printf("Server is listening at: %s:%d\n", inet_ntoa(serv_addr.sin_addr), (int) ntohs(serv_addr.sin_port));

    // ACCEPT (WILL NEED SPECIFIC THREAD)
    client = sizeof(struct sockaddr_in);
    if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &client)) == ERROR)
    {
      perror("ERROR on accept client");
      return ERROR;
    }    

    // READ/WRITE (WILL NEED THREADS)
    bzero(buffer, BUFFERSIZE);


    n = read(newsockfd, buffer, BUFFERSIZE);
    if (n == ERROR) {
      printf("ERROR reading from socket");
    }
    printf("Here is the message: %s\n", buffer);

    /* write in the socket */ 
    n = write(newsockfd,"I got your message", 18);
    if (n == ERROR){
      printf("ERROR writing to socket");

    } 
    // CLOSE SOCKETS (AUXILIARIES AND PRINCIPAL)
    close(newsockfd);
    close(sockfd);
  }
  else 
  {
      return ERROR;
  }        
  return SUCCESS;
}

