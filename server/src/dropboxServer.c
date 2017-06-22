#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxServer.h"

void syncClientServer(int isServer, int socketId, char *userId, PFILA2 fileList) {
  char nameOfFile[BUFFERSIZE];
  char dateOfFile[BUFFERSIZE];
  char buffer[BUFFERSIZE];
  char bufferPath[BUFFERSIZE];
  char operationFilename[BUFFERSIZE];
  char operation[BUFFERSIZE];
  char serverFileList[BUFFERSIZE];
  char path[255];
  char clientLocalFileList[BUFFERSIZE];
  char file[BUFFERSIZE];
  char *forIterator;
  char *subString;
  int n;
  int numCommands;

  //lê pares de arquivo e data de modificação
  bzero(buffer, BUFFERSIZE);
  strcpy(buffer, receiveMessage(socketId, "", FALSE));
  
  printf("LEU PARES DE ARQUIVO E DATA DO CLIENT: %s\n", buffer);

  bzero(operationFilename, BUFFERSIZE);
  bzero(clientLocalFileList, BUFFERSIZE);
  strcat(clientLocalFileList, "#");

  numCommands = 1;
  for (forIterator = strtok_r(buffer,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)) {
      if (numCommands % 2 != 0) { //impar
        bzero(nameOfFile, BUFFERSIZE);
			  strcpy(nameOfFile, forIterator);
		  }
		  else if (numCommands % 2 == 0) { //par
        bzero(dateOfFile, BUFFERSIZE);
			  strcpy(dateOfFile, forIterator);
        strcat(clientLocalFileList, nameOfFile);
        strcat(clientLocalFileList, "#");

        //compara se o arquivo do server é mais novo, se for, download no cliente
        //compara se o arquivo do server é mais antigo, se for, upload no client
        strcat(operationFilename, compareDatesFromFileList(fileList, nameOfFile, dateOfFile, isServer));                        
      }
      numCommands++;
  }

  printf("PRIMEIRO OPERATION FILE NAME (UPDATE): %s\n", operationFilename);

  bzero(serverFileList, BUFFERSIZE);
  strcpy(serverFileList, getServerFileNames(fileList));

  //compara se o arquivo que está no server, existe, se não existe, deleta no cliente
  //compara se o arquivo do server não existe no client, se não existe, download no client
  //se existir no servidor e não no cliente, deleta no servidor
  //se nao existir no servidor e existir no client, upload do client
  bzero(buffer, BUFFERSIZE);
  strcpy(buffer, clientLocalFileList);
  //compara a lista do server com cada arquivo do client
  for (forIterator = strtok_r(buffer,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)) {
    bzero(file, BUFFERSIZE);
    sprintf(file,"#%s#",forIterator);
    if(strstr(serverFileList, file)==NULL) {
      if(isServer) {
        sprintf(operationFilename, "%s%s#%s#", operationFilename, "delete", forIterator);
      } else {
        sprintf(operationFilename, "%s%s#%s#", operationFilename, "upload", forIterator);
      }
    } 
  }

  printf("SEGUNDO OPERATION FILE NAME (LISTA COM DELETE E UPLOAD): %s\n", operationFilename);

  bzero(buffer, BUFFERSIZE);
  bzero(file, BUFFERSIZE);
  bzero(path, 255);
  strcpy(buffer, serverFileList);

  //compara a lista do client com cada arquivo do server
  strcpy(path, "./clientsDirectories/sync_dir_");
  sprintf(path,"%s%s/",path, userId);

  for (forIterator = strtok_r(buffer,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)) {
    bzero(file, BUFFERSIZE);
    sprintf(file,"#%s#",forIterator);
    if(strstr(clientLocalFileList, file)==NULL) {
      if(isServer) {
        sprintf(operationFilename, "%s%s#%s#", operationFilename, "download", forIterator);
      } else {
        bzero(bufferPath, BUFFERSIZE);
        sprintf(bufferPath, "%s%s", path, forIterator);
        removeFileFromSystem(bufferPath);
        removeFileFromUser(forIterator, fileList, userId, SERVER);
      }
    } 
  }

  printf("TERCEIRO OPERATION FILE NAME (LISTA COM DOWNLOAD): %s\n", operationFilename);

  //envia pares de operacao#arquivo
  bzero(buffer, BUFFERSIZE);
  strcat(buffer, operationFilename);
  n = write(socketId, buffer, BUFFERSIZE);
  if (n == ERROR) {
    perror("ERROR reading from socket\n");
    exit(ERROR);
  }

  printf("ENVIOU PARES OPERACAO#ARQUIVO PARA O CLIENT: %s\n", buffer);

  numCommands = 1;
  for (forIterator = strtok_r(buffer,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)) {
      if (numCommands % 2 != 0) { //impar
        bzero(operation, BUFFERSIZE);
			  strcpy(operation, forIterator);
		  }
		  else if (numCommands % 2 == 0) { //par
        bzero(nameOfFile, BUFFERSIZE);
			  strcpy(nameOfFile, forIterator);
        
        //executa recebimento ou envio
        if(strcmp(operation, "download") == 0){
          bzero(bufferPath, BUFFERSIZE);
          sprintf(bufferPath, "%s%s", path, nameOfFile);
          printf("ENVIOU ARQUIVO(PATH): %s#%s\n", bufferPath, path);

          //recebendo autorização para começar
          receiveMessage(socketId, "downloadingFile", TRUE);
          sendMessage(socketId, "okDownloadingFile");

          //envia arquivo
          send_(socketId, bufferPath);
        } else if(strcmp(operation, "upload") == 0){
          printf("RECEBEU ARQUIVO(PATH): %s\n", path);

          //recebendo autorização para começar
          receiveMessage(socketId, "uploadingFile", TRUE);
          sendMessage(socketId, "okUploadingFile");

          //recebe arquivo
          if(receive_(socketId, path) == SUCCESS){
            struct stat file_stat = getAttributes(path);
            char lastModified[36];
            bzero(lastModified, 36);
            strftime(lastModified, 36, "%Y.%m.%d %H:%M:%S", localtime(&file_stat.st_mtime));
            addFileToUser(basename(path), ".txt", lastModified, file_stat.st_size, fileList);   
          }
        } else if(strcmp(operation, "delete") == 0){
          //recebendo autorização para começar
          receiveMessage(socketId, "removingFile", TRUE);
          sendMessage(socketId, "okRemovingFile");
        }
      }
      numCommands++;
  }
  return 0;
}

