#include "../include/dropboxUtils.h"

void createDirectory(char *argv, int server) {
    
    if(server) {
        char root[100] = "./clientsDirectories/sync_dir_";
        strcat(root, argv);
        mkdir(root, 0777);
        printf("Directory ./sync_dir_%s created successfully.\n", argv);
    }
    else {
        char home[100] = "/home/";
        char usr[50];
        if(!getlogin_r(usr, 50)) {
            strcat(home, usr);
            strcat(home, "/sync_dir_");
            strcat(home, argv);
            mkdir(home, 0777);
            printf("Directory %s created successfully", home);
        }
        else {
            exit(ERROR);
        }
        
    }
}

void initializeList(PFILA2 list){
  if(CreateFila2(list) != LISTSUCCESS) { // 0 = linked list initialized successfully
    perror("ERROR initializing linked list");
    exit(ERROR);
  }
}

int searchForFile(char *fileName, PFILA2 fileList) {
  int first;
  first = FirstFila2(fileList);
  
  if (first == LISTSUCCESS) {
    void *fileFound;
    UserFiles *fileWanted;
    fileWanted = (UserFiles*) GetAtIteratorFila2(fileList);
    if (strcmp(fileWanted->name, fileName) == 0) {
      return SUCCESS;
    }
    else {
      int iterator = 0;
      while (iterator == 0) {
        iterator = NextFila2(fileList);
        fileFound = GetAtIteratorFila2(fileList);
        if (fileFound == NULL) {
            return ERROR;
        }
        else {
          fileWanted = (UserFiles*) fileFound;
          if (strcmp(fileWanted->name, fileName) == 0) {
              return SUCCESS;
          }
        }
      }
      return ERROR;
    }
  }
  else {
     return ERROR;
  }
}


//FAZER!! tentar fazer de uma forma que sirva tanto pro client quanto para o server
int addFileToUser(char *name, char *extension, char *lastModified, int size, FILA2 fileList){
    //adicionar um novo filename ao user
    
    UserFiles *newFile = (UserFiles *) malloc(sizeof(UserFiles));
    strcpy(newFile->name, name);
    strcpy(newFile->extension, extension);
    strcpy(newFile->last_modified, lastModified);
    newFile->size = size;

    if(AppendFila2(&fileList, (void *) newFile) == LISTSUCCESS) {
        return SUCCESS;
    }
    
    printf("ERROR on adding file to LIST");
    return ERROR;
}

int removeFileFromUser(char *fileName, FILA2 fileList){
    //fazer remoção do filename do user
    if(searchForFile(fileName, &fileList) == SUCCESS) {
        if(DeleteAtIteratorFila2(&fileList) == LISTSUCCESS) {
            printf("FILE %s removed successfully from list", fileName);
            return SUCCESS;
        }
        else {
            printf("FILE couldn't be deleted from list");
        }
    }
    printf("FILE not found in list");
    return ERROR;
}

int updateFileFromUser(char *fileName, FILA2 fileList){
    //atualiza Size ao fazer download de um novo
    
    
    return 0;
}