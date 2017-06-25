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


// char *listFiles(PFILA2 clientList, char *userId, int socket) {
//   printf("LIST FILES:\n");
//   char buffer[BUFFERSIZE];
//   ClientInfo *user;
//   if(searchForUserId(clientList, userId) == SUCCESS) {
//     user = (ClientInfo *) GetAtIteratorFila2(clientList);
//     bzero(buffer, BUFFERSIZE);
//     getFiles(buffer, &(user->filesList), socket);
    
//     printf("%s", buffer);
//   }
//   return buffer;
// }

// char *getFiles(char *buffer, PFILA2 fila, int socket) {
//   char bufferPrint[BUFFERSIZE];
//   int n;

//   sendMessage(socket, "List of Files:");

//   int first;
//   first = FirstFila2(fila);
//   bzero(buffer, BUFFERSIZE);
//   if (first == LISTSUCCESS) {
//     void *fileFound;
//     UserFiles *fileWanted;
//     fileWanted = (UserFiles*) GetAtIteratorFila2(fila);

//     strcat(buffer, fileWanted->name);
//     strcat(buffer, "\n");

//     sendMessage(socket, fileWanted->name);

//     int iterator = 0;
//     while (iterator == 0) {
//       iterator = NextFila2(fila);
//       fileFound = GetAtIteratorFila2(fila);
//       if (fileFound == NULL) {
//           sendMessage(socket, "exit");
//           return buffer;
//       }
//       else {
//         fileWanted = (UserFiles*) fileFound;
//         strcat(buffer, fileWanted->name);
//         strcat(buffer, "\n");
        
//         sendMessage(socket, fileWanted->name);
//       }
//     }
//   }
//   else {
//     sendMessage(socket, "Server has empty directory.");
//     sendMessage(socket, "exit");

//     strcat(buffer, "Server has empty directory. \n");
//     return buffer;
//   }
//   return buffer;
// }
