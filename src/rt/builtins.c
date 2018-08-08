#include <stdbool.h>

#include "parser/atom.h"
#include "rt/builtins.h"
#include "rt/rt.h"
#include "ll/ll.h"


atom_t *builtins_TRAP_SET (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   sym = sym;
   if (nargs != 2) {
      fprintf (stderr, "--------------------------------------\n");
      fprintf (stderr, "Too many arguments for TRAP_SET (found %zu). Possible "
                       "unterminated list.\n", nargs);
      for (size_t i=0; i<nargs; i++) {
         fprintf (stderr, "element [%zu] = ", i);
         atom_print (args[i], 0, stderr);
      }
      return NULL;
   }

   atom_t *existing = rt_symbol_remove (rt->traps, args[0]);
   atom_del (existing);

   return atom_dup (rt_symbol_add (rt->traps, args[0], args[1]));
}

atom_t *builtins_TRAP_CLEAR (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   sym = sym;
   atom_t *ret = NULL;
   for (size_t i=0; i<nargs; i++) {
      atom_del (ret);
      ret = rt_symbol_remove (rt->symbols, args[i]);
   }

   return ret;
}

atom_t *builtins_TRAP (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   sym = sym;
   nargs = nargs;

   atom_t *ret = NULL;
   atom_t *ZERO = atom_new (atom_INT, "0");
   if (!ZERO) {
      return NULL;
   }

   bool repeat = true;

   while (repeat) {
      const atom_t *trap = rt_symbol_find (rt->traps, args[0]);
      if (!trap)
         break;

      fprintf (stderr, "Received trap [%s], executing handler\n",
                        atom_to_string (atom_list_index (trap, 0)));

      if (atom_list_index (trap, 1)->type == atom_NATIVE) {
         atom_t *tmp = atom_list_new ();
         atom_list_ins_tail (tmp, atom_dup (atom_list_index (trap, 1)));
         atom_list_ins_tail (tmp, atom_new (atom_QUOTE, NULL));
         atom_list_ins_tail (tmp, atom_dup (atom_list_index (trap, 0)));
         for (size_t i=1; args[i]; i++) {
            atom_list_ins_tail (tmp, atom_dup (args[i]));
         }
         ret = rt_eval (rt, sym, tmp);
         atom_del (tmp);
      } else {
         ret = rt_eval (rt, sym, atom_list_index (trap, 1));
      }

      if (ret && atom_cmp (ret, ZERO)==0) {
         repeat = true;
      } else {
         repeat = false;
      }
   }

   atom_del (ZERO);
   return ret;
}

static atom_t *debug_help (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   static const char *help_msg[] = {
      "Commands must be entered on a single line only. Each line is",
      "limited to 1024 bytes.",
      "",
      "help:      This message",
      "bt:        Display the call stack",
      "locals:    Display the local symbol table",
      "globals:   Display the global symbol table",
      "kill:      Terminate the program and runtime",
      "eval:      Print the evaluation of a single expression",
      "resume:    Attempt to resume execution, using the",
      "           expression specified as the evaluation result."
   };

   rt = rt;
   sym = sym;
   args = args;
   nargs = nargs;

   for (size_t i=0; i<sizeof help_msg/sizeof help_msg[0]; i++) {
      fprintf (stderr, "%s\n", help_msg[i]);
   }

   return NULL;
}

static atom_t *debug_bt (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   rt = rt;
   sym = sym;
   args = args;
   nargs = nargs;

   return NULL;
}

static atom_t *debug_locals (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   rt = rt;
   sym = sym;
   args = args;
   nargs = nargs;

   return NULL;
}

static atom_t *debug_globals (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   rt = rt;
   sym = sym;
   args = args;
   nargs = nargs;

   return NULL;
}

static atom_t *debug_kill (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   rt = rt;
   sym = sym;
   args = args;
   nargs = nargs;

   return NULL;
}

static atom_t *debug_eval (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   rt = rt;
   sym = sym;
   args = args;
   nargs = nargs;

   return NULL;
}

static atom_t *debug_resume (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   rt = rt;
   sym = sym;
   args = args;
   nargs = nargs;

   return NULL;
}

struct cmd_t {
   const char *cmd;
   atom_t *(*fptr) (rt_t *, atom_t *, atom_t **, size_t);
};

