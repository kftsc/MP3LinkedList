// SIcheng Tian
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <wordexp.h>

//#include "get_path.h"
#include "sh.h"

#define MAXLINE 128

int sh( int argc, char **argv, char **envp )
{
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  //char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd;
  //char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;
  char buff[MAXLINE];
  int inputSize = 0;
  int* SIGNAL = NULL;
  char **input = NULL;
  wordexp_t *wordexp = NULL;


  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;		/* Home directory to start
						  out with*/
     
  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
  {
    perror("getcwd");
    exit(2);
  }
  owd = calloc(strlen(pwd) + 1, sizeof(char));    // haven't free
  memcpy(owd, pwd, strlen(pwd));
  prompt[0] = ' '; prompt[1] = '\0';

  /* Put PATH into a linked list */
  pathlist = get_path();
  //signal(SIGINT, handle_signalshell);
//  signal(SIGINT, handle_signalshell);
  while ( go )
  {
	  //pathlist = get_path();
	  input = NULL;
    /* print your prompt */
	  printf("%s ", prompt);
	  printf("[%s]> ", pwd);
    /* get command line and process */
	  if (fgets(buff, MAXLINE,stdin) != NULL) {
		  if (buff[strlen(buff) - 1] == '\n') {
			  buff[strlen(buff) - 1] = 0;
			  input = stringToArray(buff, &inputSize);
		  }
	  }else {
		  clearerr(stdin);
		  //rewind(stdin);
		  fprintf(stderr, "END OF FILE\n");
		  //printf("# End of File!!!: %d\n", feof(stdin));
		  //fgets(buff, MAXLINE,stdin);
		  //fflush(stdin);
		  //printf("stdin: %s\n", stdin);
		  continue;
	  }

    /* check for each built in command and implement */
	  if (input[0] == NULL) { // cannot use strcmp with NULL
		  freeInput(input); // memory leak ?????
		  continue;
	  }
	  if (strcmp(input[0], "exit") == 0) {
		  printf("# executing build-in command: %s\n", buff);
		  go = 0;
		  printf("# exit the shell\n");
	  }else if (strcmp(input[0], "pwd") == 0) {
		  printf("# executing build-in command: %s\n", buff);
		  printf("%s\n", pwd);
	  }else if (strcmp(input[0], "cd") == 0) {
		  //printf("%s\n", input[1]);
		  printf("# executing build-in command: %s\n", buff);
		  if (inputSize == 1) {
			  chdir(homedir);
		  }else if ((strcmp(input[1], "-") == 0) && inputSize == 2) {
			  chdir(owd);
		  }else if (inputSize == 2){
			  if (chdir(input[1]) != 0) {
				  printf("No Such Directory\n");
				  freeInput(input);
				  continue;
			  }

		  }else {
			  printf("NO SUCH COMMAND\n");
			  freeInput(input);
			  continue;
		  }
		  //memcpy(owd, pwd, strlen(pwd));
		  strcpy(owd, pwd);     // why use memcpy?????????????
		  free(pwd);
		  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL ){
		      perror("getcwd");
		      exit(2);
		   }
	  }else if (strcmp(input[0], "list") == 0) {
		  //printf("%d\n",inputSize);
		  printf("# executing build-in command: %s\n", buff);
		  if (inputSize == 1) {
			  list(pwd);
		  }else {
			  for (int i = 1; i < inputSize;i++) {
				  printf("_%s:\n", input[i]);
				  list(input[i]);
			  }
		  }
	  }else if (strcmp(input[0], "which") == 0) {
		  printf("# executing build-in command: %s\n", buff);
//		  if (inputSize == 2) {
//			  commandpath = which(input[1], pathlist); // ?????
//			  if (commandpath == NULL) {
//				  printf("# cannot find the command in pathlist\n");
//			  }else {
//				  free(commandpath);
//			  }
//		  }
		  for(int i = 1; input[i] != NULL; i++) {
			  commandpath = which(input[i], pathlist);
			  if (commandpath != NULL) {
				  free(commandpath);
			  }
//			  else {
//				  printf("# cannot find the command in pathlist\n");
//			  }
		  }
	  }else if (strcmp(input[0], "where") == 0) {
		  printf("# executing build-in command: %s\n", buff);
//		  if (inputSize == 2) {
//			  where(input[1], pathlist);
//		  }
		  for (int i = 1; input[i] != NULL; i++) {
			  where(input[i], pathlist);
		  }

	  }else if (strcmp(input[0], "pid") == 0) {
		  printf("# executing build-in command: %s\n", buff);
		  printf("current process ID: [%d]\n", getpid());
	  }else if (strcmp(input[0], "kill") == 0) {
		  printf("# executing build-in command: %s\n", buff);
		  int id;
		  if (inputSize == 2) {
			  sscanf(input[1], "%d\n", &id);
			  //printf("The Integer value id:  %d\n", id);
			  kill(id, SIGTERM);
		  }else if (inputSize == 3) {
			  // get the prefix
			  char prefix[2];
			  memcpy(prefix, &input[1][0], 1);
			  prefix[1] = '\0';
			  //printf("the prefix: %s\n", prefix);
			  if (strcmp(prefix, "-") == 0) {
				  // convert input id to int
				  sscanf(input[2], "%d\n", &id);
			  	  //printf("The Integer value id:  %d\n", id);
			  	  // get the substring of the first arg
			  	  int len = strlen(input[1]);
			  	  //printf("the length: %d\n", len);
			  	  char sub[len];
			  	  memcpy(sub, &input[1][1], (len - 1));
			  	  sub[len - 1] = '\0';
			  	  //printf("the sub: %s\n", sub);
			  	  // conver signal to int
			  	  int sig;
			  	  sscanf(sub, "%d\n", &sig);
			  	  kill(id, sig);
			  }
		  }
	  }else if (strcmp(input[0], "prompt") == 0) {
		  if (inputSize == 1) {
			  printf("input prompt prefix: ");
			  char pbuff[MAXLINE];
			  fgets(pbuff, MAXLINE, stdin);
			  pbuff[strlen(pbuff) - 1] = '\0';
			  strcpy(prompt, pbuff);
		  }else if (inputSize == 2) {
			  strcpy(prompt, input[1]);
		  }
	  }else if (strcmp(input[0], "printenv") == 0) {
		  if (inputSize == 1) {
			  for(int i = 0; envp[i] != NULL; i++) {
				  printf("%s\n", envp[i]);
			  }
		  }else if (inputSize == 2) {
			  printf("%s= %s\n", input[1], getenv(input[1]));
		  }else{
			  fprintf(stderr, "%s: ERROR Too Much Arguments.\n", buff);  // stderr same as tcsh?????
		  }
	  }else if (strcmp(input[0], "setenv") == 0) {
		  if (inputSize == 1) {
			  for(int i = 0; envp[i] != NULL; i++) {
			  	printf("%s\n", envp[i]);
			  }
		  }else if (inputSize == 2) {
			  setenv(input[1],"" ,1);
			  if (strcmp(input[1], "HOME") == 0) {
				  char a[128];
				  strcpy(a, "");
				  homedir = &a[0];
			  }else if (strcmp(input[1], "PATH") == 0) {
				  freePath(pathlist);
				  free(pa);
				  pathlist = get_path();  // free up and assign to new
			  }
		  }else if (inputSize == 3) {
			  setenv(input[1], input[2], 1);  // need to overwrite
			  if (strcmp(input[1], "HOME") == 0) {
				  char a[128];
				  strcpy(a, input[2]);
				  homedir = &a[0];
			  }else if (strcmp(input[1], "PATH") == 0) {
				  freePath(pathlist);
				  free(pa);
				  pathlist = get_path();  // free up and assign to new
			  }
		  }else{
			  fprintf(stderr, "%s: ERROR Too Much Arguments.\n", buff);
		  }
	  }
     /*  else  program to exec */
	  else {
		  //printf("# else stuff\n");
       /* find it */
       /* do fork(), execve() and waitpid() */
		  commandpath = which(input[0], pathlist);
		  if (commandpath != NULL) {
			  arg = commandpath;    // find path in pathlist and excute
		  }else {
			  arg = input[0];       //  handle ./   /   absolute path
		  }
		  //printf("what is input[0]: %s\n", input[0]);
		  //printf("what is commandpath: %s\n", commandpath);
		  //printf("what is arg: %s\n", arg);
		  DIR* dir = NULL;
		  if (access(arg, X_OK) == 0) {
			  printf("# executing: %s\n", arg);
			  if ((dir = opendir(arg)) == NULL) {
				  pid = fork();
				  if (pid == 0) {
					  //signal(SIGINT, handle_signal);
					  //printf("here\n");
					  //kill(pid, SIGINT);
					  wordexp = handleWildCard(input);
					  execvp(arg, wordexp->we_wordv);
					  //execve(arg, input, envp);
					  printf("error in execve\n");
					  exit(2);
				  }else{
					  //kill(pid, SIGINT);
					  waitpid(pid,NULL,0);
				  }
			  }else {
				  printf("It Is a Directory\n");
				  closedir(dir);
			  }
		  }
		  else {
			  //printf("NO SUCH COMMAND\n");
			  //commandpath = which(input[0], pathlist = get_path());
			  fprintf(stderr, "%s: Command not found.\n", input[0]);
		  }
      /* else */
        /* fprintf(stderr, "%s: Command not found.\n", args[0]); */

		  if (commandpath != NULL) {
			  free(commandpath);
		  }
	  }
	  //printf("here\n");
	  freeInput(input);
	  wordfree(wordexp);
	  //free(gbuf);
  }
  freePath(pathlist);   // free linked list   (cast to pointer?)
  free(prompt);
  free(owd);
  free(pwd);
  free(pa);
  return 0;
} /* sh() */



