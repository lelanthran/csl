
#ifndef H_BUILTINS
#define H_BUILTINS

#include "rt/rt.h"
#include "parser/atom.h"

typedef atom_t *(rt_builtins_fptr_t) (rt_t *rt, const atom_t *, const atom_t **, size_t);

#ifdef __cplusplus
extern "C" {
#endif

atom_t *builtins_TRAP_SET (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_TRAP_CLEAR (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_TRAP (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_TRAP_DFL (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);

atom_t *builtins_LIST (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_FIRST (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_REST (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_LENGTH (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);

atom_t *builtins_NAPPEND (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);

atom_t *builtins_SET (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_DEFINE (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_UNDEFINE (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);

atom_t *builtins_EVAL (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_PRINT (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);

atom_t *builtins_CONCAT (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);

atom_t *builtins_LET (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_DEFUN (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_DEFEXT (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_FUNCALL (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);

atom_t *builtins_PLUS (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_MINUS (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_DIVIDE (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_MULTIPLY (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);

atom_t *builtins_LT (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_LE (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_GT (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_GE (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_EQ (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);

atom_t *builtins_BIT_AND (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_BIT_OR (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_BIT_XOR (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_BIT_NAND (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_BIT_NOR (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_BIT_NXOR (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_BIT_NOT (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);

atom_t *builtins_LOG_AND (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_LOG_OR (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_LOG_NAND (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_LOG_NOR (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);
atom_t *builtins_LOG_NOT (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);


atom_t *builtins_IF (rt_t *rt, const atom_t *sym, const atom_t **args, size_t nargs);

#ifdef __cplusplus
};
#endif

#endif


