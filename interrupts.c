/*
  interrupts.c: by Sean Krail (Spring 2016)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

const useconds_t timeout = 1;           // 1 microsecond
const uint32_t sleep_cycles = 2000000;  // roughly 5 microseconds


/*
	Prints whether SIGALRM is a member of the process's signal mask.
	  If it's a member,
	    then it is being blocked or held (i.e., sighold(SIGALRM)).
	  Else it's not member,
	    then it is unblocked or released (i.e., sigrelse(SIGALRM)).
*/
void
print_alrm_mask(char *msg)
{
	sigset_t mask_set;
	sigprocmask(0, NULL, &mask_set);
	if (sigismember(&mask_set, SIGALRM))
		printf("%s: SIGALRM is disabled.\n", msg);
	else printf("%s: SIGALRM is enabled.\n", msg);
}

/*
	SIGALRM Handler
		Implementented with sigset(3c).
		Must recall sigset each time the handler is called
		  because the handler gets reset to default.
*/
void
sigalrm_handler(int signal)
{
	sigset(SIGALRM, sigalrm_handler);
	printf("\nTIMER INTERRUPT...\n\n");
	ualarm(timeout, 0);
}

int
main(int argc, char **argv, char **envp)
{
	int i, j;

	sigset(SIGALRM, sigalrm_handler);
	
	printf("timer set for 1 microsecond intervals\n");
	ualarm(timeout, 0);

	print_alrm_mask("[!!]\tbefore critical sections");
	
	/*
		2 non-critical sections.
		  SIGALRM's handler will be called multiple times while sleeping.
	*/
	for (i = 0; i < 2; i++)
	{
		printf("[A%i]\t", i); print_alrm_mask("middle of non-critical section");

		printf("[A%i]\tsleeping for 5 microseconds\n", i);
		
		for (j = 0; j < sleep_cycles; j++);
		
		printf("[A%i]\twoke up\n", i);
	}
	
	print_alrm_mask("[!!]\tbetween critical sections");

	/*
		2 critical sections.
		  SIGALRM's handler will only be called twice,
		    once after each sigrelse call:
		    "[B#]    releasing SIGALRM"
	*/
	for (i = 0; i < 2; i++)
	{
		printf("[B%i]\tholding SIGALRM\n", i);
		sighold(SIGALRM);  // disable signal
		
		printf("[B%i]\t", i); print_alrm_mask("middle of critical section");
		
		printf("[B%i]\tsleeping for 5 microseconds\n", i);
		for (j = 0; j < sleep_cycles; j++);
		printf("[B%i]\twoke up\n", i);
		
		printf("[B%i]\treleasing SIGALRM\n", i);
		sigrelse(SIGALRM);  // enable signal
	}
	
	print_alrm_mask("[!!]\tafter critical sections");

	return 0;
}
