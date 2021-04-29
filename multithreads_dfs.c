#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>

//gettid  clone 
int x[] = {-2, -2, -1, 1, 2, 2, 1, -1};
int y[] = {-1, 1, 2, 2, 1, -1, -2, -2};
int deadEndCount = 0;
int flag = 0;
int foundCount = 0;
int deadBoardSize = 8;

extern int next_thread_id;
extern int max_squares;
extern char *** dead_end_boards;
int** hasNway(char** visit, int row, int col, int m, int n);
void* dfs(void* arguments);
void deadEndPrint(char*** dead_end_boards, int x, int m, int n);
void extend(char*** matrix);
char** copyBoard(char** visit, int m, int n);
int max(int x, int y);
void printBoard(char** visit, int m, int n);
char** createVisit(int m, int n);
void* calldfs(void* arguments);
void addToBoard(char*** dead_end_boards, char** visit, int m, int n, int index);
void freePassRes(int** matrix, int m, int n);
void freeMatrix(char** matrix, int m, int n);
// void freeBoard(char*** board, int count, int m, int n);

//void* calldfs(void arg);
pthread_mutex_t mutex_on_dfs = PTHREAD_MUTEX_INITIALIZER;

struct passTodfs{
  char** visit;
  int row;
  int col;
  int sum;
  int ID;
  int sizem;
  int sizen;
  int threshold;
  int parent;
  int threadMax;
};

