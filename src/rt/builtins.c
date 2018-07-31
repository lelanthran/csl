#include <stdbool.h>

#include "parser/atom.h"
#include "rt/builtins.h"
#include "ll/ll.h"


atom_t *builtins_LIST (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   bool error = true;
   atom_t *ret = NULL;

   nargs = nargs;
   rt = rt;
   sym = sym;

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

atom_t *builtins_NAPPEND (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   bool error = true;

   nargs = nargs;
   rt = rt;
   sym = sym;

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

atom_t *builtins_DEFINE (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   sym = sym;
   if (nargs != 2) {
      fprintf (stderr, "--------------------------------------\n");
      fprintf (stderr, "Too many arguments for DEFINE (found %zu). Possible "
                       "unterminated list.\n", nargs);
      for (size_t i=0; i<nargs; i++) {
         fprintf (stderr, "element [%zu] = ", i);
         atom_print (args[i], 0, stderr);
      }
      return NULL;
   }

   return rt_symbol_add (rt->symbols, atom_dup (args[0]), atom_dup (args[1]));
}

atom_t *builtins_UNDEFINE (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   sym = sym;
   atom_t *ret = NULL;
   for (size_t i=0; i<nargs; i++) {
      atom_del (ret);
      ret = rt_symbol_remove (rt->symbols, args[i]);
   }

   return ret;
}

atom_t *builtins_EVAL (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   nargs = nargs;

   atom_t *ret = NULL;

   for (size_t i=0; args[i]; i++) {
      atom_del (ret);
      ret = rt_eval (rt, sym, args[i]);
   }

   return ret;
}

atom_t *builtins_PRINT (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   nargs = nargs;
   rt = rt;
   sym = sym;

   for (size_t i=0; args[i]; i++) {
      atom_print (args[i], 0, stdout);
   }

   return atom_new (atom_NIL, NULL);
}

atom_t *builtins_CONCAT (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   nargs = nargs;
   rt = rt;
   sym = sym;

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

atom_t *builtins_LET (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   nargs = nargs;

   atom_t *symbols = sym ? atom_concatenate (sym, args[0], NULL)
                         : atom_dup (args[0]);

   atom_t *ret = rt_eval (rt, symbols, args[1]);

   atom_del (symbols);

   return ret;
}

atom_t *builtins_FUNCALL (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   bool error = true;

   atom_t *tmpsym = NULL,
          *allsyms = NULL,
          *ret = NULL;

   if (nargs!=2) {
      fprintf (stderr, "--------------------------------------\n");
      fprintf (stderr, "Too many arguments for FUNCALL (found %zu). Possible "
                       "unterminated list.\n", nargs);
      for (size_t i=0; i<nargs; i++) {
         fprintf (stderr, "element [%zu] = ", i);
         atom_print (args[i], 0, stderr);
      }
      return NULL;
   }

   printf ("*****************************************************\n");
   atom_print (args[0], 5, stdout);
   printf ("*****************************************************\n");
   atom_print (args[1], 5, stdout);
   printf ("*****************************************************\n");

   const atom_t *fargs = args[0],
                *fparam = atom_list_index (args[1], 0),
                *body = atom_list_index (args[1], 1);

   if (!(tmpsym = atom_list_pair (fparam, fargs))) {
      fprintf (stderr, "Unable to construct parameters\n");
      goto errorexit;
   }

   if (!(allsyms = atom_concatenate (tmpsym, sym, NULL))) {
      fprintf (stderr, "Unable to concatenate parameters\n");
      goto errorexit;
   }

   ret = rt_eval (rt, allsyms, body);

   printf ("*****************************************************\n");
   atom_print (ret, 5, stdout);
   printf ("*****************************************************\n");

   error = false;

errorexit:

   atom_del (tmpsym);
   atom_del (allsyms);

   return ret;
}

static atom_t *builtins_operator (rt_t *rt, atom_t *sym,
                                  atom_t **args, size_t nargs,
                                  char op, double startval)
{
   bool error = true;
   atom_t *ret = NULL;
   double final = startval;
   bool used_float = false;

   nargs = nargs;
   rt = rt;
   sym = sym;

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
                   final -= is_int ? GETINT : GETFLOAT;
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

atom_t *builtins_PLUS (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   return builtins_operator (rt, sym, args, nargs, '+', 0);
}

atom_t *builtins_MINUS (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   return builtins_operator (rt, sym, args, nargs, '-', -1);
}

atom_t *builtins_DIVIDE (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   return builtins_operator (rt, sym, args, nargs, '/', -1);
}

atom_t *builtins_MULTIPLY (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   return builtins_operator (rt, sym, args, nargs, '*', 1);
}
