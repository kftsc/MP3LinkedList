// Sicheng Tian
#include "sh.h"
#include <signal.h>
#include <stdio.h>

void sig_handler(int signal); 

int main( int argc, char **argv, char **envp )
{
  /* put signal set up stuff here */
	signal(SIGINT, sig_handler);
	signal(SIGTSTP, SIG_IGN);
	//signal(EOF, sig_handler);
  return sh(argc, argv, envp);
}

void sig_handler(int signal)
{

	//kill(pid, SIGINT);
	if (signal == SIGINT) {
		printf("input signal: %d\n", signal);
	}
//	else if (signal == EOF) {
//		printf("EOF\n");
//	}
  /* define your signal handler */
}

