/************************************ 
********* HEADER PARA SERVER ********
************************************/

int validateServerArguments(int argc, char *argv[]);
void sendServerFiles(int socket, char *buffer, char *path);
//char *listFiles(PFILA2 clientList, char *userId, int socket);
//char *getFiles(char *buffer, PFILA2 fila, int socket);