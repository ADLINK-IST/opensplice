#include "io.h"
#include "if.h"

extern void do_line (void)
{
   char c;

   outputc('#');
   while ((c = Get()) != '\n')
   {
      outputc(c);
   }
}
