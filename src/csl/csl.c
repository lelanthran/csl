#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "csl/csl.h"

#include "xstring/xstring.h"

struct csl_src_t {
   char    *fname;
   size_t   line;
   size_t   charpos;
   FILE    *inf;
};

csl_src_t *csl_src_load (const char *fname)
{
   bool error = true;
   csl_src_t *ret = NULL;

   if (!(ret = calloc (1, sizeof *ret)))
      goto errorexit;

   ret->fname = xstr_dup (fname);
   ret->line = 1;
   ret->charpos = 1;
   ret->inf = fopen (fname, "rt");

   if (!ret->fname || !ret->inf)
      goto errorexit;

   error = false;

errorexit:

   if (error) {
      csl_src_del (ret);
      ret = NULL;
   }

   return ret;
}

void csl_src_del (csl_src_t *csl)
{
   if (!csl)
      return;

   free (csl->fname);
   if (csl->inf)
      fclose (csl->inf);

   free (csl);
}

