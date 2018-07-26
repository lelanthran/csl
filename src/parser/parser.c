#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>


#include "parser/parser.h"


#include "xvector/xvector.h"
#include "xstring/xstring.h"
#include "xerror/xerror.h"

typedef struct atom_t atom_t;
struct atom_t {
   // Tells us what type of data we are dealing with
   int   type;

   // We could use a union here (getting strong type guarantees) but it
   // means more onerous dispatching for functions on that data. This way
   // we have to cast, but at least we can keep the functions to call in a
   // lookup table.
   void *data;

};


typedef atom_t *(atom_newfunc_t) (atom_t *dst, const char *);
typedef void (atom_delfunc_t) (atom_t *);

atom_t *atom_new_list (atom_t *dst, const char *str)
{
   str = str;
   dst->data = xvector_new ();
   return dst;
}

atom_t *atom_new_string (atom_t *dst, const char *str)
{
   dst->data = xstr_dup (str);
   return dst;
}

atom_t *atom_new_symbol (atom_t *dst, const char *str)
{
   dst->data = xstr_dup (str);
   return dst;
}

atom_t *atom_new_int (atom_t *dst, const char *str)
{
   bool error = true;
   atom_t *ret = NULL;

   dst->data = malloc (sizeof (int64_t));
   if (!dst->data)
      return NULL;

   int64_t tmp;
   if (sscanf (str, "%" PRIi64, &tmp)!=1) {
      XERROR ("[%s] is not an integer\n", str);
      goto errorexit;
   }

   *(int64_t *)dst->data = tmp;

   ret = dst;
   error = false;

errorexit:

   if (error) {
      free (dst->data);
      ret = NULL;
   }

   return ret;
}

atom_t *atom_new_float (atom_t *dst, const char *str)
{
   bool error = true;
   atom_t *ret = NULL;

   dst->data = malloc (sizeof (int64_t));
   if (!dst->data)
      return NULL;

   double tmp;
   if (sscanf (str, "%lf", &tmp)!=1) {
      XERROR ("[%s] is not an integer\n", str);
      goto errorexit;
   }

   *(double *)dst->data = tmp;

   ret = dst;
   error = false;

errorexit:

   if (error) {
      free (dst->data);
      ret = NULL;
   }

   return ret;
}


void atom_del_list (atom_t *atom)
{
   xvector_t *tmp = atom->data;
   size_t len = XVECT_LENGTH (tmp);

   for (size_t i=0; i<len; i++) {
      atom_t *a = XVECT_INDEX (tmp, i);
      atom_del_list (a);
   }
   xvector_free (tmp);
}

void atom_del_nonlist (atom_t *atom)
{
   free (atom->data);
}

typedef struct atom_dispatch_t atom_dispatch_t;
struct atom_dispatch_t {
   enum atom_type_t  type;
   atom_newfunc_t   *new_fptr;
   atom_delfunc_t   *del_fptr;
};

static const atom_dispatch_t *atom_find_funcs (enum atom_type_t type)
{
   static const atom_dispatch_t funcs[] = {
      { atom_LIST,   atom_new_list,    atom_del_list     },
      { atom_STRING, atom_new_string,  atom_del_nonlist  },
      { atom_SYMBOL, atom_new_symbol,  atom_del_nonlist  },
      { atom_INT,    atom_new_int,     atom_del_nonlist  },
      { atom_FLOAT,  atom_new_float,   atom_del_nonlist  },
   };

   for (size_t i=0; i<sizeof funcs/sizeof funcs[0]; i++) {
      if (funcs[i].type == type)
         return &funcs[i];
   }

   return NULL;
}

static void atom_del (atom_t *atom)
{
   if (!atom)
      return;

   const atom_dispatch_t *funcs = atom_find_funcs (atom->type);
   if (funcs) {
      funcs->del_fptr (atom);
   }
   free (atom);
}

static atom_t *atom_new (enum atom_type_t type, const char *string)
{
   bool error = true;
   const atom_dispatch_t *funcs = atom_find_funcs (type);
   atom_t *ret = NULL;

   XERROR ("atom_new (%i, %s)\n", type, string);

   if (!(ret = calloc (1, sizeof *ret)))
      goto errorexit;

   if (!funcs)
      goto errorexit;

   if (!(funcs->new_fptr (ret, string)))
      goto errorexit;

   ret->type = type;

   error = false;

errorexit:

   if (error) {
      atom_del (ret);
      ret = NULL;
   }

   return ret;
}


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

   while ((*idx) < max) {
      atom_t *na = NULL;
      const char *string = token_string (tokens[(*idx)]);
      enum atom_type_t type = atom_UNKNOWN;
      switch (token_type (tokens[(*idx)])) {
         case token_INT:      type = atom_INT;     break;
         case token_FLOAT:    type = atom_FLOAT;   break;
         case token_SYMBOL:   type = atom_STRING;  break;
         case token_OPERATOR: type = atom_STRING;  break;
         case token_STRING:   type = atom_STRING;  break;

         case token_STARTL:   type = atom_LIST;    break;
         case token_ENDL:     type = atom_ENDL;    break;

         default:
            XERROR ("Unknown atom [%s]\n", token_string (tokens[(*idx)]));
            goto errorexit;
      }

      if (!(na = atom_new (type, string)))
         goto errorexit;

      (*idx)++;

      if (type==atom_LIST) {
         if (!(rparser (na, tokens, idx, max))) {
            XERROR ("Error from recursive function\n");
            goto errorexit;
         }

         if (!(xvector_ins_tail (parent->data, na)))
            goto errorexit;
      }

      if (type==atom_ENDL)
         return true;
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


