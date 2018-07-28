#include <stdlib.h>
#include <stdbool.h>

#include "rt/rt.h"
#include "ll/ll.h"

#include "xerror/xerror.h"

rt_t *rt_new (void)
{
   bool error = true;
   rt_t *ret = NULL;

   if (!(ret = calloc (1, sizeof *ret)))
      goto errorexit;

   ret->symbols = atom_list_new ();
   ret->stack = atom_list_new ();
   ret->roots = atom_list_new ();
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

atom_t *rt_eval_symbol (rt_t *rt, atom_t *atom)
{
   size_t len = atom_list_length (rt->symbols);
   const atom_t *entry = NULL;

   for (size_t i=0; i<len; i++) {
      if ((entry = atom_list_index (atom, 0)))
         break;
   }
   if (!entry) {
      XERROR ("Failed to find symbol [%s]\n", atom_to_string (atom));
      return NULL;
   }

   return atom_dup (atom_list_index (entry, 1));
}

static atom_t *rt_funcall (atom_t *func, atom_t *argslist)
{

}

static atom_t *rt_list_eval (rt_t *rt, atom_t *atom)
{
   atom_t *ret = NULL;
   atom_t *func = NULL;

   void **args = NULL;
   size_t nargs = 0;

   if (!(func = rt_eval_symbol (rt, ll_index (atom->data, 0))))
      goto errorexit;

   args = ll_new ();
   nargs = ll_length (atom->data) - 1;

   for (size_t i=0; i<nargs; i++) {
      atom_t *tmp = rt_eval (rt, ll_index (atom->data, i));
      if (!tmp) {
         XERROR ("Fatal error during evaluation\n");
         goto errorexit;
      }
      if (!ll_ins_tail (&args, tmp))
         goto errorexit;
   }

   if (func->type==atom_FFI) {
      // TODO: Implement FFI
   } else {
      ret = rt_funcall (func, args);
   }

errorexit:
   return ret;
}

atom_t *rt_eval (rt_t *rt, atom_t *atom)
{
   bool error = true;
   atom_t *ret = NULL;
   const atom_t *tmp = NULL;

   switch (atom->type) {
      case atom_FFI:
      case atom_STRING:
      case atom_INT:
      case atom_FLOAT:     tmp = atom;                      break;

      case atom_SYMBOL:    tmp = rt_eval_symbol (rt, atom); break;

      case atom_LIST:      tmp = rt_list_eval (rt, atom);   break;

      case atom_ENDL:
      case atom_UNKNOWN:   tmp = NULL;                      break;
   }

   if (!tmp) {
      XERROR ("Corrupt object [%p]\n", atom);
      goto errorexit;
   }

   if (!(ret = atom_dup (tmp)))
      goto errorexit;

   error = false;

errorexit:

   if (error) {
      atom_del (ret);
      ret = NULL;
   }


   return ret;
}


