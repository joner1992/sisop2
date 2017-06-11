#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxServer.h"
#include <stdio.h>

int receive_image(int socket) { // Start function 

  int recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;
  
  char imagearray[10241];
  FILE *image;
  
  //Find the size of the image
  do{
  stat = read(socket, &size, sizeof(int));
  }while(stat<0);
  
  printf("Packet received.\n");
  printf("Packet size: %i\n",stat);
  printf("Image size: %i\n",size);
  printf(" \n");
  
  char buffer[] = "Got it";
  
  //Send our verification signal
  do{
    stat = write(socket, &buffer, sizeof(int));
  }while(stat<0);
  
  printf("Reply sent\n");
  printf(" \n");
  
  image = fopen("./files/out/test.txt", "w");
  
  if( image == NULL) {
  printf("Error has occurred. Image file could not be opened\n");
  return -1; }
  
  //Loop while we have not received the entire file yet

  struct timeval timeout = {10,0};
  
  fd_set fds;
  int buffer_fd;
  
  while(recv_size < size) {
  //while(packet_index < 2){
  
    FD_ZERO(&fds);
    FD_SET(socket,&fds);
  
    buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);
  
    if (buffer_fd < 0)
       printf("error: bad file descriptor set.\n");
  
    if (buffer_fd == 0)
       printf("error: buffer read timeout expired.\n");
  
    if (buffer_fd > 0)
    {
        do{
               read_size = read(socket,imagearray, 10241);
            }while(read_size <0);
  
            printf("Packet number received: %i\n",packet_index);
        printf("Packet size: %i\n",read_size);
  
  
        //Write the currently read data into our image file
         write_size = fwrite(imagearray,1,read_size, image);
         printf("Written image size: %i\n",write_size); 
  
             if(read_size !=write_size) {
                 printf("error in read write\n");    }
  
  
             //Increment the total number of bytes read
             recv_size += read_size;
             packet_index++;
             printf("Total received image size: %i\n",recv_size);
             printf(" \n");
             printf(" \n");
    }
  
  }


  fclose(image);
  printf("Image successfully Received!\n");
  return 1;
}

int sockfd;
struct sockaddr_in serv_addr;
int server_port;
FILA2 clientList;
pthread_mutex_t userVerificationMutex;


void initializeUserList() {
  if(CreateFila2(&clientList) != LISTSUCCESS) { // 0 = linked list initialized successfully
    perror("ERROR initializing linked list");
    exit(ERROR);
  }
}

int verifyUserAuthentication(char *buffer, int newsockfd) {
  if(searchForUserId(&clientList, buffer) == ERROR) {
    Client_Info* client_struct = (Client_Info *) malloc(sizeof(Client_Info));
    //se não existe gerar diretório para user
    createDirectory(buffer);
    strcpy(client_struct->userId, buffer);
    client_struct->logged_in = 1;
    client_struct->devices[0] = newsockfd;
    client_struct->devices[1] = -1;
    AppendFila2(&clientList, (void *) client_struct);
  }
  else {
  // //criar thread read and write para user
    Client_Info* client_struct = (Client_Info*) GetAtIteratorFila2(&clientList);
    pthread_mutex_lock(&client_struct->loginMutex);
      if(!client_struct->logged_in) {
        if(client_struct->devices[0] != newsockfd && client_struct->devices[1] != newsockfd) {
          if(client_struct->devices[0] == -1) {
            client_struct -> devices[0] = newsockfd;
          }
          else {
            client_struct->devices[1] = newsockfd;
          }
        }
        client_struct->logged_in = 1;
      }
      else {
        if(client_struct->devices[0] == -1) {
          client_struct->devices[0] = newsockfd;
        }
        else if(client_struct->devices[1] == -1){
          client_struct->devices[1] = newsockfd;
        }
        else {
          printf("Client %s reached the limit of devices connected\n\n", client_struct->userId);
          disconnectClient(newsockfd);
          close(newsockfd);
          return ERROR;
        }
      }
    pthread_mutex_unlock(&client_struct->loginMutex);

  }
  pthread_mutex_unlock(&userVerificationMutex);
  return SUCCESS;
}

int getPort(char *argv) {
  char *endptr;
=======
 
  //VARIABLES FOR SOCKET 
  int sockfd, newsockfd;
  socklen_t client;
  struct sockaddr_in serv_addr, cli_addr;
>>>>>>> Envio de arquivos, PDF quebrado
  
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
    //READ FROM THE USER
    bzero(buffer, BUFFERSIZE);
    n = read(newsockfd, buffer, BUFFERSIZE);
    if (n == ERROR) {
      printf("ERROR reading from socket\n");
      exit(ERROR);
    } 

    if(n != 0)
      printf("RECEBEU: %s\n", buffer);
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

    printf("Write your message to the client: ");
    fgets(buffer, BUFFERSIZE, stdin);

    n = write(newsockfd, buffer, strlen(buffer));
    if (n == ERROR) {
      perror("ERROR writing to socket\n");
      exit(ERROR);
    }
<<<<<<< HEAD
=======
    // LISTEN
    listen(sockfd, 5);
    printf("\n Server is listening at: %s:%d\n", inet_ntoa(serv_addr.sin_addr), (int) ntohs(serv_addr.sin_port));
>>>>>>> Envio de arquivos, PDF quebrado

    printf("ENVIOU: %s\n", buffer);
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
<<<<<<< HEAD
      exit(ERROR);
    }  

    // READ/WRITE (WILL NEED THREADS)
    bzero(buffer, BUFFERSIZE);
    n = read(newsockfd, buffer, BUFFERSIZE);
    if (n == ERROR) {
      printf("ERROR reading from socket");
    }

    // //verificar se user já existe na lista
    pthread_mutex_lock(&userVerificationMutex);
      if(verifyUserAuthentication(buffer, newsockfd) != ERROR) {
        //CREATE READING THREAD
        pthread_t readThread;
        pthread_attr_t attributesReadThread;
        pthread_attr_init(&attributesReadThread);
        pthread_create(&readThread,&attributesReadThread,readUser,&newsockfd);
        
        //CREATE WRITING THREAD
        pthread_t writeThread;
        pthread_attr_t attributesWriteThread;
        pthread_attr_init(&attributesWriteThread);
        pthread_create(&writeThread,&attributesReadThread,writeUser,&newsockfd);
      }

      // pthread_join(readThread, NULL);
      // pthread_join(writeThread, NULL);

      // close(newsockfd);
  }
}


int main(int argc, char *argv[]) 
{ 
  if(validateServerArguments(argc, argv) != ERROR) 
  {
    initializeUserList();
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
    

    receive_image(newsockfd);
    
    close(newsockfd);
    close(sockfd);
    
  }
  else 
  {
      return ERROR;
  }        
  return 0;
}

