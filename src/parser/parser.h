
#ifndef H_PARSER
#define H_PARSER

#include <stdbool.h>


typedef struct parse_tree_t parser_tree_t;

#ifdef __cplusplus
extern "C" {
#endif

   parser_tree_t *parser_new (void);
   void parser_del (parser_tree_t *ptree);

   bool parser_parse (token_t **tokens);


#ifdef __cplusplus
};
#endif

#endif


