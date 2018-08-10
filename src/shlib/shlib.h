
#ifndef H_SHLIB
#define H_SHLIB

typedef struct shlib_t shlib_t;

#ifdef __cplusplus
extern "C" {
#endif

   shlib_t *shlib_new (void);
   void shlib_del (shlib_t *shlib);


#ifdef __cplusplus
};
#endif

#endif