static const struct cmd_t *debug_find_cmd (const char *cmd)
{
   static const struct cmd_t cmds[] = {
      { "help",      debug_help     },
      { "bt",        debug_bt       },
      { "locals",    debug_locals   },
      { "globals",   debug_globals  },
      { "kill",      debug_kill     },
      { "eval",      debug_eval     },
      { "resume",    debug_resume   },
   };

   for (size_t i=0; i<sizeof cmds/sizeof cmds[0]; i++) {
      if ((strcmp (cmd, cmds[i].cmd))==0)
         return cmds[i].fptr;
   }

   fprintf (stderr, "Command [%s] not found.\n", cmd);
   return NULL;
}

static atom_t *debugger (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs)
{
   char line[1024 + 3];
   char **cmd = NULL;

   fprintf (stderr, "Entered debugger console. Type \"help\" for a list "
                    "of commands.\n");

   memset (line, 0, sizeof line);

   atom_t *ret = NULL;
   fprintf (stderr, "\n> ");
   while (!feof (stdin) && !ferror (stdin) &&
           fgets (line, sizeof line - 2, stdin)) {

      atom_del (ret);
      ret = NULL;

      xstr_delarray (cmd); cmd = NULL;

      char *tmp = strchr (line, '\n');
      if (tmp)
         *tmp = 0;

      cmd = xstr_split (line, " ");
      if (!cmd) {
         fprintf (stderr, "Command [%s] not understood\n", line);
         continue;
      }
      atom_t *(*fptr) (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs);

      fptr = debug_find_cmd (cmd[0]);
      if (!fptr) {
         fprintf (stderr, "\n> ");
         continue;
      }

      if ((ret = fptr (rt, sym, args, nargs)))
         break;

      fprintf (stderr, "\n> ");
   }

   xstr_delarray (cmd); cmd = NULL;

   return ret;
}

atom_t *builtins_TRAP_DFL (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   rt = rt;
   sym = sym;
   nargs = nargs;

   fprintf (stderr, "Default trap handler in '");
   for (size_t i=0; args[i]; i++) {
      atom_print (args[i], 0, stderr);
   }
   fprintf (stderr, "'\n");
   fprintf (stderr, "\nLOCAL SYMBOL TABLE\n");
   atom_print (sym, 0, stderr);
   rt_print (rt, stderr);

   return debugger (rt, (atom_t *)sym, (atom_t **)args,  nargs);
}

