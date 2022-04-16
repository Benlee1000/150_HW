#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    char *newenviron[] = { NULL };
    if (argc < 0) {
      printf("arg is negative\n");
        exit(1);
    }
    
    if (argv[0] == "") {
      printf("command is empty\n");
        exit(1);
    }
    pid_t rc;
    switch (rc = fork()) {
    case -1:
        perror("fork() failed");
        exit(1);
        break;
    case 0:
        execve(argv[0], argv, newenviron);
        break;
    default:
        sleep(argc);
        kill(rc, SIGTERM);
        break;
    }
}