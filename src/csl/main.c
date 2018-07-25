#include <stdio.h>
#include <stdlib.h>

#include "csl/csl.h"


#include "xerror/xerror.h"

#define TESTFILE       ("csl/test_input.csl")

int main (void)
{
   int ret = EXIT_FAILURE;

   csl_src_t *srctree = csl_src_load (TESTFILE);

   if (!srctree) {
      XERROR ("Failed to open [%s]:%m\n", TESTFILE);
      goto errorexit;
   }

   ret = EXIT_SUCCESS;

errorexit:

   csl_src_del (srctree);

   xerror_set_logfile (NULL);

   return ret;
}

