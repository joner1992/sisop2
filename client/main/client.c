#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxClient.h"

int sockfd, n, server_port;
char userId[MAXNAME];
struct sockaddr_in serv_addr;
struct hostent *server;

int aux_sockfd, m;
struct sockaddr_in aux_serv_addr;
struct hostent *aux_server;

char lastModification[BUFFERSIZE] = "0";
pthread_mutex_t clientFileListMutex;

int disconnectSync = 0;
FILA2 fileList;

FILE *file;

void setHost(char *argv) {
  server = gethostbyname(argv);
  
  if (server == NULL) {
    perror("ERROR, no such host\n");
    exit(ERROR);
  }
  
}

void setAuxHost(char *argv){
  aux_server = gethostbyname(argv);
  
  if (aux_server == NULL) {
    perror("ERROR, no such host for aux_socket");
    exit(ERROR);
  }
}

int getPort(char *argv) {
  char *endptr;
  int port = strtoimax(argv, &endptr,10);
  if(port <= 0) {
    return ERROR;
  }
  return port;

}

void setUserId(char *argv){
  if(strlen(argv) > MAXNAME) {
    perror("ERROR username exceeds maximum length");
    exit(ERROR);
  }
  strcpy(userId, argv);
}

void createSocket() {
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
      perror("ERROR opening socket\n");
      exit(ERROR);
    } 

}

void createAuxSocket(){
  if((aux_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) {
    perror("ERROR opening aux socket\n");
    exit(ERROR);
  }
}

int connectSocket() {
    char buffer[BUFFERSIZE];
    int n;
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_port = htons(server_port);    
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8); 

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
      perror("ERROR connecting to server\n");
      exit(ERROR);
    }
    bzero(buffer, BUFFERSIZE);
    strcpy(buffer, userId);
    n = write(sockfd, buffer, BUFFERSIZE);
    if (n == ERROR) {
      perror("ERROR writing to socket\n");
      exit(ERROR);
    }
    while(1){
      //verificar aqui porque não ta respondendo
      bzero(buffer, BUFFERSIZE);
      n = read(sockfd, buffer, BUFFERSIZE);
      if (n == ERROR) {
        perror("ERROR writing to socket\n");
        exit(ERROR);
      }

      if(strcmp(buffer, "OK") == 0){
        createDirectory(userId, 0);
        return SUCCESS;
      } else if(strcmp(buffer, "NOTOK") == 0) {
        printf("Maximum devices connections reached :(\n");
        exit(ERROR);
        return ERROR;
      }
    }
}

void connectAuxSocket() {
    char buffer[BUFFERSIZE];
    int n;
    aux_serv_addr.sin_family = AF_INET;  
    aux_serv_addr.sin_port = htons(server_port);    
    aux_serv_addr.sin_addr = *((struct in_addr *)aux_server->h_addr);
    bzero(&(serv_addr.sin_zero), 8); 

    if (connect(aux_sockfd,(struct sockaddr *) &aux_serv_addr,sizeof(aux_serv_addr)) < 0) 
    {
      perror("ERROR connecting to server\n");
      exit(ERROR);
    }
    
    bzero(buffer, BUFFERSIZE);
    strcat(buffer, "aux_");
    strcat(buffer, userId);
    
    n = write(aux_sockfd, buffer, BUFFERSIZE);
    if (n == ERROR) {
      perror("ERROR writing to socket\n");
      exit(ERROR);
    }
}

char *adaptEntry(char *cmd) {
  char aux[BUFFERSIZE];
  strcpy(aux, cmd);
  int i;
  for(i = 0; i<sizeof(aux); i++) {
    if(aux[i] == ' ' || aux[i] == '\n'){
      aux[i] = '#';
    }
  }
  return aux;
}

