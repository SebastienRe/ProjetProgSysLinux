#define Q1
#ifdef Q1
//code question1
#endif

#ifndef stdio
#include <stdio.h>
#endif
#include "../TP1/TP1B.c"

//TP2
int NbCommandes = 0;
ENRCOMM* tabEnr; //tableau de commandes

void fctAlarm(int sig){
    alarm(1);
    printf("DEBUT RAPPORT\n");
    for(int i=0 ; i < NbCommandes ; i++){
        afficheCom(tabEnr[i]);
    }
    printf("FIN\n");
}

/* Prends en arguments un nom de fichier contenant des commandes, 
 * les stocke dans un tableau (File2TabArgv), les exécute toutes en même temps
 * affiche FIN une fois que toutes les commandes sont exécutées.
 */
int ExecFileBatchLimiteB(char* fileName, char* N){
    int res, ret;
    int nbProcessusMax = atoi(N);
    int numeroCommande = 0;
    int nbCommandesFinis = 0;
    time_t debut, fin, now;

    time(&debut);
    tabEnr=File2TabCom(fileName,&NbCommandes) ;
    signal(SIGALRM, fctAlarm);
    alarm(1);
    while (nbCommandesFinis < NbCommandes){
        if ( numeroCommande < (nbProcessusMax + nbCommandesFinis) && numeroCommande != NbCommandes ) {
            while(numeroCommande < (nbProcessusMax + nbCommandesFinis) ){
                printf("lancement de");
                for(int j=0 ; tabEnr[numeroCommande].argv[j] != NULL ; j++)
                    printf(" %s",tabEnr[numeroCommande].argv[j]);
                time(&now);
                printf(" à %ld\n",now);
                ExecuteBatch(&tabEnr[numeroCommande]);
                numeroCommande++;
            }
        }
        else{
            res = wait(&ret);
            int i=0;
            while (res != tabEnr[i].pid)//recherche du bon ENRCOMM dans le tableau d'ENRCOMM 
                i++;
            time(&tabEnr[i].fin);
            tabEnr[i].statut = 0;//cmd terminée
            tabEnr[i].retour = WEXITSTATUS(ret);
            afficheCom(tabEnr[i]);
            nbCommandesFinis += 1;
        }
    }
    
    time(&fin);
    printf("FIN\n");
    for(int i=0 ; i < NbCommandes ; i++){
        afficheCom(tabEnr[i]);
    }
    printf("temps total : %ld\n",fin-debut);
}

int main(int argc, char **argv)
{
    ExecFileBatchLimiteB(argv[1], argv[2]);

return 0;
}