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
   int      last_char;
   srcnode_t *root;
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

   printf ("[%p] Entering \n", sn);
   free (sn->fname);
   if (sn->type==TYPE_LIST) {
      printf ("[%p] Iterating \n", sn);
      xvector_iterate (sn->data.children, (void (*) (void *))srcnode_del);
      xvector_free (sn->data.children);
   } else {
      free (sn->data.token);
   }

   printf ("[%p] Leaving \n", sn);
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
   ret->type = type;

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

   if (csl->last_char && csl->last_char!=EOF) {
      ret = csl->last_char;
      csl->last_char = 0;
      return ret;
   }

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

static void push_back_char (csl_src_t *csl, int c)
{
   csl->last_char = c;
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

   if (c!=EOF)
      push_back_char (csl, c);

   return xstr_dup (temps);
}

static bool make_srcnode (csl_src_t *csl, FILE *inf, srcnode_t *parent);

static void read_list (csl_src_t *csl, FILE *inf, srcnode_t *parent)
{
   parent->data.children = xvector_new ();

   while (1) {
      if (!make_srcnode (csl, inf, parent))
         break;
   }

}

static bool make_srcnode (csl_src_t *csl, FILE *inf, srcnode_t *parent)
{
   char *str = NULL;

   str = get_next_token (csl, inf);
   if (!str || *str==')') {
      free (str);
      return false;
   }

   if (*str == '(') {
      srcnode_t *child = srcnode_new (csl, parent, TYPE_LIST, xvector_new ());
      read_list (csl, inf, child);
      xvector_ins_tail (parent->data.children, child);
   } else {
      srcnode_new (csl, parent, TYPE_STRING, str);
   }

   free (str);

   return true;
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

   ret->root = srcnode_new (ret, NULL, TYPE_LIST, xvector_new ());

   while (!feof (ret->inf) && !ferror (ret->inf)) {
      make_srcnode (ret, ret->inf, ret->root);
   }

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

   srcnode_del (csl->root);

   free (csl);
}