void *syncSocket() {
  int n;
  char buffer[BUFFERSIZE];
  char operation[BUFFERSIZE];
  char fileName[BUFFERSIZE];
  char completePath[BUFFERSIZE];
  char lastModified[BUFFERSIZE];
  char *forIterator;
  char *subString;
  int numCommands;
  int ignoreUpdate;

  while(1){    
    if(disconnectSync == 1) {
      printf("DISCONNECTING SYNCSOCKET\n");
      close(sockfd);
      pthread_exit(NULL);
    }

    printf("TENTANDO RECEBER TEST\n");
    //recebe TEST para ver se a thread existe
    while(1) {
      bzero(buffer, BUFFERSIZE);
      n = read(sockfd, buffer, BUFFERSIZE);
      if (n == ERROR) {
        perror("ERROR reading from socket\n");
        exit(ERROR);
      } else if(n > 0){
        break;
      }      
    }

    printf("COMEÇOU SYNC\n");

    bzero(buffer, BUFFERSIZE);
    strcpy(buffer, getListFilesFromUser(buffer, &fileList, CLIENT));

    numCommands = 1;
    for (forIterator = strtok_r(buffer,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)) {
      if(numCommands %2 != 0){
        //limpa lista atual de arquivos
        cleanList(&fileList, forIterator);
      }
      numCommands++;
    }

    printf("LIMPOU LISTA\n");
      
    
    //adiciona novos arquivos
    getFilesFromUser(userId, &fileList, CLIENT);
    printf("ADICIONOU ARQUIVOS NA LISTA\n");

    //envia requisição de data
    bzero(buffer, BUFFERSIZE);
    strcpy(buffer, "sendFileListDate");
    n = write(sockfd, buffer, BUFFERSIZE);
    if (n == ERROR) {
      perror("ERROR reading from socket\n");
      exit(ERROR);
    }

    //recebe a data do servidor
    while(1) {
      bzero(buffer, BUFFERSIZE);
      n = read(sockfd, buffer, BUFFERSIZE);
      if (n == ERROR) {
        perror("ERROR reading from socket\n");
        exit(ERROR);
      } else if(n > 0){
        break;
      }      
    }

    printf("RECEBEU A DATA: %s\n", buffer);

    if(strcmp(lastModification, buffer) > 0) {
      bzero(buffer, BUFFERSIZE);
      strcpy(buffer, "Client#");
      strcat(buffer, lastModification);
      printf("last modification depois de client#: %s\n", lastModification);
      n = write(sockfd, buffer, BUFFERSIZE);
      if (n == ERROR) {
        perror("ERROR reading from socket\n");
        exit(ERROR);
      }
      ignoreUpdate = 0;
    } else if(strcmp(lastModification, buffer) < 0) {
      bzero(lastModification, BUFFERSIZE);
      strcpy(lastModification, buffer);
      printf("SETOU DATA DO CLIENT COMO: %s\n", lastModification);
      bzero(buffer, BUFFERSIZE);
      strcpy(buffer, "Server#");
      n = write(sockfd, buffer, BUFFERSIZE);
      if (n == ERROR) {
        perror("ERROR reading from socket\n");
        exit(ERROR);
      }
      ignoreUpdate = 0;
    } else if(strcmp(lastModification, buffer) == 0){
      printf("DATA IGUAL NAO FACA NADA!\n");
      bzero(buffer, BUFFERSIZE);
      strcpy(buffer, "nada");
      n = write(sockfd, buffer, BUFFERSIZE);
      if (n == ERROR) {
        perror("ERROR reading from socket\n");
        exit(ERROR);
      }
      ignoreUpdate = 1;
    }

    printf("IGNORE UPDATE: %d\n", ignoreUpdate);
    if(ignoreUpdate != 1){

      printf("SYNC: %s\n", buffer);

      //envia lista para server
      bzero(buffer, BUFFERSIZE);
      strcpy(buffer, getListFilesFromUser(buffer, &fileList, CLIENT));
      n = write(sockfd, buffer, BUFFERSIZE);
      if (n == ERROR) {
        perror("ERROR reading from socket\n");
        exit(ERROR);
      }

      printf("ENVIA LISTA PARA SERVER: %s\n", buffer);

      //le operacao#nomearquivo#operacao#nomearquivo#operacao#nomearquivo#operacao#nomearquivo#
      while(1){
        bzero(buffer, BUFFERSIZE);
        n = read(sockfd, buffer, BUFFERSIZE);
        if (n == ERROR) {
          perror("ERROR reading from socket\n");
          exit(ERROR);
        } else if(n > 0){
          //recebeu a data no buffer
          break;
        }
      }

      printf("RECEBEU OPERACAO#ARQUIVO: %s\n", buffer);

      if(strcmp(buffer, "#") != 0){
        numCommands = 1;
        for (forIterator = strtok_r(buffer,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)) {
          if (numCommands % 2 != 0) { //impar
            bzero(operation, BUFFERSIZE);
            strcpy(operation, forIterator);
            printf("OPERATION: %s\n", operation);
          }
          else if (numCommands % 2 == 0) { //par
            bzero(fileName, BUFFERSIZE);
            strcpy(fileName, forIterator);
            printf("FILENAME: %s\n", fileName);

            //EXECUTA OPERAÇÃO RECEBIDA EM RELAÇÃO AO SERVER
            if(strcmp(operation, "delete") == 0){
              bzero(completePath, BUFFERSIZE);
              strcat(completePath, getUserDirectory(userId));
              strcat(completePath, fileName);

              removeFileFromSystem(completePath);
              removeFileFromUser(fileName, &fileList, userId, CLIENT);
            } else if(strcmp(operation, "download") == 0) {
              printf("DOWNLOAD (getUserDirectory): %s\n", getUserDirectory(userId));

              bzero(buffer, BUFFERSIZE);
              n = write(sockfd, buffer, BUFFERSIZE);
              if (n == ERROR) {
                perror("ERROR writing to socket\n");
                exit(ERROR);
              }

              receive_(sockfd, getUserDirectory(userId));
              printf("RECEBEU: %s\n", fileName);

              bzero(completePath, BUFFERSIZE);
              strcat(completePath, getUserDirectory(userId));
              strcat(completePath, fileName);

              printf("ENVIANDO COMPLETEPATH PARA GETATTRIBUTES: %s\n", completePath);

              struct stat file_stat = getAttributes(completePath);
              bzero(lastModified, 36);
              strftime(lastModified, 36, "%Y.%m.%d %H:%M:%S", localtime(&file_stat.st_mtime));
              printf("TERMINOU GETATTRIBUTES(lastModified): %s\n", lastModified);
              printf("TERMINOU GETATTRIBUTES(FILESTAT STSIZE): %d\n", file_stat.st_size);

              addFileToUser(fileName, ".txt", lastModified, file_stat.st_size, &fileList);

              printf("ADDFILES DONE: %s\n", lastModified);
            } else if(strcmp(operation, "upload") == 0) {
              bzero(completePath, BUFFERSIZE);
              strcat(completePath, getUserDirectory(userId));
              strcat(completePath, fileName);   
              printf("UPLOAD (COMPLETEPATH): %s\n", completePath);
              send_(sockfd, completePath);
            }
          }
          numCommands++;
        }
        printf("SAIU DO FOR\n");
      }
    }
    sleep(10);    
  }
}

