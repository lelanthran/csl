#include <stdio.h>
#include <stdlib.h>

#include "parser/parser.h"
#include "parser/atom.h"
#include "rt/rt.h"

#include "xerror/xerror.h"


#define TESTFILE     ("rt/test_input.csl")

int main (void)
{
   int ret = EXIT_FAILURE;

   atom_t *parser = parser_new ();
   rt_t *rt = NULL;

   if (!(rt = rt_new ())) {
      XERROR ("Could not create runtime\n");
      goto errorexit;
   }

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


   ret = EXIT_SUCCESS;

errorexit:

   parser_del (parser);
   rt_del (rt);

   for (size_t i=0; tokens && tokens[i]; i++) {
      token_del (tokens[i]);
   }
   free (tokens);

   xerror_set_logfile (NULL);

   return ret;
}

