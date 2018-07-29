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

static atom_t *rt_add_symbol (rt_t *rt, atom_t *name, atom_t *value)
{
   bool error = true;
   atom_t *ret = NULL;
   atom_t *tmp[] = {
      name, value, NULL,
   };

   ret = builtins_LIST (rt, tmp, 2);
   if (!ret) {
      goto errorexit;
   }

   tmp[0] = rt->symbols;
   tmp[1] = ret;

   // TODO: Remove existing entry first
   if (!(builtins_NAPPEND (rt, tmp, 2)))
      goto errorexit;

   error = false;

errorexit:

   atom_del (ret);
   atom_del (name);
   atom_del (value);

   if (error) {
      ret = NULL;
   } else {
      ret = rt->symbols;
   }

   return ret;
}

static const atom_t *rt_find_symbol (rt_t *rt, atom_t *name)
{
   const char *strname = NULL;
   size_t len = atom_list_length (rt->symbols);

   if (!name)
      return NULL;

   strname = atom_to_string (name);

   for (size_t i=0; i<len; i++) {
      const atom_t *nvpair = atom_list_index (rt->symbols, i);
      const char *n = atom_to_string (atom_list_index (nvpair, 0));
      if (strcmp (n, strname)==0) {
         // atom_del (name);
         return nvpair;
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
      if (!rt_add_symbol (ret, atom_new (atom_STRING, g_native_funcs[i].name),
                               rt_atom_native (g_native_funcs[i].fptr)))
         goto errorexit;
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

const atom_t *rt_eval_symbol (rt_t *rt, atom_t *atom)
{
   const atom_t *entry = NULL;

   entry = rt_find_symbol (rt, atom);
   if (!entry) {
      XERROR ("Failed to find symbol [%s]\n", atom_to_string (atom));
      return NULL;
   }

   return atom_list_index (entry, 1);
}

static atom_t *rt_funcall_native (rt_t *rt, atom_t **args, size_t nargs)
{
   rt_builtins_fptr_t *fptr = args[0]->data;

   return fptr (rt, &args[1], nargs);
}

static atom_t *rt_list_eval (rt_t *rt, atom_t *atom)
{
   atom_t *ret = NULL;
   atom_t *func = NULL;

   void **args = NULL;
   size_t nargs = 0;

   args = ll_new ();
   nargs = ll_length ((void **)atom->data);

   for (size_t i=0; i<nargs; i++) {

      atom_t *tmp = ll_index (atom->data, i);
      if (rt->flags & FLAG_QUOTE) {
         tmp = atom_dup (tmp);
      } else {
         tmp = rt_eval (rt, tmp);
      }
      rt->flags &= ~FLAG_QUOTE;

      if (!tmp) {
         XERROR ("Fatal error during evaluation\n");
         goto errorexit;
      }
      if (tmp->type==atom_QUOTE) {
         atom_del (tmp);
         rt->flags |= FLAG_QUOTE;
         continue;
      }

      if (!ll_ins_tail (&args, tmp))
         goto errorexit;
   }

   func = ll_index (args, 0);

   if (func && func->type==atom_FFI) {
      // TODO: Implement FFI
   }
   if (func && func->type==atom_NATIVE) {
      ret = rt_funcall_native (rt, (atom_t **)args, nargs);
   }

   if (!ret) {
      ret = atom_list_new ();
      for (size_t i=0; i<nargs; i++) {
         ll_ins_tail ((void ***)&ret->data, atom_dup (ll_index (args, i)));
      }
   }

errorexit:
   ll_iterate (args, (void (*) (void *))atom_del);
   ll_del (args);

   return ret;
}

atom_t *rt_eval (rt_t *rt, atom_t *atom)
{
   bool error = true;
   atom_t *tmp = NULL;

   switch (atom->type) {
      case atom_NATIVE:
      case atom_FFI:
      case atom_STRING:
      case atom_INT:
      case atom_QUOTE:
      case atom_FLOAT:     tmp = atom_dup (atom);                       break;

      case atom_SYMBOL:    tmp = atom_dup (rt_eval_symbol (rt, atom));  break;

      case atom_LIST:      tmp = rt_list_eval (rt, atom);               break;

      case atom_ENDL:
      case atom_UNKNOWN:   tmp = NULL;                                  break;
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


