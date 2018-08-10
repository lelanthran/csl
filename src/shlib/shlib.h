
#ifndef H_SHLIB
#define H_SHLIB

#include <stdbool.h>

typedef struct shlib_t shlib_t;

#ifdef __cplusplus
extern "C" {
#endif

   shlib_t *shlib_new (void);
   void shlib_del (shlib_t *shlib);

   // Don't include any path or extensions. Appropriate paths will be
   // used.
   bool shlib_loadlib (shlib_t *shlib, const char *name);

#ifdef __cplusplus
};
#endif

#endif

