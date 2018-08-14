#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "parser/atom.h"

#include "ll/ll.h"
#include "xerror/xerror.h"
#include "xstring/xstring.h"

typedef atom_t *(atom_newfunc_t) (atom_t *dst, const char *);
typedef void (atom_delfunc_t) (atom_t *);
typedef void (atom_prnfunc_t) (const atom_t *, size_t, FILE *);
typedef atom_t *(atom_dupfunc_t) (atom_t *dst, const atom_t *);
typedef int (atom_cmpfunc_t) (const atom_t *lhs, const atom_t *rhs);

static atom_t *a_new_list (atom_t *dst, const char *str)
{
   str = str;
   dst->data = ll_new ();
   return dst;
}

static atom_t *a_new_string (atom_t *dst, const char *str)
{
   char *tmp = xstr_dup (str);

   if (tmp && tmp[0]=='"') {
      size_t nbytes = strlen (tmp);
      memmove (&tmp[0], &tmp[1], nbytes);
      char *equote = strrchr (tmp, '"');
      if (equote)
         *equote = 0;
   }
   dst->data = tmp;
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

static atom_t *a_new_fptr (atom_t *dst, const char *str)
{
   return (sscanf (str, "%p", &dst->data)==1) ? dst : NULL;
}

static atom_t *a_new_buffer (atom_t *dst, const char *str)
{
   size_t nbytes = 0;
   const char *tmp = &str[1];
   while (tmp && *tmp && *tmp++!='|') {
      nbytes++;
   }

   tmp = &str[1];
   nbytes /= 2;

   dst->data = malloc (nbytes + sizeof (size_t));

   if (!dst->data)
      return NULL;

   *(size_t *)dst->data = nbytes;
   uint8_t *b = dst->data;
   for (size_t i=sizeof (size_t); i<nbytes + sizeof (size_t); i++) {
      sscanf (tmp, "%02hhx", &b[i]);
      tmp += 2;
   }

   return dst;
}


static void a_del_list (atom_t *atom)
{
   void **tmp = atom->data;
   size_t len = ll_length (tmp);

   for (size_t i=0; i<len; i++) {
      atom_t *a = ll_index (tmp, i);
      atom_del (a);
   }
   ll_del (tmp);
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

static void a_pr_list (const atom_t *atom, size_t depth, FILE *outf)
{
   void **children = atom->data;
   size_t nchildren = ll_length (children);

   if (depth) fprintf (outf, "\n");
   print_depth (depth, outf);
   fprintf (outf, "(");
   for (size_t i=0; i<nchildren; i++) {
      atom_t *child = ll_index (children, i);
      fprintf (outf, " ");
      atom_print (child, depth + 1, outf);
      // fprintf (outf, "\n");
      // print_depth (depth, outf);
   }
   fprintf (outf, ")");
}

static void a_pr_quote (const atom_t *atom, size_t depth, FILE *outf)
{
   depth = depth;
   fprintf (outf, "quot[%s]", (char *)atom->data);
}

static void a_pr_string (const atom_t *atom, size_t depth, FILE *outf)
{
   depth = depth;
   fprintf (outf, "str[%s]", (char *)atom->data);
}

static void a_pr_symbol (const atom_t *atom, size_t depth, FILE *outf)
{
   depth = depth;
   fprintf (outf, "sym[%s]", (char *)atom->data);
}

static void a_pr_int (const atom_t *atom, size_t depth, FILE *outf)
{
   depth = depth;
   fprintf (outf, "int[%" PRIi64 "]", *(int64_t *)atom->data);
}

static void a_pr_float (const atom_t *atom, size_t depth, FILE *outf)
{
   depth = depth;
   fprintf (outf, "flt[%f]", *(double *)atom->data);
}

static void a_pr_ffi (const atom_t *atom, size_t depth, FILE *outf)
{
   depth = depth;
   fprintf (outf, "ffi[%p]", atom->data);
}

static void a_pr_buffer (const atom_t *atom, size_t depth, FILE *outf)
{
   depth = depth;

   size_t nbytes = *(size_t *)atom->data;

   fprintf (outf, "buf[");
   uint8_t *b = atom->data;
   for (size_t i=sizeof (size_t); i<nbytes + sizeof (size_t); i++) {
      fprintf (outf, "0x%02x-", b[i]);
   }
   fprintf (outf, "]");
}

static void a_pr_native (const atom_t *atom, size_t depth, FILE *outf)
{
   depth = depth;
   fprintf (outf, "native[%p]", atom->data);
}

static atom_t *a_dup_list (atom_t *dst, const atom_t *src)
{
   bool error = true;
   void **tmp = ll_new ();
   void **stmp = src->data;

   size_t len = ll_length (stmp);

   for (size_t i=0; i<len; i++) {

      atom_t *ea = ll_index (stmp, i);

      atom_t *na = atom_dup (ea);
      if (!na)
         goto errorexit;

      if (!(ll_ins_tail (&tmp, na)))
         goto errorexit;
   }

   dst->data = tmp;

   error = false;

errorexit:
   if (error) {
      ll_iterate (tmp, (void (*) (void *))atom_del);
      ll_del (tmp);
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

static atom_t *a_dup_fptr (atom_t *dst, const atom_t *src)
{
   dst->data = src->data;

   return dst;
}

static atom_t *a_dup_buffer (atom_t *dst, const atom_t *src)
{
   size_t nbytes = *(size_t *)src->data;

   dst->data = malloc (nbytes + sizeof (size_t));
   if (!dst->data)
      return NULL;

   memcpy (dst->data, src->data, nbytes + sizeof (size_t));
   return dst;
}

static int a_cmp_list (const atom_t *lhs, const atom_t *rhs)
{
   size_t lhs_len = atom_list_length (lhs),
          rhs_len = atom_list_length (rhs);

   for (size_t i=0; i<lhs_len && i<rhs_len; i++) {
      const atom_t *lhs_child = atom_list_index (lhs, i),
                   *rhs_child = atom_list_index (rhs, i);

      int tmp = atom_cmp (lhs_child, rhs_child);
      if (tmp) {
         return tmp;
      }
   }

   if (lhs_len < rhs_len)
      return -1;

   if (lhs_len > rhs_len)
      return 1;

   return 0;
}

static int a_cmp_string (const atom_t *lhs, const atom_t *rhs)
{
   const char *lhs_s = atom_to_string (lhs),
              *rhs_s = atom_to_string (rhs);
   return strcmp (lhs_s, rhs_s);
}

static int a_cmp_int (const atom_t *lhs, const atom_t *rhs)
{
   int64_t lhs_i = *(int64_t *)lhs->data,
           rhs_i = *(int64_t *)rhs->data;

   if (lhs_i < rhs_i)      return -1;
   if (lhs_i > rhs_i)      return 1;

   return 0;
}

static int a_cmp_float (const atom_t *lhs, const atom_t *rhs)
{
   double lhs_f = *(double *)lhs->data,
          rhs_f = *(double *)rhs->data;

   if (lhs_f < rhs_f)      return -1;
   if (lhs_f > rhs_f)      return 1;

   return 0;
}

static int a_cmp_fptr (const atom_t *lhs, const atom_t *rhs)
{
   return lhs->data - rhs->data;
}

static int a_cmp_buffer (const atom_t *lhs, const atom_t *rhs)
{
   size_t nblhs = *(size_t *)lhs->data,
          nbrhs = *(size_t *)rhs->data;

   size_t nbytes = nblhs < nbrhs ? nblhs : nbrhs;

   return memcmp (lhs->data, rhs->data, nbytes);
}

typedef struct atom_dispatch_t atom_dispatch_t;
struct atom_dispatch_t {
   enum atom_type_t  type;
   atom_newfunc_t   *new_fptr;
   atom_delfunc_t   *del_fptr;
   atom_prnfunc_t   *prn_fptr;
   atom_dupfunc_t   *dup_fptr;
   atom_cmpfunc_t   *cmp_fptr;
};

static const atom_dispatch_t *atom_find_funcs (enum atom_type_t type)
{
   static const atom_dispatch_t funcs[] = {
{ atom_NIL,    a_new_list,   a_del_list,    a_pr_list,   a_dup_list,   a_cmp_list   },
{ atom_LIST,   a_new_list,   a_del_list,    a_pr_list,   a_dup_list,   a_cmp_list   },
{ atom_QUOTE,  a_new_string, a_del_nonlist, a_pr_quote,  a_dup_string, a_cmp_string },
{ atom_STRING, a_new_string, a_del_nonlist, a_pr_string, a_dup_string, a_cmp_string },
{ atom_SYMBOL, a_new_string, a_del_nonlist, a_pr_symbol, a_dup_string, a_cmp_string },
{ atom_INT,    a_new_int,    a_del_nonlist, a_pr_int,    a_dup_int,    a_cmp_int    },
{ atom_FLOAT,  a_new_float,  a_del_nonlist, a_pr_float,  a_dup_float,  a_cmp_float  },
{ atom_FFI,    a_new_fptr,   NULL,          a_pr_ffi,    a_dup_fptr,   a_cmp_fptr   },
{ atom_BUFFER, a_new_buffer, a_del_nonlist, a_pr_buffer, a_dup_buffer, a_cmp_buffer },
{ atom_NATIVE, a_new_fptr,   NULL,          a_pr_native, a_dup_fptr,   a_cmp_fptr   },
{ atom_UNKNOWN, NULL,        NULL,          NULL,        NULL,         NULL         },
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
   if (funcs && funcs->del_fptr) {
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

   if (funcs && funcs->new_fptr) {
      if (!(funcs->new_fptr (ret, string)))
         goto errorexit;
   }

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

   if (!atom)
      return NULL;

   const atom_dispatch_t *funcs = atom_find_funcs (atom->type);

   if (!funcs)
      return NULL;

   if (!(ret = calloc (1, sizeof *ret)))
      goto errorexit;

   memcpy (ret->buffer, atom->buffer, sizeof ret->buffer);
   if (funcs->dup_fptr && !(funcs->dup_fptr (ret, atom)))
      goto errorexit;

   ret->type = atom->type;
   ret->flags = atom->flags;

   error = false;

errorexit:

   if (error) {
      atom_del (ret);
      ret = NULL;
   }

   return ret;
}

atom_t *atom_concatenate (const atom_t *a, ...)
{
   bool error = true;
   atom_t *ret = NULL;
   va_list ap;


   va_start (ap, a);

   if (!(ret = atom_list_new ()))
      goto errorexit;

   while (a) {

      size_t len = atom_list_length (a);

      for (size_t i=0; i<len; i++) {
         atom_t *tmp = atom_dup (atom_list_index (a, i));
         if (!(atom_list_ins_tail (ret, tmp))) {
            goto errorexit;
         }
      }

      a = va_arg (ap, const atom_t *);
   }

   error = false;

errorexit:

   if (error) {
      atom_del (ret);
      ret = NULL;
   }

   va_end (ap);

   return ret;
}

atom_t *atom_list_new (void)
{
   return atom_new (atom_LIST, NULL);
}

atom_t *atom_list_pair (const atom_t *lnames, const atom_t*lvalues)
{
   bool error = true;
   atom_t *ret = atom_list_new ();

   if (!ret || !lnames || !lvalues) {
      fprintf (stderr, "NULL values found\n");
      goto errorexit;
   }

   size_t nlen = atom_list_length (lnames),
          vlen = atom_list_length (lvalues);

   if (nlen != vlen) {
      fprintf (stderr, "Lists unequal length [%zu : %zu]\n",
                        nlen, vlen);
      goto errorexit;
   }

   for (size_t i=0; i<nlen; i++) {

      atom_t *pair = atom_list_new ();

      const atom_t *name = atom_list_index (lnames, i),
                   *value = atom_list_index (lvalues, i);

      if (!pair) {
         fprintf (stderr, "Cannot create new list\n");
         goto errorexit;
      }

      if ((!atom_list_ins_tail (pair, atom_dup (name))) ||
          (!atom_list_ins_tail (pair, atom_dup (value)))) {
         atom_del (pair);
         goto errorexit;
      }

      if (!atom_list_ins_tail (ret, pair))
         goto errorexit;
   }

   error = false;

errorexit:
   if (error) {
      atom_del (ret);
      ret = NULL;
   }

   return ret;
}

void atom_print (const atom_t *atom, size_t depth, FILE *outf)
{
   if (!atom) {
      fprintf (outf, "NULL ATOM\n");
      return;
   }

   const atom_dispatch_t *funcs = atom_find_funcs (atom->type);

   if (funcs && funcs->prn_fptr) {
#if 0 // This is irritating, taking it out for now
      if (atom->buffer[0]) {
         fprintf (outf, "[%s ", atom->buffer);
      }
#endif
      funcs->prn_fptr (atom, depth, outf);
#if 0 // This is irritating, taking it out for now
      if (atom->buffer[0]) {
         fprintf (outf, "] ");
      }
#endif

      // print_depth (depth, outf);
      // fprintf (outf, "   Flags: [0x%02x]\n", atom->flags);
   }
}

int atom_fltint_cmp (const atom_t *lhs, const atom_t *rhs)
{
   if (lhs->type == atom_INT) {

      int64_t ilhs = *(int64_t *)lhs->data;
      double drhs = *(double *)rhs->data;

      if (ilhs < drhs)  return -1;
      if (ilhs > drhs)  return 1;
      if (ilhs == drhs) return 0;

   } else {

      double dlhs = *(double *)lhs->data;
      int64_t irhs = *(int64_t *)rhs->data;

      if (dlhs < irhs)  return -1;
      if (dlhs > irhs)  return 1;
      if (dlhs == irhs) return 0;

   }

   return -1;
}

int atom_cmp (const atom_t *lhs, const atom_t *rhs)
{
   if ( (lhs->type == atom_INT && rhs->type == atom_FLOAT) ||
        (lhs->type == atom_FLOAT && rhs->type ==atom_INT)) {
      return atom_fltint_cmp (lhs, rhs);
   }

   const atom_dispatch_t *func_lhs = atom_find_funcs (lhs->type),
                         *func_rhs = atom_find_funcs (rhs->type);

   int ret = -1;

   if (func_lhs != func_rhs)
      return -1;

   if (func_lhs->cmp_fptr) {
      ret = func_lhs->cmp_fptr (lhs, rhs);
   }

   return ret;
}

size_t atom_list_length (const atom_t *atom)
{
   if (atom->type!=atom_LIST)
      return 0;

   void **tmp = atom->data;
   return ll_length (tmp);
}

const atom_t *atom_list_index (const atom_t *atom, size_t index)
{

   if (atom->type!=atom_LIST)
      return NULL;

   return ll_index (atom->data, index);
}

atom_t *atom_list_remove (atom_t *atom, size_t index)
{
   if (atom->type!=atom_LIST)
      return NULL;

   return ll_remove ((void ***)&atom->data, index);
}

atom_t *atom_list_ins_tail (atom_t *atom, void *el)
{
   if (atom->type!=atom_LIST)
      return NULL;

   return ll_ins_tail ((void ***)&atom->data, el);
}

atom_t *atom_list_ins_head (atom_t *atom, void *el)
{
   if (atom->type!=atom_LIST)
      return NULL;

   return ll_ins_head ((void ***)&atom->data, el);
}

atom_t *atom_list_remove_tail (atom_t *atom)
{
   return ll_remove_tail ((void ***)&atom->data);
}

atom_t *atom_list_remove_head (atom_t *atom)
{
   return ll_remove_head ((void ***)&atom->data);
}

atom_t *atom_string_new (const char *s)
{
   return atom_new (atom_STRING, s);
}

atom_t *atom_symbol_new (const char *s)
{
   return atom_new (atom_SYMBOL, s);
}

atom_t *atom_int_new (int64_t i)
{
   char tmps[18];

   sprintf (tmps, "%" PRIi64, i);
   return atom_new (atom_INT, tmps);
}

atom_t *atom_float_new (double d)
{
   char tmps[18];

   sprintf (tmps, "%5.5lf", d);
   return atom_new (atom_FLOAT, tmps);
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

atom_t **atom_array_dup (const atom_t **atoms)
{
   bool error = true;
   atom_t **ret = NULL;

   if (!atoms)
      return NULL;

   size_t natoms = 0;
   for (size_t i=0; atoms[i]; i++)
      natoms++;

   ret = malloc (sizeof *ret * (natoms + 1));
   if (!ret)
      goto errorexit;
   memset (ret, 0, sizeof *ret * (natoms + 1));

   for (size_t i=0; atoms[i]; i++) {
      ret[i] = atom_dup (atoms[i]);
   }

   error = false;

errorexit:
   if (error) {
      atom_array_del (ret);
      ret = NULL;
   }

   return ret;
}

void atom_array_del (atom_t **atoms)
{
   if (!atoms)
      return;

   for (size_t i=0; atoms[i]; i++) {
      atom_del (atoms[i]);
   }

   free (atoms);
}

