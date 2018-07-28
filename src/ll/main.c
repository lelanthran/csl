
#include <stdio.h>
#include <stdlib.h>

#include "ll/ll.h"

#include "xerror/xerror.h"
#include "xstring/xstring.h"

int main (void)
{
   int ret = EXIT_FAILURE;

   static const char *tstrings[] = {
      "one", "two", "three", "four", "five", "six",
      "seven", "eight", "nine", "ten",
   };

   const size_t tstrings_len = sizeof tstrings/sizeof tstrings[0];

   void **ll = ll_new ();

   if (!ll) {
      XERROR ("Unable to create new list\n");
      goto errorexit;
   }

   for (size_t i=0; i<tstrings_len; i++) {
      char *tmp = xstr_dup (tstrings[i]);
      if (!tmp) {
         XERROR ("Out or memory allocating [%s]\n", tstrings[i]);
         goto errorexit;
      }

      if (!(ll_ins_tail (&ll, tmp))) {
         XERROR ("Failed to append [%s] to ll\n", tmp);
         goto errorexit;
      }

      printf ("Stored [%s]\n", tmp);
   }

   ret = EXIT_SUCCESS;

   size_t nitems;

errorexit:

   nitems = ll_length (ll);

   for (size_t i=0; i<nitems; i++) {
      char *tmp = ll_index (ll, i);
      printf ("Freeing [%s]\n", tmp);
      free (tmp);
   }

   ll_del (ll);

   xerror_set_logfile (NULL);

   return ret;
}