// build in command
char *which(char *command, struct pathelement *pathlist )
{
   /* loop through pathlist until finding command and return it.  Return
   NULL when not found. */
	//char* cmd = NULL;
	char buff[128];
	char path[128];
	strcpy(buff, command);
	//char* path = (char*) malloc(strlen(buff) * sizeof(char));
	//printf("check if command changed: %s\n", command);
	while(pathlist) {
		sprintf(path, "%s/%s", pathlist->element, buff);
		//printf("check if command/path changed2: %s\n", command);
		//printf("1\n");
		if (access(path, X_OK) == 0) {
			printf("[%s]\n", path);
			//strcpy(command, buff);
			//printf("check if command changed2: %s\n", command);
			//printf("path length: %ld\n", strlen(path));
			char* thepath = (char*) malloc((strlen(path) + 1) * sizeof(char));
			strcpy(thepath, path);
			return thepath;
		}
		pathlist = pathlist->next;
	}
	//printf("check if command changed4: %s\n", command);
	//free(path);
	//strcpy(command, buff);     // do i need this ????
	//printf("check if command changed3: %s\n", command);
	return NULL;

} /* which() */

char *where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */
	char buff[128];
	char path[128];
	strcpy(buff, command);
	//printf("input is: %s\n", buff);
	while(pathlist) {
		sprintf(path, "%s/%s", pathlist->element, buff);
		//printf("path is{%s\n}", path);
		if (access(path, F_OK) == 0) {
			printf("%s: [%s]\n",buff ,path);
		}
		pathlist = pathlist->next;
	}
	return NULL;
} /* where() */

