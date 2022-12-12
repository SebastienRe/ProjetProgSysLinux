#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int alarmTemp;

void fctALRM(int sig) {
    printf("Processus tué au bout de %d secondes car il n’est pas terminé.\n", alarmTemp);
    exit(1);
}

void AfficheArgv(char** arg){
    for (int i=0; arg[i] != NULL; i++)
        printf("Arg %d : %s\n", i, arg[i]);
}

int Executer(char **Commande){
    int pid, res, ret;
    pid = fork();
    if ( pid == 0 ) { // le fils
        execvp(Commande[0],Commande);
        exit(254);
    }
    else if (pid > 0){ // le père
        res = wait(&ret);
        printf("PERE: mon fils %d termine; exit :%d\n",res, WEXITSTATUS(ret));
        return WEXITSTATUS(ret);
    }
    else {
        fprintf(stderr,"PERE: pb, je n'ai pas pu creer mon fils\n");
        return 255;
    }
}

int main(int argc, char **argv){
    signal(SIGALRM,fctALRM);
    char** cmd = (char **)malloc(10 * sizeof(char *) * 1000);
    cmd = &argv[2];
    alarmTemp = atoi(argv[1]);
    alarm(alarmTemp);
    Executer(cmd);
    printf("Processus terminé\n");
    return 0;
}