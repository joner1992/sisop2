#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxClient.h"
#include <stdio.h>

int send_(int socket, FILE* file) {
   int size, read_size, stat, packet_index;
   char send_buffer[10240], read_buffer[256];
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