void list ( char *dir )
{
  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
	struct dirent *d = NULL;
	DIR* cdir = opendir(dir);
	if (cdir == NULL) {
		printf("No Such Directary\n");
		return;
	}
	readdir(cdir);   // .
	readdir(cdir);   // ..
	while((d = readdir(cdir)) != NULL) {
		printf("%s\n", d->d_name);
	}
	closedir(cdir);

} /* list() */




// helper function

wordexp_t* handleWildCard(char **input){
	wordexp_t *p = (wordexp_t *) malloc(sizeof(wordexp_t));
	int index = 1;

	wordexp(input[0], p, 0);
	while(input[index] != NULL) {
		wordexp(input[index], p, WRDE_DOOFFS | WRDE_APPEND);
		index++;
	}
	return p;

}


char **stringToArray(char *input, int *size) {
	char buffer[MAXLINE];
	strcpy(buffer, input);
	char *t = strtok(buffer, " ");
	int count = 1;
	while (strtok(NULL, " "))
		count++;
	char **strarr = (char **) malloc((count + 1) * sizeof(char*));
	strcpy(buffer, input);
	t = strtok(buffer, " ");
	count = 0;
	while(t) {
		int len = strlen(t);
		//printf("\t############%d\n", len);
		strarr[count] = (char * ) malloc((len+1) * sizeof(char));   // why plus one
		strcpy(strarr[count], t);
		t = strtok(NULL, " ");
		count++;
	}
	strarr[count] = NULL;  // initialize the last one to NULL
	*size = count;

	return strarr;
}

void freeInput(char **input) {
	int count = 0;
	while (input[count] != NULL) {
		//printf("\t#free each\n");
		free(input[count]);
		count++;
	}
	//printf("\t#free whole\n");
	free(input);
}

void freePath(struct pathelement *pathlist) {
	struct pathelement *tmp = NULL;
	while(pathlist != NULL) {
		tmp = pathlist;
		pathlist = pathlist->next;
		free(tmp);
	}
}


