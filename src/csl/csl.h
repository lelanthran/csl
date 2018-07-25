
#ifndef H_CSL
#define H_CSL

typedef struct csl_src_t csl_src_t;

#ifdef __cplusplus
extern "C" {
#endif

   csl_src_t *csl_src_load (const char *fname);
   void csl_src_del (csl_src_t *csl);

#ifdef __cplusplus
};
#endif

#endif


