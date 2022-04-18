#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[]) {

	char *newenviron[] = { NULL };

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
			printf("Execute");
			execve(argv[2], new_arg, newenviron);
			break;
		default:
			sleep(atoi(argv[1]));
        	kill(rc, SIGTERM);
        	break;
	}
}