int simulate( int argc, char * argv[] ){
    /*
    argv validation*/
    setvbuf( stdout, NULL, _IONBF, 0 );
    if(argc != 6){
      fprintf( stderr, "ERROR: <Invalid arguments>\n");
      return EXIT_FAILURE;
    }

    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    int r = atoi(argv[3]);
    int c = atoi(argv[4]);
    int x = atoi(argv[5]);
    int area = m * n;
    //int sum = 1;

    if(m <= 2 || n <= 2){
      fprintf( stderr, "ERROR: Invalid argument(s)\n");
      fprintf( stderr, "USAGE: a.out <m> <n> <r> <c> <x>\n");
      return EXIT_FAILURE;
    }

    if(r < 0 || r > m - 1 || c < 0 || c > n - 1 || x < 0 || x > area){
      fprintf( stderr, "ERROR: Invalid argument(s)\n");
      fprintf( stderr, "USAGE: a.out <m> <n> <r> <c> <x>\n");
      return EXIT_FAILURE;
    }

    printf("MAIN: Solving Sonny's knight's tour problem for a %dx%d board\n", m, n);
    printf("MAIN: Sonny starts at row %d and column %d\n", r, c);

    char** visit = createVisit(m, n);
    visit[r][c] = 'S';    //printf("happy!!!!!!!!\n");

    
    max_squares++; //set to be one
    int** hasNwayRes = hasNway(visit, r, c, m, n);
    freeMatrix(visit, m, n);
    int firstDiv = hasNwayRes[0][0];
    if(firstDiv > 1){ //all first-layer threads
        pthread_t* tid = (pthread_t*) calloc(8, sizeof(pthread_t));
        printf("MAIN: %d possible moves after move #1; creating %d child threads...\n", firstDiv, firstDiv);
      for(int i = 0; i < firstDiv; i++){
        pthread_mutex_lock(&mutex_on_dfs);
        struct passTodfs* passings = (struct passTodfs*) malloc(sizeof(struct passTodfs));
        char** visit = createVisit(m, n);
        visit[r][c] = 'S';
        char** newVisit = copyBoard(visit, m, n);
        int** thisRes = hasNway(visit, r, c, m, n);
        int newr = thisRes[i + 1][0];
        int newc = thisRes[i + 1][1];
        newVisit[newr][newc] = 'S';
        passings -> visit = newVisit;
        passings -> row = newr;
        passings -> col = newc;
        passings -> sum = 2;
        passings -> ID = next_thread_id;
        passings -> sizem = m;
        passings -> sizen = n;
        passings -> threshold = x;
        passings -> parent = i;
        passings -> threadMax = 1;
        next_thread_id++;
        pthread_mutex_unlock(&mutex_on_dfs);
        pthread_create(&tid[i], NULL, dfs, (void*) passings);
        int* res = (int*) calloc(1, sizeof(int));
        pthread_join(tid[i], (void **)&res);
        if(*res >  passings -> threadMax){
          passings -> threadMax = *res;
        }
        printf("MAIN: Thread %d joined (returned %d)\n", passings -> ID, passings -> threadMax);
        freePassRes(thisRes, m, n);
        free(res);
        freeMatrix(visit, m, n);
        freeMatrix(newVisit, m, n);
        free(passings);
      }
      free(tid);
      freePassRes(hasNwayRes, m, n);
      /*
      result verification
      */
      if(flag == 0){
        printf("MAIN: All threads joined; best solution(s) visited %d squares out of %d\n", max_squares, area);
        if(x == 1 && deadEndCount == 1){
          printf("MAIN: Dead end board covering at least 1 square is:\n" );
        }else if(deadEndCount == 1){
          printf("MAIN: Dead end board covering at least %d squares is:\n", x);
        }else if(x == 1){
          printf("MAIN: Dead end boards covering at least 1 square are:\n");
        }else{
          printf("MAIN: Dead end boards covering at least %d squares are:\n", x);
        }
        for(int i = 0; i < deadEndCount; i++){
          printBoard(dead_end_boards[i], m, n);
        }
      }
      if(flag == 1){
        printf("MAIN: All threads joined; full knight's tour of %d achieved\n", foundCount);
      }
  }else if(firstDiv == 1){ //need modified after submitte
        struct passTodfs* passings = (struct passTodfs*) malloc(sizeof(struct passTodfs));
        char** visit = createVisit(m, n);
        visit[r][c] = 'S';
        char** newVisit = copyBoard(visit, m, n);
        int** thisRes = hasNway(visit, r, c, m, n);
        int newr = thisRes[1][0];
        int newc = thisRes[1][1];
        newVisit[newr][newc] = 'S';
        passings -> visit = newVisit;
        passings -> row = newr;
        passings -> col = newc;
        passings -> sum = 2;
        passings -> ID = next_thread_id;
        passings -> sizem = m;
        passings -> sizen = n;
        passings -> threshold = x;
        passings -> parent = 8;
        passings -> threadMax = 1;
        next_thread_id++;
        dfs((void*) passings);
      if(flag == 0){
        if(max_squares == 1){
          printf("MAIN: All threads joined; best solution(s) visited 1 square out of %d\n", area);
        }else{
          printf("MAIN: All threads joined; best solution(s) visited %d squares out of %d\n", max_squares, area);
        }
        for(int i = 0; i < deadEndCount; i++){
          printBoard(dead_end_boards[i], m, n);
        }
      }
      if(flag == 1){
        printf("MAIN: All threads joined; full knight's tour of %d achieved\n", foundCount);
      }
      freeMatrix(visit, m, n);
      freeMatrix(newVisit, m, n);
      freePassRes(thisRes, m, n);
      freePassRes(hasNwayRes, m, n);
      free(passings);
  }else{
    char** visit = createVisit(m, n);
    visit[r][c] = 'S';
    if(x <= 1){
    addToBoard(dead_end_boards, visit, m, n, deadEndCount);
    printf("MAIN: Dead end at move #1\n");
    printf("MAIN: All threads joined; best solution(s) visited 1 square out of %d\n", area);
    if(x == 1){
      printf("MAIN: Dead end board covering at least 1 square is:\n");
    }else{ 
      printf("MAIN: Dead end board covering at least %d squares is:\n", x);
    }
    printBoard(visit, m, n);
    }else
    {
    printf("MAIN: Dead end at move #1\n");
    printf("MAIN: Dead end board covering at least %d squares are:\n", x);
    }
    freePassRes(hasNwayRes, m, n);
    freeMatrix(visit, m, n);
  }
  return EXIT_SUCCESS;
}

