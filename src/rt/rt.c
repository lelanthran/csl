#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "rt/rt.h"
#include "rt/builtins.h"
#include "shlib/shlib.h"

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

atom_t *rt_trap_a (rt_t *rt, atom_t *sym, atom_t *trap, atom_t **args, atom_t **extra)
{
   atom_t *ret = NULL;
   atom_t **args_array = NULL;

   size_t nargs = 0;
   for (size_t i=0; extra && extra[i]; i++)
      nargs++;

   for (size_t i=0; args && args[i]; i++)
      nargs++;

   args_array = malloc (sizeof *args_array * (nargs + 2));
   if (!args_array) {
      fprintf (stderr, "Out of memory handling trap [%s]\n",
                        (char *)trap->data);
      exit (-1);
   }
   memset (args_array, 0, sizeof *args_array * (nargs + 2));

   args_array[0] = trap;
   size_t i = 0;
   while (args && args[i]) {
      args_array[i+1] = args[i];
      i++;
   }
   size_t j = 0;
   while (extra && extra[j]) {
      args_array[i+1] = extra[j];
      i++; j++;
   }

   ret = builtins_TRAP (rt, sym, (const atom_t **)args_array, nargs+1);

   free (args_array);

   atom_del (trap);

   for (size_t i=0; extra && extra[i]; i++) {
      atom_del (extra[i]);
   }
   free (extra);

   return ret;
}

atom_t *rt_trap_v (rt_t *rt, atom_t *sym, atom_t *trap, atom_t **args, va_list ap)
{
   bool error = true;
   size_t nargs = 0;
   atom_t **extra = NULL;
   atom_t *ret = NULL;
   atom_t *arg = NULL;

   while ((arg = va_arg (ap, atom_t *))!=NULL) {
      nargs++;
      atom_t **tmp = realloc (extra, sizeof *tmp * (nargs + 1));
      if (!tmp) {
         fprintf (stderr, "Fatal OOM error in trap handling\n");
         atom_print (trap, 0, stderr);
         exit (-1);
      }
      extra = tmp;

      extra[nargs - 1] = arg;
      extra[nargs] = 0;
   }

   if (!(ret = rt_trap_a (rt, sym, trap, args, extra)))
      goto errorexit;

   error = false;

errorexit:

   for (size_t i=0; extra && extra[i]; i++) {
      atom_del (extra[i]);
   }

   if (error) {
      atom_del (ret);
      ret = NULL;
   }

   return ret;
}

atom_t *rt_trap (rt_t *rt, atom_t *sym, atom_t *trap, atom_t **args, ...)
{
   va_list ap;
   atom_t *ret = NULL;

   va_start (ap, args);

   ret = rt_trap_v (rt, sym, trap, args, ap);

   va_end (ap);

   return ret;
}

void rt_warn_v (rt_t *rt, const atom_t *sym, atom_t *warning, va_list ap)
{
   rt = rt;
   sym = sym;

   atom_t *arg = warning;

   fprintf (stderr, "WARNING: [%s]\n", atom_to_string (warning));
   atom_del (warning);
   while ((arg = va_arg (ap, atom_t *))!=NULL) {
      atom_print (arg, 0, stderr);
      atom_del (arg);
   }
}

void rt_warn (rt_t *rt, const atom_t *sym, atom_t *warning, ...)
{
   va_list ap;

   va_start (ap, warning);

   rt_warn_v (rt, sym, warning, ap);

   va_end (ap);

}


