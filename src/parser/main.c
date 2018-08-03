

#include <stdio.h>
#include <stdlib.h>

#include "parser/parser.h"

#include "xerror/xerror.h"


#define TESTFILE     ("token/test_input.csl")

int main (void)
{
   int ret = EXIT_FAILURE;

   atom_t **atoms = NULL;

   token_t **tokens = token_read (TESTFILE);
   if (!tokens) {
      XERROR ("Unable to read tokens from [%s]\n", TESTFILE);
      goto errorexit;
   }

   if (!(atoms = parser_parse (tokens))) {
      XERROR ("Failed to parse tokens\n");
      goto errorexit;
   }

   for (size_t i=0; atoms[i]; i++) {
      atom_print (atoms[i], 1, stdout);
      atom_del (atoms[i]);
   }
   ll_del (atoms);

   ret = EXIT_SUCCESS;

errorexit:

   for (size_t i=0; tokens[i]; i++) {
      token_del (tokens[i]);
   }
   free (tokens);

   xerror_set_logfile (NULL);

   return ret;
}

