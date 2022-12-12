#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int mdpIsOk = 0;
#define MDP "leMdp\n"
#define MAX_CHAR_MDP 20

void ctrlCpressed(int sig){
    char buffer[MAX_CHAR_MDP];
    printf("<ctrl C>\n"
           "Taper le mot de passe : ");
    fgets( buffer, MAX_CHAR_MDP, stdin );
    if (strcmp(buffer,MDP) == 0)
        exit(1);
}


int main(int argc, char **argv){

    signal(SIGINT, ctrlCpressed);
    for (int i=0; i<atoi(argv[1]) ; i++){
        sleep(1);
        printf("%s\n",argv[2]);
    }

    return 0;
}