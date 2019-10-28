
#include "get_path.h"
 #include <wordexp.h>



int pid;
int sh( int argc, char **argv, char **envp);
//build in command
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void printenv(char **envp);
void mycd(int inputSize);


// helper function
wordexp_t *handleWildCard(char **input);
char **stringToArray(char *input, int *size);
void freeInput(char **input);
void freePath(struct pathelement *pathlist);

#define PROMPTMAX 32
#define MAXARGS 10
