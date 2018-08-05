
#include <stdio.h>
#include <stdlib.h>

#include "token/token.h"

#include "xerror/xerror.h"
#include "xstring/xstring.h"

#define TESTFILE     ("token/test_input.csl")

int main (void)
{
   int ret = EXIT_FAILURE;

   char *input = xstr_readfile (TESTFILE);
   if (!input) {
      XERROR ("Unable to read file [%s]\n", TESTFILE);
      return EXIT_FAILURE;
   }

   char *tmp = input;
   token_t **tokens = token_read_string (&tmp, TESTFILE);

   if (!tokens) {
      XERROR ("Unable to read tokens from [%s]\n", TESTFILE);
      goto errorexit;
   }

   for (size_t i=0; tokens[i]; i++) {
      printf ("[%s:%zu,%zu] %i : [%s]\n", token_fname (tokens[i]),
                                          token_line (tokens[i]),
                                          token_charpos (tokens[i]),
                                          token_type (tokens[i]),
                                          token_string (tokens[i]));
      token_del (tokens[i]);
   }
   free (tokens);

   ret = EXIT_SUCCESS;

errorexit:

   free (input);

   xerror_set_logfile (NULL);
   return ret;
}

