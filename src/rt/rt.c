#include <stdlib.h>
#include <stdbool.h>

#include "rt/rt.h"

rt_t *rt_new (void)
{
   bool error = true;
   rt_t *ret = NULL;

   if (!(ret = calloc (1, sizeof *ret)))
      goto errorexit;

   ret->symbols = atom_list_new ();
   ret->stack = atom_list_new ();
   ret->roots = atom_list_new ();
   error = false;

errorexit:

   if (error) {
      rt_del (ret);
      ret = NULL;
   }

   return ret;
}

void rt_del (rt_t *rt);
