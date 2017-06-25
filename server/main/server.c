#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxServer.h"


//Variables for socket
int sockfd;
struct sockaddr_in serverAddress;
int serverPort;
char usersPath[255] = "./clientsDirectories/sync_dir_";

// Queue for clients structs
struct chain_list* clientList;

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
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(serverPort); //CHANGE LINE!!!
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  bzero(&(serverAddress.sin_zero), 8);     
  
  // BIND SOCKET
  if (bind(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == ERROR) {
    perror("ERROR on binding server socket");
    exit(ERROR);
  }
}

void *clientSyncThread(void *threadArg){
  char buffer[BUFFERSIZE];
  char clientDate[TIMESIZE];
  char path[255];
  socketsStruct *clientSockets = threadArg;
  //search for clientInfo structure
  chain_node *clientNode = chain_find(clientList, clientSockets->userId);
  
  bzero(buffer, BUFFERSIZE);
  //get list of files and transform it to string
  strcpy(buffer, fileListToArray(clientNode->client->fileList));

  //send list of files to the client
  sendMessage(clientSockets->syncSocket, buffer); 
  bzero(path, 255);
  sprintf(path, "%s%s/", usersPath, clientNode->client->userId);
  
  //start uploading files
  sendServerFiles(clientSockets->syncSocket, buffer, path);

  //send current User last modification
  sendMessage(clientSockets->syncSocket, clientNode->client->lastModification); 

  while(1) {  
    //receive client date
    bzero(buffer, BUFFERSIZE);
    strcpy(buffer, receiveMessage(clientSockets->syncSocket, "", FALSE));

    //if client date > server date
    if(strcmp(buffer, clientNode->client->lastModification) > 0) {
      sendMessage(clientSockets->syncSocket, "sendEverything");
      printf("[SERVER] sendEverything SENT to CLIENT\n");
      //remove everything from server
      removeFileFromSystem(clientNode->client->userId, SERVER);    

      //clean userFileList
      chain_clear(clientNode->client->fileList);
      
      //receive list of files from client
      bzero(buffer, BUFFERSIZE);
      strcpy(buffer, receiveMessage(clientSockets->syncSocket, "", FALSE));
      
      //receive all files adding them to the filesList when client synced
      receiveServerFiles(clientSockets->syncSocket, buffer, path, clientNode->client->fileList);
      printf("[SERVER] sendEverything RECEIVED filelist CLIENT %s\n", buffer);
      //receive the client datetime and set
      bzero(clientNode->client->lastModification, TIMESIZE);
      strcpy(clientNode->client->lastModification, receiveMessage(clientSockets->syncSocket, "", FALSE));
      
    } else if(strcmp(buffer, clientNode->client->lastModification) < 0) { //if client date < server date
      sendMessage(clientSockets->syncSocket, "receiveEverything");
      printf("[SERVER] receiveEverything SENT to CLIENT\n");
      //remove everything from client and send everything from server
      bzero(buffer, BUFFERSIZE);
      //get list of files and transform it to string
      strcpy(buffer, fileListToArray(clientNode->client->fileList));
      printf("[SERVER] receiveEverything SENT filelist CLIENT %s\n", buffer);
      //send list of files to the client
      sendMessage(clientSockets->syncSocket, buffer); 
      
      //start uploading files
      sendServerFiles(clientSockets->syncSocket, buffer, path);

      //send current User last modification
      sendMessage(clientSockets->syncSocket, clientNode->client->lastModification);
    } else {
      sendMessage(clientSockets->syncSocket, "doNothing");
      printf("[SERVER] doNothing SENT to CLIENT\n");
    }
    sleep(10);
  }
}

