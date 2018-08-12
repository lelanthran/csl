#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>


int testlib_add (int x, int y)
{
   printf ("In [%s]\n", __func__);
   printf ("x: [%i]\n", x);
   printf ("y: [%i]\n", y);
   return x + y;
}

int testlib_sub (int x, int y)
{
   printf ("In [%s]\n", __func__);
   printf ("x: [%i]\n", x);
   printf ("y: [%i]\n", y);
   return x - y;
}

uint64_t testlib_big (char *str,
                      uint8_t i8,
                      uint64_t i64,
                      double d64,
                      float f32)
{
   uint64_t ret = 0xb1b2b3b4b5b6b7b8L;
   printf ("In [%s]\n", __func__);
   printf ("str:     [%s]\n", str);
   printf ("i8:      [%x]\n", i8);
   printf ("i64:     [%" PRIx64 "]\n", i64);
   printf ("d64:     [%f]\n", d64);
   printf ("f32:     [%f]\n", f32);

   return ret;
}

