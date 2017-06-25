
/************************************ 
********* HEADER PARA CLIENT ********
************************************/

/* ACIMA ESTÃO AS FUNCIONALIDADES MÍNIMAS PARA O CLIENTE */
int validateClientArguments(int argc, char *argv[]);
void removeFileFromSystem(char *userId);
void receiveServerFiles(int socket, char *buffer, char *path, chain_list *list);