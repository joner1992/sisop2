#include "../../utils/include/dropboxUtils.h"


/************************************ 
********* HEADER PARA SERVER ********
************************************/

// Estruturas estarao dispostas no servidor na forma de uma lista encadeada de clientes
struct	file_info	{
  char name[MAXNAME]; // refere-se ao nome do arquivo
  char extension[MAXNAME]; // refere-se ao tipo de extensao do arquivo
  char last_modified[MAXNAME]; // refere-se a data da ultima mofidicacao no arquivo
  int size; // tamanho do arquivo em bytes.
};

// struct	client	{
//   int devices[2]; // dispositivos de usuario
//   char userid[MAXNAME]; // id do usuario no servidor, que devera ser unico. Informado pela linha de comando
//   struct file_info[MAXFILES]; //metadados de cada arquivo que o cliente possui no servidor
//   int logged_in; // cliente esta logado ou nao
// };


/*  Sincroniza o servidor com o diretorio 
    “sync_dir_<nomeusuario>” com o cliente*/
void sync_server();

/*  Recebe um arquivo file do cliente.
    Deverá ser executada quando for realizar upload de um arquivo.
    file – path/filename.ext do arquivo a ser recebido */
void receive_file(char *file);

/*  Envia o arquivo file para o usuário.
    Deverá ser executada quando for realizar download de um arquivo.
    file – filename.ext */
void send_file(char *file);


/* ACIMA ESTÃO AS FUNCIONALIDADES MÍNIMAS PARA O SERVIDOR */

int validateServerArguments(int argc, char *argv[]);
