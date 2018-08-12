
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
#define TESTFUNC_BIG    ("testlib_big")

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

   uint64_t result = 0;

   uint8_t i8 = 0xa5;
   uint64_t i64 = 0xf1f2f3f4f5f6f7f8L;
   double d64 = 3.1459 / 3.33;
   char *string = "My Test String (More Than Eight Bytes)";

   struct shlib_pair_t args[] = {
      { shlib_POINTER, &string },
      { shlib_UINT8_T, &i8     },
      { shlib_UINT64_T,&i64    },
      { shlib_DOUBLE,  &d64    },
      { 0,  NULL    },
   };

   int errcode = shlib_funcall (sl, TESTFUNC_BIG, LIBRARY,
                                shlib_UINT64_T, &result,
                                args);

   if (errcode) {
      XERROR ("Function call failed with [%i]\n", errcode);
      goto errorexit;
   }

   ret = EXIT_SUCCESS;

errorexit:

   shlib_del (sl);

   xerror_set_logfile (NULL);

   return ret;
}
