/*
Ecrire la fonction NbFichier(char *repertoire) qui affiche le nombre de fichiers contenus dans
« repertoire » en exécutant la commande
« ls repetoire | wc –l »
*/
int NbFichier (char *repertoire){
    int pid1, pid2,resw,retw,t[2];
    pipe(t);
    pid1=fork();
    if (pid1 == 0) { //execution de ls
        close(1); dup(t[1]); close(t[1]); close(t[0]); // redirection de stdout vers t
        execlp("ls","ls",repertoire,NULL);
        exit(254);
        } //pid1==0
    else if (pid1> 0) {//père du 1er fork
        pid2=fork();//fork du wc
        if (pid2 == 0) { //fils du 2ème fork
            close(0); dup(t[0]); close(t[1]); close(t[0]); // redirection de stdin vers t
            execlp("wc","wc","-l",NULL);
            exit(254);} //pid2==0
        else if (pid2 > 0){ //père du 2ème fork
            close(t[1]); close(t[0]); //fermeture de t
            resw=wait(&retw); resw=wait(&retw); //attendre la fin des 2 fils
            return WEXITSTATUS(resw); 
        } //pid2>0
        else { //pid2<0, donc erreur
            close(t[1]); close(t[0]); //fermeture de t
            kill(pid1,SIGKILL); // kill ls
            resw=wait(&retw);
            return 252;// pid2<0
        } 
    } // pid1>0
    else {close(t[1]); close(t[0]); return 255; } // pid1 < 0 }
}