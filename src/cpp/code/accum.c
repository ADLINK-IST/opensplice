#include "accum.h"
#include "cpp_malloc.h"
#include "os.h"

char * init_accum (void)
{
   ACCUM *a;

   a = NEW(ACCUM);
   check_os_malloc(a);
   a->used = 0;
   a->have = 8;
   a->buf = os_malloc(8);
   check_os_malloc(a->buf);
   return ((char *)a);
}


void accum_char (char * A, char c)
{
#define a ((ACCUM *)A)
   while (a->used >= a->have)
   {
      a->buf = os_realloc(a->buf, a->have += 8);
      check_os_malloc(a->buf);
   }
   a->buf[a->used++] = c;
}
#undef a

char accum_regret( char * A)
{
#define a ((ACCUM *)A)
   if (a->used > 0)
   {
      return (a->buf[--a->used]);
   }
   else
   {
      return (0);
   }
}
#undef a

char *accum_result (char * A)
{
#define a ((ACCUM *)A)
   char *cp;

   cp = os_realloc(a->buf, a->used + 1);
   check_os_malloc(cp);
   cp[a->used] = '\0';
   OLD(a);
   return (cp);
}
#undef a

char *accum_sofar (char * A)
{
#define a ((ACCUM *)A)
   char *cp;

   cp = os_malloc(a->used + 1);
   check_os_malloc(cp);
   memcpy(cp, a->buf, a->used);
   cp[a->used] = '\0';
   return (cp);
}
#undef a

char *accum_buf (char * A)
{
#define a ((ACCUM *)A)
   return (a->buf);
}
#undef a

int accum_howfar(char * A)
#define a ((ACCUM *)A)
{
   return (a->used);
}
#undef a
