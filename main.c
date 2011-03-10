#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <signal.h>
#include <string.h>

#include "myutils.h"
#include "parselib.h"

#define READ_END 0
#define WRITE_END 1

void handler(int signum) {
  putc('\n', stdout); /*may change to stderr or soemthing else*/
}

/* uses the given a mushStage checks to see if command (argv[0]) is "cd"
 *    if so it will check to see if there are more than 2 argument if so it
 *    will print an error, it also does standard errno error checkin returned
 *    from chdir() functoin and prints appropiate error
 * on success returns stage->next, if the command wasn't "cd" it returns stage,
 *    on error returns NULL
 */
mushStage chngdir(mushStage stage) {
  if (stage && !strcmp("cd", stage->argv[0])) { /*not sure if it should be placed elsewhere*/
    if (stage->argc > 2) {
      fprintf(stderr, "cd: too many arguments.\n");
      fflush(stderr);
      return NULL;
    }
    if (-1 == chdir(stage->argv[1])) {
      perror("cd");
      return NULL;
    }
    return stage->next;
  }
  return stage;
}

/* Takes in a mushStage structure pointer and if the argv[0] is exit then exits
 * the current process
 */
void isExitTTY(mushStage stage) {
  if (stage && !strcmp("exit", stage->argv[0])) {
    exit(EXIT_SUCCESS);
  }
}

int main(int argc, char *argv[]) {

  mushStage front;
  mushStage temp;
  pid_t forkRes;
  int status;
  int **newPipe;
  char *inPath;
  char *outPath;
  FILE *in;
  FILE *out;
  FILE *fp = stdin;
  int stages = 1;
  int i;
  int pipe_num = 0;
  /*sigset_t mask;*/
  struct sigaction sa;
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  sigprocmask(SIG_SETMASK, &sa.sa_mask, NULL);

  if(-1 == sigaction(SIGINT, &sa, NULL)) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  /*   struct termios term;*/

  while (1) {

    if (argc > 2) {
      fprintf(stderr, "usage: %s [ scriptfile ]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
    if (argc > 1 && !(fp = fopen(argv[1], "r"))) {
      perror(argv[1]);
      exit(EXIT_FAILURE);
    }

    /*      isatty(STDIN_FILENO)
	    if(isatty(fileno(stdin))) {
	    perror("standard input is not a terminal device");
	    exit(EXIT_FAILURE);
	    }
    */
    if (argc == 1) {
      printf("8-P ");
      fflush(stdout);
    }
    
    inPath = NULL;
    outPath = NULL;
    in = NULL;
    out = NULL;


    /*generate stages*/
    front = parseLine(fp, &inPath, &outPath, &stages);
    if((temp = front)) {

      /*generate pipes*/
      if (stages > 1) {
	newPipe  = (int**) safe_malloc(sizeof(int**) * (stages - 1));
	for (i = 0; i < stages-1; i++) {
	  newPipe[i] = (int*) safe_malloc(sizeof(int*) * 2);
	  if (pipe(newPipe[i]) < 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
	  }
	}
      }
      
      if (inPath) {
	if (!(in = fopen(inPath, "r"))) {
	  perror("fopen inPath");
	  exit(EXIT_FAILURE);
	}
      }
      
      if (outPath) {
	if (!(out = fopen(outPath, "w+"))) {
	  perror("fopen outPath");
	  exit(EXIT_FAILURE);
	}
      }


      if (front->next) { /*more than 1 stage*/
	isExitTTY(temp);
	temp = chngdir(temp);
	while (temp && (forkRes=fork())) {
	  isExitTTY(temp->next);
	  if (temp != chngdir(temp)) {
            temp = chngdir(temp);
	  }
	  else {
            if(temp->next && front != temp) 
	      pipe_num++;
            temp = temp->next;
	  }
	}

	/*clean up pipes*/
	if (!forkRes) { /*child*/

	  if (temp == front) { /*first stage*/
            if (in) {
	      if ((dup2(fileno(in), STDIN_FILENO)) == -1)
		perror("1st stage dup2 in");
            }
            if ((dup2(newPipe[pipe_num][WRITE_END], STDOUT_FILENO)) == -1)
	      perror("1st stage dup2 out");
            if((close(newPipe[pipe_num][READ_END])) == -1)
	      perror("1st stage close READ_END");
            /*close other pipes*/
            for (i = 0; i < stages - 1; i++) {
	      if (i != pipe_num) {
		if((close(newPipe[i][READ_END])) == -1)
		  perror("stage close READ_END");
		if((close(newPipe[i][WRITE_END])) == -1)
		  perror("stage close WRITE_END");
	      }
            }
	  }
	  else if (temp->next == NULL) { /*final stage*/
            if (out) {
	      if ((dup2(fileno(out), STDOUT_FILENO)) == -1)
		perror("last stage dup2 in");
            }
            if ((dup2(newPipe[pipe_num][READ_END], STDIN_FILENO)) == -1)
	      perror("last stage dup2 out");
            if((close(newPipe[pipe_num][WRITE_END])) == -1)
	      perror("last stage close RITE_END");
            /*close other pipes*/
            for (i = 0; i < stages - 1; i++) {
	      if (i != pipe_num) {
		if((close(newPipe[i][READ_END])) == -1)
		  perror("stage close READ_END");
		if((close(newPipe[i][WRITE_END])) == -1)
		  perror("stage close WRITE_END");
	      }
            }

	  }
	  else { /*middle stages*/
            if ((dup2(newPipe[pipe_num][READ_END], STDIN_FILENO)) == -1)
	      perror("stage dup2 READ_END");
            if ((dup2(newPipe[pipe_num+1][WRITE_END], STDOUT_FILENO)) == -1)
	      perror("stage dup2 WRITE_END");
               
               
            if((close(newPipe[pipe_num][WRITE_END])) == -1)
	      perror("stage close WRITE_END 1");
            if((close(newPipe[pipe_num+1][READ_END])) == -1)
	      perror("stage close READ_END 2");
               
            for (i = 0; i < stages - 1; i++) {
	      if (i != pipe_num && i != pipe_num + 1) {
		if((close(newPipe[i][READ_END])) == -1)
		  perror("stage close READ_END 3");
		if((close(newPipe[i][WRITE_END])) == -1)
		  perror("stage close WRITE_END 4");
	      }
            }
	  }
	}
      }
      else { /*only one stage*/
	isExitTTY(temp);
	temp = chngdir(temp);
	if(temp && (forkRes=fork()))
	  temp = temp->next;
	if (!forkRes) {
	  if (in) {
            if ((dup2(fileno(in), STDIN_FILENO)) == -1)
	      perror("only 1 stage dup2 in");
	  }
	  if (out) {
            if ((dup2(fileno(out), STDOUT_FILENO)) == -1)
	      perror("only 1 stage dup2 out");
	  }
	}
      }
   
   
      /*execute*/
      if (temp) {
	execvp(temp->argv[0] ,temp->argv);
	perror(temp->argv[0]);
	exit(EXIT_FAILURE);
      }
      /*TODO: check for res error*/

      wait(&status);

      if (WIFEXITED(status) && WEXITSTATUS(status)) {
	perror("program child exited with an error");
      }

      if (in) {
	if((fclose(in)) != 0) {
	  perror("fclose in");
	}
      } 
      if (out) {
	if((fclose(out)) != 0) {
	  perror("fclose in");
	}
      }

      while(front) {
	free(front);
	front = NULL;
      }

      if (argc > 1) {
	exit(EXIT_SUCCESS);
      }
    }
  }
  return EXIT_SUCCESS;
}
