
#ifndef H_RT
#define H_RT

#include <stdio.h>
#include <stdarg.h>

#include "parser/atom.h"
#include "shlib/shlib.h"


typedef struct rt_t rt_t;
struct rt_t {
   atom_t *symbols;
   atom_t *stack;
   atom_t *traps;

   shlib_t *shlib;

   bool flags; // Reserved for internal use
};

#include "rt/builtins.h"

typedef atom_t *(rt_builtins_fptr_t) (rt_t *rt, const atom_t *, const atom_t **, size_t);

#ifdef __cplusplus
extern "C" {
#endif

   rt_t *rt_new (void);
   void rt_del (rt_t *rt);

   const atom_t *rt_symbol_add (atom_t *symbols, const atom_t *name, const atom_t *value);
   const atom_t *rt_symbol_find (const atom_t *symbols, const atom_t *name);
   atom_t *rt_symbol_remove (atom_t *symbols, const atom_t *name);

   const atom_t *rt_add_native_func (rt_t *rt, const char *name,
                                               rt_builtins_fptr_t *fptr);

   const atom_t *rt_set_native_trap (rt_t *rt, const char *name,
                                               rt_builtins_fptr_t *fptr);

   // Note: rt_trap() functions free the symbol and the parameters
   // passed before returning. Caller must free the return value.
   //
   // The variadic functions each look for successive atom_t * and stop
   // when they encounter a NULL.
   atom_t *rt_trap_a (rt_t *rt, atom_t *sym, atom_t *trap, atom_t **args);
   atom_t *rt_trap_v (rt_t *rt, atom_t *sym, atom_t *trap, va_list ap);
   atom_t *rt_trap (rt_t *rt, atom_t *sym, atom_t *trap, ...);

   void rt_warn_v (rt_t *rt, atom_t *sym, atom_t *trap, va_list ap);
   void rt_warn (rt_t *rt, atom_t *sym, atom_t *warning, ...);

   atom_t *rt_eval (rt_t *rt, const atom_t *symbols, const atom_t *atom);



   void rt_print_numbered_list (atom_t *list, FILE *outf);
   void rt_print_globals (rt_t *rt, FILE *outf);
   void rt_print_call_stack (rt_t *rt, FILE *outf);
   void rt_print_traps (rt_t *rt, FILE *outf);

   void rt_print (rt_t *rt, FILE *outf);

#ifdef __cplusplus
};
#endif

#endif