void *clientCommandsThread(void *threadArg) {
  char buffer[BUFFERSIZE];
  char *forIterator;
  char *subString;
  char fileName[BUFFERSIZE];
  char content[BUFFERSIZE];
  char command[BUFFERSIZE];
  char path[255] = "./clientsDirectories/sync_dir_";
  char completePath[255];
  char lastModification[BUFFERSIZE];
  int numCommands;
  pthread_t syncThread;
  socketsStruct *clientSockets = threadArg;
  //search for clientInfo structure
  chain_node *clientNode = chain_find(clientList, clientSockets->userId);
  
  //testing if the user can really start the threads
  if(clientNode->client->numDevices >= 2){
    printf("[Server] ERROR Maximum number of devices reached\n");
    sendMessage(clientSockets->commandsSocket, "ConnectionERROR");
    //close sockets
    close(clientSockets->commandsSocket);
    close(clientSockets->syncSocket);
    //leave      
    pthread_exit(NULL);
  } else {
    //if he can, add one to number of devices
    sendMessage(clientSockets->commandsSocket, "ConnectionOK");
    clientNode->client->numDevices++;
  }

  //create sync thread
  if((pthread_create( &syncThread, NULL, clientSyncThread, threadArg)) != 0){
    printf("[Server] ERROR on syncThread creation\n");
    close(clientSockets->commandsSocket);
    close(clientSockets->syncSocket);
    exit(1);
  }

  while(1) {
    bzero(buffer, BUFFERSIZE);
    bzero(command, BUFFERSIZE);
    bzero(fileName, BUFFERSIZE);
    bzero(content, BUFFERSIZE);
    bzero(lastModification, BUFFERSIZE);
    bzero(completePath, 255);
    numCommands = 0;

    //receive message from client
    strcpy(buffer, receiveMessage(clientSockets->commandsSocket, "", FALSE));
    
    for (forIterator = strtok_r(buffer,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)){
      if (numCommands == 0){
			  strcpy(command, forIterator);
		  }
		  else if (numCommands == 1){
			  strcpy(fileName, forIterator);
		  }
		  else{ //se usarmos
			  strcpy(content, forIterator);
		  }
		  numCommands++;
    }

    if(strcmp(command, "list") == 0) {
      /*-------------------------------------------------------------------------------------------
      ------------------------------------ LIST ---------------------------------------------------
      -------------------------------------------------------------------------------------------*/
      chain_node* iterator = clientNode->client->fileList->header;
      while (iterator != NULL) {
        sendMessage(clientSockets->commandsSocket, iterator->file->name);
        iterator = iterator->next;
      }
      sendMessage(clientSockets->commandsSocket, "endList");
      //strcpy(buffer, listFiles(&clientList, newAuxThread->userId, newAuxThread->socketId));

    } else if(strcmp(command, "exit") == 0) {
      /*-------------------------------------------------------------------------------------------
      ------------------------------------ EXIT ---------------------------------------------------
      -------------------------------------------------------------------------------------------*/
      //remove one of the user devices
      clientNode->client->numDevices--;
      printf("[Server] User %s disconnecting, number of devices connected: %d\n", clientNode->client->userId, clientNode->client->numDevices);
      //finish the sync thread
      pthread_cancel(syncThread);
      //close sockets
      close(clientSockets->commandsSocket);
      close(clientSockets->syncSocket);
      //leave      
      pthread_exit(NULL);

    } else if(strcmp(command, "upload") == 0) {
      /*-------------------------------------------------------------------------------------------
      ------------------------------------ UPLOAD -------------------------------------------------
      -------------------------------------------------------------------------------------------*/  
      sprintf(completePath,"%s%s/",path, clientNode->client->userId);
      //receive file from user
      if(newReceive(clientSockets->commandsSocket, completePath) == SUCCESS) {
        //aqui o completePath está sendo concatenado com o fileName
        //receive the current lastModification
        updateLocalTime(lastModification);
        printf("[SERVER] Updating FILE DATETIME to %s\n", lastModification);
        //set in the addfiles
        strcat(completePath, basename(fileName));
        struct stat fileStat = getAttributes(completePath);
        //add the file to filelist
        addFilesToFileList(clientNode->client->fileList, basename(fileName), lastModification, fileStat.st_size);
        bzero(clientNode->client->lastModification, TIMESIZE);
        updateLocalTime(clientNode->client->lastModification);
        printf("[SERVER] Updating CLIENT DATETIME to %s\n", clientNode->client->lastModification);
      }
    } else if(strcmp(command, "download") == 0) {
      /*-------------------------------------------------------------------------------------------
      ------------------------------------ DOWNLOAD -----------------------------------------------
      -------------------------------------------------------------------------------------------*/
      //create the path to download(including filename)
      sprintf(completePath,"%s%s/%s",path, clientNode->client->userId, fileName);        
      //send the file to user
      newSend(clientSockets->commandsSocket, completePath);
      //send the current file date
      struct chain_node* fileNode = chain_find(clientNode->client->fileList, basename(fileName));
      strcpy(lastModification, fileNode->file->lastModified);
      sendMessage(clientSockets->commandsSocket, lastModification);
    } 
  }
}

