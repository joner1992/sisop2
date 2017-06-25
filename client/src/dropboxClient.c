#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxClient.h"


int connect_server(char *host, int port) {
  return 0;
}

void sync_client(){
  return;
}

void send_file(char *file) {
  return;
}

void get_file(char *file){
  return;
}

void close_connection(){
  return;
}

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

void removeFileFromSystem(char *userId) {
  // These are data types defined in the "dirent" header
  char path[BUFFERSIZE];
  strcpy(path, getUserDirectory(userId));

  DIR *theFolder = opendir(path);
  struct dirent *next_file;
  char filepath[256];

  while ( (next_file = readdir(theFolder)) != NULL )
  {
    // build the path for each file in the folder
    sprintf(filepath, "%s/%s", path , next_file->d_name);
    if(isRegularFile(next_file)) {
      remove(filepath);
    }
  }
  closedir(theFolder);
}

void receiveServerFiles(int socket, char *buffer, char *path, chain_list *list) {
  //read name of the operation and name file
  char serverFileInformations[BUFFERSIZE];
  char fileName[BUFFERSIZE];
  char fileModificationDate[BUFFERSIZE];
  char *forIterator;
  char *subString;
  int numCommands = 1;

  bzero(serverFileInformations, BUFFERSIZE);
  strcpy(serverFileInformations, buffer);

  for (forIterator = strtok_r(serverFileInformations,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)) {
    if (numCommands%2 != 0){
      bzero(fileName, BUFFERSIZE);
      strcpy(fileName, forIterator);
    }
    else if (numCommands%2 == 0){
      bzero(fileModificationDate, BUFFERSIZE);
      strcpy(fileModificationDate, forIterator);
      if(receive_(socket, path) == SUCCESS) {
        addFilesToFileList(list, fileName, fileModificationDate, 0);
      }

    }
    numCommands++;
  }
}