int** hasNway(char** visit, int row, int col, int m, int n){
    int res = 0;
    int** wayStorage = (int**) calloc(9, sizeof(int*));
    
    for(int i = 0; i < 8; i++){
      int newR = row + x[i];
      int newC = col + y[i];
      if(newR >= 0 && newR < m && newC >= 0 && newC < n && visit[newR][newC] == '.'){
        wayStorage[res + 1] = (int*) calloc(2, sizeof(int));
        wayStorage[res + 1][0] = newR;
        wayStorage[res + 1][1] = newC;
        res++;
      }
    }
    wayStorage[0] = calloc(1, sizeof(int));
    wayStorage[0][0] = res;
    return wayStorage;
}


void* calldfs(void* arguments){
  dfs(arguments);
  return NULL;
}

void* dfs(void* arguments){
  struct passTodfs* passings = arguments;
  char** visit;
  int row; 
  int col;
  int sum;
  int ID; 
  int m;
  int n;
  int x;
  int parent;
  int tMax;
  visit = passings->visit;
  row = passings->row;
  col = passings->col;
  sum = passings->sum;
  ID = passings->ID;
  m = passings->sizem;
  n = passings->sizen;
  x = passings->threshold; 
  parent = passings->parent;
  tMax = passings-> threadMax;
  int** storage = hasNway(visit, row, col, m, n);
  int div = storage[0][0];
  int thistMax = tMax;
  //base case
  if(div == 0){
    if(sum == m * n){
      printf("THREAD %d: Sonny found a full knight's tour!\n", ID);
      pthread_mutex_lock(&mutex_on_dfs);
      max_squares = m * n;
      flag = 1;
      foundCount++;
      passings -> threadMax = m*n;
      tMax = m * n;
      freeMatrix(passings->visit, m, n);
      pthread_mutex_unlock(&mutex_on_dfs);
      int * tx = calloc( 1, sizeof( int ) );
      *tx += tMax;
      pthread_exit(tx);
      free(tx);
      free(storage);
      return NULL;
    }
    pthread_mutex_lock(&mutex_on_dfs);
    printf("THREAD %d: Dead end at move #%d\n", ID, sum);
    max_squares = max(max_squares, sum);
    passings -> threadMax = max(passings -> threadMax, sum);
    if(sum >= x){
      //deadEndCount++;
      if(deadEndCount + 1 > deadBoardSize){
          deadBoardSize = deadBoardSize + 8;
          dead_end_boards = realloc(dead_end_boards, sizeof(char**) * deadBoardSize);
          if(dead_end_boards == NULL){
            fprintf( stderr, "realloc() failed\n");
          }
      }
      addToBoard(dead_end_boards, passings->visit, m, n, deadEndCount);
      //printBoard(visit, m, n);
      deadEndCount++;
    }
    pthread_mutex_unlock(&mutex_on_dfs);
    int * tx = calloc( 1, sizeof( int ) );
    *tx += sum;
    pthread_exit(tx);
    free(tx);
    free(storage);
    if(sum < x){
      freeMatrix(passings->visit, m, n);
    }
    return NULL;
  }
  if(div == 1){
        pthread_mutex_lock(&mutex_on_dfs);
        struct passTodfs* passings = (struct passTodfs*) malloc(sizeof(struct passTodfs));
        int newr = storage[1][0];
        int newc = storage[1][1];
        visit[newr][newc] = 'S';
        passings -> visit = visit;
        passings -> row = newr;
        passings -> col = newc;
        passings -> sum = sum + 1;
        passings -> ID = ID;
        passings -> sizem = m;
        passings -> sizen = n;
        passings -> threshold = x;
        passings -> parent = parent;
        passings -> threadMax = tMax;
        sum++;
        pthread_mutex_unlock(&mutex_on_dfs);
        //printf("current area is %d\n", sum);
        dfs((void*) passings);
  }
    if(div > 1){
        int thisSum = sum;
        printf("THREAD %d: %d possible moves after move #%d; creating %d child threads...\n", ID, div, thisSum, div);
    pthread_t* tid = (pthread_t*) calloc(div, sizeof(pthread_t));
    for(int i = 0; i < div; i++){
        pthread_mutex_lock(&mutex_on_dfs);
        char** newVisit = copyBoard(visit, m, n);
        int newr = storage[i + 1][0];
        int newc = storage[i + 1][1];
        newVisit[newr][newc] = 'S';
        struct passTodfs* passings = (struct passTodfs*) malloc(sizeof(struct passTodfs));
        passings -> visit = newVisit;
        passings -> row = newr;
        passings -> col = newc;
        passings -> sum = thisSum + 1;
        passings -> ID = next_thread_id;
        passings -> sizem = m;
        passings -> sizen = n;
        passings -> threshold = x;
        passings -> parent = i;
        passings -> threadMax = thistMax;
        next_thread_id++;
        sum++;
        pthread_mutex_unlock(&mutex_on_dfs);
        pthread_create(&tid[i], NULL, calldfs, (void*) passings);
        int* res = (int*) calloc(1, sizeof(int));
        pthread_join(tid[i], (void **)&res);
        if(*res > tMax){
          tMax = *res;
        }
        printf("THREAD %d: Thread %d joined (returned %d)\n", ID, passings -> ID , *res);
        free(passings);
    }
    free(tid);
    int * tx = calloc( 1, sizeof( int ) );
    *tx += tMax;
    pthread_exit(tx);
    free(tx);
  }
  free(storage);
  return NULL;
}


