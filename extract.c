
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void extract(char * file){
    int fd;
    int len;
    fd = open(file, O_RDONLY);
    if(fd == -1)
    {
      perror( "open() failed" );
      return;
    }
    len = lseek(fd, 0, SEEK_END);
    if(len < 7){
    	return;
    }
    char output[len/7];
    int count = 0;
    lseek(fd, 0, SEEK_SET);
    for (int i = 0 ; i < len - 7; i += 7 ){
           char buffer[8];
           int rc = read( fd, buffer, 7 );
           buffer[rc] = '\0';  
           output[count] = buffer[6];
           count++;
    }
    printf("%s\n",  output);
    close(fd);
    return;
}

int main(int argc, char * argv[]) {
    if(argc != 2)
    {
        printf("exactly one file name required");
        return 1;
    }
    extract(argv[1]);
    return 0;
}