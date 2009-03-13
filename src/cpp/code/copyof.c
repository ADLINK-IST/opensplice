#include "cpp_malloc.h"

char *copyofstr (char * str)
{
   char * cp;

   if (str == 0)
   {
      return (0);
   }
   cp = os_malloc (strlen (str) + 1);
   if (cp == 0)
   {
      return (0);
   }
   strcpy(cp, str);
   return (cp);
}

char *copyofblk (char * blk, int len)
{
   char *cp;

   if (blk == 0)
   {
      return (0);
   }
   cp = os_malloc(len);
   if (cp == 0)
   {
      return (0);
   }
   memcpy(cp, blk, len);
   return (cp);
}
