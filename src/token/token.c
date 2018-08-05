#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "token/token.h"
#include "ll/ll.h"

#include "xstring/xstring.h"
#include "xerror/xerror.h"

struct token_t {
   enum token_type_t type;

   char *string;
   char *fname;
   size_t line;
   size_t charpos;
};

static bool is_operator (int c)
{
   static const char *operators = "()+-*/!',:";
   return strchr (operators, (char)c);
}

static enum token_type_t guess_type (const char *str)
{
   if (!str || !*str)
      return token_UNKNOWN;

   if (str[0] == '(')         return token_STARTL;
   if (str[0] == '\'')        return token_QUOTE;
   if (str[0] == ')')         return token_ENDL;
   if (str[0] == '"')         return token_STRING;
   if (is_operator (str[0]))  return token_OPERATOR;
   enum token_type_t type = token_SYMBOL;
   if (isdigit (str[0]))
      type = token_INT;

   if (strchr (str, '.') && type==token_INT)
      type = token_FLOAT;

   return type;
}

static token_t *token_new (const char *str, const char *fname,
                                            size_t line,
                                            size_t charpos)
{
   bool error = true;
   token_t *ret = NULL;

   if (!(ret = calloc (1, sizeof *ret)))
      goto errorexit;

   ret->string = xstr_dup (str);
   ret->fname = xstr_dup (fname);

   if (!ret->string || !ret->string[0] || !ret->fname)
      goto errorexit;

   ret->line = line;
   ret->charpos = charpos;
   ret->type = guess_type (str);

   error = false;

errorexit:
   if (error) {
      token_del (ret);
      ret = NULL;
   }

   return ret;
}

static token_t *fnext_token (FILE *inf, const char *fname,
                                        size_t *line,
                                        size_t *cpos)
{
   bool error = true;
   token_t *ret = NULL;
   bool in_str = false;
   int c = ' ';

   // Make this smaller if your program crashes while in this function.
   char tmps[MAX_TOKEN_LENGTH];

   size_t ll, cp;

   ll = line ? *line : 0;
   cp = cpos ? *cpos : 0;

   memset (tmps, 0, sizeof tmps);

   // Skip all whitespace
   while (!feof (inf) && !ferror (inf)) {
      c = fgetc (inf);
      if (!(isspace (c)))
         break;
   }

   if (isspace (c))
      goto errorexit;

   ungetc (c, inf);

   for (size_t i=0; i<sizeof tmps; i++) {

      if (ferror (inf) || feof (inf))
         break;

      if (i >= (sizeof tmps - 1))
         goto errorexit;

      c = fgetc (inf);

      if (c == ';') {
         c = fgetc (inf);
         while (!ferror (inf) && !feof (inf)) {
            if (c == '\n') {
               break;
            }
            c = fgetc (inf);
         }

         ll++; cp = 1;
         continue;
      }

      if (c == '\n') {
         ll++; cp = 1;
      } else {
         cp++;
      }

      if (c==EOF)
         break;

      if (in_str) {

         if (c == '"') {
            in_str = false;
            tmps[i] = '"';
            break;
         }

         if (c == '\\') {
            c = fgetc (inf);
            cp++;
         }

         tmps[i] = (char)c;
         continue;
      }

      if (c == '"') {
         in_str = true;
         tmps[i] = '"';
         continue;
      }

      if (is_operator (c)) {
         if (i) {
            ungetc (c, inf);
            cp--;
         } else {
            tmps[0] = (char)c;
         }
         break;
      }

      if (isspace (c))
         break;

      tmps[i] = (char)c;
   }

   if (in_str) {
      XERROR ("End of input while reading string [%s]\n", tmps);
      goto errorexit;
   }

   if (!(ret = token_new (tmps, fname, (*line), (*cpos))))
      goto errorexit;

   error = false;
errorexit:

   if (error) {
      token_del (ret);
      ret = NULL;
   }

   if (line) (*line) = ll;
   if (cpos) (*cpos) = cp;

   return ret;
}

