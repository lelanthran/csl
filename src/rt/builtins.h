
#ifndef H_BUILTINS
#define H_BUILTINS

#include "rt/rt.h"
#include "parser/atom.h"

typedef atom_t *(rt_builtins_fptr_t) (rt_t *rt, atom_t *, atom_t **, size_t);

#ifdef __cplusplus
extern "C" {
#endif

atom_t *builtins_LIST (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs);
atom_t *builtins_NAPPEND (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs);

atom_t *builtins_DEFINE (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs);
atom_t *builtins_UNDEFINE (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs);

atom_t *builtins_EVAL (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs);
atom_t *builtins_PRINT (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs);

atom_t *builtins_CONCAT (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs);
atom_t *builtins_LET (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs);

atom_t *builtins_PLUS (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs);
atom_t *builtins_MINUS (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs);
atom_t *builtins_DIVIDE (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs);
atom_t *builtins_MULTIPLY (rt_t *rt, atom_t *sym, atom_t **args, size_t nargs);


#ifdef __cplusplus
};
#endif

#endif


