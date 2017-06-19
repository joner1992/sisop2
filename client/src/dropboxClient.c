#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxClient.h"


/* UTILITARY FUNCTIONS FOR CLIENT */
int validateClientArguments(int argc, char *argv[]){
  int exit = ERROR;
  if(argc == 2)
  {
	// verify if it is help
	  if(strcmp(argv[1], "--help") == 0)
    {
      printf("Usage: \n client -ip <serverIp> "
             "-p <serverPort> -U <username> \n");
      return exit;
    }
	} 
	else if(argc == 7)
  {
	 // verify arguments
	  if(strcmp(argv[1], "-ip") == 0)
    {
      if(strcmp(argv[3], "-p") == 0)
      {
        if(strcmp(argv[5], "-U") == 0)
        {
            exit = SUCCESS;
      			return exit;
      	} 
      } 
    }
	}
	if(exit == -1)
  {
  		printf("Invalid arguments. For more information client --help\n\n");
	}
		return exit;
}

void syncClient(int normalFlow){
  if(normalFlow == 0){

  }
}