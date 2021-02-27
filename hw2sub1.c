#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
//consider the 64-bit machine
//gcc -Wall -Werror hw2v5.c
//./a.out lion.txt nosuchfile.txt sleepy.txt lion.txt lion.txt nosuchfile.txt
//./a.out nosuchfile.txt

//function for seperating the string into one/multiple words
int seperate(char* unsepWord){
    int res = 0;
    int flag = 0;
    //flag from 0 to 1 means we just start a word
    //flag from 1 to 0 means we just completed a word
    //int len = strlen(unsepWord);
    for(int i = 0; *(unsepWord + i) != '\0'; i++){
          //if the non-alpha char is at the last position of string, we cannot let res++
        if(isalpha(*(unsepWord + i)) && flag == 0){
            res++;
            flag = 1;
        }
        
        if(!isalpha(*(unsepWord + i)) && flag == 1){
            flag = 0;
        }
    }
    return res;
}

// function counts the number of word, line, and digit
int* count(char* file){
    int* res = (int*) calloc(4, sizeof(int));
    FILE* fd;
    //file surely existed
    fd = fopen(file, "r");
    int word = 0;
    int line = 0;
    int digit = 0;
    char* currS = (char*) calloc(128, sizeof(char));
    //count word
    while(fscanf(fd, "%s", currS) != EOF){
        //printf("%s\n", currS);
        int split;
        split = seperate(currS);
        word += split;
    }
    fclose(fd);
    //count digit & line
    FILE* f;
    f = fopen(file, "r");
    char currC = fgetc(f);
    while(currC != EOF){
        if(isdigit(currC))
            digit++;
        if(currC == '\n')
            line++;
        currC = fgetc(f);
    }
    fclose(f);
    //special process for the case of no \n but line exists: skahdf\nsafsad
    FILE* forline;
    forline = fopen(file, "r");
    int verify;
    verify = fseek(forline,-1,SEEK_END);
    if(verify == 0){
        char* buffer = (char*) calloc(2, sizeof(char));
        fread(buffer, 1, 1, forline);
        *(buffer + 1) = '0';
        if(*buffer != '\n'){
            line++;
        }
        free(buffer);
    }
    
    *res = word;
    *(res + 1) = line;
    *(res + 2) = digit;
    return res;
}

//function for finding the child
int childNumFinder(pid_t* pids, int pid, int pidslen){
    for(int i = 0; i < pidslen; i++){
        if(*(pids + i) == pid){
            return i;
        }
    }
    return 0;
}

int main( int argc, char ** argv )
{
    //immedietely read/write each I/O
    //setvbuf(stdout, NULL, _IONBF, 0);
    fflush(stdout);
    // check if the # of arguments are valid
    if ( argc < 2)
    {
        fprintf( stderr, "ERROR: <Invalid arguments>\n");
        return EXIT_FAILURE;
    }
    
    //we need number(files) + 1 child process
    //processNumber = argc;
    int processNumber;
    processNumber = argc; //local machine edtion
    int childLeft = processNumber;
    //create pipe, no bracket allowed
    int* p = (int*) calloc(3, sizeof(int));
    int pipe_rc = pipe(p);
    if(pipe_rc == -1){
        fprintf( stderr, "ERROR: <pipe() failed>\n");
        return EXIT_FAILURE;
    }
    //parent's first line printing
    if((argc - 1) == 1){
        printf( "PARENT: Collecting counts for 1 file and the sentinel...\n");
    }else{
        printf( "PARENT: Collecting counts for %d files and the sentinel...\n", argc - 1);
    }
    pid_t* pids = calloc(processNumber + 1, sizeof(pid_t));
    //call fork() for creating processNumber child process
    for(int i = 0; i < processNumber; i++){
        if(i == processNumber - 1){
            printf( "PARENT: Calling fork() to create child process for the sentinel\n");
        }else{
            printf( "PARENT: Calling fork() to create child process for \"%s\"\n", *(argv + i + 1));
        }

        pid_t pid;
        fflush(stdout);
        pid = fork();
        if(pid == -1){
            fprintf( stderr, "ERROR: <fork() failed>\n");
            return EXIT_FAILURE;
        }
        if(pid == 0){
            free(pids);
            pids = NULL;
            int cpid;
            cpid = getpid();
            if(i == processNumber - 1){
                printf( "CHILD: Calling execl() to execute sentinel.out...\n");
                char* argvHelper = (char*) calloc(3, sizeof(char));
                sprintf(argvHelper, "%d", *(p + 1));
                fflush(stdout);
                execl("./sentinel.out", "./sentinel.out", argvHelper, NULL);
                free(argvHelper);
                exit(cpid);
            }
            printf( "CHILD: Processing \"%s\"\n", *(argv + i + 1));
            //close the read pipe for child's process
            close(*p);
            
            //write into buffer
            //use stat() to determine whether the file is exist or not
            struct stat buf;
            //(1) ret = bytes
            int ret = stat(*(argv + i + 1), &buf);
            if(ret == -1){
                perror("ERROR: stat() failed: ");
                printf("CHILD: Failed to process \"%s\"\n", *(argv + i + 1));
                close(*p);
                break;
            }
            int* helper = (int*) calloc(5, sizeof(int));
            int* res = count(*(argv + i + 1));
            int byte = (int) buf.st_size;
            write(*(p + 1), &byte, 4);
            for(int i = 1; i < 4; i++){
                write(*(p + 1), &*(res + i - 1), 4);
            }
            //close the write pipe for child's process
            //close(*(p + 1));
            free(helper);
            printf("CHILD: Done processing \"%s\"\n", *(argv + i + 1));
            exit(cpid);
        }
        //if is the parent process
        *(pids + i) = pid;
    }
//    parent(processNumber, pids);
    int childNum = 0;
    //waitpid
    while(childNum < processNumber){
        if(childLeft == processNumber)
        {
            close(*(p + 1));
        }
        int status;
        int currpid;
        currpid = waitpid(-1, &status, 0);
        int whichChild;
        whichChild = childNumFinder(pids, currpid, processNumber);
        
        if(WIFEXITED( status )){
            int* buffer = (int*) calloc(2, sizeof(int));
            int index = 0;
            int* storage = (int*) calloc(5, sizeof(int));
            while(index < 4){
                read(*p, buffer, 4);
                *(storage + index) = *buffer;
                *(buffer + 1) = '\0';
                index++;
            }
            if(whichChild == processNumber - 1){//print for sentinal
                printf("PARENT: Sentinel -- %d byte%s, %d word%s, %d line%s, %d digit%s\n", *(storage), (*(storage) != 1) ? "s":"", *(storage + 1), (*(storage + 1) != 1) ? "s":"", *(storage + 2), (*(storage + 2) != 1) ? "s":"", *(storage + 3), (*(storage + 3) != 1) ? "s":"");
            }else{
                printf("PARENT: File \"%s\" -- %d byte%s, %d word%s, %d line%s, %d digit%s\n", *(argv + whichChild + 1), *(storage), (*(storage) != 1) ? "s":"", *(storage + 1), (*(storage + 1) != 1) ? "s":"", *(storage + 2), (*(storage + 2) != 1) ? "s":"", *(storage + 3), (*(storage + 3) != 1) ? "s":"");
            }
            free(buffer);
            free(storage);
        }else{
            fprintf(stderr, "ERROR: <CHILD process abnormally stopped>\n");
            continue;
        }
        childLeft--;
        childNum++;
    }
    close(*p);
    free(pids);
    free(p);
    printf( "PARENT: All done -- exiting...\n");
    return EXIT_SUCCESS;
}

