int Ls(char *repertoire){
    int pid_w, ret;
    int pid = fork();

    if(pid == 0){
        //fils
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