void *auxSocketFunctions() {
  char buffer[BUFFERSIZE];
  char bufferForServer[BUFFERSIZE];
  char fileName[BUFFERSIZE];
  char completePath[255];
  char command[BUFFERSIZE];
  char *forIterator;
  char *subString;
  char lastModified[36];
  int numCommands;

  while(1) {
    bzero(buffer, BUFFERSIZE);
    bzero(command, BUFFERSIZE);
    bzero(fileName, BUFFERSIZE);
    bzero(bufferForServer, BUFFERSIZE);
    bzero(completePath, BUFFERSIZE);
    bzero(lastModified, 36);
    numCommands = 0;

    int i = 0;

    printf(">> ");
    fgets(buffer, BUFFERSIZE, stdin);
    strcpy(buffer, adaptEntry(buffer));
    strcpy(bufferForServer, buffer);
    // while (buffer[i]!='#' && i < 8) {
    //     cmd[i] = buffer[i];
    //     i++;
    // }

    for (forIterator = strtok_r(buffer,"#", &subString); forIterator != NULL; forIterator = strtok_r(NULL, "#", &subString)){
      if (numCommands == 0){
			  strcpy(command, forIterator);
		  }
		  else if (numCommands == 1){
			  strcpy(fileName, forIterator);
		  }
		  numCommands++;
    }

    printf("COMMAND: %s FILENAME: %s\n", command, fileName);

    strcpy(completePath, getUserDirectory(userId));

    if(strcmp(command, "upload") == 0) {
      
      //UPLOAD BEGIN
      //Enviando o comando para o servidor
      n = write(aux_sockfd, bufferForServer, BUFFERSIZE); 
      if (n == ERROR) {
        perror("ERROR writing to socket\n");
        exit(ERROR);
      }
      
      //Recebendo o nome do arquivo
      bzero(buffer, BUFFERSIZE);
      n = read(aux_sockfd, buffer, BUFFERSIZE);
      if (n == ERROR) {
          perror("ERROR read from socket\n");
          exit(ERROR);
      }
      /*
        CONVERSAR SOBRE, O UPLOAD TA SUPONDO QUE O ARQUIVO SEMPRE VAI ESTAR LOCALMENTE NO CLIENT NAO?
        PQ O BUFFER TA VINDO ./FILES/IN/NOMEDOARQUIVO.EXTENSAO
      */
      //path completo do arquivo no buffer
      printf("ENTRANDO NO SEND_(UPLOAD): %s\n", buffer);
      send_(aux_sockfd, buffer);

    } else if (strcmp(command, "download") == 0) {
      //Enviando o comando para o servidor
      n = write(aux_sockfd, bufferForServer, BUFFERSIZE); 
      if (n == ERROR) {
        perror("ERROR writing to socket\n");
        exit(ERROR);
      }
      //Pasta de destino
      //nesse caso completePath é realmente o caminho do arquivo
      printf("ENTRANDO NO RECEIVE_(DOWNLOAD): %s\n", completePath);
      // char path[255]= "./clientsDirectories/sync_dir_";
      //   sprintf(path,"%s%s/%s",path, newAuxThread->userId, fileName);
      if(receive_(aux_sockfd, completePath) == SUCCESS) {
        //aqui o completePath está sendo concatenado com o fileName
        strcat(completePath, fileName);
        struct stat file_stat = getAttributes(completePath);
        strftime(lastModified, 36, "%Y.%m.%d %H:%M:%S", localtime(&file_stat.st_mtime));
        pthread_mutex_lock(&clientFileListMutex);
          addFileToUser(basename(buffer), ".txt", lastModified, file_stat.st_size, &fileList);
        pthread_mutex_unlock(&clientFileListMutex);

        bzero(lastModification, BUFFERSIZE);
        updateLocalTime(lastModification);
      }
    } else if(strcmp(command, "exit") == 0) {
      n = write(aux_sockfd, bufferForServer, BUFFERSIZE);
      if (n == ERROR) {
        perror("ERROR writing to socket\n");
        exit(ERROR);
      }

      printf("exiting\n");
      disconnectSync = 1;
      close(aux_sockfd);
      pthread_exit(NULL);
    }
    else if(strcmp(command, "list") == 0) {
      n = write(aux_sockfd, bufferForServer, BUFFERSIZE);
      if (n == ERROR) {
        perror("ERROR writing to socket\n");
        exit(ERROR);
      }
      while(1){
        bzero(buffer, BUFFERSIZE);
        strcpy(buffer, receiveMessage(aux_sockfd, "exit", 1));

        if(strcmp(buffer, "exit") == 0){
          break;
        }        
      }  
    }
  }
}

