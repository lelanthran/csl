#include <stdlib.h>
#include <string.h>

#include <ffi.h>

#include "shlib/shlib.h"

#include "xvector/xvector.h"
#include "xstring/xstring.h"
#include "xerror/xerror.h"

typedef struct nvpair_t nvpair_t;

struct nvpair_t {
   char *name;
   void *handle;
};

static nvpair_t *nv_new (const char *name, void *handle)
{
   nvpair_t *ret = malloc (sizeof *ret);
   if (!ret)
      return NULL;

   memset (ret, 0, sizeof *ret);

   if (!(ret->name = xstr_dup (name))) {
      free (ret);
      return NULL;
   }

   ret->handle = handle;

   return ret;
}

static void nv_del (nvpair_t *nv)
{
   if (!nv)
      return;

   free (nv->name);
   free (nv);
}

static void *nv_find (xvector_t *xv, const char *name)
{
   size_t len = XVECT_LENGTH (xv);

   for (size_t i=0; i<len; i++) {
      nvpair_t *nv = XVECT_INDEX (xv, i);
      if ((strcmp (name, nv->name))==0)
         return nv->handle;
   }

   return NULL;
}

struct shlib_t {
   xvector_t *libs;
   xvector_t *funcs;
};

shlib_t *shlib_new (void)
{
   shlib_t *ret = malloc (sizeof *ret);
   if (!ret)
      return NULL;

   memset (ret, 0, sizeof *ret);

   ret->libs = xvector_new ();
   ret->funcs = xvector_new ();

   if (!ret->libs || !ret->funcs) {
      shlib_del (ret);
      return NULL;
   }

   return ret;
}

void shlib_del (shlib_t *shlib)
{
   if (!shlib)
      return;

   size_t len = XVECT_LENGTH (shlib->libs);

   for (size_t i=0; i<len; i++) {
      nvpair_t *lib = XVECT_INDEX (shlib->libs, i);
      xshare_close (lib->handle);
      nv_del (lib);
   }
   xvector_free (shlib->libs);


   len = XVECT_LENGTH (shlib->funcs);

   for (size_t i=0; i<len; i++) {
      nvpair_t *func = XVECT_INDEX (shlib->funcs, i);
      nv_del (func);
   }
   xvector_free (shlib->funcs);

   free (shlib);
}


xshare_library_t shlib_loadlib (shlib_t *shlib, const char *name)
{
   bool error = true;
   void *handle = NULL;
   nvpair_t *nv = NULL;

   if (!shlib || !name)
      return false;

   if ((handle = nv_find (shlib->libs, name)))
      return handle;

   handle = xshare_open (name);
   if (!handle) {
      XERROR ("Cannot load library [%s]: %s\n", name, xshare_errmsg ());
      goto errorexit;
   }

   nv = nv_new (name, handle);
   if (!nv)
      goto errorexit;

   if (!xvector_ins_tail (shlib->libs, nv))
      goto errorexit;

   error = false;

errorexit:
   if (error) {
      if (handle) {
         xshare_close (handle);
         handle = NULL;
      }
      nv_del (nv);
   }

   return handle;
}

xshare_symbol_t shlib_loadfunc (shlib_t *shlib, const char *func,
                                                const char *lib)
{
   bool error = true;
   nvpair_t *nv = NULL;
   void *libhandle = NULL;
   void *funchandle = NULL;

   if (!shlib || !func || !lib)
      return false;

   if ((funchandle = nv_find (shlib->funcs, func)))
      return funchandle;

   if (!(libhandle = shlib_loadlib (shlib, lib)))
      goto errorexit;

   if (!(funchandle = xshare_symbol (libhandle, func))) {
      XERROR ("Cannot locate symbol [%s]: %s\n", func, xshare_errmsg ());
      goto errorexit;
   }

   if (!(nv = nv_new (func, funchandle)))
      goto errorexit;

   if (!(xvector_ins_tail (shlib->funcs, nv)))
      goto errorexit;

   error = false;

errorexit:

   if (error) {
      nv_del (nv);
      funchandle = NULL;
   }

   return funchandle;
}

