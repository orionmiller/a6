#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "myutils.h"
#include "parselib.h"

int main(void) {

   mushStage front = parseLine(stdin);
   pid_t parent = getpid();
   int stage = 0;
   int status;
   

   if (parent == getpid()) {
      while (front) {
         fork();
         if (parent == getppid()) {
            printStage(front,stage);
            exit(EXIT_SUCCESS);
         }
         else {
            front = front->next;
            stage++;
         }
      }
      wait(&status);
      if (status != 0) {
         perror("program child exited with an error");
      }
   }

   return EXIT_SUCCESS;
}
