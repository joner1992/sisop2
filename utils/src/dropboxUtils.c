#include "../include/dropboxUtils.h"
int BUFFER_TRANSFER = 32 * 1024;
int DEBUG = 0;

void createDirectory(char *userId, int isServer) {
    
    if(isServer) {
        char root[100] = "./clientsDirectories/sync_dir_";
        strcat(root, userId);
        mkdir(root, 0777);
        printf("Directory ./sync_dir_%s created successfully.\n", userId);
    } else {
        char home[100] = "/home/";
        char usr[50];
        if(!getlogin_r(usr, 50)) {
            strcat(home, usr);
            strcat(home, "/sync_dir_");
            strcat(home, userId);
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

int send_(int socket, char* filename) {
   int size, read_size, stat, packet_index;
   char send_buffer[BUFFER_TRANSFER], read_buffer[256];
   packet_index = 1;
   
   FILE *file = fopen(filename, "rb");

   if(file == NULL) {
        printf("Error - File not found or can't open"); 
        return -1;
   } 
    
    if(DEBUG) printf("[Debug] Sending %s\n", basename(filename));
    if(DEBUG) printf("[Debug] completePath: %s\n", filename);
    
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
      stat=read(socket, &read_buffer , 255);
       if(DEBUG) printf("Bytes read: %i\n",stat);
   } while (stat < 0);

    if(DEBUG) printf("Received data in socket\n");
    if(DEBUG) printf("Socket data: %c\n", read_buffer);

   while(!feof(file)) {
      read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, file);

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

int receive_(int socket, char path[255]) {

  int recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;
  char filearray[BUFFER_TRANSFER + 1];
  char fullPath[255];
  char filename[255];
  
  bzero(fullPath, 255);
  bzero(filename, 255);
  
  FILE *file;
  
  do {
    stat = read(socket, filename, sizeof(filename));
  } while(stat<0);
  
  if(DEBUG) printf("[Debug] Receiveing %s\n", filename);
  if(DEBUG) printf("[Debug] Location to save: %s\n", path);
  
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

char *removeFileNameFromPath(char *path){
    char fileName[BUFFERSIZE];
    char command[BUFFERSIZE];
    char *forIterator;
    char *subString;

    bzero(command, BUFFERSIZE);

    for (forIterator = strtok_r(path,"/", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "/", &subString)){
        strcpy(fileName, forIterator);
    }

    return fileName;
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

struct chain_list* chain_create_list() {
  struct chain_list* list = malloc(sizeof(chain_list));
  list->size = 0;
  list->header = NULL;

  if(pthread_mutex_init(&list->lock, NULL) != 0) {
    printf("\n Mutex init failed\n");
    free(list);
    return NULL;
  }
  return list;
}

struct chain_node* chain_create_client_node(char* userId) {
  struct chain_node* node = malloc(sizeof(chain_node));
  clientInfo* client = malloc(sizeof(clientInfo));

  if(pthread_mutex_init(&client->loginMutex, NULL) != 0) {
        printf("\nCreate Client Node Mutex init failed\n");
        free(client);
        free(node);
        return NULL;
  }

  strcpy(client->userId, userId);
  node->client = client;

  node->file = NULL;
  node->next = NULL;
  return node;
}

struct chain_node* chain_create_file_node(char* name, char* lastModified, int size) {
  struct chain_node* node = malloc(sizeof(chain_node));
  userFile* file = malloc(sizeof(userFile));

  if(pthread_mutex_init(&file->fileMutex, NULL) != 0) {
        printf("\nCreate Client Node Mutex init failed\n");
        free(file);
        free(node);
        return NULL;
  }

  strcpy(file->name, name);
  strcpy(file->lastModified, lastModified);
  file->size = size;

  node->file = file;

  node->client = NULL;
  node->next = NULL;
  return node;
}

int chain_add(chain_list* list, chain_node* node) {
  if(list != NULL) {
    if(list->header != NULL) {
      if(node->client && list->header->file != NULL) {
          printf("ADD ERROR: Incompatible list, node client trying to add on file list\n");
          return -1;

      } else if(node->file && list->header->client != NULL) {
          printf("ADD ERROR: Incompatible list, node file trying to add on client list\n");
          return -1;
      }
    }
    pthread_mutex_lock(&list->lock);
    node->next = list->header;
    list->header = node;
    list->size++;
    pthread_mutex_unlock(&list->lock);
    return 1;
  }
    printf("ADD ERROR: List not initalized\n");
    return -1;
}

int chain_remove(chain_list* list, char* chainname) {
  if(list == NULL) {
    printf("REMOVE ERROR: List not initialized\n");
    return -1;
  }
  pthread_mutex_lock(&list->lock);

  chain_node* previous = NULL;
  chain_node* node = list->header;

  if (node == NULL) {
    pthread_mutex_unlock(&list->lock);
    return 1;
  }

  if(node->client) {
    while (node!=NULL && strcmp(node->client->userId,chainname)!=0) {
      previous = node;
      node = node->next;
    }
  } else if (node->file) {
    while (node!=NULL && strcmp(node->file->name,chainname)!=0) {
      previous = node;
      node = node->next;
    }
 }

 if (node == NULL) {
   pthread_mutex_unlock(&list->lock);
   return 1;
 }

 list->size--;

 if (previous == NULL) { //Retira o primeiro elemento
    list->header = node->next;
 } else { //Retira do meio
    previous->next = node->next;
 }

  if(node->client != NULL) free(node->client);
  if(node->file != NULL) free(node->file);
  free(node);
  pthread_mutex_unlock(&list->lock);
  return 1;
}

struct chain_node* chain_find(chain_list* list, char *chainname) {
  if(list == NULL){
    printf("[Cross] ERROR chain_find list not initialized\n");
    return NULL;
  }
  pthread_mutex_lock(&list->lock);
  chain_node* node = list->header;

  if(node == NULL){
    pthread_mutex_unlock(&list->lock);
    return NULL;
  }

  if(node->client) {
    while (node!=NULL && strcmp(node->client->userId,chainname)!=0) {
        node = node->next;
    }
  } else if(node->file) {
    while (node!=NULL && strcmp(node->file->name,chainname)!=0) {
        node = node->next;
    }
  }

  pthread_mutex_unlock(&list->lock);
  return node;
}

int chain_clear(chain_list* list) {
    if(list == NULL) {
      printf("CLEAR ERROR: List not initalized\n");
      return -1;
    }

    pthread_mutex_lock(&list->lock);
    chain_node* node = list->header;

    while (node != NULL) {
      chain_node* next = node->next;

      free(node);
      node = next;
    }

    list->size = 0;
    list->header = NULL;
    pthread_mutex_unlock(&list->lock);
    return 1;
}

int chain_print(chain_list* list) {
  if(list == NULL) {
    printf("PRINT ERROR: List not initalized\n");
    return -1;
  }
  
  pthread_mutex_lock(&list->lock);
  printf("\n---------------------------\n");

  if(list->header == NULL) printf("Empty List\n");
  
  chain_node* node = list->header;
  if(node->client) {
    while(node != NULL) {
      printf("[Node Client] UserId: %s\n", node->client->userId);
      node = node->next;
    }
  }
  
  if (node->file) {
    while(node != NULL) {
      printf("[Node File] Name: %s  LastModified: %s Size: %d\n", node->file->name, node->file->lastModified, node->file->size);
      node = node->next;
    }
  }
  printf("\n---------------------------\n");
  pthread_mutex_unlock(&list->lock);
  
  return 1;
}

void updateLocalTime(char *buffer) {
  time_t rawtime;
	struct tm * timeinfo;

	time (&rawtime);
	timeinfo = localtime (&rawtime);

	sprintf(buffer, "%d%0.2d%0.2d%d%d%d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	return 0;
}

void addFilesToFileList(chain_list* list, char* name, char* lastModified, int size) {
  if(list == NULL) {
    printf("REMOVE ERROR: List not initialized\n");
    return -1;
  }
  
  chain_node* node = chain_find(list, name);
  
  if(node) {
    strcpy(node->file->lastModified, lastModified);
    node->file->size = size;
  } else {
    node = chain_create_file_node(name, lastModified, size);
    chain_add(list, node);  
  }
}

char* fileListToArray(chain_list* list) {
  if(list == NULL) {
    printf("PRINT ERROR: List not initalized\n");
    return -1;
  }

  pthread_mutex_lock(&list->lock);

  char array[BUFFERSIZE];
  bzero(array, BUFFERSIZE);
  sprintf(array,"#");
  chain_node* node = list->header;

  //if empty return '##'
  if(node == NULL) {
    sprintf(array,"%s#", array);
    pthread_mutex_unlock(&list->lock);
    return array;
  }

  //interate over list concating list
  while(node != NULL) {
    if(node->file == NULL) {
      printf("[fileListToArray] ERROR: node->file == NULL\n");
      exit(1);
    }
    sprintf(array,"%s%s#%s#", array, node->file->name, node->file->lastModified);
    node = node->next;
  }

  pthread_mutex_unlock(&list->lock);

  return array;
}

void getFilesFromUser(char* userId, chain_list* list, int isServer, char *lastModification) {
  char root[255] = "./clientsDirectories/sync_dir_";
  char pathDirectory[255];
  char pathFile[255];
  struct dirent *file;
  struct stat fileAttributes;
  DIR* directory;
    
  bzero(pathDirectory, 255);
  bzero(pathFile, 255);

  if(isServer) {
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
        
        addFilesToFileList(list, file->d_name, lastModification, fileAttributes.st_size);
      }
    }
  }
}

void removeFileFromSystem(char *userId, int isServer) {
  // These are data types defined in the "dirent" header
  char path[255];
  bzero(path, 255);
  if(isServer) {
    sprintf(path, "./clientsDirectories/sync_dir_%s/", userId);
  } else {
    strcpy(path, getUserDirectory(userId));
  }

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

void sendServerFiles(int socket, char *buffer, char *path) {
  //read name of the operation and name file
  char serverFileInformations[BUFFERSIZE];
  char fileName[BUFFERSIZE];
  char fileModificationDate[BUFFERSIZE];
  char *forIterator;
  char *subString;
  char completePath[255];
  int numCommands = 1;

  bzero(serverFileInformations, BUFFERSIZE);
  strcpy(serverFileInformations, buffer);

  for (forIterator = strtok_r(serverFileInformations,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)) {
    bzero(completePath, 255);
    strcpy(completePath, path);   

    if ((numCommands%2) != 0){
      bzero(fileName, BUFFERSIZE);
      strcpy(fileName, forIterator);
    }
    else if ((numCommands%2) == 0){
      bzero(fileModificationDate, BUFFERSIZE);
      strcpy(fileModificationDate, forIterator);
      
      strcat(completePath, fileName);
      send_(socket, completePath);
    }
    numCommands++;
  }
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