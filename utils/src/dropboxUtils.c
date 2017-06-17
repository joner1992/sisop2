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

//FAZER!! tentar fazer de uma forma que sirva tanto pro client quanto para o server
int adicionaFilenameAoUser(char *userId){
    //adicionar um novo filename ao user
    return 0;
}

int removeFilenameAoUser(char *userId){
    //fazer remoção do filename do user
    return 0;
}

int atualizaFilenameAoUser(char *userId){
    //atualiza Size ao fazer download de um novo
    return 0;
}