#include "../include/dropboxUtils.h"

int BUFFER_TRANSFER = 32 * 1024;
int DEBUG = 1;

void createDirectory(char *argv, int server) {
    
    if(server) {
        char root[100] = "./clientsDirectories/sync_dir_";
        strcat(root, argv);
        mkdir(root, 0777);
        printf("Directory ./sync_dir_%s created successfully.\n", argv);
    } else {
        char home[100] = "/home/";
        char usr[50];
        if(!getlogin_r(usr, 50)) {
            strcat(home, usr);
            strcat(home, "/sync_dir_");
            strcat(home, argv);
            mkdir(home, 0777);
            printf("Directory %s created successfully.\n", home);
        } else {
            exit(ERROR);
        }
    }
}

char *getUserDirectory(char *userId) {
    char home[100] = "/home/";
    char usr[50];
    if(!getlogin_r(usr, 50)) {
        strcat(home, usr);
        strcat(home, "/sync_dir_");
        strcat(home, userId);
        strcat(home, "/");
    } else {
        exit(ERROR);
    }
    return home;
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
    } else {
      int iterator = 0;
      while (iterator == 0) {
        iterator = NextFila2(fileList);
        fileFound = GetAtIteratorFila2(fileList);
        if (fileFound == NULL) {
            return ERROR;
        } else {
          fileWanted = (UserFiles*) fileFound;
          if (strcmp(fileWanted->name, fileName) == 0) {
              return SUCCESS;
          }
        }
      }
      return ERROR;
    }
  } else {
     return ERROR;
  }
}


//FAZER!! tentar fazer de uma forma que sirva tanto pro client quanto para o server
int addFileToUser(char *name, char *extension, char *lastModified, int size, PFILA2 fileList){

    if(searchForFile(name, fileList) != SUCCESS) {
        UserFiles *newFile = (UserFiles *) malloc(sizeof(UserFiles));
        strcpy(newFile->name, name);
        strcpy(newFile->extension, extension);
        strcpy(newFile->last_modified, lastModified);
        newFile->size = size;
        if(AppendFila2(fileList, (void *) newFile) == LISTSUCCESS) {
            return SUCCESS;
        }
    }
    else{
        UserFiles *newFile;
        newFile = (UserFiles *) GetAtIteratorFila2(fileList);
        strcpy(newFile->name, name);
        strcpy(newFile->extension, extension);
        strcpy(newFile->last_modified, lastModified);
        newFile->size = size;
        return SUCCESS;      
    }
    return ERROR;
}

int removeFileFromUser(char *fileName, PFILA2 fileList, char *userId, int isServer){
    //fazer remoção do filename do user
    if(searchForFile(fileName, fileList) == SUCCESS) {
        if(DeleteAtIteratorFila2(fileList) == LISTSUCCESS) {
            printf("FILE %s removed successfully from list\n", fileName);
            return SUCCESS;
        }
        else {
            printf("FILE couldn't be deleted from list\n");
        }
    }
    printf("FILE not found in list\n");
    return ERROR;
}