atom_t *builtins_LIST (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
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

atom_t *builtins_NAPPEND (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   bool error = true;

   nargs = nargs;
   rt = rt;
   sym = sym;

   if (args[0]->type!=atom_LIST)
      return NULL;

   for (size_t i=1; args[i]; i++) {
      if (!(atom_list_ins_tail ((atom_t *)args[0], atom_dup (args[i]))))
         goto errorexit;
   }

   error = false;

errorexit:

   return error ? NULL : (atom_t *)args[0];
}

atom_t *builtins_SET (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   sym = sym;
   if (nargs != 2) {
      fprintf (stderr, "--------------------------------------\n");
      fprintf (stderr, "Too many arguments for SET (found %zu). Possible "
                       "unterminated list.\n", nargs);
      for (size_t i=0; i<nargs; i++) {
         fprintf (stderr, "element [%zu] = ", i);
         atom_print (args[i], 0, stderr);
      }
      return NULL;
   }

   atom_t *existing = rt_symbol_remove (rt->symbols, args[0]);
   atom_del (existing);

   return atom_dup (rt_symbol_add (rt->symbols, args[0], args[1]));
}

atom_t *builtins_DEFINE (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
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

   if (rt_symbol_find (rt->symbols, args[0])) {
      fprintf (stderr, "--------------------------------------\n");
      fprintf (stderr, "Symbol [%s] already DEFINED.\n", atom_to_string (args[0]));
      return NULL;
   }

   return atom_dup (rt_symbol_add (rt->symbols, args[0], args[1]));
}

atom_t *builtins_UNDEFINE (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   sym = sym;
   atom_t *ret = NULL;
   for (size_t i=0; i<nargs; i++) {
      atom_del (ret);
      ret = rt_symbol_remove (rt->symbols, args[i]);
   }

   return ret;
}

atom_t *builtins_EVAL (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   nargs = nargs;

   atom_t *ret = NULL;

   for (size_t i=0; args[i]; i++) {
      atom_del (ret);
      ret = rt_eval (rt, sym, args[i]);
   }

   return ret;
}

atom_t *builtins_PRINT (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   nargs = nargs;
   rt = rt;
   sym = sym;

   for (size_t i=0; args[i]; i++) {
      atom_print (args[i], 0, stdout);
   }

   return atom_new (atom_NIL, NULL);
}

atom_t *builtins_CONCAT (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
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

atom_t *builtins_LET (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   nargs = nargs;

   atom_t *symbols = sym ? atom_concatenate (sym, args[0], NULL)
                         : atom_dup (args[0]);

   atom_t *ret = rt_eval (rt, symbols, args[1]);

   atom_del (symbols);

   return ret;
}

atom_t *builtins_DEFUN (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   sym = sym;

   if (nargs!=3) {
      fprintf (stderr, "--------------------------------------\n");
      fprintf (stderr, "Too many arguments for FUNCALL (found %zu). Possible "
                       "unterminated list.\n", nargs);
      for (size_t i=0; i<nargs; i++) {
         fprintf (stderr, "element [%zu] = ", i);
         atom_print (args[i], 0, stderr);
      }
      return NULL;
   }

   atom_t *fname = atom_dup (args[0]),
          *fval = atom_list_new ();

   atom_list_ins_tail (fval, atom_dup (args[1]));
   atom_list_ins_tail (fval, atom_dup (args[2]));
   fname->flags |= ATOM_FLAG_FUNC;

   atom_t *ret = atom_dup (rt_symbol_add (rt->symbols, fname, fval));

   // atom_del (ret);
   atom_del (fname);
   atom_del (fval);

   return ret;
}

atom_t *builtins_FUNCALL (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
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

errorexit:

   atom_del (tmpsym);
   atom_del (allsyms);

   return ret;
}

static atom_t *builtins_opcalc (rt_t *rt, const atom_t *sym,
                                const atom_t **args, size_t nargs,
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

atom_t *builtins_PLUS (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   return builtins_opcalc (rt, sym, args, nargs, '+', 0);
}

atom_t *builtins_MINUS (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   return builtins_opcalc (rt, sym, args, nargs, '-', -1);
}

atom_t *builtins_DIVIDE (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   return builtins_opcalc (rt, sym, args, nargs, '/', -1);
}

atom_t *builtins_MULTIPLY (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   return builtins_opcalc (rt, sym, args, nargs, '*', 1);
}

static atom_t *builtins_opcmp (rt_t *rt, const atom_t *sym,
                               const atom_t **args, size_t nargs,
                               int r1, int r2)
{
   sym = sym;
   rt = rt;

   if (nargs < 2) {
      fprintf (stderr, "--------------------------------------\n");
      fprintf (stderr, "Too few arguments for CMP (found %zu).\n", nargs);
      for (size_t i=0; i<nargs; i++) {
         fprintf (stderr, "element [%zu] = ", i);
         atom_print (args[i], 0, stderr);
      }
      return NULL;
   }

   const atom_t *lhs = args[0];
   const atom_t *rhs = args[1];

   size_t i=1;
   int result = r1;
   while ((result==r1 || result==r2) && lhs && rhs) {
      result = atom_cmp (lhs, rhs);
      lhs = rhs;
      rhs = args[++i];
   }

   // We use 0 for false, and 1 for true
   result = (result==r1 || result==r2) ? 1 : 0;

   return atom_int_new (result);
}

atom_t *builtins_LT (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   return builtins_opcmp (rt, sym, args, nargs, -1, -1);
}

atom_t *builtins_LE (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   return builtins_opcmp (rt, sym, args, nargs, -1, 0);
}

atom_t *builtins_GT (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   return builtins_opcmp (rt, sym, args, nargs, 1, 1);
}

atom_t *builtins_GE (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   return builtins_opcmp (rt, sym, args, nargs, 1, 0);
}

atom_t *builtins_EQ (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   return builtins_opcmp (rt, sym, args, nargs, 0, 0);
}


atom_t *builtins_IF (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs)
{
   rt = rt;
   sym = sym;
   nargs = nargs;

   const atom_t *expr = args[0],
                *iftrue = args[1],
                *iffalse = args[2];

   if (expr && ((expr->type==atom_INT && *(int64_t *)expr->data!=0) ||
                (expr->type==atom_FLOAT && *(double *)expr->data!=0))) {
      return rt_eval (rt, sym, iftrue);
   } else {
      return rt_eval (rt, sym, iffalse);
   }

   // Shut the compiler up, this never gets executed.
   return NULL;
}

