/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "if.h"
#include "io.h"

#ifdef DEBUG_WHILE
extern int debugging;
#endif

extern void do_while (void)
{
   input_mark();
   do_if(0);
}

extern void do_endwhile(void)
{
   if (in_false_if())
   {
      do_endif(0);
      input_unmark();
#ifdef DEBUG_WHILE

      if (debugging)
      {
         outputs("//endwhile:");
         outputd(curline());
         outputs(",");
         outputs(curfile());
         outputs("\\\\");
      }
#endif
      out_at(curline(), curfile());
   }
   else
   {
      do_endif(0);
      input_recover();
      input_mark();
      do_if(0);
   }
}
