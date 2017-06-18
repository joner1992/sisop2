#include "../include/dropboxUtils.h"
int BUFFER_TRANSFER = 32 * 1024;
int DEBUG = 0;

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
int addFileToUser(char *name, char *extension, char *lastModified, int size, PFILA2 fileList){

    if(searchForFile(name, fileList) != SUCCESS) {
        UserFiles *newFile = (UserFiles *) malloc(sizeof(UserFiles));
        strcpy(newFile->name, name);
        strcpy(newFile->extension, extension);
        strcpy(newFile->last_modified, lastModified);
        newFile->size = size;
        if(AppendFila2(fileList, (void *) newFile) == LISTSUCCESS) {
            printf("File added to list correctly! \n");
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
        printf("File has been updated in list structure!\n");
        return SUCCESS;      
    }
    printf("ERROR on adding file to LIST");
    return ERROR;
}

int removeFileFromUser(char *fileName, PFILA2 fileList){
    //fazer remoção do filename do user
    if(searchForFile(fileName, fileList) == SUCCESS) {
        if(DeleteAtIteratorFila2(fileList) == LISTSUCCESS) {
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

int send_(int socket, char* filename) {
   int size, read_size, stat, packet_index;
   char send_buffer[BUFFER_TRANSFER], read_buffer[256];
   packet_index = 1;
   
   FILE *file = fopen(filename, "rb");

   if(file == NULL) {
        printf("Error - File not found or can't open"); 
        return -1;
   } 
    
    if(DEBUG) printf("\nSending %s\n", basename(filename));
    
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

int receive_(int socket, char path[255]) { // Start function 

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
  
  if(DEBUG)  printf("Receiveing %s", filename);
  
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

  if (stat(pathFile,&attributes)) {
        perror("ERROR Get attributes from file");
        exit(-1);
  }

  return attributes;
}