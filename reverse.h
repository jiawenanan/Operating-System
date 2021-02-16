char * reverse( char * s )
{
char * buffer;
int i, len = strlen( s );
buffer = (char*)malloc(len*sizeof(char) + 1);
if(buffer == NULL){
    printf("memory application failed\n");
    return 0;
}
memset(buffer, 0, len*sizeof(char) + 1);
for ( i = 0 ; i < len ; i++ ) buffer[i] = s[len-i-1];
buffer[len] =  '\0';
strcpy( s, buffer );
free(buffer);
return s;
}
