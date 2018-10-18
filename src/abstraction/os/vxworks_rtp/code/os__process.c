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
#include <assert.h>
#include "os_errno.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <rtpLib.h>
#include <rtpLibCommon.h>
#include <taskLib.h>
#include <sys/stat.h>

#include "os_heap.h"
#include "os_stdlib.h"

#include "../posix/code/os_process.c"

static int file_exists (const char *file_path)
{
   struct stat file_stat;
   STATUS res;
   int result = 0;

   res = stat (file_path, &file_stat);

   if (res == OK)
   {
      result = S_ISREG (file_stat.st_mode);
   }

   return result;
}

static int search_path (char *exe_path, const char *exe)
{
   int result = 0;
   char *path = os_getenv ("PATH");
   char *dir;
   char *tok;

   if (file_exists (exe))
   {
      os_strcpy (exe_path, exe);
      result = 1;
   }
   else if (path)
   {
      dir = strtok_r (path, ":", &tok);
      do
      {
         os_strcpy (exe_path, dir);
         os_strcat (exe_path, "/");
         os_strcat (exe_path, exe);

         // check without extension first
         if (file_exists (exe_path))
         {
            result = 1;
            break;
         }

         os_strcat (exe_path, ".vxe");

         if (file_exists (exe_path))
         {
            result = 1;
            break;
         }

         path = NULL;
      } while (dir);
   }

   return result;
}

/** \file os/vxworks6.6/code/os__process.c
 *  \brief VxWorks RTP process management
 *
 * Implements process management for VxWorks RTP
 * by including the POSIX implementation
 */

/** \brief Create a process that is an instantiation of a program
 *
 * First an argument list is build from \b arguments.
 * Then \b os_procCreate creates a process by forking the current
 * process.
 *
 * The child process processes the lock policy attribute from
 * \b procAttr and sets the lock policy accordingly by calling
 * \b mlockall if required. If the process has root privileges
 * it processes the user credentials from \b procAttr and sets
 * the user credentials of the child process accordingly.
 * The child precess then replaces the running program with the
 * program provided by the \b executable_file by calling \b execve.
 *
 * The parent process processes the scheduling class and
 * scheduling priority attributes from \b procAttr and
 * sets the scheduling properties of the child process
 * accordingly by calling \b sched_setscheduler.
 */
os_result
os_procCreate
(
    const char *executable_file,
    const char *name,
    const char *arguments,
    os_procAttr * procAttr,
    os_procId *procId
)
{
   os_result rv = os_resultFail;
   const char *argv[64];
   int argc = 1;
   int go_on = 1;
   int i = 0;
   int sq_open = 0;
   int sq_close = 0;
   int dq_open = 0;
   int dq_close = 0;
   char *argin;
   RTP_ID id;
   char environment[512];
   const char **newenviron;
   char path[512];
   char *debug;
   int currentprocindex = -1;
   char *currentprocname;
   size_t newenvsize;

   assert (executable_file != NULL);
   assert (name != NULL);
   assert (arguments != NULL);
   assert (procAttr != NULL);
   assert (procId != NULL);

   /* first translate the input string into an argv structured list */
   argin = os_malloc (strlen(arguments) + 1);
   os_strcpy (argin, arguments);
   argv[0] = os_strcpy (os_malloc(strlen(name) + 1), name);
   while (go_on && (unsigned int)argc <= (sizeof(argv)/(sizeof(char *))))
   {
      while (argin[i] == ' ' || argin[i] == '\t')
      {
         i++;
      }
      if (argin[i] == '\0' )
      {
         break;
      }
      else if (argin[i] == '\'')
      {
         if (sq_open == sq_close)
         {
            sq_open++;
            argv[argc] = &argin[i];
         }
         else
         {
            sq_close++;
         }
         i++;
      }
      else if (argin[i] == '\"')
      {
         if (dq_open == dq_close)
         {
            dq_open++;
         }
         else
         {
            dq_close++;
         }
         i++;
      }
      else
      {
         argv[argc] = &argin[i];
         argc++;
         while ((argin[i] != ' ' && argin[i] != '\t') || (sq_open != sq_close)
                || (dq_open != dq_close))
         {
            if (argin[i] == '\0')
            {
               go_on = 0;
               break;
            }
            else if (argin[i] == '\'')
            {
               sq_close++;
               if ((sq_open == sq_close) && (dq_open == dq_close))
               {
                  argin[i] = '\0';
               }
               i++;
            }
            else if (argin[i] == '\"')
            {
               dq_close++;
               if ((dq_open == dq_close) && (sq_open == sq_close))
               {
                  argin[i] = '\0';
               }
               i++;
            }
            else
            {
               i++;
            }
         }
         argin[i] = '\0';
         i++;
      }
   }

   argv [argc] = NULL;

   /* Set the process name via environment variable SPLICE_PROCNAME */
   snprintf (environment, sizeof (environment), "SPLICE_PROCNAME=%s", name);

   currentprocname = getenv( "SPLICE_PROCNAME" );
   /* Count the size of the existing env, and locate the SPLICE_PROCNAME
      if there is one, as this will be substituted later */
   for ( newenvsize = 0; environ[newenvsize] != NULL ; newenvsize++ )
   {
       if ( strncmp( "SPLICE_PROCNAME=", environ[newenvsize], 16 ) == 0 )
       {
           currentprocindex = newenvsize;
       }
   }
   if ( currentprocindex == -1 )
   {
       /* There is no SPLICE_PROCNAME in the env, so it will be added
          in the newenv at the same index the null is currently */
       currentprocindex = newenvsize;
   }
   /* add one for the procname if its a new entry */
   newenviron = (const char **)malloc(sizeof(char *)
                             * (1 + newenvsize + ( currentprocname == NULL ? 1 : 0)));
   do
   {
       newenviron[newenvsize] = environ[newenvsize];
   } while ( newenvsize-- > 0 );
   /* set up the SPLICE_PROCNAME */
   newenviron[currentprocindex] = environment;
   /* Add the null if its an additional entry */
   if ( currentprocname == NULL )
   {
       newenviron[currentprocindex+1] = NULL;
   }


   if (search_path (path, executable_file))
   {
      debug = os_getenv ("DDS_PROC_DEBUG");

      id = rtpSpawn (path, argv, (const char ** )newenviron, procAttr->schedPriority, 0x10000,
                     debug ? RTP_DEBUG : 0,
#define e500v2diab 1
#define e500v2gnu 2
#if ((_VX_TOOL==e500v2diab) || (_VX_TOOL==e500v2gnu))
             VX_SPE_TASK
#else
             0
#endif
#undef e500v2diab
#undef e500v2gnu
             );

      if (id != ERROR)
      {
         *procId = id;
      }
   }
   free( newenviron );
   rv = (id == ERROR) ? os_resultFail : os_resultSuccess;
   return rv;
}

