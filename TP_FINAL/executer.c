#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#define MAX_ARG 100
#define TAILLE_MAX 1000 // Tableau de taille 1000

typedef struct NOEUD {
    int type;// 0 pour commande, 1 pour séquence, 2 pour parralele
    char ** cmd ;
    struct NOEUD* fils ;
    struct NOEUD* frere ;
}NOEUD;

char *Argv2Ligne(char** Commande){
    char *chaine = (char *)malloc(MAX_ARG * sizeof(char));

    for (int i=0; Commande[i] != NULL; i++){
        strcat(chaine, " ");
        strcat(chaine, Commande[i]);
    }
    return chaine;
}

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

void AfficheArgv(char** arg){
    for (int i=0; arg[i] != NULL; i++)
        printf("%s ", arg[i]);
}

void AfficheTabCmd(char*** tabCmd){
    for (int i = 0; tabCmd[i] != NULL; i++){
        AfficheArgv(tabCmd[i]);
        printf("\n");
    }
}
    

void analyse(char ***cmd, char **redirection, char ***suite){
    int i = 0;
    while ((*cmd)[i] != NULL){
        //printf("cmd[%d] : %s\n", i, (*cmd)[i]);
        if (strcmp((*cmd)[i], ">") == 0){
            *redirection = ">";
            (*cmd)[i] = NULL;
            *suite = &(*cmd)[i+1];
            break;
        } else if (strcmp((*cmd)[i], "<") == 0){
            *redirection = "<";
            (*cmd)[i] = NULL;
            *suite = &(*cmd)[i+1];
            break;
        } else if (strcmp((*cmd)[i], ">>") == 0){
            *redirection = ">>";
            (*cmd)[i] = NULL;
            *suite = &(*cmd)[i+1];
            break;
        } else if (strcmp((*cmd)[i], "|") == 0){
            *redirection = "|";
            (*cmd)[i] = NULL;
            *suite = &(*cmd)[i+1];
            break;
        }
        i++;
    }
}

