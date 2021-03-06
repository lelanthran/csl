#include <stdio.h>
#include <stdint.h>

#include <stddef.h>

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
   printf ("i8:      [0x%x]\n", i8);
   printf ("i64:     [0x%" PRIx64 "]\n", i64);
   printf ("d64:     [%f]\n", d64);
   printf ("f32:     [%f]\n", f32);

   return ret;
}

// Ordering smallest to largest makes the best structure to test with;
// largest to smallest have most of the fields naturally aligned. Putting
// a int8_t in between every field ensures that none of them are aligned.
struct testlib_t {
   int8_t   i81;

   int16_t  i16;

   int8_t   i82;

   int32_t  i32;

   int8_t   i83;

   int64_t  i64;

   uint8_t  buf[7];

   int32_t  final;
};

uint64_t testlib_struct (struct testlib_t *ts)
{
   uint64_t ret = 0xc1c2c3c4c5c6c7c8;

   printf ("In [%s][%zu][%p]\n", __func__, sizeof *ts, ts);
   printf ("i81:   [%zu] [0x%02x]\n", offsetof (struct testlib_t, i81), ts->i81);
   printf ("i16:   [%zu] [0x%04x]\n", offsetof (struct testlib_t, i16), ts->i16);
   printf ("i82:   [%zu] [0x%02x]\n", offsetof (struct testlib_t, i82), ts->i82);
   printf ("i32:   [%zu] [0x%08x]\n", offsetof (struct testlib_t, i32), ts->i32);
   printf ("i83:   [%zu] [0x%02x]\n", offsetof (struct testlib_t, i83), ts->i83);
   printf ("i64:   [%zu] [0x%" PRIx64 "]\n", offsetof (struct testlib_t, i64), ts->i64);
   printf ("buf:   [%zu] [%p]\n", offsetof (struct testlib_t, buf), ts->buf);
   printf ("i32:   [%zu] [0x%x]\n", offsetof (struct testlib_t, final), ts->final);

   for (size_t i=0; i<sizeof ts->buf; i++) {
      printf ("0x%02x-", ts->buf[i]);
   }
   printf ("\n");

   return ret;
}
