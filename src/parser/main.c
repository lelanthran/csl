

#include <stdio.h>
#include <stdlib.h>

#include "parser/parser.h"

#include "xerror/xerror.h"


#define TESTFILE     ("token/test_input.csl")

int main (void)
{
   int ret = EXIT_FAILURE;

   token_t **tokens = token_read_file (TESTFILE);
   if (!tokens) {
      XERROR ("Unable to read tokens from [%s]\n", TESTFILE);
      goto errorexit;
   }

   size_t ntokens = 0;
   for (size_t i=0; tokens[i]; i++) {
      printf ("[%zu] : [%s]\n", i, token_string (tokens[i]));
      ntokens++;
   }

   size_t index = 0;
   while (index < ntokens) {
      atom_t *atom = parser_parse (tokens, &index);
      printf ("[%zu]: ", index);
      atom_print (atom, 0, stdout);
      atom_del (atom);
   }

   ret = EXIT_SUCCESS;

errorexit:

   for (size_t i=0; tokens[i]; i++) {
      token_del (tokens[i]);
   }
   free (tokens);

   xerror_set_logfile (NULL);

   return ret;
}

