/* fork.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
  setvbuf( stdout, NULL, _IONBF, 0 ); 
  pid_t pidA;    /* process id (pid) -- unsigned short */
  pid_t pidB;

  /* create a new (child) process */
  pidA = fork();
  //pidB = fork();

  if ( pidA == -1 )
  {
    perror( "fork() for child A failed" );
    return EXIT_FAILURE;
  }

  if ( pidA > 0)
  {
    /* the PID of the child process is returned in the parent */
    printf("PARENT: okay, start here.\n");
    int statusA;
    //pid_t child_pidA;
    //child_pidA =
      waitpid(pidA, &statusA, 0);
      if ( WIFSIGNALED( statusA ) )   /* child process was terminated   */
      {                              /*  by a signal (e.g., seg fault) */
        printf("PARENT: child A...abnormally (killed by a signal)\n" );
      }
      else /* if ( WEXITED( status ) ) */
      { pidB = fork();
          if ( pidB == -1 )
          {
            perror( "fork() for child B failed" );
            return EXIT_FAILURE;
          }
          if(pidB == 0)
          {
              printf("CHILD B: and happy birthday to me!\n");
              printf("CHILD B: see ya later....self-terminating!\n");
              return 2;
          }
        int statusB;
        //pid_t child_pidB;
        //child_pidB =
        waitpid(pidB, &statusB, 0);
        if(WIFSIGNALED( statusB ))
        {
            printf("PARENT: child B...abnormally (killed by a signal)\n" );
        }
        else{
            printf("PARENT: both child processes terminated successfully.\n");
            printf("PARENT: phew, i'm glad they're gone!\n");
        }
        //int exit_status = WEXITSTATUS( status );
      }
  }
  else if ( pidA == 0)
  {
    printf("CHILD A: happy birthday to me!\n");
    printf("CHILD A: i'm bored....self-terminating....good-bye!\n");
    return 1;
  }


  /* By adding this last line of output, what is the exact output */
  /*  of this code?  Draw a new diagram.                          */
  //printf( "My pid is %d and I'm terminating....\n", getpid() );
    return EXIT_SUCCESS;
}


/* OUTPUT POSSIBILITIES:

goldsd@linux:~/s21$ ./a.out
PARENT: my child process id is 737
PARENT: my pid is 736
CHILD: happy birthday to me!  My pid is 737.
CHILD: my parent's pid is 736

goldsd@linux:~/s21$ ./a.out
PARENT: my child process id is 739
CHILD: happy birthday to me!  My pid is 739.
CHILD: my parent's pid is 738
PARENT: my pid is 738

What are all of the possible outputs for this code?

                                   fork()
                                  /      \
                                 /        \
                                /          \
                            <PARENT>     <CHILD>
PARENT: my child process id is 737     CHILD: happy birthday to me!  My pid is 737.
PARENT: my pid is 736                  CHILD: my parent's pid is 736


   (1) lines shown for <PARENT> interleave with lines shown for <CHILD>

   (2) lines shown for <PARENT> appear in the given order; same for <CHILD>


TO DO: write out each possible output given the diagram above





goldsd@linux:~/s21$ ./a.out
PARENT: my child process id is 1168
PARENT: my pid is 1167
CHILD: happy birthday to me!  My pid is 1168.
CHILD: my parent's pid is 1

 When the parent process terminates, the child process is inherited
  by the root node of the process tree, i.e., process with pid 1

 */
