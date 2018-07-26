
#ifndef H_PARSER
#define H_PARSER

#include <stdbool.h>

#include "token/token.h"

enum atom_type_t {
   atom_UNKNOWN = 0,
   atom_LIST,
   atom_STRING,
   atom_SYMBOL,
   atom_INT,
   atom_FLOAT,
   atom_ENDL
};

typedef struct parser_tree_t parser_tree_t;

#ifdef __cplusplus
extern "C" {
#endif

   parser_tree_t *parser_new (void);
   void parser_del (parser_tree_t *ptree);

   bool parser_parse (parser_tree_t *ptree, token_t **tokens);


#ifdef __cplusplus
};
#endif

#endif


