
#ifndef H_SHLIB
#define H_SHLIB

#include "xshare/xshare.h"

typedef enum {
   shlib_NONE = 0,
   shlib_VOID,
   shlib_NULL,
   shlib_UINT8_T,
   shlib_UINT16_T,
   shlib_UINT32_T,
   shlib_UINT64_T,
   shlib_INT8_T,
   shlib_INT16_T,
   shlib_INT32_T,
   shlib_INT64_T,
   shlib_DOUBLE,
   shlib_SIZE_T,
   shlib_S_CHAR,
   shlib_S_SHORT,
   shlib_S_INT,
   shlib_S_LONG,
   shlib_S_LONG_LONG,
   shlib_U_CHAR,
   shlib_U_SHORT,
   shlib_U_INT,
   shlib_U_LONG,
   shlib_U_LONG_LONG,
   shlib_POINTER
} shlib_type_t;


typedef struct shlib_t shlib_t;

struct shlib_pair_t {
   shlib_type_t type;
   const void *data;
};

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

   // Parameters after 'return_value' must be in pairs, with the first of
   // the pair being shlib_type_t and the second being the actual type.
   int shlib_funcall (shlib_t *shlib, const char *func, const char *lib,
                      shlib_type_t return_type, void *return_value,
                      struct shlib_pair_t **args);

#ifdef __cplusplus
};
#endif

#endif

