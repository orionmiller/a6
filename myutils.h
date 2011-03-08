/* Public functions of the myutils library
 */

#include <stdlib.h>
#include <stdio.h>

#ifndef MYUSILS_H
#define MYUTILS_H

#define DEFAULT_SIZE 512
#define SHORT_SIZE 20
#define TRUE 1
#define FALSE 0

#define CLEAN 1
#define SAME -1

/* Wrapper function to safely allocate memory using
 *   malloc()
 * exits on error
 */
void *safe_malloc(size_t size);


/* Wrapper function to safely reallocate memory using
 *   realloc()
 * exits on error
 */
void *safe_realloc(void *p, size_t size);

/* Takes in a file pointer and generates a string the string terminates 
 * on a newline or null character
 * uses safe_malloc() and exits on error
 */
char *read_long_line(FILE *file);


/* Concatinates the src string onto the destination string and uses
 * safe_mallooc
 * options is either constant SAME or CLEAN
 *    SAME
 *    which doesn't change the memory address but reallocates
 *    if used space is >= have the size
 *
 *    CLEAN
 *    clean_malloc a new buffer using the designated size
 * returns the address to the begginning of allocated memmory
 */
char *long_line_cat(char *dst, char *src, int option, int *size);

/* Uses safe_malloc() to allocate the specified size of memory from
 * parameter size and sets all values in the memory to null
 */
void *clean_malloc(size_t size);

/*
 */
char *line_to_word(char *string, char *prefix, int size);

/*Generates a string by concatinating a the given string ont a given prefix
 * string
 */
char *short_line(char *string, char *prefix);

/* Converts an integer to a string (base 10)*/
char *itoa(int num);

#endif
