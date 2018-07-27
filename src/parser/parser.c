#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>


#include "parser/parser.h"
#include "parser/atom.h"


#include "xvector/xvector.h"
#include "xstring/xstring.h"
#include "xerror/xerror.h"


struct parser_tree_t {

   atom_t *root;

};

parser_tree_t *parser_new (void)
{
   bool error = true;
   parser_tree_t *ret = NULL;

   if (!(ret = calloc (1, sizeof *ret)))
      goto errorexit;

   if (!(ret->root = atom_new (atom_LIST, NULL)))
      goto errorexit;

   error = false;

errorexit:

   if (error) {
      parser_del (ret);
      ret = NULL;
   }

   return ret;
}

void parser_del (parser_tree_t *ptree)
{
   if (!ptree)
      return;

   atom_del (ptree->root);
   free (ptree);
}

static bool rparser (atom_t *parent, token_t **tokens, size_t *idx, size_t max)
{
   bool error = true;

   while ((*idx) < max - 1) {
      atom_t *na = NULL;
      const char *string = token_string (tokens[(*idx)]);
      enum atom_type_t type = atom_UNKNOWN;
      switch (token_type (tokens[(*idx)])) {
         case token_INT:      type = atom_INT;     break;
         case token_FLOAT:    type = atom_FLOAT;   break;
         case token_SYMBOL:   type = atom_SYMBOL;  break;
         case token_OPERATOR: type = atom_SYMBOL;  break;
         case token_STRING:   type = atom_STRING;  break;

         case token_STARTL:   type = atom_LIST;    break;
         case token_ENDL:     type = atom_ENDL;    break;

         default:
            XERROR ("Unknown atom [%s]\n", token_string (tokens[(*idx)]));
            goto errorexit;
      }

      (*idx)++;

      if (type==atom_ENDL)
         return true;

      if (!(na = atom_new (type, string)))
         goto errorexit;

      if (type==atom_LIST) {
         if (!(rparser (na, tokens, idx, max))) {
            XERROR ("Error from recursive function\n");
            goto errorexit;
         }

      }

      if (!(xvector_ins_tail (parent->data, na)))
         goto errorexit;

   }

   error = false;

errorexit:

   return !error;
}

bool parser_parse (parser_tree_t *ptree, token_t **tokens)
{
   bool error = true;
   size_t index = 0;
   size_t ntokens = 0;

   for (ntokens=0; tokens[ntokens]; ntokens++)
      ;

   if (!rparser (ptree->root, tokens, &index, ntokens))
      goto errorexit;

   error = false;

errorexit:

   return !error;
}

void parser_print (parser_tree_t *ptree, size_t depth, FILE *outf)
{
   if (!outf)
      outf = stdout;

   if (!ptree)
      return;

   atom_print (ptree->root, depth, outf);
}
