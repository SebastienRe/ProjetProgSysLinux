#define Q7

#include "lib.c"

int main(int argc, char **argv){
    #ifdef Q1
    Ls("dir");
    #endif
    #ifdef Q2
    LsDansFichier("dir", "log.txt");
    #endif
    #ifdef Q3
    NbFichier("dir");
    #endif
    #ifdef Q4
    int a= NbFichier("dir");
    printf("NbFichier: %d\n", a);
    #endif
    #ifdef Q5and6
    miniBash();
    #endif
    #ifdef Q7
    ExecFileBatchLimite(argv[1], argv[2]);
    #endif
    
    return 0;
}