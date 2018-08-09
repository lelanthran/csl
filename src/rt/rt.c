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

const atom_t *rt_symbol_add (atom_t *symbols, const atom_t *name, const atom_t *value)
{
   bool error = true;
   const atom_t *ret = NULL;
   const atom_t *tmp[] = {
      name, value, NULL,
   };

   if (!symbols || !name || !value)
      return NULL;

   atom_t *tlist = builtins_LIST (NULL, NULL, tmp, 2);
   if (!tlist) {
      goto errorexit;
   }

   tlist->flags = name->flags;
   ((atom_t *)atom_list_index (tlist, 0))->flags = name->flags;
   ((atom_t *)atom_list_index (tlist, 1))->flags = name->flags;

   tmp[0] = symbols;
   tmp[1] = tlist;

   if ((ret = builtins_NAPPEND (NULL, NULL, tmp, 2))==NULL)
      goto errorexit;

   error = false;

errorexit:

   if (error) {
      ret = NULL;
   } else {
      ret = rt_symbol_find (symbols, name);
   }

   //atom_del (ret);
   atom_del (tlist);
   //atom_del (name);
   //atom_del (value);

   return ret;
}

const atom_t *rt_symbol_find (const atom_t *symbols, const atom_t *name)
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

   if (!symbols || !name)
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

static const atom_t *add_native_func (atom_t *symbols,
                                      const char *name,
                                      rt_builtins_fptr_t *fptr)
{
   atom_t *sym = NULL;
   atom_t *value = NULL;
   const atom_t *ret = NULL;

   sym = atom_new (atom_SYMBOL, name);
   value = rt_atom_native (fptr);

   ret = rt_symbol_add (symbols, sym, value);

   atom_del (sym);
   atom_del (value);

   return ret;
}

const atom_t *rt_add_native_func (rt_t *rt, const char *name,
                                            rt_builtins_fptr_t *fptr)
{
   return add_native_func (rt->symbols, name, fptr);
}

const atom_t *rt_set_native_trap (rt_t *rt, const char *name,
                                            rt_builtins_fptr_t *fptr)
{
   return add_native_func (rt->traps, name, fptr);
}

static struct g_native_funcs_t {
   const char *name;
   rt_builtins_fptr_t *fptr;
} g_native_funcs[] = {
   {  "bi_list",        builtins_LIST        },
   {  "bi_nappend",     builtins_NAPPEND     },
   {  "bi_set",         builtins_SET         },
   {  "bi_define",      builtins_DEFINE      },
   {  "bi_undefine",    builtins_UNDEFINE    },
   {  "bi_eval",        builtins_EVAL        },
   {  "bi_print",       builtins_PRINT       },
   {  "bi_concat",      builtins_CONCAT      },
   {  "bi_let",         builtins_LET         },
   {  "bi_defun",       builtins_DEFUN       },
   {  "bi_funcall",     builtins_FUNCALL     },
   {  "bi_trap_set",    builtins_TRAP_SET    },
   {  "bi_trap_clear",  builtins_TRAP_CLEAR  },
   {  "bi_trap",        builtins_TRAP        },

   {  "<",              builtins_LT          },
   {  "<=",             builtins_LE          },
   {  ">",              builtins_GT          },
   {  ">=",             builtins_GE          },
   {  "=",              builtins_EQ          },

   {  "if",             builtins_IF          },

   {  "+",              builtins_PLUS        },
   {  "-",              builtins_MINUS       },
   {  "/",              builtins_DIVIDE      },
   {  "*",              builtins_MULTIPLY    },
};

static const char *g_default_traps[] = {
   // First, we try to handle all the system signals. If these are set to
   // builtins_TRAP_DFL then we don't try to catch them
   "SIGHUP",
   "SIGINT",
   "SIGQUIT",
   "SIGILL",
   "SIGABRT",
   "SIGFPE",
   "SIGKILL",
   "SIGSEGV",
   "SIGPIPE",
   "SIGALRM",
   "SIGTERM",
   "SIGUSR1",
   "SIGUSR2",
   "SIGCHLD",
   "SIGCONT",
   "SIGSTOP",
   "SIGTSTP",
   "SIGTTIN",
   "SIGTTOU",
   "SIGBUS",
   "SIGPOLL",
   "SIGPROF",
   "SIGSYS",
   "SIGTRAP",
   "SIGURG",
   "SIGVTALRM",
   "SIGXCPU",
   "SIGXFSZ",

   // All of these traps we catch, these are all specific to oiur running
   // program.
   "TRAP_NOPARAM",
   "TRAP_EVAL_FAIL",
};

