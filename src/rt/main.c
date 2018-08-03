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

   atom_t **atoms = NULL;

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

   if (!(atoms = parser_parse (tokens))) {
      XERROR ("Failed to parse tokens\n");
      goto errorexit;
   }

   for (size_t i=0; atoms[i]; i++) {
      atom_print (atoms[i], 1, stdout);
      atom_t *result = rt_eval (rt, NULL, atoms[i]);
      if (!result) {
         XERROR ("Failed to execute expression\n");
         goto errorexit;
      }
      printf ("RESULT:\n");
      atom_print (result, 0, stdout);
      printf (":END-RESULT\n");
      atom_del (result);
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

   for (size_t i=0; atoms[i]; i++) {
      atom_del (atoms[i]);
   }
   ll_del (atoms);

   xerror_set_logfile (NULL);

   return ret;
}

