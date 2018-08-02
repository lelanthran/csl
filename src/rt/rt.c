#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "rt/rt.h"
#include "rt/builtins.h"

#include "ll/ll.h"


#include "xerror/xerror.h"

#define FLAG_QUOTE         (1 << 0)

static atom_t *rt_atom_native (rt_builtins_fptr_t *fptr)
{
   atom_t *ret = atom_new (atom_UNKNOWN, NULL);
   if (!ret)
      return NULL;

   ret->data = fptr;
   ret->type = atom_NATIVE;

   return ret;
}

atom_t *rt_symbol_add (atom_t *symbols, atom_t *name, atom_t *value)
{
   bool error = true;
   atom_t *ret = NULL;
   atom_t *tmp[] = {
      name, value, NULL,
   };

   atom_t *tlist = builtins_LIST (NULL, NULL, tmp, 2);
   if (!tlist) {
      goto errorexit;
   }

   tlist->flags = name->flags;
   ((atom_t *)atom_list_index (tlist, 0))->flags = name->flags;
   ((atom_t *)atom_list_index (tlist, 1))->flags = name->flags;

   tmp[0] = symbols;
   tmp[1] = tlist;

   // TODO: Remove existing entry first
   if ((ret = builtins_NAPPEND (NULL, NULL, tmp, 2))==NULL)
      goto errorexit;

   error = false;

errorexit:

   if (error) {
      ret = NULL;
   } else {
      ret = tlist;
   }

   //atom_del (ret);
   //atom_del (tlist);
   atom_del (name);
   atom_del (value);

   return ret;
}

const atom_t *rt_symbol_find (atom_t *symbols, const atom_t *name)
{
   const char *strname = NULL;

   if (!name || !symbols)
      return NULL;

   size_t len = atom_list_length (symbols);

   strname = atom_to_string (name);

   for (size_t i=0; i<len; i++) {
      const atom_t *nvpair = atom_list_index (symbols, i);
      const char *n = atom_to_string (atom_list_index (nvpair, 0));
      if (strcmp (n, strname)==0) {
         // atom_del (name);
         return nvpair;
      }
   }

   return NULL;
}

atom_t *rt_symbol_remove (atom_t *symbols, const atom_t *name)
{
   const char *strname = NULL;
   size_t len = atom_list_length (symbols);

   if (!name)
      return NULL;

   strname = atom_to_string (name);

   for (size_t i=0; i<len; i++) {
      const atom_t *nvpair = atom_list_index (symbols, i);
      const char *n = atom_to_string (atom_list_index (nvpair, 0));
      if (strcmp (n, strname)==0) {
         return atom_list_remove (symbols, i);
      }
   }

   return NULL;
}

static struct g_native_funcs_t {
   const char *name;
   rt_builtins_fptr_t *fptr;
} g_native_funcs[] = {
   {  "list",        builtins_LIST        },
   {  "nappend",     builtins_NAPPEND     },
   {  "define",      builtins_DEFINE      },
   {  "undefine",    builtins_UNDEFINE    },
   {  "eval",        builtins_EVAL        },
   {  "print",       builtins_PRINT       },
   {  "concat",      builtins_CONCAT      },
   {  "let",         builtins_LET         },
   {  "defun",       builtins_DEFUN       },
   {  "funcall",     builtins_FUNCALL     },

   {  "+",           builtins_PLUS        },
   {  "-",           builtins_MINUS       },
   {  "/",           builtins_DIVIDE      },
   {  "*",           builtins_MULTIPLY    },
};

rt_t *rt_new (void)
{
   bool error = true;
   rt_t *ret = NULL;

   if (!(ret = calloc (1, sizeof *ret)))
      goto errorexit;

   ret->symbols = atom_list_new ();
   ret->stack = atom_list_new ();
   ret->roots = atom_list_new ();

   for (size_t i=0; i<sizeof g_native_funcs/sizeof g_native_funcs[0]; i++) {
      atom_t *tmp =
         rt_symbol_add (ret->symbols,
                        atom_new (atom_SYMBOL, g_native_funcs[i].name),
                        rt_atom_native (g_native_funcs[i].fptr));
      if (!tmp)
         goto errorexit;
      atom_del (tmp);
   }

   error = false;

errorexit:

   if (error) {
      rt_del (ret);
      ret = NULL;
   }

   return ret;
}

void rt_del (rt_t *rt)
{
   if (!rt)
      return;

   atom_del (rt->symbols);
   atom_del (rt->stack);
   atom_del (rt->roots);

   free (rt);
}

