#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "parser/atom.h"

#include "xvector/xvector.h"
#include "xerror/xerror.h"
#include "xstring/xstring.h"

typedef atom_t *(atom_newfunc_t) (atom_t *dst, const char *);
typedef void (atom_delfunc_t) (atom_t *);
typedef void (atom_prnfunc_t) (atom_t *, size_t, FILE *);

static atom_t *atom_new_list (atom_t *dst, const char *str)
{
   str = str;
   dst->data = xvector_new ();
   return dst;
}

static atom_t *atom_new_string (atom_t *dst, const char *str)
{
   dst->data = xstr_dup (str);
   return dst;
}

static atom_t *atom_new_int (atom_t *dst, const char *str)
{
   bool error = true;
   atom_t *ret = NULL;

   dst->data = malloc (sizeof (int64_t));
   if (!dst->data)
      return NULL;

   int64_t tmp;
   if (sscanf (str, "%" PRIi64, &tmp)!=1) {
      XERROR ("[%s] is not an integer\n", str);
      goto errorexit;
   }

   *(int64_t *)dst->data = tmp;

   ret = dst;
   error = false;

errorexit:

   if (error) {
      free (dst->data);
      ret = NULL;
   }

   return ret;
}

static atom_t *atom_new_float (atom_t *dst, const char *str)
{
   bool error = true;
   atom_t *ret = NULL;

   dst->data = malloc (sizeof (int64_t));
   if (!dst->data)
      return NULL;

   double tmp;
   if (sscanf (str, "%lf", &tmp)!=1) {
      XERROR ("[%s] is not a float\n", str);
      goto errorexit;
   }

   *(double *)dst->data = tmp;

   ret = dst;
   error = false;

errorexit:

   if (error) {
      free (dst->data);
      ret = NULL;
   }

   return ret;
}


static void atom_del_list (atom_t *atom)
{
   xvector_t *tmp = atom->data;
   size_t len = XVECT_LENGTH (tmp);

   for (size_t i=0; i<len; i++) {
      atom_t *a = XVECT_INDEX (tmp, i);
      atom_del (a);
   }
   xvector_free (tmp);
}

static void atom_del_nonlist (atom_t *atom)
{
   free (atom->data);
}

static void print_depth (size_t depth, FILE *outf)
{
   for (size_t i=0; i<(depth * 3); i++)
      fprintf (outf, " ");
}

static void atom_pr_list (atom_t *atom, size_t depth, FILE *outf)
{
   xvector_t *children = atom->data;
   size_t nchildren = XVECT_LENGTH (children);

   for (size_t i=0; i<nchildren; i++) {
      atom_t *child = XVECT_INDEX (children, i);
      atom_print (child, depth + 1, outf);
   }
}

static void atom_pr_string (atom_t *atom, size_t depth, FILE *outf)
{
   print_depth (depth, outf);
   fprintf (outf, " ->str[%s]\n", (char *)atom->data);
}

static void atom_pr_symbol (atom_t *atom, size_t depth, FILE *outf)
{
   print_depth (depth, outf);
   fprintf (outf, " ->sym[%s]\n", (char *)atom->data);
}

static void atom_pr_int (atom_t *atom, size_t depth, FILE *outf)
{
   print_depth (depth, outf);
   fprintf (outf, " ->int[%" PRIi64 "]\n", *(int64_t *)atom->data);
}

static void atom_pr_float (atom_t *atom, size_t depth, FILE *outf)
{
   print_depth (depth, outf);
   fprintf (outf, " ->flt[%f]\n", *(double *)atom->data);
}

typedef struct atom_dispatch_t atom_dispatch_t;
struct atom_dispatch_t {
   enum atom_type_t  type;
   atom_newfunc_t   *new_fptr;
   atom_delfunc_t   *del_fptr;
   atom_prnfunc_t   *prn_fptr;
};

static const atom_dispatch_t *atom_find_funcs (enum atom_type_t type)
{
   static const atom_dispatch_t funcs[] = {
      { atom_LIST,   atom_new_list,    atom_del_list,    atom_pr_list    },
      { atom_STRING, atom_new_string,  atom_del_nonlist, atom_pr_string  },
      { atom_SYMBOL, atom_new_string,  atom_del_nonlist, atom_pr_symbol  },
      { atom_INT,    atom_new_int,     atom_del_nonlist, atom_pr_int     },
      { atom_FLOAT,  atom_new_float,   atom_del_nonlist, atom_pr_float   },
   };

   for (size_t i=0; i<sizeof funcs/sizeof funcs[0]; i++) {
      if (funcs[i].type == type)
         return &funcs[i];
   }

   return NULL;
}

void atom_del (atom_t *atom)
{
   if (!atom)
      return;

   const atom_dispatch_t *funcs = atom_find_funcs (atom->type);
   if (funcs) {
      funcs->del_fptr (atom);
   }

   free (atom);
}

atom_t *atom_new (enum atom_type_t type, const char *string)
{
   bool error = true;
   const atom_dispatch_t *funcs = atom_find_funcs (type);
   atom_t *ret = NULL;

   if (!(ret = calloc (1, sizeof *ret)))
      goto errorexit;

   if (!funcs)
      goto errorexit;

   if (!(funcs->new_fptr (ret, string)))
      goto errorexit;

   ret->type = type;

   error = false;

errorexit:

   if (error) {
      atom_del (ret);
      ret = NULL;
   }

   return ret;
}

void atom_print (atom_t *atom, size_t depth, FILE *outf)
{
   const atom_dispatch_t *funcs = atom_find_funcs (atom->type);

   if (funcs) {
      funcs->prn_fptr (atom, depth, outf);
   }
}

atom_t *atom_list_new (void)
{
   return atom_new (atom_LIST, NULL);
}

