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

   rt_t *rt = NULL;

   if (!(rt = rt_new ())) {
      XERROR ("Could not create runtime\n");
      goto errorexit;
   }

   token_t **tokens = token_read_file (TESTFILE);
   if (!tokens) {
      XERROR ("Unable to read tokens from [%s]\n", TESTFILE);
      goto errorexit;
   }

   size_t ntokens = 0;
   for (size_t i=0; tokens[i]; i++) {
      XERROR ("[%zu]: [%s]\n", i, token_string (tokens[i]));
      ntokens++;
   }

   size_t index = 0;
   while (index < ntokens) {
      atom_t *atom = NULL;
      atom_t *result = NULL;

      atom = parser_parse (tokens, &index);
      if (!atom) {
         XERROR ("Parser error near [%s]\n", token_string (tokens[index]));
         goto errorexit;
      }

      printf ("Printing atom:\n");
      atom_print (atom, 0, stdout);

      result = rt_eval (rt, NULL, atom);

      if (!result) {
         XERROR ("Eval error near [%s]\n", token_string (tokens[index]));
         atom_del (atom);
         goto errorexit;
      }
      printf ("RESULT:\n");
      atom_print (result, 0, stdout);
      printf (":END-RESULT\n");
      atom_del (result);
      atom_del (atom);
      result = NULL;
   }

   printf ("RUNTIME:\n");
   rt_print (rt, stdout);

   ret = EXIT_SUCCESS;

errorexit:

   rt_del (rt);

   for (size_t i=0; tokens && tokens[i]; i++) {
      token_del (tokens[i]);
   }
   free (tokens);

   xerror_set_logfile (NULL);

   return ret;
}

