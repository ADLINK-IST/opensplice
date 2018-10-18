/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/** \file os/posix/code/os_mkstemp.c
 *  \brief Posix mkstemp support */

#include <stdlib.h>
#include <INTEGRITY.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "os_errno.h"
#include <assert.h>
#include <string.h>

int os_mkstemp(char *template)
{
   int result;
   Error err;
   int len;
   Time t;
   int i;
   int frac;

   len = strlen(template);
   assert( template[len-1] == 'X' && template[len-2] == 'X' 
           && template[len-3] == 'X' && template[len-4] == 'X' 
           && template[len-5] == 'X' && template[len-6] == 'X' ); 

   do
   {
      err = GetClockTime(HighestResStandardClock, &t);
      assert (err == Success);
      for (i = 0 ; i < 6; i++)
      {
        frac = t.Fraction;
        template[len-6+i]=((frac >> (i*4)) & 0xf) + 'A';
      }
      result = open(template, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
   } while (result == -1 && os_getErrno() == EEXIST);
   return (result);
}