void deadEndPrint(char*** dead_end_boards, int x, int m, int n){
  for(int i = 0; i < deadEndCount; i++){
    printBoard(dead_end_boards[i], m, n);
    printf("%dth board printed\n", i + 1);
  }
}

void printBoard(char** visit, int m, int n){
  for(int j = 0; j < m; j++){
      char * currLine = calloc(n + 4, sizeof(char));
      if(j == 0){
        currLine[0] = '>';
        currLine[1] = '>';
        for(int k = 0; k < n; k++){
        currLine[k + 2] = visit[j][k];
      }
      }else if(j == m - 1){
        currLine[0] = ' ';
        currLine[1] = ' ';
        currLine[n + 2] = '<';
        currLine[n + 3] = '<'; 
        for(int k = 0; k < n; k++){
        currLine[k + 2] = visit[j][k];
      }
      }else{
        currLine[0] = ' ';
        //  printf("what\n");
        currLine[1] = ' ';
        //  printf("the\n");
        for(int k = 0; k < n; k++){
        currLine[k + 2] = visit[j][k];
        //  printf("hell\n");
      }
      }
      printf("MAIN: %s\n", currLine);
      free(currLine);
  }
}

char** copyBoard(char** visit, int m, int n){
   char** newVisit = (char**) calloc(m, sizeof(int*));
   for(int i = 0; i < m; i++){
    newVisit[i] = calloc(n, sizeof(int));
    for(int j = 0; j < n; j++){
      newVisit[i][j] = visit[i][j];
    }
   }
    return newVisit;
}

int max(int x, int y){
  return x >= y ? x : y;
}

char** createVisit(int m, int n){
  char** res = calloc(m, sizeof(char*));
  for(int i = 0; i < m; i++){
    res[i] = calloc(n, sizeof(char));
    for(int j = 0; j < n; j++){
        res[i][j] = '.';
    }
  }
  return res;
}

void addToBoard(char*** dead_end_boards, char** visit, int m, int n, int index){
  dead_end_boards[index] = (char**) calloc(m, sizeof(int*));
  for(int i = 0; i < m; i++){
    dead_end_boards[index][i] = calloc(n, sizeof(int));
    for(int j = 0; j < n; j++){
      dead_end_boards[index][i][j] = visit[i][j];
    }
  }
}

void freePassRes(int** matrix, int m, int n){
    for(int i = 0; i < m; i++){
      free(matrix[i]);
    }
    free(matrix);
}

void freeMatrix(char** matrix, int m, int n){
    for(int i = 0; i < m; i++){
      free(matrix[i]);
    }
    free(matrix);
}
