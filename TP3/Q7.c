typedef struct cmd {
    int pid ;
    int statut ;
    int retour ;
    time_t debut ;
    time_t fin ;
    char ** argv ;
}ENRCOMM ;

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

void AfficheArgv(char** arg){
    for (int i=0; arg[i] != NULL; i++)
        printf("%s ", arg[i]);
}

void AfficheArgvComplet(char** arg){
    for (int i=0; arg[i] != NULL; i++)
        printf("Arg %d : %s\n", i, arg[i]);
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

/* Execute la commande donnée en param
 * remplis la structure
 */
int Executer(ENRCOMM *Commande){
    int pid1, pid2, res, ret, file, t[2];
    time(&Commande->debut);
    Commande->statut = 1;//cmd en cours

    char * redirection = (char *) malloc(2);//> ou < ou | ou >>
    char ** suite = (char **)malloc(MAX_ARG * sizeof(char *));//suite de la commande

    char ** cmd = (char **)malloc(MAX_ARG * sizeof(char *));//copie de la commande
    for (int i=0; Commande->argv[i] != NULL; i++)
        cmd[i] = strdup(Commande->argv[i]);
        
    analyse(&cmd, &redirection, &suite);//analyse de la commande

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
        execvp(cmd[0],cmd);
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
        Commande->pid = pid1;
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

#define TAILLE_MAX 1000 // Tableau de taille 1000
ENRCOMM* File2TabCom(char* fileName, int * nbCommandExcecuted){// retourne un tableau de structure extrait du fichier
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

//affiche la structure
void afficheCom(ENRCOMM com){
    AfficheArgv(com.argv);
    printf(": %d %d %ld %ld %ld\n",com.pid, com.retour, com.debut, com.fin, com.fin - com.debut);
}

/* contient en argument le nom du fichier contenant des commandes,
 * les stocke dans un tableau (File2TabCom) puis les exécute une à une et affiche FIN une fois que
 * toutes les commandes sont exécutées.
 */
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

/* qui reçoit un argument de type ARGV, l'exécute, mais n'attend qu'il se termine
 */
 int ExecuteBatch(ENRCOMM *Commande){
    int pid1, pid2, res, ret, file, t[2];
    time(&Commande->debut);
    Commande->statut = 1;//cmd en cours

    char * redirection = (char *) malloc(2);//> ou < ou | ou >>
    char ** suite = (char **)malloc(MAX_ARG * sizeof(char *));//suite de la commande

    char ** cmd = (char **)malloc(MAX_ARG * sizeof(char *));//copie de la commande
    for (int i=0; Commande->argv[i] != NULL; i++)
        cmd[i] = strdup(Commande->argv[i]);
        
    analyse(&cmd, &redirection, &suite);//analyse de la commande

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
        execvp(cmd[0],cmd);
        exit(254);
    }
    else if (pid1 > 0){ // le père
        Commande->pid = pid1;
        if (redirection == "|"){
            pid2=fork();//fork du wc
            if (pid2 == 0) { //fils du 2ème fork
                close(0); dup(t[0]); close(t[1]); close(t[0]); // redirection de stdin vers t
                execvp(suite[0],suite);
                exit(254);
            }
            else if (pid2 > 0){ //père du 2ème fork
                close(t[1]); close(t[0]); //fermeture de t
                ret=wait(&res);//attente de la fin du fils 1
                Commande->pid = pid2;
            }
            else { //pid2<0, donc erreur
                close(t[1]); close(t[0]); //fermeture de t
                kill(pid1,SIGKILL); // kill cmd
                ret=wait(&res);
                return 252;// pid2<0
            }
        }
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
            while (res != tabEnr[i].pid )//recherche du bon ENRCOMM dans le tableau d'ENRCOMM 
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