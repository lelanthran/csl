
#ifndef H_PARSER
#define H_PARSER

#include <stdio.h>
#include <stdbool.h>

#include "token/token.h"

enum atom_type_t {
   atom_UNKNOWN = 0,
   atom_LIST,
   atom_STRING,
   atom_SYMBOL,
   atom_INT,
   atom_FLOAT,
   atom_FFI,
   atom_ENDL
};

typedef struct parser_tree_t parser_tree_t;

#ifdef __cplusplus
extern "C" {
#endif

   parser_tree_t *parser_new (void);
   void parser_del (parser_tree_t *ptree);

   bool parser_parse (parser_tree_t *ptree, token_t **tokens);

   bool parser_compile (parser_tree_t *ptree);

   void parser_print (parser_tree_t *ptree, size_t depth, FILE *outf);


#ifdef __cplusplus
};
#endif

#endif