int send_(int socket, char* filename) {
   int size, read_size, stat, packet_index;
   char send_buffer[BUFFER_TRANSFER], read_buffer[256];
   packet_index = 1;
   bzero(send_buffer, sizeof(BUFFER_TRANSFER));
   bzero(read_buffer, sizeof(read_buffer));
   
   FILE *file = fopen(filename, "rb");

   if(file == NULL) {
        printf("Error - File not found or can't open\n"); 
        write(socket, "File not found", 255);
        return ERROR;
   } else {
        write(socket, "sendingFile", 255);
   }
    
   if(DEBUG) printf("\nSending %s\n", basename(filename));
   usleep(10);  
   write(socket, basename(filename), 255);
    
   if(DEBUG) printf("Getting file Size\n");   

   fseek(file, 0, SEEK_END);
   size = ftell(file);
   fseek(file, 0, SEEK_SET);
    
   if(DEBUG) printf("Total file size: %i\n",size);

   //Send Size
   if(DEBUG) printf("Sending file Size\n");
   write(socket, (void *)&size, sizeof(int));

   //Send Picture as Byte Array
   if(DEBUG) printf("Sending file as Byte Array\n");

   do { //Read while we get errors that are due to signals.
      stat = read(socket, &read_buffer , 255);
      if(DEBUG) printf("Bytes read: %i\n",stat);
   } while (stat < 0);

   if(DEBUG) printf("Received data in socket\n");
   if(DEBUG) printf("Socket data: %c\n", read_buffer);

   while(!feof(file)) {

    read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, file);


    usleep(50);
    //Send data through our socket 
    do {
        stat = write(socket, send_buffer, read_size);  
    } while (stat < 0);

    if(DEBUG) { 
        printf("Packet Number: %i\n",packet_index);
        printf("Packet Size Sent: %i\n\n\n",read_size);     
    }

    packet_index++;  

    //Zero out our send buffer
    bzero(send_buffer, sizeof(send_buffer));
   }
     
  fclose(file);
}

int receive_(int socket, char path[255]) { // Start function 

  int recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;
  char filearray[BUFFER_TRANSFER + 1];
  char fullPath[255];
  char filename[255];
  
  bzero(fullPath, 255);
  bzero(filearray, BUFFER_TRANSFER+1);
  bzero(filename, 255);
  
  FILE *file;
  
  do {
    stat = read(socket, filename, sizeof(filename));
  } while(stat < 0);
  
  if(strcmp(filename, "File not found") == 0){
    printf("FILE NOT FOUND RECEIVED\n");
    return ERROR;
  } else if(strcmp(filename, "sendingFile") == 0) {
    printf("SENDINGFILE RECEIVED\n");
  }

  bzero(filename, 255);
  //reading fileName
  do {
    stat = read(socket, filename, sizeof(filename));
  } while(stat < 0);

  if(DEBUG)  printf("Receiveing %s\n", filename);
  
  //Find the size of the file
  do {
    stat = read(socket, &size, sizeof(int));
  } while(stat<0);
  
   if(DEBUG) {
        printf("Packet received.\n");
        printf("Packet size: %i\n",stat);
        printf("File size: %i\n",size);
        printf(" \n");
   }
   
  char buffer[] = "Got it";
  
  //Send our verification signal
  do {
    usleep(10);  
    stat = write(socket, &buffer, sizeof(int));
  } while(stat<0);
  
   if(DEBUG) printf("Reply sent\n\n");
  
    sprintf(fullPath, "%s%s",path,filename);
  
  
  file = fopen(fullPath, "wb");
  
  if( file == NULL) {
    printf("Error has occurred. File could not be opened\n");
    return -1; 
  }
  
  //Loop while we have not received the entire file yet

  struct timeval timeout = {10,0};
  
  fd_set fds;
  int buffer_fd;
  
  while(recv_size < size) {
  
    FD_ZERO(&fds);
    FD_SET(socket,&fds);
  
    buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);
  
    if (buffer_fd < 0)
       printf("error: bad file descriptor set.\n");
  
    if (buffer_fd == 0)
       printf("error: buffer read timeout expired.\n");
  
    if (buffer_fd > 0) {
        do{
               read_size = read(socket,filearray, BUFFER_TRANSFER + 1);
        } while(read_size <0);
        
        if(DEBUG) {
            printf("Packet number received: %i\n",packet_index);
            printf("Packet size: %i\n",read_size);
        }
  
        //Write the currently read data into our file file
         write_size = fwrite(filearray,1,read_size, file);
          if(DEBUG) printf("Written file size: %i\n",write_size); 
  
             if(read_size !=write_size) {
                 printf("error in read write\n");    }
  
  
             //Increment the total number of bytes read
             recv_size += read_size;
             packet_index++;
             
              if(DEBUG) printf("Total received file size: %i\n\n\n",recv_size);
    }
  
  }

  fclose(file);
   if(DEBUG) printf("File successfully Received!\n");
  return 1;
}

