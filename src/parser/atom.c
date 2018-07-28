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
typedef atom_t *(atom_dupfunc_t) (atom_t *dst, const atom_t *);

static atom_t *a_new_list (atom_t *dst, const char *str)
{
   str = str;
   dst->data = xvector_new ();
   return dst;
}

static atom_t *a_new_string (atom_t *dst, const char *str)
{
   dst->data = xstr_dup (str);
   return dst;
}

static atom_t *a_new_int (atom_t *dst, const char *str)
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

static atom_t *a_new_float (atom_t *dst, const char *str)
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


static void a_del_list (atom_t *atom)
{
   xvector_t *tmp = atom->data;
   size_t len = XVECT_LENGTH (tmp);

   for (size_t i=0; i<len; i++) {
      atom_t *a = XVECT_INDEX (tmp, i);
      atom_del (a);
   }
   xvector_free (tmp);
}

static void a_del_nonlist (atom_t *atom)
{
   free (atom->data);
}

static void print_depth (size_t depth, FILE *outf)
{
   for (size_t i=0; i<(depth * 3); i++)
      fprintf (outf, " ");
}

static void a_pr_list (atom_t *atom, size_t depth, FILE *outf)
{
   xvector_t *children = atom->data;
   size_t nchildren = XVECT_LENGTH (children);

   for (size_t i=0; i<nchildren; i++) {
      atom_t *child = XVECT_INDEX (children, i);
      atom_print (child, depth + 1, outf);
   }
}

static void a_pr_string (atom_t *atom, size_t depth, FILE *outf)
{
   print_depth (depth, outf);
   fprintf (outf, " ->str[%s]\n", (char *)atom->data);
}

static void a_pr_symbol (atom_t *atom, size_t depth, FILE *outf)
{
   print_depth (depth, outf);
   fprintf (outf, " ->sym[%s]\n", (char *)atom->data);
}

static void a_pr_int (atom_t *atom, size_t depth, FILE *outf)
{
   print_depth (depth, outf);
   fprintf (outf, " ->int[%" PRIi64 "]\n", *(int64_t *)atom->data);
}

static void a_pr_float (atom_t *atom, size_t depth, FILE *outf)
{
   print_depth (depth, outf);
   fprintf (outf, " ->flt[%f]\n", *(double *)atom->data);
}

static atom_t *a_dup_list (atom_t *dst, const atom_t *src)
{
   bool error = true;
   xvector_t *tmp = xvector_new ();
   xvector_t *stmp = src->data;

   size_t len = XVECT_LENGTH (stmp);

   for (size_t i=0; i<len; i++) {

      atom_t *ea = XVECT_INDEX (stmp, i);

      atom_t *na = atom_dup (ea);
      if (!na)
         goto errorexit;

      if (!(xvector_ins_tail (tmp, na)))
         goto errorexit;
   }

   dst->data = tmp;

   error = false;

errorexit:
   if (error) {
      xvector_iterate (tmp, (void (*) (void *))atom_del);
      xvector_free (tmp);
   }
   return error ? NULL : dst;
}

static atom_t *a_dup_string (atom_t *dst, const atom_t *src)
{
   char *tmp = xstr_dup ((char *)src->data);

   if (!tmp && src->data)
      return NULL;

   dst->data = tmp;

   return dst;
}

static atom_t *a_dup_int (atom_t *dst, const atom_t *src)
{
   int64_t *tmp = malloc (sizeof *tmp);
   if (!tmp)
      return NULL;

   *tmp = *(int64_t *)src->data;

   dst->data = tmp;

   return dst;
}

static atom_t *a_dup_float (atom_t *dst, const atom_t *src)
{
   double *tmp = malloc (sizeof *tmp);
   if (!tmp)
      return NULL;

   *tmp = *(double *)src->data;

   dst->data = tmp;

   return dst;
}

typedef struct atom_dispatch_t atom_dispatch_t;
struct atom_dispatch_t {
   enum atom_type_t  type;
   atom_newfunc_t   *new_fptr;
   atom_delfunc_t   *del_fptr;
   atom_prnfunc_t   *prn_fptr;
   atom_dupfunc_t   *dup_fptr;
};

static const atom_dispatch_t *atom_find_funcs (enum atom_type_t type)
{
   static const atom_dispatch_t funcs[] = {
{ atom_LIST,   a_new_list,   a_del_list,    a_pr_list  , a_dup_list   },
{ atom_STRING, a_new_string, a_del_nonlist, a_pr_string, a_dup_string },
{ atom_SYMBOL, a_new_string, a_del_nonlist, a_pr_symbol, a_dup_string },
{ atom_INT,    a_new_int,    a_del_nonlist, a_pr_int,    a_dup_int    },
{ atom_FLOAT,  a_new_float,  a_del_nonlist, a_pr_float,  a_dup_float  },
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

atom_t *atom_dup (const atom_t *atom)
{
   bool error = true;
   atom_t *ret = NULL;
   const atom_dispatch_t *funcs = atom_find_funcs (atom->type);

   if (!funcs)
      return NULL;

   if (!atom)
      return NULL;

   if (!(ret = calloc (1, sizeof *ret)))
      goto errorexit;

   if (!(funcs->dup_fptr (ret, atom)))
      goto errorexit;

   ret->type = atom->type;

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

size_t atom_list_length (const atom_t *atom)
{
   if (atom->type!=atom_LIST)
      return 0;

   xvector_t *tmp = atom->data;
   return XVECT_LENGTH (tmp);
}

const atom_t *atom_list_car (const atom_t *atom)
{

   if (atom->type!=atom_LIST)
      return NULL;

   xvector_t *tmp = atom->data;
   return XVECT_INDEX (tmp, 0);
}

const atom_t *atom_list_cdr (const atom_t *atom)
{
   if (atom->type!=atom_LIST)
      return NULL;

   xvector_t *tmp = atom->data;
#warning Note to change from xvector_t to native arrays
   return NULL;
}

const char *atom_to_string (const atom_t *atom)
{
   if (atom->type==atom_LIST) {
      return "LIST";
   }

   if (atom->type==atom_STRING || atom->type==atom_SYMBOL) {
      return (char *)atom->data;
   }

   memset ((char *)atom->buffer, 0, sizeof atom->buffer);

   if (atom->type==atom_INT) {
      sprintf ((char *)atom->buffer, "%" PRIi64, *(int64_t *)atom->data);
   }

   if (atom->type==atom_INT) {
      sprintf ((char *)atom->buffer, "%.5lf", *(double *)atom->data);
   }

   return atom->buffer;
}


