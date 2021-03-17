#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h> 

// cache size are all digits && positive?
int validSize(char* input){
		for(int i = 0; *(input + i) != '\0'; i++){
			if(!isdigit(*(input + i))){
				return 1;
			}
		}
		return 0;
}

//hash function
int hash(char* unhashedWord, int size){
	 int sum = 0;
	 int res = 0;
	 for(int i = 0; *(unhashedWord + i) != '\0'; i++){
	 	 sum += *(unhashedWord + i);
	 }
	 res = sum % size;
	 return res;
}

//how many seperate words in unsepword
int wordCount(char* unsepWord){
	int res = 1;
	//int len = strlen(unsepWord);
    for(int i = 0; *(unsepWord + i) != '\0'; i++){
    	//if the non-alpha char is at the last position of string, we cannot let res++
    	if(!isalpha(*(unsepWord + i))){
    		res++;
    	}
    }
    return res;
}

//seperate function
char** seperate(char* unsepWord){
	char** res;
	int num = wordCount(unsepWord);
	res = (char**) calloc(num + 1, sizeof(char*));
	//for temp use
	char* wordBuffer;
	int len = 128;
	wordBuffer = (char*) calloc(len, sizeof(char));
	int bufferIndex = 0;
	int resIndex = 0;
	int wordLength = strlen(unsepWord);
	for(int i = 0; *(unsepWord + i) != '\0'; i++){
		if(isalpha(*(unsepWord + i))){
			*(wordBuffer + bufferIndex) = *(unsepWord + i);
			//printf("%c\n",*(wordBuffer + bufferIndex));
			bufferIndex++;
		}else{
			//if the non-alphabetic char is at the last position, flushing is not needed
			if(i == wordLength - 1){
				break;
			}
			*(wordBuffer + bufferIndex) = '\0';
			*(res + resIndex) = wordBuffer;
			//printf("%s\n",*(res + resIndex));
			resIndex++;
			//free(wordBuffer);
			//wordBuffer = NULL;
			wordBuffer = (char*) calloc(len, sizeof(char));
			bufferIndex = 0;
		}
	}
	//printf("happy3");
	//printf("%s\n", *(sepWords));
	*(wordBuffer + bufferIndex) = '\0';
	*(res + resIndex) = wordBuffer;
	//free(wordBuffer);
	//wordBuffer = NULL;
	return res;
}

int smallerOne(int a, int b){
    if(a > b){
        return b;
    }else{
        return a;
    }
}

int largerOne(int a, int b){
    if(a < b){
        return b;
    }else{
        return a;
    }
}
//gcc -Wall -Werror hw1.c
//leaks ./a.out 17 lion.txt test2.txt

//main function
int main(int argc, char ** argv){
       int len = 128;
       int globalMax = 0;
        //(2)
		//immedietely read/write each I/O
		setvbuf(stdout, NULL, _IONBF, 0);
	   //(1)
       //check if arguments are valid
       //# of arguments
		if(argc < 3)
		{
			fprintf(stderr, "ERROR: <Invalid number of Arguments>\n");
			return EXIT_FAILURE;
		}
		//cache size are all digits && positive?
		if(validSize(* (argv + 1) ) == 1)
		{
			fprintf(stderr, "ERROR: <Invalid format of cache size: consisting non-numeric char>\n");
			return EXIT_FAILURE;
		}
		int size;
		size = atoi(*(argv + 1));
		if(size == 0){
			fprintf(stderr, "ERROR: <Cache size cannot be Zero>\n");
			return EXIT_FAILURE;
		}
		 //(3)
		 //cache
		 char** cache;
		 cache = (char**) calloc(size, sizeof(char*));
		 if(cache == NULL){
		 	fprintf(stderr, "ERROR: <Function calloc() calll failed>\n");
			return EXIT_FAILURE;
		 }
		 //(4)
		//loop for opening each file
		for(int i = 0; i < argc - 2; i++)
		{
			FILE* fd;
			fd = fopen(*(argv+ 2 + i),"r");
		    if(fd == NULL){
		    	fprintf(stderr, "ERROR: <File open failed>\n");
			    return EXIT_FAILURE;
		    }
		    //per file has a new constructed storage
		    char* storage = (char*) calloc(len, sizeof(char));
		    if(storage == NULL){
		    	fprintf(stderr, "ERROR: <Calloc call failed>\n");
			    return EXIT_FAILURE;
		    }
		     while(fscanf(fd, "%s", storage) != EOF){
		    	//non-alpha characters are word delimiters
		    	//divide each char array to several valid seperate word
		    	char** sepWords = seperate(storage);
		    	int wordNumber = wordCount(storage);
		    	for(int i = 0; i < wordNumber; i++){
		    		//printf("%s\n", *(sepWords + i));
		    		if(*(sepWords + i) == NULL) {
		    			free(*(sepWords + i));
		    			break;
		    		}
		    		int currLen = strlen(*(sepWords + i));
		    		//only strlen(word) > 3 will be hashed and stored in cache
		    		if(currLen < 3){
		    			free(*(sepWords + i));
		    			continue;
		    		}
		    		int hashCode = hash(*(sepWords + i), size);
                    globalMax = largerOne(globalMax, hashCode);
				    //printf("happy4");
		    		if(*(cache + hashCode) == NULL){
		    			//remenber to add 1
		    			*(cache + hashCode) = (char*) calloc(currLen + 1, sizeof(char));
		    			strcpy( *(cache + hashCode), *(sepWords + i));
		    			printf("Word \"%s\" ==> %d (calloc)\n", *(sepWords + i), hashCode);
		    		}else{
		    			*(cache + hashCode) = (char*) realloc(*(cache + hashCode), ((currLen + 1) * sizeof(char)));
		    			strcpy( *(cache + hashCode), *(sepWords + i));
		    			printf("Word \"%s\" ==> %d (realloc)\n", *(sepWords + i), hashCode);
		    		}
		    		free(*(sepWords + i));
		    		*(sepWords + i) = NULL;
		    		//printf("happy666666");
		    	}
		    	free(sepWords);
		    }
		    //free the storage when one file is processed
		    free(storage);
		    //free the storage when one file is processed
		    fclose(fd);
		}
		//print all non-NULL pointer in cache
		//cache is recycling used by multiple files
        int smaller = smallerOne(size, globalMax);
		for(int i = 0; i <= smaller; i++){
			if(*(cache + i) != NULL){
				printf("Cache index %d ==> \"%s\"\n", i, *(cache + i));
			}
			//free cache + i
			free(*(cache + i));
		}
		//free the cache;
		free(cache);
		return EXIT_SUCCESS;
}
//gcc -Wall -Werror hw1fn.c
// ./a.out 17 lion.txt 
// ./a.out 20000000 lion.txt test2.txt
// ./a.out 200000000 lion.txt test2.txt 
