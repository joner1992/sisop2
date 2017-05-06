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
  if(argc == 2) 
  {
	 // verify if it is help
		  if(strcmp(argv[1], "--help") == 0)
      {
        printf("Usage: server [option]\nOption:\n "
          " -p <serverPort>\n");
        return exit;
      }

	} 
	else if(argc == 3)
  {
  // verify arguments
		if(strcmp(argv[1], "-p") == 0)
    {
      exit = atoi(argv[2]);
     	return exit;
    }
	}
	if(exit == ERROR)
  {
		printf("Invalid arguments. For more information: server --help\n\n");
	}
		return exit;
}