/** \brief Get the process effective scheduling class
 *
 * Possible Results:
 * - process scheduling class is OS_SCHED_REALTIME
 * - process scheduling class is OS_SCHED_TIMESHARE
 * - process scheduling class is OS_SCHED_DEFAULT if
 *   the class effective could not be determined
 */
os_schedClass
os_procAttrGetClass(void)
{
    os_schedClass class;
    int policy;

    policy = sched_getscheduler(taskIdSelf());
    switch (policy)
    {
       case SCHED_FIFO:
       case SCHED_RR:
          class = OS_SCHED_REALTIME;
          break;
       case SCHED_OTHER:
          class = OS_SCHED_TIMESHARE;
          break;
       case -1:
          OS_REPORT(OS_WARNING, "os_procAttrGetClass", 1,
                      "sched_getscheduler failed with error %d", os_getErrno());
          class = OS_SCHED_DEFAULT;
          break;
       default:
          OS_REPORT(OS_WARNING, "os_procAttrGetClass", 1,
                      "sched_getscheduler unexpected return value %d", policy);
          class = OS_SCHED_DEFAULT;
          break;
    }
    return class;
}

/** \brief Get the process effective scheduling priority
 *
 * Possible Results:
 * - any platform and scheduling class dependent valid priority
 */
os_int32
os_procAttrGetPriority(void)
{
    struct sched_param param;

    param.sched_priority = 0;
    if (sched_getparam(taskIdSelf(), &param) == -1)
    {
       OS_REPORT (OS_WARNING, "os_procAttrGetPriority", 1,
                    "sched_getparam failed with error %d", os_getErrno());
    }
    return param.sched_priority;
}

/** \brief Initialize process attributes
 *
 * Set \b procAttr->schedClass to \b OS_SCHED_DEFAULT
 * (take the platforms default scheduling class, Time-sharing for
 * non realtime platforms, Real-time for realtime platforms)
 * Set \b procAttr->schedPriority to \b 100
 * Set \b procAttr->lockPolicy to \b OS_LOCK_DEFAULT
 * (no locking on non realtime platforms, locking on
 * realtime platforms)
 * Set \b procAttr->userCred.uid to 0
 * (don't change the uid of the process)
 * Set \b procAttr->userCred.gid to 0
 * (don't change the gid of the process)
 */
void
os_procAttrInit (
    os_procAttr *procAttr)
{
    assert (procAttr != NULL);
    procAttr->schedClass = OS_SCHED_DEFAULT;
    procAttr->schedPriority = 100;
    procAttr->lockPolicy = OS_LOCK_DEFAULT;
    procAttr->userCred.uid = 0;
    procAttr->userCred.gid = 0;
    procAttr->activeRedirect = 0;
}

void os_procInit(void)
{
   char *process_env_name;
   static RTP_DESC rtpStruct;
   RTP_ID id = 0;

   process_env_name = os_getenv("SPLICE_PROCNAME");
   if (process_env_name != NULL)
   {
      processName = process_env_name;
   }
   else
   {
      if ( rtpInfoGet(id,&rtpStruct) == OK)
      {
          processName = rtpStruct.pathName;
      }
      else
      {
          processName = "";
      }
   }
}

os_int32
os_procGetProcessName(
    char *procName,
    os_uint32 procNameSize)
{
   return ((os_int32)snprintf(procName, procNameSize, "%s", processName));
}

#undef _OS_PROC_PROCES_NAME_LEN

