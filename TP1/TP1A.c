
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

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
    p = strtok(li," \t\n");//on coupe la ligne en morceaux
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
        printf("Arg %d : %s\n", i, arg[i]);
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


void AfficheArgv(char** arg){
    for (int i=0; arg[i] != NULL; i++)
        printf("Arg %d : %s\n", i, arg[i]);
}

int Executer (char** Commande, char* redirection, char** suite){
    int pid1, pid2, res, ret, file, t[2];
    pipe(t);
    pid1 = fork();
    if ( pid1 == 0 ) { // le fils
        if (redirection == ">"){
            file = open(suite[0], O_WRONLY | O_CREAT | O_TRUNC, 0666);
            close(1);//ferme stdout
            dup(file);//redirige stdout vers file
        } else if (redirection == ">>"){
            file = open(suite[0], O_WRONLY | O_CREAT | O_APPEND, 0666);
            close(1);//ferme stdout
            dup(file);//redirige stdout vers file
        } else if (redirection == "<"){
            file = open(suite[0], O_RDONLY);
            close(0);//ferme stdin
            dup(file);//redirige stdin vers file
        } else if (redirection == "|"){
            close(1); dup(t[1]); // redirection de stdout vers t1
            close(t[1]); close(t[0]); //fermeture de t1
        }
        execvp(Commande[0],Commande);
        exit(254);
    }
    else if (pid1 > 0){ // le père
        if (redirection == "|"){
            pid2=fork();//fork du wc
            if (pid2 == 0) { //fils du 2ème fork
                close(0); dup(t[0]); close(t[1]); close(t[0]); // redirection de stdin vers t
                execvp(suite[0],suite);
                exit(254);} //pid2==0
            else if (pid2 > 0){ //père du 2ème fork
                close(t[1]); close(t[0]); //fermeture de t
                ret=wait(&res);//attente de la fin du fils
            } //pid2>0
            else { //pid2<0, donc erreur
                close(t[1]); close(t[0]); //fermeture de t
                kill(pid1,SIGKILL); // kill cmd
                ret=wait(&res);
                return 252;// pid2<0
            }
        }
        res = wait(&ret);//attente de la fin du fils
        //printf("PERE: mon fils %d termine; exit :%d\n",res, WEXITSTATUS(ret));
        return WEXITSTATUS(ret);
    }
    else { // pid1 < 0, donc erreur
        return 252;
    }
}

void analyse(char ***cmd, char **redirection, char ***suite){
    int i = 0;
    while ((*cmd)[i] != NULL){
        if (strcmp((*cmd)[i], ">") == 0){
            *redirection = ">";
            (*cmd)[i] = NULL;
            *suite = &(*cmd)[i+1];
        } else if (strcmp((*cmd)[i], "<") == 0){
            *redirection = "<";
            (*cmd)[i] = NULL;
            *suite = &(*cmd)[i+1];
        } else if (strcmp((*cmd)[i], ">>") == 0){
            *redirection = ">>";
            (*cmd)[i] = NULL;
            *suite = &(*cmd)[i+1];
        } else if (strcmp((*cmd)[i], "|") == 0){
            *redirection = "|";
            (*cmd)[i] = NULL;
            *suite = &(*cmd)[i+1];
        }
        i++;
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
        Executer(Ligne2Argv(buffer));
        printf("Entrer Commande>");
    }
    printf("<ctrlD>\nFIN\n");
}

//Exercice 6
/* qui lit des commandes dans un fichier, les transforme en Argv et les ranges toutes dans un
 * tableau  qu'il retourne. Il retourne en plus dans le 2ème argument le nombre de commandes lues.
 */#define TAILLE_MAX 1000 // Tableau de taille 1000
char *** File2TabArgv(char* fileName, int * nbCommandExcecuted){
    *nbCommandExcecuted = 0;
    char ***tabCmd = (char ***)malloc(MAX_ARG * sizeof(char *) * TAILLE_MAX);
    char chaine[TAILLE_MAX] = "";

    FILE* fichier = NULL;
    fichier = fopen(fileName, "r");
    if (fichier != NULL){
        // On peut lire dans le fichier
        while (fgets(chaine, TAILLE_MAX, fichier) != NULL){ // On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL)
            tabCmd[(*nbCommandExcecuted)++] = Ligne2Argv(chaine);
        }
    }
    else{
        // On affiche un message d'erreur si on veut
        printf("Impossible d'ouvrir le fichier %s", fileName);
    }
    fclose(fichier); // On ferme le fichier qui a été ouvert
    return tabCmd;
}

//Exercice 7
/* contient en argulent le nom du fichier contenant des commandes,
 * les stocke dans un tableau (File2TabArgv) puis les exécute une à une et affiche FIN une fois que
 * toutes les commandes sont exécutées.
 */
void ExecFile(char* fileName){
    char ***tabArgv ;
    int NbCommandes;
    tabArgv=File2TabArgv(fileName,&NbCommandes) ;
    for(int i=0 ; i < NbCommandes ; i++){
        Executer(tabArgv[i]);
    }
    printf("FIN\n");
}

//Exercice 8
/* qui reçoit un argument de type ARGV, l'exécute, mais n'attend qu'il se termine
 *
 */
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

/* Prends en arguments un nom de fichier contenant des commandes, 
 * les stocke dans un tableau (File2TabArgv), les exécute toutes en même temps
 * affiche FIN une fois que toutes les commandes sont exécutées.
 */
int ExecFileBatch(char* fileName){
    int res, ret;
    char ***tabArgv ;
    int NbCommandes = 0;
    int nbCommandesFinis = 0;
    tabArgv=File2TabArgv(fileName,&NbCommandes) ;
    for(int i=0 ; i < NbCommandes ; i++){
        int nbReturn = ExecuteBatch(tabArgv[i]);
    }

    while ( NbCommandes > nbCommandesFinis){
        res = wait(&ret);
        printf("PERE: mon fils %d termine; exit :%d\n",res, WEXITSTATUS(ret));
        nbCommandesFinis += 1;
    }
    printf("FIN\n");
}

int main(int argc, char **argv)
{
    /*char **arg= Ligne2Argv("ls -l");
    AfficheArgv(arg);
    char* chaine = Argv2Ligne(arg);
    printf("%s\n",chaine);
    Executer(arg);*/

    //miniBash();
    //ExecFileBatch("cmd.txt");

return 0;
}