static const struct {
   ffi_type      *type_ffi;
   shlib_type_t   type_shlib;
} g_types[] = {

   { &ffi_type_void,     shlib_VOID     },
   { &ffi_type_uint8,    shlib_UINT8_T  },
   { &ffi_type_sint8,    shlib_INT8_T   },
   { &ffi_type_uint16,   shlib_UINT16_T },
   { &ffi_type_sint16,   shlib_INT16_T  },
   { &ffi_type_uint32,   shlib_UINT32_T },
   { &ffi_type_sint32,   shlib_INT32_T  },
   { &ffi_type_uint64,   shlib_UINT64_T },
   { &ffi_type_sint64,   shlib_INT64_T  },
   { &ffi_type_float,    shlib_FLOAT    },
   { &ffi_type_double,   shlib_DOUBLE   },
   { &ffi_type_uchar,    shlib_U_CHAR   },
   { &ffi_type_schar,    shlib_S_CHAR   },
   { &ffi_type_ushort,   shlib_U_SHORT  },
   { &ffi_type_sshort,   shlib_S_SHORT  },
   { &ffi_type_uint,     shlib_U_INT    },
   { &ffi_type_sint,     shlib_S_INT    },
   { &ffi_type_ulong,    shlib_U_LONG   },
   { &ffi_type_slong,    shlib_S_LONG   },
   { &ffi_type_pointer,  shlib_POINTER  },

};

ffi_type *get_ffi_type (shlib_type_t type)
{
   for (size_t i=0; i<sizeof g_types/sizeof g_types[0]; i++) {
      if (g_types[i].type_shlib == type)
         return g_types[i].type_ffi;
   }

   return NULL;
}

static const struct {
   shlib_type_t type;
   size_t len;
} g_typelen[] = {
   { shlib_NONE,        0                             },
   { shlib_VOID,        0                             },
   { shlib_NULL,        0                             },
   { shlib_UINT8_T,     sizeof (uint8_t)              },
   { shlib_UINT16_T,    sizeof (uint16_t)             },
   { shlib_UINT32_T,    sizeof (uint32_t)             },
   { shlib_UINT64_T,    sizeof (uint64_t)             },
   { shlib_INT8_T,      sizeof (int8_t)               },
   { shlib_INT16_T,     sizeof (int16_t)              },
   { shlib_INT32_T,     sizeof (int32_t)              },
   { shlib_INT64_T,     sizeof (int64_t)              },
   { shlib_FLOAT,       sizeof (float)                },
   { shlib_DOUBLE,      sizeof (double)               },
   { shlib_SIZE_T,      sizeof (size_t)               },
   { shlib_S_CHAR,      sizeof (signed char)          },
   { shlib_S_SHORT,     sizeof (signed short)         },
   { shlib_S_INT,       sizeof (signed int)           },
   { shlib_S_LONG,      sizeof (signed long)          },
   { shlib_S_LONG_LONG, sizeof (signed long long)     },
   { shlib_U_CHAR,      sizeof (unsigned char)        },
   { shlib_U_SHORT,     sizeof (unsigned short)       },
   { shlib_U_INT,       sizeof (unsigned int)         },
   { shlib_U_LONG,      sizeof (unsigned long)        },
   { shlib_U_LONG_LONG, sizeof (unsigned long long)   },
   { shlib_POINTER,     sizeof (void *)               },
};

int shlib_funcall (shlib_t *shlib, const char *func, const char *lib,
                   shlib_type_t return_type, void *return_value,
                   struct shlib_pair_t *args)
{
   // TODO: Call prep_cif only once and keep the value in the fptr struct
   int ret = -1;
   void *fptr = NULL;

   ffi_cif cif;

   ffi_type ret_type;

   ffi_type **arg_types = NULL;
   void **arg_values = NULL;

   unsigned int nargs = 0;

   if (!(fptr = shlib_loadfunc (shlib, func, lib)))
      goto errorexit;

   for (size_t i=0; args[i].type!=0; i++)
      nargs++;

   ret_type = *(get_ffi_type (return_type));

   arg_types = malloc ((sizeof *arg_types) * (nargs + 1));
   arg_values = malloc ((sizeof *arg_values) * (nargs + 1));

   if (!arg_types || !arg_values)
      goto errorexit;

   memset (arg_types, 0, (sizeof *arg_types) * (nargs + 1));
   memset (arg_values, 0, (sizeof *arg_values) * (nargs + 1));

   for (size_t i=0; args[i].type!=0; i++) {
      arg_types[i] = get_ffi_type (args[i].type);
      arg_values[i] = (void *)args[i].data;
   }

   ffi_status ffi_sc = ffi_prep_cif (&cif, FFI_DEFAULT_ABI,
                                           nargs,
                                           &ret_type,
                                           arg_types);
   if (ffi_sc != FFI_OK)
      goto errorexit;

   ffi_call (&cif, fptr, return_value, arg_values);

   ret = 0;

errorexit:

   free (arg_types);
   free (arg_values);

   return ret;
}

size_t shlib_sizeof (shlib_type_t type)
{
   for (size_t i=0; i<sizeof g_typelen / sizeof g_typelen[0]; i++) {
      if (g_typelen[i].type==type)
         return g_typelen[i].len;
   }

   return 0;
}

