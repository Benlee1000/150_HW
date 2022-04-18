#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

// void kill_parent(int signum) {
//     printf("Child finished first\n");
//     kill(getppid(), SIGTERM);
// }


int main(int argc, char *argv[]) {

    char *newenviron[] = { NULL };
    struct sigaction info;
    // int start = 0;
    // int trigger = atoi(argv[1]);

    // printf("%s %s %s", argv[0], argv[1], argv[2]);
    if (*argv[1] == '-') {
      printf("arg is negative\n");
        exit(1);
    }
    
    if (argc == 1) {
      printf("command is empty\n");
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
            wait();
            printf("Child Finished First");
            exit(0);
            break;
        default:
            sleep(atoi(argv[1]));
            // signal(SIGCHLD, kill_parent);
            printf("Parent finished first");
            kill(rc, SIGTERM);
            break;
    }
}


