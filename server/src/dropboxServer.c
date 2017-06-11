#include <stdio.h>
#include <string.h>

#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxServer.h"

void sync_server() {
  printf("FUNFOU LINKAGEM");
  return;
}

void receive_file(char *file) {
  return;
}

void send_file(char *file){
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
      return LISTSUCCESS;
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
            return LISTSUCCESS;
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

void createDirectory(char *argv) {
  char root[50] = "mkdir -p ./sync_dir_";
  strcat(root, argv);
  system(root);  
  printf("Directory ./sync_dir_%s created successfully.\n", argv);
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

