#include <stdlib.h>

#include "ll/ll.h"


void **ll_new (void)
{
   return calloc (2, sizeof (void **));
}

void ll_del (void **ll)
{
   if (!ll)
      return;

   free (ll);
}

size_t ll_length (void **ll)
{
   if (!ll)
      return 0;

   size_t ret = 0;
   for (ret=0; ll[ret]; ret++)
      ;

   return ret;
}

void *ll_index (void **ll, size_t i)
{
   if (!ll)
      return NULL;

   return ll[i];
}

void *ll_ins_tail (void ***ll, const void *el)
{
   size_t nitems = 0;

   if (!ll || !(*ll) || !el)
      return NULL;

   nitems = ll_length (*ll);
   size_t newsize = nitems + 2;

   void **tmp = realloc ((*ll), newsize * sizeof (void *));
   if (!tmp)
      return NULL;

   (*ll) = tmp;

  (*ll)[nitems] = el;
  (*ll)[nitems + 1] = 0;

   return (*ll)[nitems];
}

