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
          //ClientList
          clientWanted = (ClientInfo*) clientFound;
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

int removeFromThreadList(PFILA2 fila, char *userId) {
  int first;
  first = FirstFila2(fila);

  if (first == LISTSUCCESS) {
    void *clientFound;

    clientThread *clientWanted;
    clientWanted = (clientThread*) GetAtIteratorFila2(fila);
   
    if (strcmp(clientWanted->userId, userId) == 0) {
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
          if (strcmp(clientWanted->userId, userId) == 0) {
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

void disconnectClientFromServer(int auxSocket, int syncSocket, char *userId, PFILA2 clientList, PFILA2 auxSocketsList, PFILA2 syncSocketList, int existedBefore) {
  int n;
  char buffer[BUFFERSIZE];
  bzero(buffer, BUFFERSIZE);
  strcpy(buffer, DISCONNECTED);
  
  //verify if numDevices == 1, if it is, set it to 0 and change logged_in to 0, if numDevices == 2, set it to 1 and maintain logged_in in 1
  if(existedBefore == 1){
    //remove the client if necessary
    //**********************tem que mudar pra deletar toda estrutura de arquivos quando deslogar totalmente***********************************
    removeClient(clientList, userId);
    //remove the auxiliary socket from auxSocketList
    removeFromThreadList(auxSocketsList, userId);
    //remove the sync socket from syncSocketList
    removeFromThreadList(syncSocketList, userId);  

    //close sockets
    close(auxSocket);
    close(syncSocket);
  } 
}

char *cropUserId(char *auxSocketName) {
  char *subString;
  strtok_r(auxSocketName, "_", &subString);
  return subString;
}