int Executer(char** Commande){
    char * redirection = (char *) malloc(2);
    char ** suite = (char **)malloc(MAX_ARG * sizeof(char *));
    analyse(&Commande, &redirection, &suite);
    //printf("redirection : %s\n", redirection);
    //printf("suite : %s\n", suite[0]);

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

char *** File2TabArgv(char* fileName){//transforme un fichier en tableau de commande
    int nbCommandExcecuted = 0;
    char ***tabCmd = (char ***)malloc(MAX_ARG * sizeof(char *) * TAILLE_MAX);
    char chaine[TAILLE_MAX] = "";

    FILE* fichier = NULL;
    fichier = fopen(fileName, "r");
    if (fichier != NULL){
        // On peut lire dans le fichier
        while (fgets(chaine, TAILLE_MAX, fichier) != NULL){ // On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL)
            tabCmd[(nbCommandExcecuted)++] = Ligne2Argv(chaine);
        }
    }
    else{
        // On affiche un message d'erreur si on veut
        printf("Impossible d'ouvrir le fichier %s", fileName);
    }
    fclose(fichier); // On ferme le fichier qui a été ouvert
    return tabCmd;
}

NOEUD* construireArbre(char *** tabCmd){
    printf("%s\n")
    int i = 0;
    NOEUD* noeudCourant = (NOEUD*)malloc(sizeof(NOEUD));

    if (strcmp(tabCmd[0][0], "SEQ") == 0){//si on a SEQ
        noeudCourant->type = 1;//on est dans un bloc SEQ
        noeudCourant->fils = construireArbre(&tabCmd[1]);//on passe au noeud suivant
        tabCmd = &tabCmd[1];
        while (strcmp(tabCmd[0][0], "ENDSEQ") != 0 || i != 0){//on cherche la fin du bloc SEQ
            if (strcmp(tabCmd[0][0], "SEQ") == 0)
                i++;
            else if (strcmp(tabCmd[0][0], "ENDSEQ") == 0)
                i--;
            tabCmd = &tabCmd[1];
        }
        if (tabCmd[1] != NULL)
            noeudCourant->frere = construireArbre(&tabCmd[1]);//on passe au noeud suivant
        else
            noeudCourant->frere = NULL;

    } else if (strcmp(tabCmd[0][0], "PAR") == 0){//si on a PAR
        noeudCourant->type = 2;//on est dans un bloc PAR
        noeudCourant->fils = construireArbre(&tabCmd[1]);//on passe au noeud suivant
        tabCmd = &tabCmd[1];
        while (strcmp(tabCmd[0][0], "ENDPAR") != 0 || i != 0){//on cherche la fin du bloc PAR
            if (strcmp(tabCmd[0][0], "PAR") == 0)
                i++;
            else if (strcmp(tabCmd[0][0], "ENDPAR") == 0)
                i--;
            tabCmd = &tabCmd[1];
        }
        if (tabCmd[1] != NULL)
            noeudCourant->frere = construireArbre(&tabCmd[1]);//on passe au noeud suivant
        else
            noeudCourant->frere = NULL;

    } else if (strcmp(tabCmd[0][0], "ENDSEQ") == 0){//si on a ENDSEQ
        noeudCourant = NULL;

    } else if (strcmp(tabCmd[0][0], "ENDPAR") == 0){//si on a ENDPAR
        noeudCourant = NULL;

    } else { // on a une commande
        noeudCourant->type = 0;//on est dans une commande
        noeudCourant->cmd = tabCmd[0];//on stocke la commande
        noeudCourant->fils = NULL;//on a pas de fils
        if (tabCmd[1] != NULL)
            noeudCourant->frere = construireArbre(&tabCmd[1]);//on passe au noeud suivant
        else 
            noeudCourant->frere = NULL;

    }
    return noeudCourant;
}

void afficheArbre(NOEUD* noeudCourant, int nbindetation){
    char indentation[20] = "";
    for (int i = 0; i < nbindetation; i++)
        strcat(indentation, "\t");

    if (noeudCourant->type == 0){//si on a une commande
        printf("%sCMD: ", indentation);
        AfficheArgv(noeudCourant->cmd);
        printf("\n");
    } else if (noeudCourant->type == 1){//si on a SEQ
        printf("%sSEQ\n", indentation);
    } else if (noeudCourant->type == 2){//si on a PAR
        printf("%sPAR\n", indentation);
    }

    if (noeudCourant->fils != NULL)//si on a un fils
        afficheArbre(noeudCourant->fils, nbindetation+1);

    if (noeudCourant->type == 1){//si on a SEQ
        printf("%sENDSEQ\n", indentation);
    } else if (noeudCourant->type == 2){//si on a PAR
        printf("%sENDPAR\n", indentation);
    }

    if (noeudCourant->frere != NULL)//si on a un frere
        afficheArbre(noeudCourant->frere, nbindetation);
}

void executeArbre(NOEUD* noeudCourant, int typePere){

    if (typePere == 1){//si on a un bloc SEQ
        if (noeudCourant->type == 0)//si on a une commande
            Executer(noeudCourant->cmd);
        else 
            if (noeudCourant->fils != NULL)//si on a un fils
                executeArbre(noeudCourant->fils, noeudCourant->type);

        if (noeudCourant->frere != NULL)//si on a un frere
            executeArbre(noeudCourant->frere, typePere);    

    } else if (typePere == 2){//si on a un bloc PAR
        if (fork() == 0){//on execute la commande dans un fils
            if (noeudCourant->type == 0)//si on a une commande
                Executer(noeudCourant->cmd);
            else 
                if (noeudCourant->fils != NULL)//si on a un fils
                    executeArbre(noeudCourant->fils, noeudCourant->type);
            exit (0);

        } else {
            if (noeudCourant->frere != NULL)//si on a un frere
                if (fork() == 0){
                    executeArbre(noeudCourant->frere, typePere);
                    exit (0);
                } else {
                    int status;
                    wait(&status);
                }
            int status;
            wait(&status);
        }
    }
}

int main(int argc, char **argv){
    if (argv[1] != NULL){

        char *** tabCmd = File2TabArgv(argv[1]);
        //AfficheTabCmd(tabCmd);

        NOEUD* racine = construireArbre(tabCmd);
        afficheArbre(racine, 0);
        executeArbre(racine, 1);

    } else {
        printf("Veuillez entrer un fichier en parametre\n");
    }
    return 0;
}