#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>

const char* MY_SIGNALS[] = {"Signal 0", "SIGUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGEMT", "SIGFPE", "SIGKILL", "SIGBUS", "SIGSEGV", "SIGSYS", "SIGPIPE", "SIGALRM", "SIGTERM", "SIGUSR1", "SIGUSR2", "SIGCHILD", "SIGCONT", "SIGTSTP", "SIGTTIN", "SIGTTOU", "SIGSTOP", "SIGXCPU", "SIGXFSZ", "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGPOLL", "SIGUSR1", "SIGUSR2"};
void sig_handler(pid_t signum);


int main(int argc, char *argv[]) {
	for (int i = 0; i < 32; i++) {
		signal(i, sig_handler);
	}
	kill(getpid(), 5);
}

void sig_handler(pid_t signum) {
	time_t now;
    struct tm * timeinfo;
    time(&now);
    timeinfo = localtime(&now);
    char *timestr = asctime(timeinfo);
    timestr[strcspn(timestr, "\n")] = 0;
    printf("Time:%s Received signal %d (%s)", timestr, signum, MY_SIGNALS[signum]);
}
