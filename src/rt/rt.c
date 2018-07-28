#include <stdlib.h>
#include <stdbool.h>

#include "rt/rt.h"

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

const atom_t *rt_eval_symbol (rt_t *rt, atom_t *atom)
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

   return atom_list_index (entry, 1);
}

const atom_t *rt_list_eval (rt_t *rt, atom_t *atom)
// TODO:
// if type==FFI
//    call libffi
// else
//    funcall (car (atom), cdr (atom))
{
#warning This function incomplete
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


