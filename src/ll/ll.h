
#ifndef H_LL
#define H_LL

#ifdef __cplusplus
extern "C" {
#endif

   void **ll_new (void);
   void ll_del (void **ll);
   void **ll_copy (void **src, size_t from_index, size_t to_index);

   size_t ll_length (void **ll);
   void *ll_index (void **ll, size_t i);
   void ll_iterate (void **ll, void (*fptr) (void *));

   void *ll_ins_tail (void ***ll, const void *el);
   void *ll_ins_head (void ***ll, const void *el);


#ifdef __cplusplus
};
#endif

#endif


