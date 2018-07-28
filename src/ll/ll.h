
#ifndef H_LL
#define H_LL

#ifdef __cplusplus
extern "C" {
#endif

   void **ll_new (void);
   void ll_del (void **ll);

   size_t ll_length (void **ll);
   void *ll_index (void **ll, size_t i);

   void *ll_ins_tail (void ***ll, const void *el);

#ifdef __cplusplus
};
#endif

#endif


