/* Constains all the code to the parselib library
 */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "myutils.h"
#include "parselib.h"

void parse(char **start, char ***end, mushToken *token, char **word);

mushToken nextToken(char *start, char **end) {

   /*possibly change to function*/
   switch (*start) 
      {
      case '\0':
         *end = start;
         return END_OF_LINE;
      case '<':
         *end = start + 1;
         return IN_REDIR;
      case '>':
         *end = start + 1;
         return OUT_REDIR;
      case '|':
         *end = start + 1;
         return PIPE; 
      }
   
   while (isNormalTokenChar(*start))
      start++;

   if(end)
      *end = start; /*used to be *end*/
   
   return NORMAL_TOKEN;
}


int isNormalTokenChar (char c) {
   const char *paranormal = " <>|\n"; /*cannot compile ith paranormal in .h*/
   int i;
   for (i = 0; paranormal[i] && c != paranormal[i]; i++)
      /*do nothing*/;

   return paranormal[i] != c;
}

mushStage parseLine(FILE *fp) {
   char *start = read_long_line(fp);
   char *word = (char *) safe_malloc(DEFAULT_SIZE);
   mushStage s = (mushStage) safe_malloc(sizeof(struct mush_stage));
   mushStage front = s;
   char **end = (char **) safe_malloc(sizeof(char**));
   mushToken token = END_OF_LINE;
   int stage = 0;
   char *exec = NULL; /*bad name*/
   int bool = 1;
   int in = 0;
   int out = 0;
   const int REDIR_T = 1;
   const int PIPE_T = 2;
   int max_argc = DEFAULT_NUM_ARGS;
   int size = DEFAULT_SIZE;   
   s = initMushStage(s);

   do {
      parse(&start, &end, &token, &word);
      if (bool) {
         exec = word;
         bool = FALSE;
      }
      switch (token)
         {
         case NORMAL_TOKEN:
            s->command = long_line_cat(s->command, word, SAME, &size);
            s->command = long_line_cat(s->command, " ", SAME, &size); 
                                           /*stop ending in a space*/
            s->argv[s->argc] = word;
            if (s->argc >= max_argc * (.05)) {
               max_argc *= 2;
               *(s->argv) = (char *) safe_realloc(*(s->argv), 
                  sizeof(char *) * max_argc);
            }
            s->argc += 1;
            break;

         case OUT_REDIR:
            s->command = long_line_cat(s->command, word, SAME, &size);
            s->command = long_line_cat(s->command, " ", SAME, &size);
            parse(&start, &end, &token, &word);
            s->command = long_line_cat(s->command, word, SAME, &size);
            s->command = long_line_cat(s->command, " ", SAME, &size);
            if (token != NORMAL_TOKEN) {
               fprintf(stderr, "%s: bad out redirection\n", exec);
               exit(EXIT_FAILURE);
            }

            if (out == REDIR_T || out == PIPE_T) {
               fprintf(stderr, "%s: bad out redirection\n", exec);
               exit(EXIT_FAILURE);
            }

            s->out = (char *) safe_malloc(strlen(word)+1); /*+1 for null*/
            snprintf(s->out, strlen(word)+1, "%s", word);
            in = REDIR_T;
            break;

         case IN_REDIR:
            s->command = long_line_cat(s->command, word, SAME, &size);
            s->command = long_line_cat(s->command, " ", SAME, &size);
            parse(&start, &end, &token, &word);
            s->command = long_line_cat(s->command, word, SAME, &size);
            s->command = long_line_cat(s->command, " ", SAME, &size);

            if (token != NORMAL_TOKEN) {
               fprintf(stderr, "%s: bad input redirection\n", exec);
               exit(EXIT_FAILURE);
            }
            if (in == REDIR_T || in == PIPE_T) {
               fprintf(stderr, "%s: bad input redirection\n", exec);
               exit(EXIT_FAILURE);
            }

            s->in = (char *) safe_malloc(strlen(word)+1); /*+1 for null*/
            snprintf(s->in, strlen(word)+1, "%s", word);
            in = REDIR_T;
            break;


         case PIPE:
            parse(&start, &end, &token, &word);
            if (token != NORMAL_TOKEN) {
               fprintf(stderr, "ivalid null argument\n");
               exit(EXIT_FAILURE);
            }
            if (in == REDIR_T) {
               fprintf(stderr, "%s: ambiguous input\n", exec);
               exit(EXIT_FAILURE);
            }
            if (out == REDIR_T) {
               fprintf(stderr, "%s: ambiguous output\n", exec);
            }

            if (!(s->in)) {
               s->in = (char *) safe_malloc(sizeof("original stdin"));
               snprintf(s->in, sizeof("original stdin"), "%s", 
                        "original stdin");
            }

            exec = word;
            s->out = short_line(itoa(stage+1), "pipe to stage ");
            s->next = (mushStage) safe_malloc(sizeof(struct mush_stage));
            s->next = initMushStage(s->next);
            s->next->in = short_line(itoa(stage), "pipe from stage ");
            s = s->next;
            size = DEFAULT_SIZE;
            s->command = long_line_cat(s->command, word, CLEAN, &size);
            s->argv[s->argc] = word;
            s->argc += 1;
            stage++;
            in = PIPE_T;
            out = PIPE_T;
            break;

         case END_OF_LINE:
            if (!(*(s->command))) {
               fprintf(stderr, "invalid null command\n");
               exit(EXIT_FAILURE);
            }
            if (!(s->in)) {
               s->in = (char *) safe_malloc(sizeof("original stdin"));
               snprintf(s->in, sizeof("original stdin"), "%s", 
                        "original stdin");
            }
            if (!(s->out)) {
               s->out = (char *) safe_malloc(sizeof("original stdout"));
               snprintf(s->out, sizeof("original stdout"), "%s", 
                        "original stdout");
            }
            break;

         default: /*may want to remove*/
            fprintf(stderr, "error handling tokens");
            exit(EXIT_FAILURE);
            break;
         }
   
      /*add to command*/

   }while(token != END_OF_LINE);


   return front;
}

void parse(char **start, char ***end, mushToken *token, char **word) {
   while (*start && **start && isDelim(**start))
      (*start)++;
   *token = nextToken(*start, *end);
   *word = line_to_word(*start, "", (**end-1)-*start); 
        /*-1 to not copy what has been*/
   *start = **end;
}


int isDelim(char c) {
   switch (c)
      {
      case ' ':
         return TRUE;
      default:
         return FALSE;
      }
}

mushStage initMushStage(mushStage stage) {
   stage->in = NULL;
   stage->out = NULL;
   stage->command = (char *) clean_malloc(sizeof(char) * DEFAULT_SIZE);
   stage->argc = 0;
   stage->argv = (char **) safe_malloc(sizeof(char **));
   *(stage->argv) = (char *) safe_malloc(sizeof(char *)* DEFAULT_NUM_ARGS);
   stage->next = NULL;
   return stage;
}


void printStage(mushStage stage, int num) {
   int i;
   printf("--------\n");
   printf("Stage %d: \"%s\"\n", num, stage->command);
   printf("--------\n");
   printf("input: %s \n", stage->in);
   printf("output: %s\n", stage->out);
   printf("argc: %d\n", stage->argc);
   printf("argv: ");
   for (i = 0; i < stage->argc; i++) {
      printf("\"%s\"", stage->argv[i]);
      if (i+1 < stage->argc) {
         printf(",");
         fflush(stdout);
      }
   }
   printf("\n\n");
}



