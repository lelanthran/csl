
#ifndef H_RT
#define H_RT

#include <stdio.h>

#include "parser/atom.h"

typedef struct rt_t rt_t;
struct rt_t {
   atom_t *symbols;
   atom_t *stack;
   atom_t *roots;

   bool flags; // Reserved for internal use
};

#ifdef __cplusplus
extern "C" {
#endif

   rt_t *rt_new (void);
   void rt_del (rt_t *rt);

   const atom_t *rt_symbol_add (atom_t *symbols, const atom_t *name, const atom_t *value);
   const atom_t *rt_symbol_find (const atom_t *symbols, const atom_t *name);
   atom_t *rt_symbol_remove (atom_t *symbols, const atom_t *name);

   atom_t *rt_eval (rt_t *rt, const atom_t *symbols, const atom_t *atom);

   void rt_print (rt_t *rt, FILE *outf);

#ifdef __cplusplus
};
#endif

#endif

