/* forked.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

/* implement these functions in fork-functions.c */

int lecex2_child(){
    int fd = open("data.txt", O_RDONLY);
    if(fd == -1){
        fprintf( stderr, "open() failed");
        abort();
        return EXIT_FAILURE;;
    }
    int rd = lseek(fd, 5, SEEK_SET);
    if ( rd == -1 )
    {
      fprintf( stderr, "lseek() failed");
      abort();
      return EXIT_FAILURE;
    }
    char buffer;
    int res;
    rd = read(fd, &buffer, 1);
    if(rd)
    {
        res = buffer;
        close(fd);
        return res;
    }
    fprintf( stderr, "extract 6th char failed");
    abort();
    close(fd);
    return EXIT_FAILURE;
}

int lecex2_parent(){
    int status;
    //pid_t child_pid;
    //child_pid =
    waitpid(-1, &status, 0);
    if(WIFEXITED( status )){
        int exit_status = WEXITSTATUS( status );
        printf("PARENT: child process reported '%c'\n", exit_status);
        return EXIT_SUCCESS;
    }else{
        printf("PARENT: child process terminated abnormally\n");
        return EXIT_FAILURE;
    }
}