char *getServerFileNames(PFILA2 fileList) {
  char concatFileNames[BUFFERSIZE];
  int n;
  int first;
  first = FirstFila2(fileList);
  bzero(concatFileNames, BUFFERSIZE);
  strcpy(concatFileNames, "#");
  
  if (first == LISTSUCCESS) {
    void *fileFound;
    UserFiles *fileWanted;
    fileWanted = (UserFiles*) GetAtIteratorFila2(fileList);
    strcat(concatFileNames, fileWanted->name);
    strcat(concatFileNames, "#");    

    int iterator = 0;
    while (iterator == 0) {
      iterator = NextFila2(fileList);
      fileFound = GetAtIteratorFila2(fileList);
      if (fileFound == NULL) {
          return concatFileNames;
      }
      else {
        fileWanted = (UserFiles*) fileFound;
        strcat(concatFileNames, fileWanted->name);
        strcat(concatFileNames, "#");
      }
    }
  }
  else {
    strcat(concatFileNames, "#");
    return concatFileNames;
  }
  return concatFileNames;
}


char *compareDatesFromFileList(PFILA2 fileList, char *fileName, char *dateOfFile, int isServer) {
  char buffer[BUFFERSIZE];
  int n;
  int first;
  bzero(buffer, BUFFERSIZE);

  first = FirstFila2(fileList);
  
  if (first == LISTSUCCESS) {
    void *fileFound;
    UserFiles *fileWanted;
    fileWanted = (UserFiles*) GetAtIteratorFila2(fileList);

    if(strcmp(fileName, fileWanted->name) == 0){
      //data do client é mais antiga que a data do server
      if(strcmp(dateOfFile, fileWanted->last_modified) < 0){
        if(isServer){
          //operacao
          strcat(buffer, "download");
          strcat(buffer, "#");
          strcat(buffer, fileName);
          strcat(buffer, "#");
        } else {
          strcat(buffer, "upload");
          strcat(buffer, "#");
          strcat(buffer, fileName);
          strcat(buffer, "#");
        }
        //data do client é mais nova que a data do server
      } else if(strcmp(dateOfFile, fileWanted->last_modified) > 0) {
        if(isServer){
          //operacao
          strcat(buffer, "upload");
          strcat(buffer, "#");
          strcat(buffer, fileName);
          strcat(buffer, "#");
        } else {
          strcat(buffer, "download");
          strcat(buffer, "#");
          strcat(buffer, fileName);
          strcat(buffer, "#");
        }
      }
    }

    int iterator = 0;
    while (iterator == 0) {
      iterator = NextFila2(fileList);
      fileFound = GetAtIteratorFila2(fileList);
      if (fileFound == NULL) {
          return buffer;
      }
      else {
        fileWanted = (UserFiles*) fileFound;
        if(strcmp(fileName, fileWanted->name) == 0){
          //data do client é mais antiga que a data do server
          if(strcmp(dateOfFile, fileWanted->last_modified) < 0){
            if(isServer){
              //operacao
              strcat(buffer, "download");
              strcat(buffer, "#");
              strcat(buffer, fileName);
              strcat(buffer, "#");
            } else {
              strcat(buffer, "upload");
              strcat(buffer, "#");
              strcat(buffer, fileName);
              strcat(buffer, "#");
            }
            //data do client é mais nova que a data do server
          } else if(strcmp(dateOfFile, fileWanted->last_modified) > 0) {
            if(isServer){
              //operacao
              strcat(buffer, "upload");
              strcat(buffer, "#");
              strcat(buffer, fileName);
              strcat(buffer, "#");
            } else {
              strcat(buffer, "download");
              strcat(buffer, "#");
              strcat(buffer, fileName);
              strcat(buffer, "#");
            }
          }
        }
      }
    }
  }
  else {
    strcat(buffer, "#");
    return buffer;
  }
  return buffer;
}

