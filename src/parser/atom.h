
#ifndef H_ATOM
#define H_ATOM

#include <stdio.h>

enum atom_type_t {
   atom_UNKNOWN = 0,
   atom_LIST,
   atom_STRING,
   atom_SYMBOL,
   atom_INT,
   atom_FLOAT,
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

};

#ifdef __cplusplus
extern "C" {
#endif

   void atom_del (atom_t *atom);
   atom_t *atom_new (enum atom_type_t type, const char *string);
   atom_t *atom_dup (const atom_t *atom);
   void atom_print (atom_t *atom, size_t depth, FILE *outf);

   atom_t *atom_list_new (void);
   size_t atom_list_length (const atom_t *atom);
   const atom_t *atom_list_car (const atom_t *atom);
   const atom_t *atom_list_cdr (const atom_t *atom);


   const char *atom_to_string (const atom_t *atom);

#ifdef __cplusplus
}
#endif


#endif


