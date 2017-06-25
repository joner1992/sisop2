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

void sendServerFiles(int socket, char *buffer, char *path) {
  //read name of the operation and name file
  char serverFileInformations[BUFFERSIZE];
  char fileName[BUFFERSIZE];
  char fileModificationDate[BUFFERSIZE];
  char *forIterator;
  char *subString;
  char completePath[255];
  int numCommands = 1;

  bzero(serverFileInformations, BUFFERSIZE);
  strcpy(serverFileInformations, buffer);

  for (forIterator = strtok_r(serverFileInformations,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)) {
    bzero(completePath, 255);
    strcpy(completePath, path);   

    if ((numCommands%2) != 0){
      bzero(fileName, BUFFERSIZE);
      strcpy(fileName, forIterator);
    }
    else if ((numCommands%2) == 0){
      bzero(fileModificationDate, BUFFERSIZE);
      strcpy(fileModificationDate, forIterator);
      
      strcat(completePath, fileName);
      send_(socket, completePath);
    }
    numCommands++;
  }
}