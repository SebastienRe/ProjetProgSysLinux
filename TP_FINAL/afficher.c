
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

//C'est un programme qui permet d'afficher la chaine de caractere passée en parametre ainsi que le timestamp lié à l'execution
int main(int argc, char **argv){
    time_t temp;
    for (int i=0; i < atoi(argv[2]); i++){
        time(&temp);
        printf("%ld ",temp);
        printf("%s\n", argv[1]);
        sleep(1);
    }
}