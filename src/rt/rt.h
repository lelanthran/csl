
#ifndef H_RT
#define H_RT

#include "parser/atom.h"

typedef struct rt_t rt_t;
struct rt_t {
   atom_t *symbols;
   atom_t *stack;
   atom_t *roots;
};

#ifdef __cplusplus
extern "C" {
#endif

   rt_t *rt_new (void);
   void rt_del (rt_t *rt);


#ifdef __cplusplus
};
#endif

#endif

