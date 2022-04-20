#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>

const char* MY_SIGNALS[] = {"Signal 0", "SIGUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGEMT", "SIGFPE", "SIGKILL", "SIGBUS", "SIGSEGV", "SIGSYS", "SIGPIPE", "SIGALRM", "SIGTERM", "SIGUSR1", "SIGUSR2", "SIGCHILD", "SIGCONT", "SIGTSTP", "SIGTTIN", "SIGTTOU", "SIGSTOP", "SIGXCPU", "SIGXFSZ", "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGPOLL", "SIGUSR1", "SIGUSR2"};
void sig_handler(pid_t signum);


int main(int argc, char *argv[]) {
	// Initialize all 31 signal handlers
	for (int i = 0; i < 32; i++) {
		signal(i, sig_handler);
	}
	
	// Loop until signal 9 is received, in order to keep catching signals
	while (1) {
		sleep(1);
	}
}

void sig_handler(pid_t signum) {

	// Get the current time
	time_t now;
    struct tm * timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    // Remove \n from the end of the time string
    char *timestr = asctime(timeinfo);
    timestr[strcspn(timestr, "\n")] = 0;

    // Print out date and signal received
    printf("%s Received signal %d (%s)\n", timestr, signum, MY_SIGNALS[signum]);
}
