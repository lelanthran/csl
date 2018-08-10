
#ifndef H_SHLIB
#define H_SHLIB

#include "xshare/xshare.h"


typedef struct shlib_t shlib_t;

#ifdef __cplusplus
extern "C" {
#endif

   shlib_t *shlib_new (void);
   void shlib_del (shlib_t *shlib);

   // Don't include any path or extensions. Appropriate paths will be
   // used.
   xshare_library_t shlib_loadlib (shlib_t *shlib, const char *name);
   xshare_symbol_t shlib_loadfunc (shlib_t *shlib, const char *func,
                                                   const char *lib);

#ifdef __cplusplus
};
#endif

#endif