static const struct {
   shlib_type_t type;
   const char *name;
} g_native_types[] = {
   { shlib_NONE,        "NONE"        },
   { shlib_VOID,        "VOID"        },
   { shlib_NULL,        "NULL"        },
   { shlib_UINT8_T,     "UINT8_T"     },
   { shlib_UINT16_T,    "UINT16_T"    },
   { shlib_UINT32_T,    "UINT32_T"    },
   { shlib_UINT64_T,    "UINT64_T"    },
   { shlib_INT8_T,      "INT8_T"      },
   { shlib_INT16_T,     "INT16_T"     },
   { shlib_INT32_T,     "INT32_T"     },
   { shlib_INT64_T,     "INT64_T"     },
   { shlib_FLOAT,       "FLOAT"       },
   { shlib_DOUBLE,      "DOUBLE"      },
   { shlib_SIZE_T,      "SIZE_T"      },
   { shlib_S_CHAR,      "S_CHAR"      },
   { shlib_S_SHORT,     "S_SHORT"     },
   { shlib_S_INT,       "S_INT"       },
   { shlib_S_LONG,      "S_LONG"      },
   { shlib_S_LONG_LONG, "S_LONG_LONG" },
   { shlib_U_CHAR,      "U_CHAR"      },
   { shlib_U_SHORT,     "U_SHORT"     },
   { shlib_U_INT,       "U_INT"       },
   { shlib_U_LONG,      "U_LONG"      },
   { shlib_U_LONG_LONG, "U_LONG_LONG" },
   { shlib_POINTER,     "POINTER"     },
   // Repeated for convenience
   { shlib_S_CHAR,      "CHAR"        },
   { shlib_S_SHORT,     "SHORT"       },
   { shlib_S_INT,       "INT"         },
   { shlib_S_LONG,      "LONG"        },
   { shlib_S_LONG_LONG, "LONG_LONG"   },
};

static int64_t g_highest_type_id = 0;

int64_t rt_highest_type_id (void)
{
   return g_highest_type_id;
}

const atom_t *rt_add_native_type (rt_t *rt,
                                  const char *name,
                                  int64_t type_id,
                                  int64_t size,
                                  int64_t alignment)
{
   bool error = true;

   const atom_t *ret = NULL;

   atom_t *sym = atom_new (atom_SYMBOL, name);
   atom_t *val = atom_list_new ();
   if (!val)
      goto errorexit;

   if ((atom_list_ins_tail (val, atom_int_new (type_id)))==NULL)
      goto errorexit;

   if (!(atom_list_ins_tail (val, atom_int_new (size))))
      goto errorexit;

   if (!(atom_list_ins_tail (val, atom_int_new (alignment))))
      goto errorexit;

   ret = rt_symbol_add (rt->symbols, sym, val);

   g_highest_type_id = g_highest_type_id < type_id ?
                           type_id : g_highest_type_id;

   error = false;

errorexit:

   if (error) {
      atom_del ((atom_t *)ret);
      ret = NULL;
   }

   atom_del (sym);
   atom_del (val);

   return ret;
}

