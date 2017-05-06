#include "../../utils/include/dropboxUtils.h"
#include "../include/dropboxServer.h"

int main(int argc, char *argv[]) {
    if(validateServerArguments(argc, argv) > 0)
        printf("SUCCESS");
    

    return 0;
}