const atom_t *rt_eval_symbol (rt_t *rt, atom_t *sym, atom_t *atom)
{
   const atom_t *entry = NULL;

   entry = rt_symbol_find (sym, atom);
   if (entry) {
      return atom_list_index (entry, 1);
   }

   entry = rt_symbol_find (rt->symbols, atom);
   if (!entry) {
      XERROR ("Failed to find symbol [%s]\n", atom_to_string (atom));
      atom_print (sym, 5, stdout);
      return NULL;
   }

   return atom_list_index (entry, 1);
}

static atom_t *rt_funcall_native (rt_t *rt, atom_t *sym,
                                  atom_t **args, size_t nargs)
{
   rt_builtins_fptr_t *fptr = args[0]->data;

   return fptr (rt, sym, &args[1], nargs);
}

static atom_t *rt_funcall_interp (rt_t *rt, atom_t *sym,
                                  atom_t **args, size_t nargs)
{
   bool error = true;
   atom_t *ret = NULL,
          *fargs = atom_list_new ();

   for (size_t i=1; args[i]; i++) {
      if (!atom_list_ins_tail (fargs, args[i]))
         goto errorexit;
   }

   atom_t *fc_args[] = { fargs, args[0], NULL };

   ret = builtins_FUNCALL (rt, sym, fc_args, 2);

errorexit:
   // atom_del (fargs);
   return ret;
}

static size_t depth;

static atom_t *rt_list_eval (rt_t *rt, atom_t *sym, atom_t *atom)
{
   atom_t *ret = NULL;
   atom_t *func = NULL;

   void **args = NULL;
   size_t nargs = 0;

   args = ll_new ();
   size_t llen = atom_list_length (atom);

   depth++;
   for (size_t i=0; i<llen; i++) {

      atom_t *tmp = (atom_t *)atom_list_index (atom, i);

      if (rt->flags & FLAG_QUOTE || tmp->flags & ATOM_FLAG_FUNC) {
         tmp = atom_dup (tmp);
      } else {
         tmp = rt_eval (rt, sym, tmp);
      }

      printf ("eval result:\n");
      atom_print (tmp, 0, stdout);

      rt->flags &= ~FLAG_QUOTE;

      if (!tmp) {
         XERROR ("Fatal error during evaluation\n");
         continue;
      }
      if (tmp->type==atom_QUOTE) {
         atom_del (tmp);
         rt->flags |= FLAG_QUOTE;
         continue;
      }

      if (!ll_ins_tail (&args, tmp))
         goto errorexit;

      nargs++;
   }

   func = ll_index (args, 0);

   if (!func)
      goto errorexit;

   switch (func->type) {
      case atom_FFI:
         // TODO: Implement FFI
         break;

      case atom_NATIVE:
         ret = rt_funcall_native (rt, sym, (atom_t **)args, --nargs);
         break;

      default:
         ret = func->flags == ATOM_FLAG_FUNC ?
               rt_funcall_interp (rt, sym, (atom_t **)args, --nargs) :
               NULL;

         if (ret) ret->flags = 0;
         break;
   }


   if (!ret) {
      ret = atom_list_new ();
      for (size_t i=0; i<nargs; i++) {
         atom_list_ins_tail (ret, atom_dup (ll_index (args, i)));
      }
   }

   printf ("return result:\n");
   atom_print (ret, 0, stdout);

errorexit:
   ll_iterate (args, (void (*) (void *))atom_del);
   ll_del (args);

   depth--;
   return ret;
}

atom_t *rt_eval (rt_t *rt, atom_t *sym, atom_t *atom)
{
   bool error = true;
   atom_t *tmp = NULL;

   switch (atom->type) {
      case atom_NATIVE:
      case atom_FFI:
      case atom_STRING:
      case atom_INT:
      case atom_QUOTE:
      case atom_FLOAT:  tmp = atom_dup (atom);                           break;

      case atom_SYMBOL: tmp = atom_dup (rt_eval_symbol (rt, sym, atom)); break;

      case atom_LIST:   tmp = rt_list_eval (rt, sym, atom);              break;

      case atom_NIL:    tmp = atom_new (atom_NIL, NULL);                 break;
      case atom_ENDL:
      case atom_UNKNOWN:   tmp = NULL;                                   break;
   }

   if (!tmp) {
      XERROR ("Corrupt object [%p]\n", atom);
      goto errorexit;
   }

   error = false;

errorexit:

   if (error) {
      atom_del (tmp);
      tmp = NULL;
   }

   return tmp;
}

void rt_print (rt_t *rt, FILE *outf)
{
   if (!outf)
      outf = stdout;

   if (!rt) {
      fprintf (outf, "NULL Runtime Object\n");
      return;
   }

   atom_print (rt->symbols, 0, outf);

}

