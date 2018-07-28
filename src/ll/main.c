
#include <stdio.h>
#include <stdlib.h>

#include "ll/ll.h"

#include "xerror/xerror.h"
#include "xstring/xstring.h"

static void printstr (void *string)
{
   printf (" Printing [%s]\n", (char *)string);
}

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

   ll_iterate (ll, free);
   ll_del (ll);
   ll = ll_new ();

   for (int i=tstrings_len-1; i>0; i--) {
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

   ll_iterate (ll, printstr);

   ret = EXIT_SUCCESS;

errorexit:

   ll_iterate (ll, free);
   ll_del (ll);

   xerror_set_logfile (NULL);

   return ret;
}

