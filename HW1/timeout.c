#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>


void sigchld_handler(pid_t signum);

int main(int argc, char *argv[]) {

    char *newenviron[] = { NULL };

    // printf("%s %s %s", argv[0], argv[1], argv[2]);
    if (argc <= 2) {
      printf("command or seconds are missing\n");
        exit(1);
    }

    else if (atoi(argv[1]) < 0) {
      printf("arg is negative\n");
        exit(1);
    }
    
    pid_t rc;
    char *new_arg[argc];

    for (int i = 2; i < argc; i++) {
        new_arg[i-2] = argv[i];
    }

    switch(rc = fork()) {
        case -1:
            perror("fork() failed");
            exit(1);
            break;
        case 0:
            execve(argv[2], new_arg, newenviron);
            break;
        default:
            signal(SIGCHLD, sigchld_handler);
            sleep(atoi(argv[1]));
            printf("Parent finished first");
            kill(rc, SIGTERM);
            break;
    }
}

void sigchld_handler(pid_t signum) {
    printf("Child finished first\n");
    kill(signum, SIGTERM);
}

