#include <stdbool.h>

#include "parser/atom.h"
#include "rt/builtins.h"
#include "ll/ll.h"


atom_t *builtins_LIST (rt_t *rt, atom_t **args, size_t nargs)
{
   bool error = true;
   atom_t *ret = NULL;

   nargs = nargs;
   rt = rt;

   if (!(ret = atom_list_new ()))
      goto errorexit;

   for (size_t i=0; args[i]; i++) {
      if (!(atom_list_ins_tail (ret, atom_dup (args[i]))))
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

atom_t *builtins_NAPPEND (rt_t *rt, atom_t **args, size_t nargs)
{
   bool error = true;

   nargs = nargs;
   rt = rt;

   if (args[0]->type!=atom_LIST)
      return NULL;

   for (size_t i=1; args[i]; i++) {
      if (!(atom_list_ins_tail (args[0], atom_dup (args[i]))))
         goto errorexit;
   }

   error = false;

errorexit:

   return error ? NULL : args[0];
}

atom_t *builtins_DEFINE (rt_t *rt, atom_t **args, size_t nargs)
{
   if (nargs != 2)
      return NULL;

   return rt_symbol_add (rt->symbols, atom_dup (args[0]), atom_dup (args[1]));
}

atom_t *builtins_UNDEFINE (rt_t *rt, atom_t **args, size_t nargs)
{
   atom_t *ret = NULL;
   for (size_t i=0; i<nargs; i++) {
      atom_del (ret);
      ret = rt_symbol_remove (rt->symbols, args[i]);
   }

   return ret;
}

atom_t *builtins_EVAL (rt_t *rt, atom_t **args, size_t nargs)
{
   atom_t *ret = NULL;

   for (size_t i=0; args[i]; i++) {
      atom_del (ret);
      ret = rt_eval (rt, args[i]);
   }

   return ret;
}

atom_t *builtins_PRINT (rt_t *rt, atom_t **args, size_t nargs)
{
   for (size_t i=0; args[i]; i++) {
      atom_print (args[i], 0, stdout);
   }

   return atom_new (atom_NIL, NULL);
}

atom_t *builtins_CONCAT (rt_t *rt, atom_t **args, size_t nargs)
{
   bool error = true;
   atom_t *ret = atom_list_new ();

   for (size_t i=0; args[i]; i++) {
      size_t len = atom_list_length (args[i]);
      for (size_t j=0; j<len; j++) {
         if (!atom_list_ins_tail (ret, atom_dup (atom_list_index (args[i], j))))
            goto errorexit;
      }
   }

   error = false;

errorexit:
   if (error) {
      atom_del (ret);
      ret = NULL;
   }

   return ret;
}

static atom_t *builtins_operator (rt_t *rt, atom_t **args, size_t nargs,
                                  char op, double startval)
{
   bool error = true;
   atom_t *ret = NULL;
   double final = startval;
   bool used_float = false;

   nargs = nargs;
   rt = rt;

   for (size_t i=0; args[i]; i++) {

      if (args[i]->type!=atom_INT &&
          args[i]->type!=atom_FLOAT)
         goto errorexit;

      bool is_int = args[i]->type==atom_INT;

#define GETINT       (*(int64_t *)args[i]->data)
#define GETFLOAT     (*(double *)args[i]->data)
      if (i==0 && startval < 0) {
         final = is_int ? GETINT : GETFLOAT;
         i++;
         is_int = args[i]->type==atom_INT;
      }

      used_float = used_float || !is_int;

      switch (op) {
         case '+': final += is_int ? GETINT : GETFLOAT;
                   break;

         case '-':
                   printf ("Pre Set final: [%lf]\n", final);
                   final -= is_int ? GETINT : GETFLOAT;
                   printf ("Post Set final: [%lf]\n", final);
                   break;

         case '/': final /= is_int ? GETINT : GETFLOAT;
                   break;

         case '*': final *= is_int ? GETINT : GETFLOAT;
                   break;
      }
#undef GETINT
#undef GETFLOAT

   }

   int64_t tmp = final * 10000000;
   if (tmp % 10000000) {
      ret = atom_float_new (final);
   } else {
      tmp = final;
      ret = atom_int_new (tmp);
   }

   error = false;

errorexit:

   if (error) {
      atom_del (ret);
      printf (" *********************** Returning NULL\n");
      ret = NULL;
   }

   return ret;
}

atom_t *builtins_PLUS (rt_t *rt, atom_t **args, size_t nargs)
{
   return builtins_operator (rt, args, nargs, '+', 0);
}

atom_t *builtins_MINUS (rt_t *rt, atom_t **args, size_t nargs)
{
   return builtins_operator (rt, args, nargs, '-', -1);
}

atom_t *builtins_DIVIDE (rt_t *rt, atom_t **args, size_t nargs)
{
   return builtins_operator (rt, args, nargs, '/', -1);
}

atom_t *builtins_MULTIPLY (rt_t *rt, atom_t **args, size_t nargs)
{
   return builtins_operator (rt, args, nargs, '*', 1);
}
