
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

   atom_t **parser_parse (token_t **tokens);

#ifdef __cplusplus
};
#endif

#endif


