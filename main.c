#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "myutils.h"
#include "parselib.h"

#define READ_END 0
#define WRITE_END 1

int main(void) {

   mushStage front;
   mushStage temp;
   pid_t forkRes;
   int status;
   int **newPipe;
   char *inPath;
   char *outPath;
   FILE *in = NULL;
   FILE *out = NULL;
   int stages = 1;
   int i;
   int pipe_num = 0;

   fprintf(stdout, ": ");
   fflush(stdout);
   inPath = NULL;
   outPath = NULL;
   /*generate stages*/
   front = parseLine(stdin, &inPath, &outPath, &stages);
   if(!(temp = front)) {
      exit(EXIT_FAILURE);
   }
   /*modify for future use*/


   /*DEBUG: Info*/
   printf("inpath: %s\n", inPath);

   printf("outpath: %s\n", outPath);

   printf("num of pipes: %d\n", stages -1);
   /*      i = 0;
           while (temp) {
           i++;
           printStage(temp, i);
           temp = temp->next;
           }

           temp = front;
   */
   fflush(stdout);
   /*-----------*/

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
      if (!(out = fopen(outPath, "w"))) {
         perror("fopen outPath");
         exit(EXIT_FAILURE);
      }
   }


   if (front->next) { /*more than 1 stage*/
      while (temp && !(forkRes=fork())) {
         temp = temp->next;
      }
      /*clean up pipes*/
      fprintf(stderr, "\"%s\": forkRes: %d\n", temp->argv[0], forkRes);
      fflush(stderr);
      if (forkRes) { /*child*/
         fprintf(stderr, "%s: cleaning pipes\n", temp->argv[0]);
         fflush(stderr);
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
                  printf("i: %d pipe_num: %d \n", i, pipe_num);
                  fflush(stdout);
                  if((close(newPipe[i][READ_END])) == -1)
                     perror("stage close READ_END 3");
                  if((close(newPipe[i][WRITE_END])) == -1)
                     perror("stage close WRITE_END 4");
               }
            }
            pipe_num++;
         }
      }
   }
   else { /*only one stage*/
      if (in) {
         if ((dup2(fileno(in), STDIN_FILENO)) == -1)
            perror("only 1 stage dup2 in");
      }
      if (out) {
         if ((dup2(fileno(out), STDOUT_FILENO)) == -1)
            perror("only 1 stage dup2 out");
      }
   }

   /*execute*/
   if (temp) {
      execv(temp->argv[0] ,temp->argv);
      perror(temp->argv[0]);
      exit(EXIT_FAILURE);
   }

   /*TODO: check for res error*/


   wait(&status);
   if (WIFEXITED(status) && WEXITSTATUS(status)) {
      perror("program child exited with an error");
   }
   fprintf(stderr, "parent finished wait\n");
   fflush(stderr);
   fprintf(stdout, "parent finished wait\n");
   fflush(stdout);
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
   in = NULL;
   out = NULL;
      
   fflush(stdin);
      
   return EXIT_SUCCESS;
}
