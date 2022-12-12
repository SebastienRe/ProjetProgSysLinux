/*Ecrire la fonction int NbFichier(char *repertoire) qui retourne le nombre de fichiers contenus
dans « repertoire » en exécutant la commande
« ls repetoire | wc –l » et en lisant le résultat de wc –l.*/
int NbFichier (char *repertoire){
    int pid1, pid2,resw,retw,t1[2], t2[2], sauv;
    pipe(t1);
    pipe(t2);
    pid1=fork();

    if (pid1 == 0) { //fils du 1er fork
        close(1); dup(t1[1]); // redirection de stdout vers t1
        close(t1[1]); close(t1[0]); //fermeture de t1
        execlp("ls","ls",repertoire,NULL);
        exit(254);
    }

    else if (pid1> 0) {//père du 1er fork

        pid2=fork();//fork du wc

        if (pid2 == 0) { //fils du 2ème fork
            close(0); dup(t1[0]); // redirection de stdin vers t1
            close(1); dup(t2[1]); // redirection de stdout vers t1
            close(t1[1]); close(t1[0]); //fermeture de t1
            close(t2[1]); close(t2[0]); //fermeture de t2
            execlp("wc","wc","-l",NULL);
            exit(254);
        }

        else if (pid2 > 0){ //père du 2ème fork
            sauv = dup(0); // sauvegarde de stdin
            close(0); dup(t2[0]); // redirection de stdin vers t2
            close(t1[1]); close(t1[0]); //fermeture de t1
            close(t2[1]); close(t2[0]); //fermeture de t2
            char buffer[50];
            if (fgets(buffer,50,stdin) != NULL) {
                close(0); dup(sauv); // restauration de stdin
                int nb = atoi(buffer);
                kill(pid1,SIGKILL); // kill ls
                kill(pid2,SIGKILL); // kill wc
                resw=wait(&retw); resw=wait(&retw); //attendre la fin des 2 fils
                return nb;
            }
        }

        else { //pid2<0, donc erreur pid2
            close(t1[1]); close(t1[0]); //fermeture de t1
            kill(pid1,SIGKILL); // kill ls
            resw=wait(&retw);
            return 252;
        }
    }
    else {// pid1 < 0, donc erreur pid1
        close(t1[1]); close(t1[0]); 
        return 255; 
    } 
}