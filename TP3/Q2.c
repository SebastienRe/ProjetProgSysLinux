/*
fichierResultat la liste des fichiers contenus dans répertoire en exécutant la commande « ls repertoire > fichierResultat »
*/
int LsDansFichier(char *repertoire, char *fichierResultat){
    int pid_w, ret;
    int file;
    int pid = fork();

    if(pid == 0){
        file = open(fichierResultat, O_WRONLY, 0777);
        close(1);//ferme stdout
        dup(file);
        execlp("ls","ls",repertoire,NULL);
        exit(201);
    }else if(pid > 0){
        //pere
        pid_w = wait(&ret);
        printf("PERE: mon fils %d termine; exit :%d\n",pid_w, WEXITSTATUS(ret));
        return WEXITSTATUS(ret);
    }else{
        printf("Erreur fork\n");
        return(200);
    }
}