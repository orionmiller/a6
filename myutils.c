/* Contains all the code to the myutils library
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include "myutils.h"

void *safe_malloc(size_t size) {
   void *out = malloc(size);
   if (out)
      return out;
   perror("malloc_safe");
   exit(errno);
}

void *safe_realloc(void *p, size_t size) {
   if (!(p = realloc(p, size))) {
      perror("safe_realloc");
      exit(errno);
   }
   return p;
}

char *read_long_line(FILE *file) {
   int c;
   size_t i, size = DEFAULT_SIZE;
   char *buff = safe_malloc(size * sizeof *buff);
   for (i = 0; (c = fgetc(file)) != EOF && c != '\n'; i++) {
      if (i >= size) {
         size = size*2;
         buff = safe_realloc(buff, size);
      }
      buff[i] = (char)c;
   }
   buff[i] = '\0';
   return buff;
}

void *clean_malloc(size_t size) {
   void *out = safe_malloc(size);
   bzero(out, size);
   return out;
}

/*has a static character to keep concatinating onto string*/
/*change name to long_line_cat*/
char *long_line_cat(char *dst, char *src, int option, int *size) {
   int used_len;

   if (option == CLEAN) {
      dst = (char *) clean_malloc(*size);
   }
   used_len = strlen(dst);

   for (; src && *src; src++, used_len++) {
      if (used_len >= *size * 0.5) {
         *size *= 2;
         dst = safe_realloc(dst, *size);
      }
      dst[used_len] = *src;
   }

   dst[used_len] = '\0';

   return dst;
}


char *line_to_word(char *string, char *prefix, int size) {
   int line_len = sizeof(char) * DEFAULT_SIZE;
   char *line = (char *) clean_malloc(line_len);
   int used_len = 0;
   int i = 0;

   used_len = strlen(line);

   for (; prefix && *prefix; prefix++, used_len++) {
      if (used_len >= line_len * 0.5) {
         line_len = line_len * 2;
         line = safe_realloc(line, line_len);
      }
      line[used_len] = *prefix;
   }

   for (; string && *string && i <= size; string++, used_len++, i++) {
      if (used_len >= line_len * 0.5) {
         line_len = line_len * 2;
         line = safe_realloc(line, line_len);
      }
      line[used_len] = *string;
   }

   line[used_len] = '\0';

   return line;
}

char *short_line(char *string, char *prefix) { 
   int line_len = sizeof(char) * SHORT_SIZE;
   char *line = (char *) clean_malloc(line_len);
   int used_len = 0;
   
   used_len = strlen(line);

   for (; *prefix; prefix++, used_len++) {
      if (used_len >= line_len * 0.5) {
         line_len = line_len * 2;
         line = safe_realloc(line, line_len);
      }
      line[used_len] = *prefix;
   }

   for (; *string; string++, used_len++) {
      if (used_len >= line_len * 0.5) {
         line_len = line_len * 2;
         line = safe_realloc(line, line_len);
      }
      line[used_len] = *string;
   }

   line[used_len] = '\0';

   return line;
}

/*needs to be tested*/
char *itoa(int num) {
   int x = 1000000000;
   char *string = (char *) safe_malloc(sizeof(char)*11); 
   /*num of digits in x + '\0' + 1 magic number*/
   int i = 0;
   bzero(string, 11);

   while ( (x / 10) >= 1 ) {
      string[i] = ((num % (x) - (num % (x / 10)))/(x / 10)) + 48;
      i++;
      x /= 10;
   }
   i = 0;
   while (string[i] && string[i] <= '0' &&  i < 8 ) /*magic number*/
      i++;
   return string+i;
}