int validateServerArguments(int argc, char *argv[]) {
  int exit = ERROR;
  if(argc == 3)
  {
  // verify arguments
		if(strcmp(argv[1], "-p") == 0)
    {
      exit = SUCCESS;
    }
	}
	else if(argc == 2) 
  {
	 // verify if it is help
		if(strcmp(argv[1], "--help") == 0)
    {
      printf("Usage: server [option]\nOption:\n "
        " -p <serverPort>\n");
      return exit;
    }
	}
	if(exit == ERROR)
  {
		printf("Invalid arguments. For more information: server --help\n\n");
	}
		return exit;
}

int searchForUserId(PFILA2 fila, char *userId) {
  int first;
  first = FirstFila2(fila);
  
  if (first == LISTSUCCESS) {
    void *clientFound;
    ClientInfo *clientWanted;
    clientWanted = (ClientInfo*) GetAtIteratorFila2(fila);
    if (strcmp(clientWanted->userId, userId) == 0) {
      return SUCCESS;
    }
    else {
      int iterator = 0;
      while (iterator == 0) {
        iterator = NextFila2(fila);
        clientFound = GetAtIteratorFila2(fila);
        if (clientFound == NULL) {
            return ERROR;
        }
        else {
          clientWanted = (ClientInfo*) clientFound;
          if (strcmp(clientWanted->userId, userId) == 0) {
              return SUCCESS;
          }
        }
      }
      return ERROR;
    }
  }
  else {
     return ERROR;
  }
}

int secondLogin(PFILA2 fila, char *userId) {
  int first;
  first = FirstFila2(fila);

  if (first == LISTSUCCESS) {
    void *clientFound;
    ClientInfo *clientWanted;
    clientWanted = (ClientInfo*) GetAtIteratorFila2(fila);
    if (strcmp(clientWanted->userId, userId) == 0) {
      //modificado para somar um no numDevices e setar logged_in
      if(clientWanted->numDevices >= MAXDEVICES){
        return ERROR;
      } else {
        clientWanted->numDevices = clientWanted->numDevices + 1;
        clientWanted->logged_in = 1;
        return SUCCESS;
      }
    }
    else {
      int iterator = 0;
      while (iterator == 0) {
        iterator = NextFila2(fila);
        clientFound = GetAtIteratorFila2(fila);
        if (clientFound == NULL) {
          return ERROR;
        }
        else {
          clientWanted = (ClientInfo*) clientFound;
          if (strcmp(clientWanted->userId, userId) == 0) {
            //modificado para somar um no numDevices e setar logged_in
            if(clientWanted->numDevices >= MAXDEVICES){
              return ERROR;
            } else {
              clientWanted->numDevices = clientWanted->numDevices + 1;
              clientWanted->logged_in = 1;
              return SUCCESS;
            }
          }
        }
      }
      return ERROR;
    }
  }
  else {
    return ERROR;
  }
}

