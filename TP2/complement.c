#define Q1
#ifdef Q1
//code question1
#endif

#ifndef stdio
#include <stdio.h>
#endif

int ExecuteBatch(char** cmd){
    int pid, res, ret;
    pid = fork();
    if ( pid == 0 ) { // le fils
        execvp(cmd[0],cmd);
        exit(254);
    }
    else if (pid > 0){ // le père
        return 0;
    }
    else {
        fprintf(stderr,"PERE: pb, je n'ai pas pu creer mon fils\n");
        return 255;
    }
}