/*Si dans la ligne de commande on entoure obligatoirement d’un espace les signes <, >, >>, la
fonction Ligne2Argv du TP1 va découper correctement la commande en mettant le caractère
dans un des arguments, et le nom du fichier associé dans le suivant. En tenant compte de cette
contrainte (afin de facilité les choses), modifier le programme minibash du TP1 pour qu’il
gère les redirections des entrées sorties.
Exemple :
Entrer Commande > ls > fichier
Entrer Commande > wc –l < fichier
12
Entrer Commande >*/

#define MAX_ARG 100
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

void miniBash(){
    printf("Entrer Commande>");
    char * buffer = (char *) malloc( MAX_ARG );
    char * redirection = (char *) malloc(2);
    char ** suite = (char **)malloc(MAX_ARG * sizeof(char *));

    while(fgets( buffer, MAX_ARG, stdin )){
        char** cmd = Ligne2Argv(buffer);
        analyse(&cmd, &redirection, &suite);// ls > fichier | "ls""NULL", ">", "fichier""NULL"
        Executer(cmd, redirection, suite);//partie 1
        printf("Entrer Commande>");
    }
    printf("<ctrlD>\nFIN\n");
}