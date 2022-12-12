#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv){
    for (int i=0; i<atoi(argv[1]) ; i++){
        sleep(1);
        printf("%s\n",argv[2]);
    }
    return 0;
}