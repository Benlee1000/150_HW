#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>


void sigchld_handler(int signum);
int kill(int fork, int);
int isdigit(int arg);
char *strcpy(char *dest, const char *src);

int main(int argc, char *argv[]) {

    char* newenviron[] = { NULL };
    char** new_arg = malloc((argc-2)*sizeof(char));
    int rc;
    int i = 0;

    /* printf("%s %s %s", argv[0], argv[1], argv[2]); */
    if(argc < 3){
        printf("Usage: timeout sec command [args ...]\n");
        exit(0);
    }

    if(atoi(argv[1]) < 0){
        printf("%d is not a positive integer\n", atoi(argv[1]));
        exit(0);
    }
    
    if(isdigit(*argv[1]) == 0){ 
        printf("%s is not a positive integer\n", argv[1]);
        exit(0);
    }

    if(access(argv[2],F_OK)!=0){
        perror("execve");
        exit(0);
    }

    
    
    /* Parse correct arguments for execve */
    for (; i < argc-2; i++) {
        new_arg[i] = (char*)malloc(sizeof(argv[i+2]));
        strcpy(new_arg[i],argv[i+2]);
    }
    new_arg[argc-2] = '\0';

    switch(rc = fork()) {
        case -1:
            perror("fork");
            exit(1);
            break;
        case 0:
            execve(argv[2], new_arg, newenviron);
            break;
        default:
            signal(SIGCHLD, sigchld_handler);
            sleep(atoi(argv[1]));
            /* printf("Parent finished first"); */
            kill(rc, SIGTERM);
            break;
    }
    return 0;
}

void sigchld_handler(int signum) {
    /* printf("Child finished first\n"); */
    /* kill(signum, SIGTERM); */
}