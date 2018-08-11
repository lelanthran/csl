
#include <stdio.h>
#include <stdlib.h>


#include "xerror/xerror.h"

#include "shlib/shlib.h"

/* This test needs a library already created. It uses the functions in
 * testlib.c found in this directory to test the calling of library
 * functions.
 */
#define LIBRARY         ("libcsl")
#define TESTFUNC_ADD    ("testlib_add")
#define TESTFUNC_SUB    ("testlib_sub")

typedef int (fptr_t) (int, int);

int main (void)
{
   int ret = EXIT_FAILURE;
   fptr_t *t1 = NULL,
          *t2 = NULL;

   shlib_t *sl = shlib_new ();

   if (!(t1 = (fptr_t *)shlib_loadfunc (sl, TESTFUNC_ADD, LIBRARY))) {
      XERROR ("Failed to load library [%s]\n", LIBRARY);
      goto errorexit;
   }

   if (!(t2 = (fptr_t *)shlib_loadfunc (sl, TESTFUNC_SUB, LIBRARY))) {
      XERROR ("Failed to load library [%s]\n", LIBRARY);
      goto errorexit;
   }

   printf ("r1: %i\n", t1 (3, 4));
   printf ("r2: %i\n", t2 (3, 4));

   ret = EXIT_SUCCESS;

errorexit:

   shlib_del (sl);

   xerror_set_logfile (NULL);

   return ret;
}
