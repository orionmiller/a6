/* Contains the public functions, constants, and structures, to the parselib
 * library
 */

#include <stdio.h>
#include "myutils.h"

#ifndef PARSELIB_H
#define PARSELIB_H


typedef enum mush_token {
   NORMAL_TOKEN,
   IN_REDIR,
   OUT_REDIR,
   PIPE,
   END_OF_LINE
} mushToken;


typedef struct mush_stage {
   /*char *in, *out;*/
   char *command;
   int argc;
   char **argv;
   /*maybe add what stage it is*/
   struct mush_stage *next;
} *mushStage;


/*change to a linked list*/

#define TRUE 1
#define FALSE 0


#define DEFAULT_COMMAND_LENGTH 512
#define DEFAULT_NUM_ARGS 20

/*definines what is considered a non normal token character*/
/*const char *paranormal = " <>|\n";*/


/* Returns the type of the next token
 * line is the starting point of the line to find the next token
 * **endToken is the the beginning is to be used as the pointer 
 *    end of the token
 * precondition: you cannot pass a space as the first character of line
 * post-condition: endToken points to end of the token
 */
mushToken nextToken(char *line, char **end);

/* parseLine takes in a FILE pointer and returns a pointer to a list
 * of mushStage types and NULL on return
 */
mushStage parseLine(FILE *fp, char **in, char **out, int *stages);


/* Determines if a character is a normal token or not
 * returns TRUE (1) if it maches the character matches a character in the 
 * paranormal string and returns FALSE (0) otherwise
 */
int isNormalTokenChar (char c);


/* prints the designated stage and it's number specified by num to stdout
 */
void printStage(mushStage stage, int num);

/* Checks to see if the character is a delimiter 
 * returns TRUE (1) if true and FALSE (0) otherwise
 * the function contains a string determing what a delimiter is
 * currently only spaces are delimiters
 */
int isDelim(char c);

/* Initialises all the values in the stage struct
 * returns the pointer given by stage paramater
 */
mushStage initMushStage(mushStage stage);



#endif
