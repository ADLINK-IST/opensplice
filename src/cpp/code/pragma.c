#include "io.h"
#include "if.h"

extern void do_pragma (void)
{
   char c;

   if (in_false_if())
   {
      while ((c = Get()) != '\n')
         ;
   }
   else
   {
      outputc('#');
      outputc('p');
      outputc('r');
      outputc('a');
      outputc('g');
      outputc('m');
      outputc('a');
      outputc(' ');
      c = Get();
      while ((c != '\n') && (c != -1))
      {
         outputc(c);
         c = Get();
      }
   }
}
