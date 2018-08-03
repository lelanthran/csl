#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>


#include "parser/parser.h"
#include "parser/atom.h"


#include "ll/ll.h"
#include "xstring/xstring.h"
#include "xerror/xerror.h"


static atom_t *rparser (token_t **tokens, size_t *index)
{
   bool error = true;
   atom_t *ret = NULL;

   if (!tokens[(*index)]) {
      return NULL;
   }

   const char *string = token_string (tokens[(*index)]);
   enum atom_type_t type = atom_UNKNOWN;

   switch (token_type (tokens[(*index)])) {
      case token_INT:      type = atom_INT;     break;
      case token_FLOAT:    type = atom_FLOAT;   break;
      case token_SYMBOL:   type = atom_SYMBOL;  break;
      case token_OPERATOR: type = atom_SYMBOL;  break;
      case token_QUOTE:    type = atom_QUOTE;   break;
      case token_STRING:   type = atom_STRING;  break;

      case token_STARTL:   type = atom_LIST;    break;
      case token_ENDL:     type = atom_ENDL;    break;

      default:
         XERROR ("Unknown atom [%s]\n", token_string (tokens[(*index)]));
         goto errorexit;
   }

   *index += 1;

   if (type==atom_ENDL) {
      return NULL;
   }

   if (!(ret = atom_new (type, string))) {
      return NULL;
   }

   if (type==atom_LIST) {
      atom_t *child;
      while ((child = rparser (tokens, index))) {
         if (!(atom_list_ins_tail (ret, child)))
            goto errorexit;
      }
   }


   error = false;

errorexit:

   if (error) {
      atom_del (ret);
      ret = NULL;
   }
   return ret;
}

atom_t *parser_parse (token_t **tokens, size_t *index)
{
   bool error = true;
   size_t ntokens = 0;
   atom_t *ret = NULL;

   if (!(ret = rparser (tokens, index)))
      goto errorexit;

   error = false;

errorexit:

   if (error) {
      atom_del (ret);
      ret = NULL;
   }

   return ret;
}

