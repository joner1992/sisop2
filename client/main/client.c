#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxClient.h"

int main(int argc, char *argv[]) {
    
  if(validateClientArguments(argc, argv) > 0)
    printf("SUCCESS");
  return 0;
}