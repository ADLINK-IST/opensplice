#include "io.h"

extern void err_head (void)
{
   fprintf(stderr, "\"%s\", line %d: ", curfile(), curline());
}

extern void Check_malloc (char * ptr)
{
   if (ptr == 0)
   {
      fprintf(stderr, "out of memory!\n");
      abort();
   }
}
