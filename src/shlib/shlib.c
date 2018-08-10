#include <stdlib.h>
#include <string.h>

#include "shlib/shlib.h"

#include "xshare/xshare.h"

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

static nvpair_t *nv_find (xvector_t *xv, const char *name)
{
   size_t len = XVECT_LENGTH (xv);

   for (size_t i=0; i<len; i++) {
      nvpair_t *nv = XVECT_INDEX (xv, i);
      if ((strcmp (name, nv->name))==0)
         return nv;
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
      xshare_close (func->handle);
      nv_del (func);
   }
   xvector_free (shlib->funcs);

   free (shlib);
}


bool shlib_loadlib (shlib_t *shlib, const char *name)
{
   bool error = true;
   void *handle = NULL;
   nvpair_t *nv = NULL;

   if (!shlib || !name)
      return false;

   if (nv_find (shlib->libs, name))
      return true;

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
      }
      nv_del (nv);
   }

   return !error;
}
