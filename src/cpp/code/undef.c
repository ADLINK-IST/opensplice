#include "io.h"
#include "if.h"
#include "symtbl.h"

extern void err_head (void);

extern void do_undef (int sharp)
{
   char *mac;

   if (! in_false_if())
   {
      mac = read_ident();
      if (! mac)
      {
         err_head();
         fprintf(stderr, "missing/illegal macro name\n");
      }
      else
      {
         undef (mac);
      }
   }
   if (sharp)
   {
      flush_sharp_line();
   }
}
