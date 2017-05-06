

/************************************ 
********* HEADER PARA SERVER ********
************************************/

/*  Sincroniza o servidor com o diretório 
    “sync_dir_<nomeusuário>” com o cliente*/
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
