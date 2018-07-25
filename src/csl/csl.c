#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "csl/csl.h"

#include "xstring/xstring.h"
#include "xvector/xvector.h"

typedef struct srcnode_t srcnode_t;

struct csl_src_t {
   char    *fname;
   size_t   line;
   size_t   charpos;
   FILE    *inf;
};

#define TYPE_START      1
#define TYPE_END        2
#define TYPE_QUOTE      3
#define TYPE_STRING     4
#define TYPE_LIST       5
struct srcnode_t {
   int type;
   union {
      xvector_t *children;
      char *token;
   } data;
   char    *fname;
   size_t   line;
   size_t   charpos;
   srcnode_t *parent;
};

void srcnode_del (srcnode_t *sn)
{
   if (!sn)
      return;

   free (sn->fname);
   if (sn->type==TYPE_LIST) {
      xvector_iterate (sn->data.children, (void (*) (void *))srcnode_del);
      xvector_free (sn->data.children);
   } else {
      free (sn->data.token);
   }

   free (sn);
}

srcnode_t *srcnode_new (csl_src_t *csl, srcnode_t *parent, int type, void *d)
{
   bool error = true;
   srcnode_t *ret = NULL;

   if (!(ret = calloc (1, sizeof *ret)))
      goto errorexit;

   ret->fname = xstr_dup (csl->fname);
   ret->line = csl->line;
   ret->charpos = csl->charpos;
   ret->parent = parent;

   if (type==TYPE_LIST) {
      ret->data.children = (xvector_t *)d;
   } else {
      ret->data.token = xstr_dup ((char *)d);
      if (!ret->data.token)
         goto errorexit;
   }

   error = false;

errorexit:
   if (error) {
      srcnode_del (ret);
      ret = NULL;
   }
   return ret;
}

static bool is_single_char_token (int c)
{
   static const char *schars = "'+-/*=()";
   return strchr (schars, (char)c) ? true : false;
}

static int get_next_char (csl_src_t *csl, FILE *inf)
{
   int ret = EOF;

   if (ferror (inf) || feof (inf))
      return EOF;

   ret = fgetc (inf);

   csl->charpos++;

   if (ret=='\n') {
      csl->charpos = 0;
      csl->line++;
   }

   return ret;
}

static char *get_next_token (csl_src_t *csl, FILE *inf)
{
   int c;
   char temps[1024];

   memset (temps, 0, sizeof temps);

   // Skip whitespace
   do {
      c = get_next_char (csl, inf);
   } while (c!=EOF && isspace (c));

   temps[0] = (char)c;

   // Read token - certain tokens are single-character tokens so we
   // process them first (there may be no spaces between them and the next
   // token).
   if (is_single_char_token (temps[0]))
      return xstr_dup (temps);

   // Read token
   size_t i=1;
   c = get_next_char (csl, inf);
   while (c!=EOF && !(isspace (c)) && !is_single_char_token (c)) {
      temps[i++] = (char)c;
      c = get_next_char (csl, inf);
   }

   return xstr_dup (temps);
}

static srcnode_t *make_srcnode (csl_src_t *csl, FILE *inf)
{
   srcnode_t *ret = NULL;
   char *str = NULL;
   xvector_t *children;

   str = get_next_token;

   return ret;
}

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

