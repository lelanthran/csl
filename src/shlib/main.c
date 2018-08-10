
#include <stdio.h>
#include <stdlib.h>


#include "shlib/shlib.h"

/* This test needs a library already created. It uses the functions in
 * testlib.c found in this directory to test the calling of library
 * functions.
 */
int main (void)
{
   int ret = EXIT_FAILURE;

   shlib_t *sl = shlib_new ();

   ret = EXIT_SUCCESS;

errorexit:

   shlib_del (sl);

   return ret;
}
