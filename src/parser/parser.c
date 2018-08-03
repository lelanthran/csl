#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>


#include "parser/parser.h"
#include "parser/atom.h"


#include "ll/ll.h"
#include "xstring/xstring.h"
#include "xerror/xerror.h"


static bool rparser (void ***list, token_t **tokens, size_t *idx, size_t max)
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
         case token_QUOTE:    type = atom_QUOTE;   break;
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
         if (!(rparser (list, tokens, idx, max))) {
            XERROR ("Error from recursive function\n");
            goto errorexit;
         }

      }

      if (!(ll_ins_tail (list, na)))
         goto errorexit;

   }

   error = false;

errorexit:

   return !error;
}

atom_t **parser_parse (token_t **tokens)
{
   bool error = true;
   size_t index = 0;
   size_t ntokens = 0;
   void **ret = ll_new ();

   if (!ret)
      goto errorexit;

   for (ntokens=0; tokens[ntokens]; ntokens++)
      ;

   if (!rparser (&ret, tokens, &index, ntokens))
      goto errorexit;

   error = false;

errorexit:

   if (error) {
      ll_iterate (ret, (void (*) (void *))free);
      ll_del (ret);
      ret = NULL;
   }

   return (atom_t **)ret;
}

