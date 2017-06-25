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

struct chain_list *getDirentsFileList(char *path) {
  char pathFile[255];
  // buffer to insert global client list
  char stringFileList[BUFFERSIZE];
  // buffer to insert updated dirent list
  char stringAuxList[BUFFERSIZE];
  struct stat fileAttributes;
  struct dirent *file;
  time_t rawTime;
  struct tm * timeInfo;
  DIR *directory;

  directory = opendir(path);
  chain_list *auxList = chain_create_list();

  if(directory) {
    while ((file = readdir(directory)) != NULL) {
      if(isRegularFile(file)) {
        bzero(pathFile, 255);
        sprintf(pathFile,"%s%s",path,file->d_name);
        fileAttributes = getAttributes(pathFile);
        
        timeInfo = localtime(&fileAttributes.st_mtime);

        char lastModified[36];
        bzero(lastModified, 36);
        sprintf(lastModified, "%d%0.2d%0.2d%d%d%d", timeInfo->tm_year + 1900, timeInfo->tm_mon+1, timeInfo->tm_mday, timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
        addFilesToFileList(auxList, file->d_name, lastModified, fileAttributes.st_size);
      }
    }
  }
  return auxList;
}

int compareLists(char *stringDirentList, char *stringFileList) {

  char direntList[BUFFERSIZE];
  char fileList[BUFFERSIZE];
  char fileName[BUFFERSIZE];
  char fileModificationDate[BUFFERSIZE];
  char comparisonChar[BUFFERSIZE];

  char *forIterator;
  char *subString;
  int numCommands = 1;

  bzero(direntList, BUFFERSIZE);
  strcpy(direntList, stringDirentList);

  bzero(fileList, BUFFERSIZE);
  strcpy(fileList, stringFileList);

  //Search for all files of stringDirentList in stringFileList
  for (forIterator = strtok_r(direntList,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)) {
    if (numCommands%2 != 0){
      bzero(fileName, BUFFERSIZE);
      strcpy(fileName, forIterator);
    }
    else if (numCommands%2 == 0){
      bzero(fileModificationDate, BUFFERSIZE);
      strcpy(fileModificationDate, forIterator);

      bzero(comparisonChar, BUFFERSIZE);
      sprintf(comparisonChar, "#%s#%s#", fileName, fileModificationDate);

      char *result = strstr(fileList, comparisonChar);
      if(result == NULL) {
        return ERROR;
      }

    }
    numCommands++;
  }

  numCommands = 1;

  bzero(direntList, BUFFERSIZE);
  strcpy(direntList, stringDirentList);

  bzero(fileList, BUFFERSIZE);
  strcpy(fileList, stringFileList);
    
  //Search for all files of stringFileList in stringDirentList
  for (forIterator = strtok_r(fileList,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)) {
    if (numCommands%2 != 0){
      bzero(fileName, BUFFERSIZE);
      strcpy(fileName, forIterator);
    }
    else if (numCommands%2 == 0){
      bzero(fileModificationDate, BUFFERSIZE);
      strcpy(fileModificationDate, forIterator);

      bzero(comparisonChar, BUFFERSIZE);
      sprintf(comparisonChar, "#%s#%s#", fileName, fileModificationDate);

      char *result = strstr(direntList, comparisonChar);
      if(result == NULL) {
        return ERROR;
      }

    }
    numCommands++;
  }

  return SUCCESS;

}