int main(int argc, char *argv[]) {

  if(validateClientArguments(argc, argv) != ERROR) {
    initializeList(&fileList);
    
    setHost(argv[2]);
    setUserId(argv[6]);
    createSocket();
    
    setAuxHost(argv[2]);
    setAuxHost(argv[2]);
    createAuxSocket();

    if((server_port = getPort(argv[4])) == ERROR ) {
      perror("ERROR bad port configuration\n");
      exit(ERROR);
    }
    
    if(connectSocket() == SUCCESS) {
      //Preenche lista de arquivos com informações de diretório da home
      getFilesFromUser(userId, &fileList, CLIENT);

      //cria socket sync
      pthread_t syncSocketThread;
      pthread_attr_t attributesSyncSocketThread;
      pthread_attr_init(&attributesSyncSocketThread);
      pthread_create(&syncSocketThread,&attributesSyncSocketThread,syncSocket,NULL);    

      connectAuxSocket();
      
      //AUX socket para comandos, upload e download
      pthread_t auxSocketThread;
      pthread_attr_t attributesAuxSocketThread;
      pthread_attr_init(&attributesAuxSocketThread);
      pthread_create(&auxSocketThread,&attributesAuxSocketThread,auxSocketFunctions,NULL);

      pthread_join(syncSocketThread, NULL);
      pthread_join(auxSocketThread, NULL);

    } else {
      printf("Max connections reached!\n");
    }


    //Normal thread para sync

    //pthread_t readWriteThread;
    //pthread_attr_t attributesReadWriteThread;
    //pthread_attr_init(&attributesReadWriteThread);
    //pthread_create(&readWriteThread,&attributesReadWriteThread,readWriteUser,NULL);
            
    //close(sockfd);

  } else {
    return ERROR;
  }

  return 0;
}
   
    