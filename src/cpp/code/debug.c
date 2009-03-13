#include "is.h"
#include "io.h"
#include "if.h"

char buf[BUFSIZ];

int debugging = 0;

extern void do_debug (void)
{
   char c;

   /* fflush(outfile);*/
   c = getnonspace();
   switch (c)
   {
      case '1':
      case 'y':
      case 'Y':
         debugging = 1;
         break;
      default:
         debugging = 0;
         break;
   }
}
