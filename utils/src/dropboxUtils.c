#include "../include/dropboxUtils.h"

void createDirectory(char *argv, int server) {
    
    if(server) {
        char root[100] = "./clientDirectories/sync_dir_";
        strcat(root, argv);
        system(root);
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
