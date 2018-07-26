
#include "parser/parser.h"


   parser_tree_t *parser_new (void);
   void parser_del (parser_tree_t *ptree);

   bool parser_parse (token_t **tokens);


