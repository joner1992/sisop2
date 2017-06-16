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
    Client_Info *clientWanted;
    clientWanted = (Client_Info*) GetAtIteratorFila2(fila);
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
          clientWanted = (Client_Info*) clientFound;
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
    Client_Info *clientWanted;
    clientWanted = (Client_Info*) GetAtIteratorFila2(fila);
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
          clientWanted = (Client_Info*) clientFound;
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
    Client_Info *clientWanted;
    clientWanted = (Client_Info*) GetAtIteratorFila2(fila);
    if (strcmp(clientWanted->userId, userId) == 0) {
      //modificado para subtrair um no numDevices e setar logged_in
      clientWanted->numDevices = clientWanted->numDevices - 1;
      if(clientWanted->numDevices == 0){
        clientWanted->logged_in = 0;
        DeleteAtIteratorFila2(fila);
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
          clientWanted = (Client_Info*) clientFound;
          if (strcmp(clientWanted->userId, userId) == 0) {
            //modificado para subtrair um no numDevices e setar logged_in
            clientWanted->numDevices = clientWanted->numDevices - 1;
            if(clientWanted->numDevices == 0){
              clientWanted->logged_in = 0;
              DeleteAtIteratorFila2(fila);
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

void disconnectClientFromServer(int newsockfd, char *userId, PFILA2 fila, int existedBefore) {
  int n;
  char buffer[BUFFERSIZE];
  bzero(buffer, BUFFERSIZE);
  strcpy(buffer, DISCONNECTED);

  //verify if numDevices == 1, if it is, set it to 0 and change logged_in to 0, if numDevices == 2, set it to 1 and maintain logged_in in 1
  if(existedBefore == 1){
    removeClient(fila, userId);
  }
  
  //send message to the user telling him to finish his sockets
  n = write(newsockfd, buffer, strlen(buffer));
  if (n == ERROR) {
    perror("ERROR writing to socket\n");
  } else {
    printf("disconnected client\n");
  }
}

char *cropUserId(char *auxSocketName) {
  char *subString;
  strtok_r(auxSocketName, "_", &subString);
  return subString;
}