int removeClient(PFILA2 fila, char *userId) {
  int first;
  first = FirstFila2(fila);

  if (first == LISTSUCCESS) {
    void *clientFound;

    ClientInfo *clientWanted;
    clientWanted = (ClientInfo*) GetAtIteratorFila2(fila);
   
    if (strcmp(clientWanted->userId, userId) == 0) {
      //modificado para subtrair um no numDevices e setar logged_in
      clientWanted->numDevices = clientWanted->numDevices - 1;
      if(clientWanted->numDevices == 0){
        clientWanted->logged_in = 0;
      } else {
        clientWanted->logged_in = 1;
      }
      return SUCCESS;
    }
    else {
      int iterator = 0;
      while (iterator == 0) {
        iterator = NextFila2(fila);
        clientFound = GetAtIteratorFila2(fila);
        if (clientFound == NULL) {
          return ERROR;
        }
        else {
          //ClientList
          clientWanted = (ClientInfo*) clientFound;
          if (strcmp(clientWanted->userId, userId) == 0) {
            //modificado para subtrair um no numDevices e setar logged_in
            clientWanted->numDevices = clientWanted->numDevices - 1;
            if(clientWanted->numDevices == 0){
              clientWanted->logged_in = 0;
            } else {
              clientWanted->logged_in = 1;
            }
            return SUCCESS;
          }
        }
      }
      return ERROR;
    }
  }
  else {
    return ERROR;
  }
}

int removeFromThreadList(PFILA2 fila, char *userId, int socket) {
  int first;
  first = FirstFila2(fila);

  if (first == LISTSUCCESS) {
    void *clientFound;

    clientThread *clientWanted;
    clientWanted = (clientThread*) GetAtIteratorFila2(fila);
   
    if ((strcmp(clientWanted->userId, userId) == 0) && (clientWanted->socketId == socket)) {
      DeleteAtIteratorFila2(fila);     
      return SUCCESS;
    }
    else {
      int iterator = 0;
      while (iterator == 0) {
        iterator = NextFila2(fila);
        clientFound = GetAtIteratorFila2(fila);
        if (clientFound == NULL) {
          return ERROR;
        }
        else {
          //ClientList
          clientWanted = (clientThread*) clientFound;
          if ((strcmp(clientWanted->userId, userId) == 0) && (clientWanted->socketId == socket)) {
            DeleteAtIteratorFila2(fila);
            return SUCCESS;
          }
        }
      }
      return ERROR;
    }
  }
  else {
    return ERROR;
  }
}

void disconnectClientFromServer(int socket, char *userId, PFILA2 auxSocketsList, PFILA2 syncSocketList, int isAux) {  
  if(isAux == 1){   
    pthread_mutex_lock(&auxSocketsListMutex);
      removeFromThreadList(auxSocketsList, userId, socket);
    pthread_mutex_unlock(&auxSocketsListMutex);
  } else {
    pthread_mutex_lock(&syncSocketsListMutex);
      removeFromThreadList(syncSocketList, userId, socket);
    pthread_mutex_unlock(&syncSocketsListMutex);
  }
}

char *cropUserId(char *auxSocketName) {
  char *subString;
  strtok_r(auxSocketName, "_", &subString);
  return subString;
}


char *listFiles(PFILA2 clientList, char *userId, int socket) {
  printf("LIST FILES:\n");
  char buffer[BUFFERSIZE];
  ClientInfo *user;
  if(searchForUserId(clientList, userId) == SUCCESS) {
    user = (ClientInfo *) GetAtIteratorFila2(clientList);
    bzero(buffer, BUFFERSIZE);
    getFiles(buffer, &(user->filesList), socket);
    
    printf("%s", buffer);
  }
  return buffer;
}

char *getFiles(char *buffer, PFILA2 fila, int socket) {
  char bufferPrint[BUFFERSIZE];
  int n;

  sendMessage(socket, "List of Files:");

  int first;
  first = FirstFila2(fila);
  bzero(buffer, BUFFERSIZE);
  if (first == LISTSUCCESS) {
    void *fileFound;
    UserFiles *fileWanted;
    fileWanted = (UserFiles*) GetAtIteratorFila2(fila);

    strcat(buffer, fileWanted->name);
    strcat(buffer, "\n");

    sendMessage(socket, fileWanted->name);

    int iterator = 0;
    while (iterator == 0) {
      iterator = NextFila2(fila);
      fileFound = GetAtIteratorFila2(fila);
      if (fileFound == NULL) {
          sendMessage(socket, "exit");
          return buffer;
      }
      else {
        fileWanted = (UserFiles*) fileFound;
        strcat(buffer, fileWanted->name);
        strcat(buffer, "\n");
        
        sendMessage(socket, fileWanted->name);
      }
    }
  }
  else {
    sendMessage(socket, "Server has empty directory.");
    sendMessage(socket, "exit");

    strcat(buffer, "Server has empty directory. \n");
    return buffer;
  }
  return buffer;
}