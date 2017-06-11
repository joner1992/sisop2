#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxUtils.h"
#include <stdio.h>

int BUFFER_TRANSFER = 32 * 1024;

int send_(int socket, FILE* file) {
   int size, read_size, stat, packet_index;
   char send_buffer[BUFFER_TRANSFER], read_buffer[256];
   packet_index = 1;

   printf("Getting file Size\n");   

   if(file == NULL) {
        printf("Error Opening Image File"); } 

   fseek(file, 0, SEEK_END);
   size = ftell(file);
   fseek(file, 0, SEEK_SET);
   printf("Total file size: %i\n",size);

   //Send Picture Size
   printf("Sending file Size\n");
   write(socket, (void *)&size, sizeof(int));

   //Send Picture as Byte Array
   printf("Sending file as Byte Array\n");

   do { //Read while we get errors that are due to signals.
      stat=read(socket, &read_buffer , 255);
      printf("Bytes read: %i\n",stat);
   } while (stat < 0);

   printf("Received data in socket\n");
   printf("Socket data: %c\n", read_buffer);

   while(!feof(file)) {
      read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, file);

      //Send data through our socket 
      do {
        stat = write(socket, send_buffer, read_size);  
      } while (stat < 0);

      printf("Packet Number: %i\n",packet_index);
      printf("Packet Size Sent: %i\n",read_size);     
      printf(" \n");
      printf(" \n");


      packet_index++;  

      //Zero out our send buffer
      bzero(send_buffer, sizeof(send_buffer));
     }
}

int receive_(int socket, FILE* file) { // Start function 

  int recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;
  
  char filearray[BUFFER_TRANSFER + 1];
  
  //Find the size of the file
  do{
  stat = read(socket, &size, sizeof(int));
  }while(stat<0);
  
  printf("Packet received.\n");
  printf("Packet size: %i\n",stat);
  printf("Image size: %i\n",size);
  printf(" \n");
  
  char buffer[] = "Got it";
  
  //Send our verification signal
  do{
    stat = write(socket, &buffer, sizeof(int));
  }while(stat<0);
  
  printf("Reply sent\n");
  printf(" \n");
  
  
  if( file == NULL) {
  printf("Error has occurred. Image file could not be opened\n");
  return -1; }
  
  //Loop while we have not received the entire file yet

  struct timeval timeout = {10,0};
  
  fd_set fds;
  int buffer_fd;
  
  while(recv_size < size) {
  //while(packet_index < 2){
  
    FD_ZERO(&fds);
    FD_SET(socket,&fds);
  
    buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);
  
    if (buffer_fd < 0)
       printf("error: bad file descriptor set.\n");
  
    if (buffer_fd == 0)
       printf("error: buffer read timeout expired.\n");
  
    if (buffer_fd > 0)
    {
        do{
               read_size = read(socket,filearray, BUFFER_TRANSFER + 1);
            }while(read_size <0);
  
            printf("Packet number received: %i\n",packet_index);
        printf("Packet size: %i\n",read_size);
  
  
        //Write the currently read data into our file file
         write_size = fwrite(filearray,1,read_size, file);
         printf("Written file size: %i\n",write_size); 
  
             if(read_size !=write_size) {
                 printf("error in read write\n");    }
  
  
             //Increment the total number of bytes read
             recv_size += read_size;
             packet_index++;
             printf("Total received file size: %i\n",recv_size);
             printf(" \n");
             printf(" \n");
    }
  
  }

  printf("Image successfully Received!\n");
  return 1;
}

