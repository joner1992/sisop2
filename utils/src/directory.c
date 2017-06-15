#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

int isRegularFile(struct dirent *file) {
  return file->d_type == DT_REG;
}

struct stat getAttributes(char* pathFile) {
  struct stat attributes;

  if (stat(pathFile,&attributes)) {
        perror("ERROR Get attributes from file");
        exit(-1);
  }

  return attributes;
}

void getFilesFromUser(char* userId) {
  char pathDirectory[255] = "./files/";
  char pathFile[255];
  struct dirent *file;
  struct stat fileAttributes;
  DIR* directory;


  sprintf(pathDirectory,"%s/%s/", pathDirectory, userId);

  directory = opendir(pathDirectory);

  if(directory) {
    while ((file = readdir(directory)) != NULL) {
      if(isRegularFile(file)) {
        sprintf(pathFile,"%s/%s",pathDirectory,file->d_name);
        fileAttributes = getAttributes(pathFile);

        printf("%s\n", file->d_name);
        printf("%ld\n", fileAttributes.st_mtime);
        printf("%lld\n", fileAttributes.st_size);
      }
    }
  }

}
