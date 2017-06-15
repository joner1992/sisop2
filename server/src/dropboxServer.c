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

void disconnectClient(int newsockfd) {
  
  int n;
  char buffer[BUFFERSIZE];
  bzero(buffer, BUFFERSIZE);
  strcpy(buffer, DISCONNECTED);
  
  n = write(newsockfd, buffer, strlen(buffer));
  if (n == ERROR) {
  perror("ERROR writing to socket\n");
    exit(ERROR);
  }
}

char *cropUserId(char *auxSocketName) {
  char *subString;
  strtok_r(auxSocketName, " ", &subString);
  char* userId = strtok_r(NULL, " ", &subString);
  
  return userId;
}
