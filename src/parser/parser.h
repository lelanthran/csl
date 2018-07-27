
#ifndef H_PARSER
#define H_PARSER

#include <stdio.h>
#include <stdbool.h>

#include "token/token.h"
#include "parser/atom.h"

typedef struct parser_tree_t parser_tree_t;

#ifdef __cplusplus
extern "C" {
#endif

   atom_t *parser_new (void);
   void parser_del (atom_t *ptree);

   bool parser_parse (atom_t *ptree, token_t **tokens);

   bool parser_compile (atom_t *ptree);

   void parser_print (atom_t *ptree, size_t depth, FILE *outf);


#ifdef __cplusplus
};
#endif

#endif


