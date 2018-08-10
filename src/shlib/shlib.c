
#include "shlib/shlib.h"

#include "xshare/xshare.h"

#include "xvector/xvector.h"

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
      dlclose (lib->handle);
      nv_del (lib);
   }
   xvector_free (shlib->libs);


   len = XVECT_LENGTH (shlib->funcs);

   for (size_t i=0; i<len; i++) {
      nvpair_t *func = XVECT_INDEX (shlib->funcs, i);
      dlclose (func->handle);
      nv_del (func);
   }
   xvector_free (shlib->funcs);

   free (shlib);
}


