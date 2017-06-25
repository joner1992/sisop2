#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxServer.h"

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
