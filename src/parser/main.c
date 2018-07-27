

#include <stdio.h>
#include <stdlib.h>

#include "parser/parser.h"

#include "xerror/xerror.h"


#define TESTFILE     ("token/test_input.csl")

int main (void)
{
   int ret = EXIT_FAILURE;

   atom_t *parser = parser_new ();

   token_t **tokens = token_read (TESTFILE);
   if (!tokens) {
      XERROR ("Unable to read tokens from [%s]\n", TESTFILE);
      goto errorexit;
   }

   if (!parser) {
      XERROR ("Unable to create parse-tree\n");
      goto errorexit;
   }

   if (!parser_parse (parser, tokens)) {
      XERROR ("Failed to parse tokens\n");
      goto errorexit;
   }

   parser_print (parser, 1, stdout);

#if 0
   for (size_t i=0; tokens[i]; i++) {
      printf ("[%s:%zu,%zu] %i : [%s]\n", token_fname (tokens[i]),
                                          token_line (tokens[i]),
                                          token_charpos (tokens[i]),
                                          token_type (tokens[i]),
                                          token_string (tokens[i]));
      token_del (tokens[i]);
   }
   free (tokens);
#endif

   ret = EXIT_SUCCESS;

errorexit:

   parser_del (parser);
   for (size_t i=0; tokens[i]; i++) {
      token_del (tokens[i]);
   }
   free (tokens);

   xerror_set_logfile (NULL);

   return ret;
}