static struct {
   const char *name;
   rt_builtins_fptr_t *fptr;
} g_native_funcs[] = {
   {  "bi_list",        builtins_LIST        },
   {  "bi_first",       builtins_FIRST       },
   {  "bi_rest",        builtins_REST        },
   {  "bi_length",      builtins_LENGTH      },

   {  "bi_nappend",     builtins_NAPPEND     },
   {  "bi_nalloc",      builtins_NALLOC      },
   {  "bi_nfree",       builtins_NFREE       },

   {  "bi_set",         builtins_SET         },
   {  "bi_define",      builtins_DEFINE      },
   {  "bi_undefine",    builtins_UNDEFINE    },
   {  "bi_eval",        builtins_EVAL        },
   {  "bi_print",       builtins_PRINT       },
   {  "bi_concat",      builtins_CONCAT      },
   {  "bi_let",         builtins_LET         },
   {  "bi_defun",       builtins_DEFUN       },
   {  "bi_defext",      builtins_DEFEXT      },
   {  "bi_defstruct",   builtins_DEFSTRUCT   },
   {  "bi_newstruct",   builtins_NEWSTRUCT   },
   {  "bi_deftype",     builtins_DEFTYPE     },
   {  "bi_funcall",     builtins_FUNCALL     },
   {  "bi_trap_set",    builtins_TRAP_SET    },
   {  "bi_trap_clear",  builtins_TRAP_CLEAR  },
   {  "bi_trap",        builtins_TRAP        },

   {  "<",              builtins_LT          },
   {  "<=",             builtins_LE          },
   {  ">",              builtins_GT          },
   {  ">=",             builtins_GE          },
   {  "=",              builtins_EQ          },

   {  "bi_if",          builtins_IF          },
   {  "bi_while",       builtins_WHILE       },

   {  "+",              builtins_PLUS        },
   {  "-",              builtins_MINUS       },
   {  "/",              builtins_DIVIDE      },
   {  "*",              builtins_MULTIPLY    },

   {  "bi_bit_and",     builtins_BIT_AND     },
   {  "bi_bit_or",      builtins_BIT_OR      },
   {  "bi_bit_xor",     builtins_BIT_XOR     },
   {  "bi_bit_nand",    builtins_BIT_NAND    },
   {  "bi_bit_nor",     builtins_BIT_NOR     },
   {  "bi_bit_nxor",    builtins_BIT_NXOR    },
   {  "bi_bit_not",     builtins_BIT_NOT     },

   {  "bi_log_and",     builtins_LOG_AND     },
   {  "bi_log_or",      builtins_LOG_OR      },
   {  "bi_log_nand",    builtins_LOG_NAND    },
   {  "bi_log_nor",     builtins_LOG_NOR     },
   {  "bi_log_not",     builtins_LOG_NOT     },

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

   // All of these traps we catch, these are all specific to our running
   // program.
   "TRAP_PARAMCOUNT",
   "TRAP_NOPARAM",
   "TRAP_EVALERR",
   "TRAP_BADPARAM",
   "TRAP_FFI",
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
   ret->shlib = shlib_new ();

   if (!ret->symbols || !ret->stack || !ret->traps || !ret->shlib)
      goto errorexit;

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

   for (size_t i=0; i<sizeof g_native_types/sizeof g_native_types[0]; i++) {
      // We put this in twice, sometime in the future I may need to store
      // alignment separately from size. For now I assume that the alignment
      // and size are the same.
      if (!(rt_add_native_type (ret, g_native_types[i].name,
                                     g_native_types[i].type,
                                     shlib_sizeof (g_native_types[i].type),
                                     shlib_sizeof (g_native_types[i].type)))) {
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
   shlib_del (rt->shlib);

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

static bool check_type_compatible (const atom_t *actual, const atom_t *expected)
{
   int64_t actual_type = *(int64_t *)actual->data,
           expected_type = *(int64_t *)expected->data;

   if (actual_type == expected_type)
      return true;

   if (actual->type==atom_STRING && expected_type==shlib_POINTER)
      return true;

   if (actual->type==atom_INT && expected_type==shlib_FLOAT)
      return true;

   if (actual->type==atom_INT && expected_type==shlib_DOUBLE)
      return true;

   size_t s_actual = shlib_sizeof (actual_type),
          s_expected = shlib_sizeof (expected_type);

   if (s_actual <= s_expected)
      return true;

   return false;
}

static shlib_type_t promote_atom_to_native_type (const atom_t *src)
{
   if (src->type==atom_INT) {
      int64_t type = *(int64_t *)src;
      return (shlib_type_t) type;
   }

   if (src->type==atom_STRING || src->type==atom_SYMBOL ||
       src->type==atom_NATIVE || src->type==atom_FFI) {
      return shlib_POINTER;
   }

   if (src->type==atom_FLOAT) {
      return shlib_DOUBLE;
   }

   return 0;
}

static void *promote_atom_to_native_data (const atom_t *src,
                                                shlib_type_t type)
{
   uint8_t *b = NULL;
   size_t blen = 0;
   void *ret = malloc (8);
   if (!ret)
      return NULL;

   switch (type) {
   case shlib_NONE:
   case shlib_VOID:
   case shlib_NULL:        free (ret); ret = NULL;                             break;

   case shlib_UINT8_T:     *(uint8_t *)ret = *(int64_t *)src->data;            break;
   case shlib_UINT16_T:    *(uint16_t *)ret = *(int64_t *)src->data;           break;
   case shlib_UINT32_T:    *(uint32_t *)ret = *(int64_t *)src->data;           break;
   case shlib_UINT64_T:    *(uint64_t *)ret = *(int64_t *)src->data;           break;
   case shlib_INT8_T:      *(int8_t *)ret = *(int64_t *)src->data;             break;
   case shlib_INT16_T:     *(int16_t *)ret = *(int64_t *)src->data;            break;
   case shlib_INT32_T:     *(int32_t *)ret = *(int64_t *)src->data;            break;
   case shlib_INT64_T:     *(int64_t *)ret = *(int64_t *)src->data;            break;
   case shlib_FLOAT:       *(float *)ret = *(float *)src->data;                break;
   case shlib_DOUBLE:      *(double *)ret = *(double *)src->data;              break;
   case shlib_SIZE_T:      *(size_t *)ret = *(int64_t *)src->data;             break;
   case shlib_S_CHAR:      *(signed char *)ret = *(int64_t *)src->data;        break;
   case shlib_S_SHORT:     *(signed short *)ret = *(int64_t *)src->data;       break;
   case shlib_S_INT:       *(signed int *)ret = *(int64_t *)src->data;         break;
   case shlib_S_LONG:      *(signed long *)ret = *(int64_t *)src->data;        break;
   case shlib_S_LONG_LONG: *(signed long long *)ret = *(int64_t *)src->data;   break;
   case shlib_U_CHAR:      *(unsigned char *)ret = *(int64_t *)src->data;      break;
   case shlib_U_SHORT:     *(unsigned short *)ret = *(int64_t *)src->data;     break;
   case shlib_U_INT:       *(unsigned int *)ret = *(int64_t *)src->data;       break;
   case shlib_U_LONG:      *(unsigned long *)ret = *(int64_t *)src->data;      break;
   case shlib_U_LONG_LONG: *(unsigned long long *)ret = *(int64_t *)src->data; break;
   case shlib_POINTER:     // Tricky
                           blen = *(size_t *)src->data;
                           b = src->data;
                           b = &b[sizeof (size_t)];

                           void **tmp = NULL;
                           if (!(tmp = malloc (sizeof *tmp))) {
                              fprintf (stderr, "OOM\n");
                              exit (-1);
                           }
                           memset (tmp, 0, sizeof *tmp);

                           tmp[0] = malloc (blen);
                           if (!tmp[0]) {
                              fprintf (stderr, "OOM\n");
                              exit (-1);
                           }
                           memcpy (tmp[0], b, blen);

                           free (ret); ret = tmp;

                           printf ("[%p]\n[%p]\n", ret, b);
                           break;
   }

   if (type==shlib_FLOAT) {
      double tmpd = *(double *)src->data;
      float tmpf = tmpd;
      *(float *)ret = tmpf;
   }

   return ret;
}

static atom_t *promote_native_to_atom (const shlib_type_t type, void *data)
{
   atom_t *ret = atom_new (atom_UNKNOWN, NULL);
   if (!ret)
      return NULL;

   ret->data = malloc (8);
   if (!ret->data) {
      free (ret);
      return NULL;
   }
   memset (ret->data, 0, 8);

   ret->type = atom_INT;

   switch (type) {
   case shlib_NONE:
   case shlib_VOID:
   case shlib_NULL:        free (ret->data); free (ret); ret = NULL;             break;

   case shlib_UINT8_T:     *(uint64_t *)ret->data = *(uint8_t  *)data;           break;
   case shlib_UINT16_T:    *(uint64_t *)ret->data = *(uint16_t *)data;           break;
   case shlib_UINT32_T:    *(uint64_t *)ret->data = *(uint32_t *)data;           break;
   case shlib_UINT64_T:    *(uint64_t *)ret->data = *(uint64_t *)data;           break;
   case shlib_INT8_T:      *(int64_t  *)ret->data = *(int8_t   *)data;           break;
   case shlib_INT16_T:     *(int64_t  *)ret->data = *(int16_t  *)data;           break;
   case shlib_INT32_T:     *(int64_t  *)ret->data = *(int32_t  *)data;           break;
   case shlib_INT64_T:     *(int64_t  *)ret->data = *(int64_t  *)data;           break;
   case shlib_FLOAT:       *(float    *)ret->data = *(float    *)data;           break;
   case shlib_DOUBLE:      *(double   *)ret->data = *(double   *)data;           break;
   case shlib_SIZE_T:      *(uint64_t *)ret->data = *(size_t   *)data;           break;
   case shlib_S_CHAR:      *(int64_t  *)ret->data = *(signed char        *)data; break;
   case shlib_S_SHORT:     *(int64_t  *)ret->data = *(signed short       *)data; break;
   case shlib_S_INT:       *(int64_t  *)ret->data = *(signed int         *)data; break;
   case shlib_S_LONG:      *(int64_t  *)ret->data = *(signed long        *)data; break;
   case shlib_S_LONG_LONG: *(int64_t  *)ret->data = *(signed long long   *)data; break;
   case shlib_U_CHAR:      *(uint64_t *)ret->data = *(unsigned char      *)data; break;
   case shlib_U_SHORT:     *(uint64_t *)ret->data = *(unsigned short     *)data; break;
   case shlib_U_INT:       *(uint64_t *)ret->data = *(unsigned int       *)data; break;
   case shlib_U_LONG:      *(uint64_t *)ret->data = *(unsigned long      *)data; break;
   case shlib_U_LONG_LONG: *(uint64_t *)ret->data = *(unsigned long long *)data; break;
   case shlib_POINTER:     ret->data = data;                                     break;
   }

   if (type==shlib_DOUBLE)
      ret->type = atom_FLOAT;

   if (type==shlib_POINTER)
      ret->type = atom_FFI;

   return ret;
}

static atom_t *rt_funcall_ffi (rt_t *rt, const atom_t *sym,
                                         const atom_t **args, size_t nargs)
{
   bool error = true;
   atom_t *ret = NULL;
   atom_t *trap = NULL;

   atom_t *eval_args = atom_list_new ();

   struct shlib_pair_t *fargs = NULL;
   void *return_value = NULL;
   shlib_type_t return_type;

   const atom_t *fspec = args[0];
   const atom_t *library = atom_list_index (fspec, 0);
   const atom_t *func = atom_list_index (fspec, 1);
   const atom_t *ret_type = atom_list_index (fspec, 2);

   const atom_t **args_found = &args[1];
   const atom_t *args_expected = (atom_list_index (fspec, 3));

   atom_t *tmp = NULL;

   tmp = rt_eval (rt, sym, ret_type);
   const atom_t *tmp_rt = atom_list_index (tmp, 0);

   if (!eval_args)
      goto errorexit;

   return_type = promote_atom_to_native_type (tmp_rt);
   atom_del (tmp);
   tmp = NULL;
   tmp_rt = NULL;

   return_value = malloc (8);
   if (!return_value) {
      fprintf (stderr, "OOM\n");
      goto errorexit;
   }
   memset (return_value, 0, 8);

   size_t nargs_found = nargs - 1;
   size_t nargs_expected = atom_list_length (atom_list_index (fspec, 3));

   if (nargs_found != nargs_expected) {
      char tmp1[38];
      sprintf (tmp1, "nargs wrong: %zu/%zu", nargs_found, nargs_expected);
      trap = rt_trap (rt, (atom_t *)sym, atom_new (atom_SYMBOL, "TRAP_BADPARAM"),
                                         (atom_t **)args,
                                         atom_new (atom_STRING, tmp1),
                                         NULL);
      goto errorexit;
   }

   fargs = malloc ((sizeof *fargs) * (nargs_found + 1));
   if (!fargs) {
      fprintf (stderr, "OOM\n");
      goto errorexit;
   }

   memset (fargs, 0, (sizeof *fargs) * (nargs_found + 1));

   for (size_t i=1; args[i]; i++) {
      const atom_t *arg_found = rt_eval (rt, sym, args_found[i - 1]);
      const atom_t *arg_expected =
                     atom_list_index ((atom_t *)args_expected, i - 1);

      tmp = rt_eval (rt, sym, arg_expected);
      const atom_t *tmp_expected = atom_list_index (tmp, 0);

      if (check_type_compatible (arg_found, tmp_expected)!=true) {
         trap = rt_trap (rt, (atom_t *)sym,
                             atom_new (atom_SYMBOL, "TRAP_BADPARAM"),
                             (atom_t **)args,
                             atom_new (atom_STRING, "Arg mismatch"),
                             atom_dup (arg_found),
                             atom_dup (arg_expected),
                             NULL);
         atom_del ((atom_t *)tmp_expected);
         goto errorexit;
      }

      fargs[i-1].type = *(int64_t *)tmp_expected->data;
      atom_del (tmp);
      tmp = NULL;
      tmp_expected = NULL;

      if (fargs[i-1].type==shlib_NONE) {
         trap = rt_trap (rt, (atom_t *)sym,
                             atom_new (atom_SYMBOL, "TRAP_BADPARAM"),
                             (atom_t **)args,
                             atom_dup (arg_found),
                             NULL);
         goto errorexit;
      }

      if (fargs[i-1].type >= shlib_POINTER)
         fargs[i-1].type = shlib_POINTER;

      fargs[i-1].data = promote_atom_to_native_data (arg_found, fargs[i-1].type);

      atom_list_ins_tail (eval_args, (atom_t *)arg_found);
   }

   int errcode = shlib_funcall (rt->shlib, atom_to_string (func),
                                           (char *)(library->data),
                                           return_type,
                                           return_value,
                                           fargs);

   if (errcode) {
      trap = rt_trap (rt, (atom_t *)sym,
                          atom_new (atom_SYMBOL, "TRAP_FFI"),
                          (atom_t **)args,
                          atom_dup (atom_list_index (fspec, 0)),
                          atom_dup (atom_list_index (fspec, 1)),
                          NULL);
      goto errorexit;
   }

   if (!(ret = promote_native_to_atom (return_type, return_value)))
      goto errorexit;

   error = false;

errorexit:

   atom_del (eval_args);

   if (error) {
      atom_del (ret);
      ret = NULL;
   }

   if (trap) {
      ret = trap;
   }

   for (size_t i=0; fargs[i].type; i++) {
      if (fargs[i].type==shlib_POINTER) {
         void **tmp = fargs[i].data;
         free (tmp[0]);
      }

      free ((void *)fargs[i].data);
   }
   free (fargs);

   free (return_value);

   return ret;
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

   if (func->type == atom_NATIVE) {

      ret = rt_funcall_native (rt, sym, (const atom_t **)args, --nargs);

   } else {

      ret = NULL;

      if (func->flags & ATOM_FLAG_FUNC)
         ret = rt_funcall_interp (rt, sym, (const atom_t **)args, --nargs);

      if (func->flags & ATOM_FLAG_FFI)
         ret = rt_funcall_ffi (rt, sym, (const atom_t **)args, nargs);

      if (ret) ret->flags = 0;
   }

   atom_list_remove_tail (rt->stack);

   if (!ret) {
      ret = atom_new (atom_NIL, NULL);
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
   atom_t *tmp = NULL;

   switch (atom->type) {
      case atom_NATIVE:
      case atom_FFI:
      case atom_STRING:
      case atom_INT:
      case atom_QUOTE:
      case atom_BUFFER:
      case atom_FLOAT:  tmp = atom_dup (atom);                           break;

      case atom_SYMBOL: tmp = atom_dup (rt_eval_symbol (rt, sym, atom)); break;

      case atom_LIST:   tmp = rt_list_eval (rt, sym, atom);              break;

      case atom_NIL:    tmp = atom_new (atom_NIL, NULL);                 break;
      case atom_ENDL:
      case atom_UNKNOWN:   tmp = NULL;                                   break;
   }

   if (!tmp) {
      fprintf (stderr, "Eval failed [%p:%s]\n", atom, atom_to_string (atom));
      atom_print (atom, 0, stderr);
      tmp  = rt_trap (rt, (atom_t *)sym,
                          atom_new (atom_SYMBOL, "TRAP_EVALERR"),
                          NULL,
                          NULL);
   }

   if (!tmp) {
      fprintf (stderr, "Unhandled trap [TRAP_EVALERR], aborting\n");
      exit (-1);
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

