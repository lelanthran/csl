#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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

void **ll_copy (void **src, size_t from_index, size_t to_index)
{
   size_t nitems;
   void **ret = ll_new ();
   bool error = true;

   if (!src)
      return NULL;

   nitems = ll_length (src);

   for (size_t i=from_index; i>=from_index && i<to_index && i<nitems; i++) {
      if (!(ll_ins_tail (&ret, src[i])))
         goto errorexit;
   }

   error = false;

errorexit:

   if (error) {
      ll_del (ret);
      ret = NULL;
   }

   return ret;
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

   for (size_t count = 0; ll[count]; count++) {
      if (count==i) {
         return ll[i];
      }
   }

   return NULL;
}

void ll_iterate (void **ll, void (*fptr) (void *))
{
   if (!ll || !fptr)
      return;

   for (size_t i=0; ll[i]; i++) {
      fptr (ll[i]);
   }
}


void *ll_ins_tail (void ***ll, void *el)
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

void *ll_ins_head (void ***ll, void *el)
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

   memmove (&(*ll)[1], &(*ll)[0], nitems + 1);
   (*ll)[0] = el;

   return (*ll)[nitems];
}

void *ll_remove (void ***ll, size_t index)
{
   if (!ll || !*ll)
      return NULL;

   size_t len = ll_length (*ll);
   if (index > len)
      return NULL;

   void *ret = (*ll)[index];

   memmove (&(*ll)[index], &(*ll)[index + 1],
            (sizeof (void *)) * (len - index));

   return ret;
}
