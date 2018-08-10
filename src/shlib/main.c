
#include <stdio.h>
#include <stdlib.h>


#include "xerror/xerror.h"

#include "shlib/shlib.h"

/* This test needs a library already created. It uses the functions in
 * testlib.c found in this directory to test the calling of library
 * functions.
 */
#define LIBRARY         ("libcsl")
#define TESTFUNC_ADD    ("testfunc_add")
#define TESTFUNC_SUB    ("testunc_sub")

int main (void)
{
   int ret = EXIT_FAILURE;

   shlib_t *sl = shlib_new ();

   if (!shlib_loadlib (sl, LIBRARY)) {
      XERROR ("Failed to load library [%s]\n", LIBRARY);
      goto errorexit;
   }

   ret = EXIT_SUCCESS;

errorexit:

   shlib_del (sl);

   xerror_set_logfile (NULL);

   return ret;
}
