
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

/*
On va maintenant manipuler une structure contenant pour chaque commande :
    • son pid : numéro de processus dans lequel s’exécute la commande,
    • son statut :
        ◦ -1 pas encore exécuté,
        ◦ 0 terminé,
        ◦ 1 en exécution.
    • son retour (EXITSTATUS), même convention que dans Execute.
    • l'epoch à laquelle la commande a été lancée,
    • l'epoch à laquelle la commande s'est terminée,
    • son tableau Argv.
*/
typedef struct cmd {
    int pid ;
    int statut ;
    int retour ;
    time_t debut ;
    time_t fin ;
    char ** argv ;
}ENRCOMM ;

//Exercice 1
/* Qui transforme une chaîne de caractères contenant une commande en un tableau d'arguments de type
 * argv (voir premier cours)
 */#define MAX_ARG 100
char **Ligne2Argv(char* ligne){
    char **arg;
    char *li;
    char *p;
    int i = 0;
    li = strdup(ligne); // strtok peut modifier ligne, on le met dans li. strdup alloue et copie
    arg = (char **)malloc(MAX_ARG * sizeof(char *)); // allocation de MAX_ARG pointeurs
    p = strtok(li," \t\n");
    while (p != NULL){
        arg[i]=strdup(p);
        i++;
        p = strtok(NULL," \t\n");
    }
    arg[i]=NULL;
    return arg;
}

//Exercice 2
/* Affiche **char
 *
 */
void AfficheArgv(char** arg){
    for (int i=0; arg[i] != NULL; i++)
        printf("%s ", arg[i]);
}

//Exercice 3
/* transforme un tableau de type ARGV en chaîne de caractères.
 *
 */
char *Argv2Ligne(char** Commande){
    char *chaine = (char *)malloc(MAX_ARG * sizeof(char));
    for (int i=0; Commande[i] != NULL; i++){
        strcat(chaine, " ");
        strcat(chaine, Commande[i]);
    }
    return chaine;
}

//Exercice 4
/* Execute la commande donnée en param
 * remplis la structure
 */
int Executer(ENRCOMM *Commande){
    int pid, res, ret;
    time(&Commande->debut);
    Commande->statut = 1;//cmd en cours
    pid = fork();
    if ( pid == 0 ) { // le fils
        execvp(Commande->argv[0],Commande->argv);
        exit(254);
    }
    else if (pid > 0){ // le père
        Commande->pid = pid;
        res = wait(&ret);
        time(&Commande->fin);
        Commande->statut = 0;//cmd terminée
        Commande->retour = WEXITSTATUS(ret);
        printf("PERE: mon fils %d termine; exit :%d\n",res, WEXITSTATUS(ret));
        return WEXITSTATUS(ret);
    }
    else {
        fprintf(stderr,"PERE: pb, je n'ai pas pu creer mon fils\n");
        return 255;
    }
}

// Exercice 5
/* Crée un mini bash
 *
 */
void miniBash(){
    printf("Entrer Commande>");
    char * buffer = (char *) malloc( MAX_ARG );
    while(fgets( buffer, MAX_ARG, stdin )){
        //Executer(Ligne2Argv(buffer));
        printf("Entrer Commande>");
    }
    printf("<ctrlD>\nFIN\n");
}

#define TAILLE_MAX 1000 // Tableau de taille 1000
ENRCOMM* File2TabCom(char* fileName, int * nbCommandExcecuted){
    *nbCommandExcecuted = 0;
    ENRCOMM *TabCom = (ENRCOMM *)malloc(MAX_ARG * sizeof(ENRCOMM));
    char chaine[TAILLE_MAX] = "";

    FILE* fichier = NULL;
    fichier = fopen(fileName, "r");
    if (fichier != NULL){
        // On peut lire dans le fichier
        while (fgets(chaine, TAILLE_MAX, fichier) != NULL){ // On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL)
            TabCom[(*nbCommandExcecuted)].statut = -1;
            TabCom[(*nbCommandExcecuted)].argv = Ligne2Argv(chaine);
            *nbCommandExcecuted+=1;
        }
    }
    else{
        // On affiche un message d'erreur si on veut
        printf("Impossible d'ouvrir le fichier %s", fileName);
    }
    fclose(fichier); // On ferme le fichier qui a été ouvert
    return TabCom;
}

//Exercice 7
/* contient en argulent le nom du fichier contenant des commandes,
 * les stocke dans un tableau (File2TabArgv) puis les exécute une à une et affiche FIN une fois que
 * toutes les commandes sont exécutées.
 */
void afficheCom(ENRCOMM com){
    AfficheArgv(com.argv);
    printf(": %d %d %ld %ld %ld\n",com.pid, com.retour, com.debut, com.fin, com.fin - com.debut);
}

void ExecFile(char* fileName){
    ENRCOMM *TabCom ;
    time_t debut, fin;
    int Nbcomm;
    time(&debut);
    TabCom = File2TabCom(fileName, &Nbcomm) ;
    for(int i=0 ; i < Nbcomm ; i++){
        Executer(&(TabCom[i]));
    }
    printf("FIN\n");
    for(int i=0 ; i < Nbcomm ; i++){
        afficheCom(TabCom[i]);
    }
    time(&fin);
    printf("temps total : %ld\n",fin-debut);
}

//Exercice 8
/* qui reçoit un argument de type ARGV, l'exécute, mais n'attend qu'il se termine
 *
 */
 int ExecuteBatch(ENRCOMM *Commande){
    int pid, res, ret;
    time(&Commande->debut);
    Commande->statut = 1;//cmd en cours
    pid = fork();
    if ( pid == 0 ) { // le fils
        execvp(Commande->argv[0],Commande->argv);
        exit(254);
    }
    else if (pid > 0){ // le père
        Commande->pid = pid;
        return 0;
    }
    else {
        fprintf(stderr,"PERE: pb, je n'ai pas pu creer mon fils\n");
        return 255;
    }
}

/* Prends en arguments un nom de fichier contenant des commandes, 
 * les stocke dans un tableau (File2TabArgv), les exécute toutes en même temps
 * affiche FIN une fois que toutes les commandes sont exécutées.
 */
int ExecFileBatchLimite(char* fileName, char* N){
    int res, ret;
    int nbProcessusMax = atoi(N);
    int NbCommandes = 0;
    int numeroCommande = 0;
    int nbCommandesFinis = 0;
    time_t debut, fin, now;

    time(&debut);
    ENRCOMM* tabEnr=File2TabCom(fileName,&NbCommandes) ;

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
    ExecFileBatchLimite(argv[1], argv[2]);

return 0;
}