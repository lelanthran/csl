
#include <stdio.h>
#include <stdlib.h>

#include "token/token.h"

#include "xerror/xerror.h"

#define TESTFILE     ("token/test_input.csl")

int main (void)
{
   int ret = EXIT_FAILURE;

   token_t **tokens = token_read (TESTFILE);
   if (!tokens) {
      XERROR ("Unable to read tokens from [%s]\n", TESTFILE);
      goto errorexit;
   }

   for (size_t i=0; tokens[i]; i++) {
      printf ("[%s:%zu,%zu] : [%s]\n", token_fname (tokens[i]),
                                       token_line (tokens[i]),
                                       token_charpos (tokens[i]),
                                       token_string (tokens[i]));
      token_del (tokens[i]);
   }
   free (tokens);

   ret = EXIT_SUCCESS;

errorexit:

   xerror_set_logfile (NULL);
   return ret;
}