int main(int argc, char *argv[]) { 
  int newSocket, newSyncSocket, firstLogin;
  struct sockaddr_in clientAddress, clientSyncAddress;
  struct chain_node* clientStruct;
  socklen_t clientLenght;
  pthread_t clientThread;
  char userId[MAXNAME];

  if(validateServerArguments(argc, argv) != ERROR) 
  {
    //Initialize chained list of client structs
    clientList = chain_create_list();
    
    if((serverPort = getPort(argv[2])) == ERROR) {
      printf ("[Server] ERROR on attributing the port\n");
      return ERROR;
    }

    createServerSocket();
    bindServerSocket();

    listen(sockfd, 20);
    clientLenght = sizeof(struct sockaddr_in);
    printf("[Server] Listening at: %s:%d\n", inet_ntoa(serverAddress.sin_addr), (int) ntohs(serverAddress.sin_port));

    while(1){

      bzero(userId, MAXNAME);

      //wait for the first user to connect with his main sync
      if((newSocket = accept(sockfd, (struct sockaddr *) &clientAddress, &clientLenght)) != ERROR){
        strcpy(userId, receiveMessage(newSocket, "",FALSE));
        printf("[Server] Received new connection from %s\n", userId);
        //verify user, if it is ok, go on, else ERROR
        if((clientStruct = chain_find(clientList, userId)) == NULL){
          //create user struct
          clientStruct = chain_create_client_node(userId);
          clientStruct->client->numDevices = 0;
          clientStruct->client->fileList = chain_create_list();
          clientStruct->client->loggedIn = 1;
          updateLocalTime(clientStruct->client->lastModification);
          getFilesFromUser(userId, clientStruct->client->fileList, SERVER, clientStruct->client->lastModification);

          //create Directory for the user
          createDirectory(userId, SERVER);

          //add user to the list
          chain_add(clientList, clientStruct);
        } 
        //wait for the first user to connect with his main sync
        if((newSyncSocket = accept(sockfd, (struct sockaddr *) &clientSyncAddress, &clientLenght)) != ERROR){

          //pass userId, the main socket and the sync socket
          socketsStruct *threadArg = malloc(sizeof(socketsStruct *));
          threadArg->commandsSocket = newSocket;
          threadArg->syncSocket = newSyncSocket;
          strcpy(threadArg->userId, userId);

          //create the thread, if it terminates, exit.
          if((pthread_create( &clientThread, NULL, clientCommandsThread, (void *)threadArg)) != 0){
            printf("[Server] ERROR on thread creation\n");
            close(newSocket);
            close(newSyncSocket);
            exit(1);
          }
        }
        else {
          printf("[Server] ERROR accepting sync connection\n");
          close(newSyncSocket);
          exit(1);
        }        
      }
      else{
        printf("[Server] ERROR accepting new connection\n");
        close(newSocket);
        exit(1);
      }        
    }
  } else {
    return ERROR;
  }
  return 0;
}