rt_t *rt_new (void)
{
   bool error = true;
   rt_t *ret = NULL;

   if (!(ret = calloc (1, sizeof *ret)))
      goto errorexit;

   ret->symbols = atom_list_new ();
   ret->stack = atom_list_new ();
   ret->traps = atom_list_new ();

   for (size_t i=0; i<sizeof g_native_funcs/sizeof g_native_funcs[0]; i++) {
      if (!rt_add_native_func (ret, g_native_funcs[i].name,
                                    g_native_funcs[i].fptr)) {
         goto errorexit;
      }
   }

   for (size_t i=0; i<sizeof g_default_traps/sizeof g_default_traps[0]; i++) {
      if (!rt_set_native_trap (ret, g_default_traps[i], builtins_TRAP_DFL)) {
         goto errorexit;
      }
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
   atom_del (rt->traps);

   free (rt);
}

const atom_t *rt_eval_symbol (rt_t *rt, const atom_t *sym, const atom_t *atom)
{
   const atom_t *entry = NULL;

   if (!rt || !atom)
      return NULL;

   entry = rt_symbol_find (sym, atom);
   if (entry) {
      return atom_list_index (entry, 1);
   }

   entry = rt_symbol_find (rt->symbols, atom);
   if (!entry) {
      XERROR ("Failed to find symbol [%s]\n", atom_to_string (atom));
      // rt_print_globals (rt, stdout);
      // rt_print_numbered_list (sym, stdout);
      return NULL;
   }

   return atom_list_index (entry, 1);
}

static atom_t *make_stack_entry (const atom_t *sym, const atom_t **args,
                                                    size_t nargs)
{
   bool error = true;

   atom_t *symtable = atom_list_new ();
   atom_t *ret = atom_list_new ();

   if (!ret) {
      goto errorexit;
   }

   const atom_t *tmp = sym ? sym : symtable;

   if (!atom_list_ins_tail (ret, atom_dup (tmp))) {
      goto errorexit;
   }

   for (size_t i=0; args[i] && i<nargs; i++) {
      if (!atom_list_ins_tail (ret, atom_dup (args[i]))) {
         goto errorexit;
      }
   }

   error = false;

errorexit:

   atom_del (symtable);

   if (error) {
      atom_del (ret);
      ret = NULL;
   }

   return ret;
}

static atom_t *rt_funcall_native (rt_t *rt, const atom_t *sym,
                                  const atom_t **args, size_t nargs)
{
   rt_builtins_fptr_t *fptr = args[0]->data;

   return fptr (rt, sym, &args[1], nargs);
}

static atom_t *rt_funcall_interp (rt_t *rt, const atom_t *sym,
                                  const atom_t **args, size_t nargs)
{
   atom_t *ret = NULL,
          *fargs = atom_list_new ();

   nargs = nargs;

   for (size_t i=1; args[i]; i++) {
      if (!atom_list_ins_tail (fargs, atom_dup (args[i])))
         goto errorexit;
   }

   const atom_t *fc_args[] = { fargs, args[0], NULL };

   ret = builtins_FUNCALL (rt, sym, fc_args, 2);

errorexit:
   atom_del (fargs);
   return ret;
}

static size_t depth;

static atom_t *rt_list_eval (rt_t *rt, const atom_t *sym, const atom_t *atom)
{
   atom_t *ret = NULL;
   atom_t *func = NULL;
   atom_t *sinfo = NULL;

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

   sinfo = make_stack_entry (atom_list_index (atom, 0),
                            (const atom_t **)args, nargs);
   if (!sinfo) {
      goto errorexit;
   }

   if (!atom_list_ins_tail (rt->stack, sinfo)) {
      goto errorexit;
   }

   switch (func->type) {
      case atom_FFI:
         // TODO: Implement FFI
         break;

      case atom_NATIVE:
         ret = rt_funcall_native (rt, sym, (const atom_t **)args, --nargs);
         break;

      default:
         ret = func->flags == ATOM_FLAG_FUNC ?
               rt_funcall_interp (rt, sym, (const atom_t **)args, --nargs) :
               NULL;

         if (ret) ret->flags = 0;
         break;
   }

   atom_list_remove_tail (rt->stack);

   if (!ret) {
      ret = atom_list_new ();
      for (size_t i=0; i<nargs; i++) {
         atom_list_ins_tail (ret, atom_dup (ll_index (args, i)));
      }
   }

errorexit:

   atom_del (sinfo);

   ll_iterate (args, (void (*) (void *))atom_del);
   ll_del (args);

   depth--;
   return ret;
}

atom_t *rt_eval (rt_t *rt, const atom_t *sym, const atom_t *atom)
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
      atom_print (atom, 0, stdout);
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

void rt_print_numbered_list (atom_t *list, FILE *outf)
{
   if (!list || !outf)
      return;

   size_t len = atom_list_length (list);
   for (size_t i=0; i<len; i++) {
      fprintf (outf, "[%zu]: ", i);
      atom_print (atom_list_index (list, i), 0, outf);
      fprintf (outf, "\n");
   }
}

void rt_print_globals (rt_t *rt, FILE *outf)
{
   fprintf (outf, "\nSYMBOL TABLE\n");
   rt_print_numbered_list (rt->symbols, outf);
}

void rt_print_call_stack (rt_t *rt, FILE *outf)
{
   fprintf (outf, "\nCALL STACK\n");
   rt_print_numbered_list (rt->stack, outf);
}

void rt_print_traps (rt_t *rt, FILE *outf)
{
   fprintf (outf, "\nTRAP HANDLERS\n");
   rt_print_numbered_list (rt->traps, outf);
}


void rt_print (rt_t *rt, FILE *outf)
{
   if (!outf)
      outf = stdout;

   if (!rt) {
      fprintf (outf, "NULL Runtime Object\n");
      return;
   }

   /*
   fprintf (outf, "\nSYMBOL TABLE\n");
   atom_print (rt->symbols, 0, outf);
   fprintf (outf, "\nCALL STACK\n");
   atom_print (rt->stack, 0, outf);
   fprintf (outf, "\nTRAP HANDLERS\n");
   atom_print (rt->traps, 0, outf);
   fprintf (outf, "\n");
   */
   rt_print_globals (rt, outf);
   rt_print_call_stack (rt, outf);
   rt_print_traps (rt, outf);
   fprintf (outf, "\n");
}

