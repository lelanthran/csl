
#ifndef H_ATOM
#define H_ATOM

#include <stdio.h>
#include <stdint.h>

enum atom_type_t {
   atom_UNKNOWN = 0,
   atom_NIL,
   atom_LIST,
   atom_QUOTE,
   atom_STRING,
   atom_SYMBOL,
   atom_INT,
   atom_FLOAT,
   atom_NATIVE,
   atom_FFI,
   atom_ENDL
};

typedef struct atom_t atom_t;
struct atom_t {
   // Tells us what type of data we are dealing with
   enum atom_type_t type;

   // We could use a union here (getting strong type guarantees) but it
   // means more onerous dispatching for functions on that data. This way
   // we have to cast, but at least we can keep the functions to call in a
   // lookup table.
   void *data;

   // Reserved for internal use, do not access
   char buffer[18];
   uint8_t flags;
};

#ifdef __cplusplus
extern "C" {
#endif

   void atom_del (atom_t *atom);
   atom_t *atom_new (enum atom_type_t type, const char *string);
   atom_t *atom_dup (const atom_t *atom);
   void atom_print (atom_t *atom, size_t depth, FILE *outf);
   int atom_cmp (const atom_t *lhs, const atom_t *rhs);

   atom_t *atom_list_new (void);
   size_t atom_list_length (const atom_t *atom);
   const atom_t *atom_list_index (const atom_t *atom, size_t index);
   atom_t *atom_list_remove (atom_t *atom, size_t index);
   atom_t *atom_list_ins_tail (atom_t *atom, void *el);
   atom_t *atom_list_ins_head (atom_t *atom, void *el);

   atom_t *atom_string_new (const char *s);
   atom_t *atom_int_new (int64_t i);
   atom_t *atom_float_new (double d);

   const char *atom_to_string (const atom_t *atom);

#ifdef __cplusplus
}
#endif


#endif