struct stat getAttributes(char* pathFile) {
  struct stat attributes;
  if (stat(pathFile,&attributes) == -1) {
        perror("ERROR Get attributes from file");
        return;
  }
  return attributes;
}

char *getFilenameExt(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int isRegularFile(struct dirent *file) {
  return file->d_type == DT_REG;
}

void getFilesFromUser(char* userId, PFILA2 filesList, int server) {
  char root[100] = "./clientsDirectories/sync_dir_";
  char pathDirectory[255];
  char pathFile[255];
  struct dirent *file;
  struct stat fileAttributes;
  DIR* directory;
    
  bzero(pathDirectory, 255);
  bzero(pathFile, 255);

  if(server) {
    sprintf(pathDirectory, "%s%s/",root,userId);
  }
  else {
      strcpy(pathDirectory, getUserDirectory(userId));
  }

  directory = opendir(pathDirectory);

  if(directory) {
    while ((file = readdir(directory)) != NULL) {
      if(isRegularFile(file)) {
        sprintf(pathFile,"%s%s",pathDirectory,file->d_name);
        fileAttributes = getAttributes(pathFile);
        
        char lastModified[36];
        bzero(lastModified, 36);
        strftime(lastModified, 36, "%Y.%m.%d %H:%M:%S", localtime(&fileAttributes.st_mtime));
        addFileToUser(file->d_name, getFilenameExt(file->d_name), lastModified, fileAttributes.st_size, filesList);
        // printf("%s\n", file->d_name);
        // printf("%ld\n", fileAttributes.st_mtime);
        // printf("%lld\n", fileAttributes.st_size);
      }
    }
  }
}

char *removeFileNameFromPath(char *path, char *stopCharacter){
    char fileName[BUFFERSIZE];
    char pathCopy[BUFFERSIZE];
    char *forIterator;
    char *subString;

    strcpy(pathCopy, path);

    bzero(fileName, BUFFERSIZE);

    for (forIterator = strtok_r(pathCopy,stopCharacter, &subString); forIterator != NULL; forIterator = strtok_r(NULL, stopCharacter, &subString)){
        strcpy(fileName, forIterator);
    }

    return fileName;
}

char *receiveMessageList (int socket, char *conditionToStop, int printing) {
    int n;
    char buffer[BUFFERSIZE];
    
    while(1){
        bzero(buffer, BUFFERSIZE);
        n = read(socket, buffer, BUFFERSIZE);
        if (n == ERROR) {
        perror("ERROR reading to socket\n");
        exit(ERROR);
        }

        if(strcmp(buffer, conditionToStop) == 0) {
            break;
        } else if(printing == 1) {
            printf("%s\n", buffer);
        } else if (printing == 2){
            printf("%s", buffer);
        }
    }
    return buffer;
}

int cleanList(PFILA2 fileList, char *name) {

  int first;
  first = FirstFila2(fileList);

  UserFiles *currentFile;
  if (first == LISTSUCCESS) {
    currentFile = (UserFiles*) GetAtIteratorFila2(fileList);
    if(strcmp(currentFile->name, name) == 0){
        DeleteAtIteratorFila2(fileList);
        return SUCCESS;
    } else {
      int iterator = 0;
      while (iterator == 0) {
        iterator = NextFila2(fileList);
        currentFile = (UserFiles*) GetAtIteratorFila2(fileList);
        if (currentFile == NULL) {
            return SUCCESS;
        }
        else {
            if(strcmp(currentFile->name, name) == 0){
                DeleteAtIteratorFila2(fileList);
                return SUCCESS;
            }
        }
      }
      return SUCCESS;
    }
  } else {
     return ERROR;
  }
}

char *getListFilesFromUser(char *buffer, PFILA2 fila, int isServer) {
  bzero(buffer, BUFFERSIZE);
  int n;
  int first;
  first = FirstFila2(fila);
  
  if (first == LISTSUCCESS) {
    void *fileFound;
    UserFiles *fileWanted;
    fileWanted = (UserFiles*) GetAtIteratorFila2(fila);
    
    strcat(buffer, fileWanted->name);
    strcat(buffer, "#");    
    strcat(buffer, fileWanted->last_modified);
    strcat(buffer, "#");

    int iterator = 0;
    while (iterator == 0) {
      iterator = NextFila2(fila);
      fileFound = GetAtIteratorFila2(fila);
      if (fileFound == NULL) {
          return buffer;
      }
      else {
        fileWanted = (UserFiles*) fileFound;
        strcat(buffer, fileWanted->name);
        strcat(buffer, "#");
        strcat(buffer, fileWanted->last_modified);
        strcat(buffer, "#");
      }
    }
  }
  else {
    strcat(buffer, "#");
    return buffer;
  }
  return buffer;
}

int removeFileFromSystem(char *path) {
    if(remove(path) == 0){
        return SUCCESS;
    }
    return ERROR;
}

void updateLocalTime(char *buffer) {
    time_t rawtime;
	struct tm * timeinfo;

	time (&rawtime);
	timeinfo = localtime (&rawtime);

	sprintf(buffer, "%d%0.2d%0.2d%d%d%d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	return 0;
}

//compara 2 listas de strings, se tiverem adicionado algo ou removido, retorna -1
int compareListsOfFiles(char *oldList, char *newList) {
    char *forIterator;
    char *subString;
    char file[BUFFERSIZE];
    char copyOfOldList[BUFFERSIZE];
    char copyOfNewList[BUFFERSIZE];

    strcpy(copyOfOldList, oldList);
    strcpy(copyOfNewList, newList);

    for (forIterator = strtok_r(copyOfOldList,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)) {
        bzero(file, BUFFERSIZE);
        sprintf(file,"#%s#",forIterator);
        if(strstr(newList, file)==NULL) {
            return -1;
        } 
    }

    for (forIterator = strtok_r(copyOfNewList,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)) {
        bzero(file, BUFFERSIZE);
        sprintf(file,"#%s#",forIterator);
        if(strstr(oldList, file)==NULL) {
            return -1;
        } 
    }

    return 1;
}

void sendMessage (int socket, char *buffer) {
  int n;
  n = write(socket, buffer, BUFFERSIZE);
  if (n == ERROR) {
    perror("ERROR writing to socket\n");
    exit(ERROR);
  }

  return 0;
}

char *receiveMessage(int socket, char *condition, int isCondition) {
    int n;
    char buffer[BUFFERSIZE];
    
    while(1){
        bzero(buffer, BUFFERSIZE);
        n = read(socket, buffer, BUFFERSIZE);
        if (n == ERROR) {
        perror("ERROR reading to socket\n");
        exit(ERROR);
        }

        if(isCondition) {
            if(strcmp(buffer, condition) == 0) {
                break;
            }
        } else {
            if(n > 0){
                break;
            }
        }

    }
    return buffer;
}

int newSend(int socket, char *pathWithFilename) {
    char usuario[TAM_MAX];
    char buffer[TAM_MAX];
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem
    ssize_t bytesLidos;
    ssize_t bytesEnviados;
    FILE* handler;

    strcpy(usuario, "usuario");

    bzero(buffer,TAM_MAX);
    strcpy(buffer, basename(pathWithFilename));

    bytesEnviados = send(socket, buffer, TAM_MAX, 0); // envia o nome do arquivo que o cliente quer receber
    if (bytesEnviados < 0)
        printf("[ERROR ][[User: %s]] Error sending the filename to be sent. \n", usuario);
    else
        printf("[Server][User: %s] The requested file by the client is: %s\n", usuario, buffer); // Escreve o nome do arquivo que o cliente quer

    //handler = fopen(diretorio,"r");
    if ((handler = fopen(pathWithFilename, "r")) == NULL) {
        printf("[ERROR ][User: %s] Error sending the file. \n", usuario);
        return;
    }

    while ((bytesLidos = fread(buffer, 1,sizeof(buffer), handler)) > 0){ // Enquanto o sistema ainda estiver lendo bytes, o arquivo nao terminou
        if ((bytesEnviados = send(socket,buffer,bytesLidos,0)) < bytesLidos) { // Se a quantidade de bytes enviados, não for igual a que a gente leu, erro
            printf("[ERROR ][User: %s] Error sending the file.", usuario);
            return;
        }
        bzero(buffer, TAM_MAX); // Reseta o buffer
    }
    fclose(handler);
    return SUCCESS;
}

int newReceive(int socket, char *pathWithoutFilename) {
    char usuario[TAM_MAX];
    char buffer[TAM_MAX]; // Buffer que armazena os pacotes que vem sido recebidos
    ssize_t bytesRecebidos = 0; // Quantidade de bytes que foram recebidos numa passagem
    ssize_t bytesEnviados; 
    FILE* handler; // Inteiro para manipulação do arquivo que botaremos no servidor
    printf("PATH WITHOUT FILENAME: %s\n", pathWithoutFilename);
    bzero(buffer, TAM_MAX);
    int flag = 1;
    flag = htonl(flag);

    strcpy(usuario, "usuario");

    while((bytesRecebidos = recv(socket, buffer, sizeof(buffer),0)) < 0){ // recebe o nome do arquivo que vai receber do cliente
    }

    printf("RECEIVED FILENAME: %s\n", buffer);

    printf("[Server][User: %s] The file to be sent by client is: %s\n", usuario, buffer); // Escreve o nome do arquivo
    
    // if ((bytesEnviados = send(socket, &flag, sizeof(flag), 0)) < 0){
    //     printf("[ERROR ][User: %s] Error sending acknowledgement to client for file receiving.", usuario); // Envia uma flag dizendo pro cliente que ta tudo pronto e a transferencia do conteudo do arquivo pode começar
    //     return ERROR;
    // }

    strcat(pathWithoutFilename,buffer);

    printf("FULL PATH WITH FILENAME: %s\n", pathWithoutFilename);

    bytesRecebidos = 0; // Reseta o numero de bytes lidos

    handler = fopen(pathWithoutFilename, "w"); // Abre o arquivo 

    printf("ABRIU O ARQUIVO: %s\n", pathWithoutFilename);

    bzero(buffer, TAM_MAX); // Reseta o buffer

    while ((bytesRecebidos = recv(socket, buffer, TAM_MAX, 0)) > 0){  // Enquanto tiver coisa sendo lida, continua lendo
    	if (bytesRecebidos < 0) { // Se a quantidade de bytes recebidos for menor que 0, deu erro
       		printf("[ERROR ][User: %s] Error when trying to receive client package.\n", usuario);
            fclose(handler);
            return ERROR;
    	}
        if (buffer[0] == '\0'){
            printf("[Server][User: %s] Client could not send file.\n", usuario);
            fclose(handler);

            fseek (handler, 0, SEEK_END); //verifica qual o tamanho do arquivo
            int size = ftell(handler);
            if (-1 == size) {   
                remove(pathWithoutFilename); // se o arquivo nao existir, ele nao vai ser criado
            }   
            return; 
        }

    	fwrite(buffer, 1, bytesRecebidos, handler); // Escreve no arquivo

        bzero(buffer, TAM_MAX); // Reseta o buffer

    	if(bytesRecebidos < TAM_MAX){ // Se o pacote que veio, for menor que o tamanho total, eh porque o arquivo acabou
    		fclose(handler);
            printf("[Server][User: %s] Successfully received client file.\n", usuario);
            return SUCCESS;
    	}
    }
}