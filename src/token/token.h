
#ifndef H_TOKEN
#define H_TOKEN

#include <stdlib.h>

typedef struct token_t token_t;

#ifdef __cplusplus
extern "C" {
#endif

   token_t **token_read (const char *fname);
   void token_del (token_t *token);

   const char *token_string (token_t *token);
   const char *token_fname (token_t *token);
   size_t      token_line (token_t *token);
   size_t      token_charpos (token_t *token);

#ifdef __cplusplus
};
#endif

#endif


