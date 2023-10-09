#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <signal.h>

#define FAIL 127


unsigned int ptrace_setoptions = PTRACE_O_TRACEEXEC | PTRACE_O_EXITKILL | PTRACE_O_TRACEFORK | PTRACE_O_TRACECLONE;
bool v = false;


int propagate_signal(int status) {
	int signal = 0;
	if(WIFSTOPPED(status)) {
		if(v) printf("Child was stopped.\nNumber of the signal received was %d.\n", WSTOPSIG(status));
	}
	if(WIFSTOPPED(status) && WSTOPSIG(status) == SIGTERM) {
		if(v) printf("Child received SIGTERM.\n");
		signal = SIGTERM;
	}
	else if(WIFSTOPPED(status) && WSTOPSIG(status) == SIGINT) {
		if(v) printf("Child received SIGINT.\n");
		signal = SIGINT;
	}
	else if(WIFSTOPPED(status) && WSTOPSIG(status) == SIGQUIT) {
		if(v) printf("Child received SIGQUIT.\n");
		signal = SIGQUIT;
	}
	else if(WIFSTOPPED(status) && WSTOPSIG(status) == SIGHUP) {
		if(v) printf("Child received SIGHUP.\n");
		signal = SIGHUP;
	}
	else if(WIFSTOPPED(status) && WSTOPSIG(status) == SIGPIPE) {
		if(v) printf("Child received SIGPIPE.\n");
		signal = SIGPIPE;
	}
	else if(WIFSTOPPED(status) && WSTOPSIG(status) == SIGCHLD) {
		if(v) printf("Child received SIGCHLD.\n");
		signal = SIGCONT;
	}
	else if(WIFSTOPPED(status) && WSTOPSIG(status) != SIGTRAP) {
		signal = WSTOPSIG(status);
	}
	return signal;
}


int handle_exit(int status) {
	if (WIFEXITED(status)) {
		if(v) fprintf(stderr, "Exit status was %d.\n",
						WEXITSTATUS(status));
		if(v) fprintf(stderr, "The return code was %d.\n",
						WEXITSTATUS(status));
		return WEXITSTATUS(status);
	}
	if (WIFSIGNALED(status)) {
		if(v) fprintf(stderr, "Child terminated by signal %d.\n",
						WTERMSIG(status));
		if(v) fprintf(stderr, "The return code was %d.\n",
						128 + WTERMSIG(status));
		return 128 + WTERMSIG(status);
	}
	return -1;
}


int handle_child(int argc, char **argv) {
	ptrace(PTRACE_TRACEME, NULL, NULL, NULL);
	raise(SIGSTOP);
	if(v) return execvp(argv[2], argv + 2);
	return execvp(argv[1], argv + 1);
}


void print_executable_path(pid_t pid) {
	char *pathname = malloc(4096);
	char buf[4096];
	sprintf(pathname, "%s%d%s", "/proc/", pid, "/exe");
	ssize_t len = readlink(pathname, buf, sizeof(buf) - 1);
	if (len != -1) {
  	buf[len] = '\0';
	} else {
		perror("READLINK");
	}
	if(v) fprintf(stderr, "The executed program was : ");
	fprintf(stderr, "%s\n", buf);
	free(pathname);
}


int main(int argc, char **argv) {	
	if(argv[1] == NULL) {
		fprintf(stderr, "Argument is missing.\n");
		return FAIL;
	}

	if(!strcmp(argv[1], "-v")) {
		if (argv[2] == NULL) {
			fprintf(stderr, "Argument is missing.\n");
			return FAIL;
		}
		v = true;
	}
	
	pid_t pid = fork();
	if(pid == 1) {
		perror("FORK");
		return FAIL;
	}

	if(pid == 0) {
		handle_child(argc, argv);
		perror("EXECVP failed : ");
		return FAIL; 
	}

	if(v) printf("Fork() was successful.\n");
	if(v) printf("Child process (tracee) was created.\n");

	int status = 0;
	int wait_check = 0;
	int signal = 0;
	pid_t new_pid = 0;


	wait(&status);

	ptrace(PTRACE_SETOPTIONS, pid, NULL, ptrace_setoptions);
	ptrace(PTRACE_CONT, pid, NULL, NULL);

	while((pid = wait(&status)) > 0) {
		if(v) printf("\nStatus information provided "
				"by call to wait() : %d.\n\n", status);

		signal = propagate_signal(status);
		
		if(WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
			if(v) printf("Child received SIGTRAP.\n");
		
			if(status>>8 == (SIGTRAP | (PTRACE_EVENT_EXEC<<8))) {
				if(v) printf("===============\nEXECVE occurred\n===============\n");
				print_executable_path(pid);
				signal = 0;
			}
			else if(status>>8 == (SIGTRAP | (PTRACE_EVENT_FORK<<8)) ||
					status>>8 == (SIGTRAP | (PTRACE_EVENT_CLONE<<8))) {
				if(v) printf("The calling process PID is %d.\n", getpid());

				if (ptrace(PTRACE_GETEVENTMSG, wait_check, NULL, &new_pid) != -1) {
					if(v) printf("==========================================="
								"=====\nA new child process "
								"with PID %d was created.\n===="
								"============================================\n",
								new_pid);
				}
				signal = 0;
			} 
			else {
				signal = SIGTRAP;
			}
		}
		ptrace(PTRACE_CONT, pid, NULL, signal);
	}
	if (WIFEXITED(status) || WIFSIGNALED(status)) {
			if(v) printf("Child exited.\n");
			return handle_exit(status);
	}


	return EXIT_SUCCESS;
}