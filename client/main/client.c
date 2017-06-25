#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxClient.h"
#include <stdio.h>

int syncSocket, n, server_port, commandsSocket, m;
char userId[MAXNAME];
char path[255];
struct sockaddr_in serv_addr;
struct hostent *server;
struct sockaddr_in aux_serv_addr;
struct hostent *aux_server;
char fileListlastModification[TIMESIZE];

pthread_t commandSocketThread;
pthread_t syncSocketThread;

chain_list *fileList;

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
    if ((syncSocket = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
      perror("ERROR opening socket\n");
      exit(ERROR);
    } 

}

void createAuxSocket(){
  if((commandsSocket = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
    perror("ERROR opening aux socket\n");
    exit(ERROR);
  }
}

int connectSocket() {
    char buffer[BUFFERSIZE];
    int n;
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_port = htons(server_port);    
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8); 

    if (connect(syncSocket,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
      perror("ERROR connecting to server\n");
      exit(ERROR);
    }
}

int connectAuxSocket() {
    char buffer[BUFFERSIZE];
    int n;
    aux_serv_addr.sin_family = AF_INET;  
    aux_serv_addr.sin_port = htons(server_port);    
    aux_serv_addr.sin_addr = *((struct in_addr *)aux_server->h_addr);
    bzero(&(serv_addr.sin_zero), 8); 

    if (connect(commandsSocket,(struct sockaddr *) &aux_serv_addr,sizeof(aux_serv_addr)) < 0) 
    {
      perror("ERROR connecting to server\n");
      exit(ERROR);
    }
    
    bzero(buffer, BUFFERSIZE);
    strcat(buffer, userId);
    
    n = write(commandsSocket, buffer, BUFFERSIZE);
    if (n == ERROR) {
      perror("ERROR writing to socket\n");
      exit(ERROR);
    }
    return SUCCESS;
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

void *commandsThread() {
  char buffer[BUFFERSIZE];
  char adaptedBuffer[BUFFERSIZE];
  char *forIterator;
  char *subString;
  char fileName[BUFFERSIZE];
  char content[BUFFERSIZE];
  char command[BUFFERSIZE];
  char completePath[BUFFERSIZE];
  char lastModification[BUFFERSIZE];
  int numCommands;
  int sizeOfFile;

  if(strcmp("ConnectionOK", receiveMessage(commandsSocket, "", FALSE)) != 0){
    printf("[Command] Connection refused: maximum number of devices reached\n");
    //cancel sync thread
    pthread_cancel(syncSocketThread);
    //close both sockets
    close(commandsSocket);
    close(syncSocket);
    //leave
    exit(SUCCESS);
  }

  while(1) {
    bzero(command, BUFFERSIZE);
    bzero(fileName, BUFFERSIZE);
    bzero(content, BUFFERSIZE);
    bzero(completePath, BUFFERSIZE);
    bzero(lastModification, BUFFERSIZE);
    numCommands = 0;
    //receive commands to process
    printf("[Command] >> ");
    fgets(buffer, BUFFERSIZE, stdin);
    strcpy(adaptedBuffer, adaptEntry(buffer));
    //send the adapted buffer to server
    sendMessage(commandsSocket, adaptedBuffer);

    for (forIterator = strtok_r(adaptedBuffer,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)) {
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

    if(strcmp(command, "upload") == 0) {
      
      send_(commandsSocket, fileName);

    } else if (strcmp(command, "download") == 0) {    
      //receive the getUserDirectory
      strcpy(completePath, getUserDirectory(userId));
      if(receive_(commandsSocket, completePath) == SUCCESS) {
        //aqui o completePath está sendo concatenado com o fileName
        //receive the current lastModification
        strcpy(lastModification, receiveMessage(commandsSocket, "", FALSE));
        //set in the addfiles
        struct stat fileStat = getAttributes(strcat(completePath, fileName));
        //add the file to filelist
        addFilesToFileList(fileList, fileName, lastModification, fileStat.st_size);
      }
    } else if(strcmp(command, "exit") == 0) {

      printf("[Command] Exiting...\n");
      //cancel sync thread
      pthread_cancel(syncSocketThread);
      //close both sockets
      close(commandsSocket);
      close(syncSocket);
      //leave
      exit(SUCCESS);

    }
    else if(strcmp(command, "list") == 0) {
      /*-------------------------------------------------------------------------------------------
      ------------------------------------ LIST ---------------------------------------------------
      -------------------------------------------------------------------------------------------*/
        printf("[Command] Listing files...\n");
        printf("|-------------------------------------|\n");
        
        bzero(buffer, BUFFERSIZE);
        strcpy(buffer, receiveMessage(commandsSocket, "", FALSE));
        if(strcmp(buffer, "endList") == 0) {
          printf("|------------EMPTY LIST------------|\n");
        } else {
          do{
            printf("|   File: %s\n", buffer);  
            bzero(buffer, BUFFERSIZE);
            strcpy(buffer, receiveMessage(commandsSocket, "", FALSE));
          } while(strcmp(buffer, "endList") != 0);
        }
        printf("|-------------------------------------|\n");
    }

  }
}

void *syncThread(){
  char stringDirentList[BUFFERSIZE], stringFileList[BUFFERSIZE];
  chain_list *direntList;

  char buffer[BUFFERSIZE];
  bzero(buffer, BUFFERSIZE);

  //remove files from user
  removeFileFromSystem(userId, CLIENT);

  //receive list of files from server
  strcpy(buffer, receiveMessage(syncSocket, "", FALSE));

  //receive all files adding them to the filesList when client just logged in
  receiveServerFiles(syncSocket, buffer, path, fileList);

  //receive the server datetime and set
  bzero(fileListlastModification, TIMESIZE);
  strcpy(fileListlastModification, receiveMessage(syncSocket, "", FALSE));
  
  while(1) {
    //execute dirent function to update all files from client
    chain_list *direntList = getDirentsFileList(path);

    //cast lists to string
    bzero(stringDirentList, BUFFERSIZE);
    bzero(stringFileList, BUFFERSIZE);
    strcpy(stringDirentList, fileListToArray(direntList));
    strcpy(stringFileList, fileListToArray(fileList));

    //compare global list with direntsList to set new date 
    if(compareLists(stringDirentList, stringFileList) == ERROR) {
      chain_clear(fileList);
      fileList = direntList;
      bzero(fileListlastModification, TIMESIZE);
      updateLocalTime(fileListlastModification);
    }

    //send date to server
    sendMessage(syncSocket, fileListlastModification);

    //receive which option it should continue
    bzero(buffer, BUFFERSIZE);
    strcpy(buffer, receiveMessage(syncSocket, "", FALSE));

    //if message equals sendEverything, send everyFile to the server
    if(strcmp(buffer, "sendEverything") == 0) {
      bzero(buffer, BUFFERSIZE);
      printf("[CLIENT] Received sendEverything\n");

      //get list of files and transform it to string
      strcpy(buffer, fileListToArray(fileList));
      //send list of files to the server
      sendMessage(syncSocket, buffer);
      printf("[SERVER] receiveEverything SENT from SERVER filelist %s\n", buffer);
      //start uploading files
      sendServerFiles(syncSocket, buffer, path);

      //send current last file modification
      sendMessage(syncSocket, fileListlastModification);


    } else if(strcmp(buffer, "receiveEverything") == 0) {
      //if message equals receiveEverything, receive everyFile
      printf("[CLIENT] Received receiveEverything\n");
      //remove files from user     
      removeFileFromSystem(userId, CLIENT);

      //clean userFileList
      chain_clear(fileList);

      //receive list of files from server
      bzero(buffer, BUFFERSIZE);
      strcpy(buffer, receiveMessage(syncSocket, "", FALSE));
      printf("[SERVER] receiveEverything RECEIVED from SERVER filelist %s\n", buffer);

      //receive all files adding them to the filesList when client just logged in
      receiveServerFiles(syncSocket, buffer, path, fileList);
      
      //receive the server datetime and set
      bzero(fileListlastModification, TIMESIZE);
      strcpy(fileListlastModification, receiveMessage(syncSocket, "", FALSE));
    }
    sleep(10);
  }
}

int main(int argc, char *argv[]) {

  if(validateClientArguments(argc, argv) != ERROR) {
    fileList = chain_create_list();
    
    setHost(argv[2]);
    setUserId(argv[6]);
    createSocket();
    
    setAuxHost(argv[2]);
    setAuxHost(argv[2]);
    createAuxSocket();


    if((server_port = getPort(argv[4])) == ERROR ) {
      perror("[Client] ERROR Bad port configuration\n");
      exit(ERROR);
    }   

    if(connectAuxSocket() == SUCCESS) {
      createDirectory(userId, CLIENT);
      bzero(path, 255);
      strcpy(path, getUserDirectory(userId));
      //Preenche lista de arquivos com informações de diretório da home
      connectSocket();

      //AUX socket para comandos, upload e download
      pthread_create(&commandSocketThread, NULL,commandsThread,NULL);
            
      //cria socket sync
      pthread_create(&syncSocketThread,NULL,syncThread,NULL);    
      
      pthread_join(commandSocketThread, NULL);
      pthread_join(syncSocketThread, NULL);
    } else {
      printf("[Client] ERROR Max connections reached!\n");
    }

  } else {
    return ERROR;
  }

  return 0;
}
   
    
    