token_t **token_read_file (const char *fname)
{
   bool error = true;
   FILE *inf = NULL;
   token_t **ret = NULL;
   void **tmpv = NULL;
   token_t *tmpt = NULL;

   size_t line = 0, charpos = 0;

   if (!(tmpv = ll_new ()))
      goto errorexit;

   if (!(inf = fopen (fname, "r"))) {
      XERROR ("Unable to open [%s]: %m\n", fname);
      goto errorexit;
   }

   while ((tmpt = fnext_token (inf, fname, &line, &charpos))) {

      if (strcmp (tmpt->string, "#load")==0) {

         token_t *load_fname = fnext_token (inf, fname, &line, &charpos);
         char *tmpfname = strrchr (load_fname->string, '"');
         if (tmpfname) *tmpfname = 0;
         tmpfname = load_fname->string[0] == '"'
                     ? &load_fname->string[1]
                     : load_fname->string;

         token_t **subv = token_read_file (tmpfname);

         token_del (load_fname);

         for (size_t i=0; subv && subv[i]; i++) {
            ll_ins_tail (&tmpv, subv[i]);
         }
         free (subv);
         token_del (tmpt);
         continue;
      }
      ll_ins_tail (&tmpv, tmpt);
   }

   ret = (token_t **)ll_copy (tmpv, 0, (size_t)-1);

   error = false;

errorexit:

   ll_del (tmpv);

   if (inf)
      fclose (inf);

   if (error) {
      for (size_t i=0; ret && ret[i]; i++) {
         token_del (ret[i]);
      }
      free (ret);
      ret = NULL;
   }

   return ret;
}

static token_t *snext_token (char **input, const char *fname,
                             size_t *line, size_t *cpos)
{
   bool error = true;
   token_t *ret = NULL;
   bool in_str = false;
   char *start = (*input);

   char tmps[MAX_TOKEN_LENGTH];
   size_t ll, cp;

   ll = line ? *line : 0;
   cp = cpos ? *cpos : 0;

   memset (tmps, 0, sizeof tmps);

   // Skip all whitespace
   while (*start && isspace (*start))
      start++;

   if (!*start)
      goto errorexit;

   char *end = start;
   for (size_t i=0; i<sizeof tmps; i++) {

      if (!end || !*end)
         break;

      if (i >= (sizeof tmps - 1))
         goto errorexit;

      if (*end == ';') {
         while (*end && *end != '\n')
            end++;
         ll++; cp = 1;
         continue;
      }

      if (*end == '\n') {
         ll++; cp = 1;
      } else {
         cp++;
      }

      if (in_str) {

         if (*end == '"') {
            in_str = false;
            tmps[i] = '"';
            end++;
            break;
         }

         if (*end == '\\') {
            end++;
            cp++;
         }

         tmps[i] = *end;
         end++;
         continue;
      }

      if (*end == '"') {
         in_str = true;
         tmps[i] = '"';
         end++;
         continue;
      }

      if (is_operator (*end)) {
         if (i) {
         } else {
            tmps[0] = *end;
            end++;
         }
         break;
      }

      if (isspace (*end))
         break;

      tmps[i] = *end;
      end++;
   }

   if (in_str) {
      XERROR ("End of input while reading string [%s]\n", tmps);
      goto errorexit;
   }

   if (!(ret = token_new (tmps, fname, (*line), (*cpos))))
      goto errorexit;

   error = false;
errorexit:

   if (error) {
      token_del (ret);
      ret = NULL;
   }

   if (line) (*line) = ll;
   if (cpos) (*cpos) = cp;

   (*input) = end;

   return ret;

}

token_t **token_read_string (char **input, const char *fname)
{
   token_t **ret = NULL;
   token_t *token = NULL;

   if (!(ret = ll_new ()))
      return NULL;

   size_t line = 0, charpos = 0;

   while ((token = snext_token (input, fname, &line, &charpos))!=NULL) {

      if (strcmp (token->string, "#load")==0) {

         token_t *load_fname = snext_token (input, fname, &line, &charpos);

         char *tmpfname = strrchr (load_fname->string, '"');
         if (tmpfname) *tmpfname = 0;
         tmpfname = load_fname->string[0] == '"'
                     ? &load_fname->string[1]
                     : load_fname->string;

         token_t **subv = token_read_file (tmpfname);

         token_del (load_fname);

         for (size_t i=0; subv && subv[i]; i++) {
            ll_ins_tail ((void ***)&ret, subv[i]);
         }
         free (subv);
         token_del (token);
         continue;
      }
      ll_ins_tail ((void ***)&ret, token);
   }

   return ret;
}

void token_del (token_t *token)
{
   if (!token)
      return;

   free (token->string);
   free (token->fname);
   free (token);
}

const char *token_string (token_t *token)
{
   return token ? token->string : NULL;
}

const char *token_fname (token_t *token)
{
   return token ? token->fname : NULL;
}

size_t token_line (token_t *token)
{
   return token ? token->line : 0;
}

size_t token_charpos (token_t *token)
{
   return token ? token->charpos : 0;
}

enum token_type_t token_type (token_t *token)
{
   return token ? token->type : 0;
}

