
/************************************ 
********* HEADER PARA CLIENT ********
************************************/

/* ACIMA ESTÃO AS FUNCIONALIDADES MÍNIMAS PARA O CLIENTE */
int validateClientArguments(int argc, char *argv[]);
struct chain_list *getDirentsFileList(char *path);
int compareLists(char *stringDirentList, char *stringFileList);