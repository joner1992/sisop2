#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxServer.h"

void sync_server() {
  printf("FUNFOU LINKAGEM");
  return;
}

/* UTILITARY FUNCTIONS FOR SERVER */
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

void sendMessage (int socket, char *buffer) {
  int n;
  n = write(socket, buffer, BUFFERSIZE);
  if (n == ERROR) {
    perror("ERROR writing to socket\n");
    exit(ERROR);
  }
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
