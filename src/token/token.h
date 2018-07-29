
#ifndef H_TOKEN
#define H_TOKEN

#include <stdlib.h>

typedef struct token_t token_t;

enum token_type_t {
   token_UNKNOWN = 0,
   token_STARTL,
   token_QUOTE,
   token_ENDL,
   token_STRING,
   token_INT,
   token_FLOAT,
   token_SYMBOL,
   token_OPERATOR
};

#ifdef __cplusplus
extern "C" {
#endif

   token_t **token_read (const char *fname);
   void token_del (token_t *token);

   const char *token_string (token_t *token);
   const char *token_fname (token_t *token);
   size_t      token_line (token_t *token);
   size_t      token_charpos (token_t *token);

   enum token_type_t token_type (token_t *token);

#ifdef __cplusplus
};
#endif

#endif


