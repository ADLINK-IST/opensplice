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
#include "os_stdlib.h"
#include <vxWorks.h>
#include <ctype.h>
#include "os_errno.h"
#include <ioLib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#if ! defined (OSPL_VXWORKS653)
#include <unistd.h>
#endif
#include <fioLib.h>
#include <intLib.h>
#if ! defined (OSPL_VXWORKS653)
#include <sysLib.h>
#endif
#ifndef VXWORKS_69
#include <qLib.h> 
#endif
#include <taskLib.h>
#include "private/kernelLibP.h"
#if ! defined (OSPL_VXWORKS653)
#include "private/vmLibP.h"
#endif
#include "private/floatioP.h"
#if ! defined (OSPL_VXWORKS653)
#include <elf.h>
#endif
#include <taskLib.h>
#include <version.h>
#if ! defined (OSPL_VXWORKS653)
#if defined ( _WRS_KERNEL )
#include "os_os_pwd.h"
#else
#include <hostLib.h>
#endif
#endif

#include "os_defs.h"
#include "os_process.h"
#include <time.h>
#include <sys/times.h>

#if defined (OSPL_VXWORKS653)
char* runtimeName = "VXWORKS";
char* runtimeVersion = "653";
char* runtimeSysModel = "#CPU";

char * sysModel()
{
  return runtimeSysModel;
}
#endif

struct putbufArgument {
    char **outputBuffer;
    size_t max_len;
    int nrCopied;
    int result;
};

LOCAL STATUS 
putbuf(
    char *inbuf,                /* pointer to source buffer */
    int length,                 /* number of bytes to copy */
    int outarg)                 /* pointer to struct putbufArgument */
{
    struct putbufArgument *arg = (struct putbufArgument *)outarg;
    int copyLen;
    
    /* always copy one byte less, to allow room for the '\0' character */
    if (arg->nrCopied < arg->max_len - 1) {
        if (arg->max_len - 1 - arg->nrCopied < length) {
            copyLen = arg->max_len - 1 - arg->nrCopied;
        } else {
            copyLen = length;
        }
        bcopy (inbuf, *arg->outputBuffer, copyLen);
        *arg->outputBuffer += copyLen;
        arg->nrCopied += copyLen;
    }
    arg->result += length;

    return (OK);
}

int 
vsnprintf(
    char *        buffer,       /* buffer to write to */
    size_t        n,            /* dummy */
    const char *  fmt,          /* format string */
    va_list       vaList)       /* optional arguments to format */
{
    int nChars;
    struct putbufArgument arg = {&buffer, 0, 0, 0 };

    arg.max_len  = n;
    nChars = fioFormatV (fmt, vaList, putbuf, (int) &arg);
    **arg.outputBuffer = EOS;

    return (arg.result);
}

int 
snprintf(
    char *        buffer,/* buffer to write to */
    size_t        n,     /* dummy */
    const char *  fmt,   /* format string */
    ...)                  /* optional arguments to format */
{
    va_list     vaList;  /* traverses argument list */
    int         nChars;
    struct putbufArgument arg = {&buffer, 0, 0, 0 };
    
    arg.max_len  = n;
    va_start (vaList, fmt);
    nChars = fioFormatV (fmt, vaList, putbuf, (int) &arg);
    va_end (vaList);

    **arg.outputBuffer = EOS;

    return (arg.result);
}

int 
getuid()
{
    return (0);
}

int geteuid()
{
    return (0);
}

#if defined ( VXWORKS_55 ) || defined ( VXWORKS_54 ) || defined ( OSPL_VXWORKS653 )
int 
getpid()
{
    return (taskIdSelf());
}
#endif

int 
access(
    const char *path, int amode)
{
#if defined (OSPL_VXWORKS653)
  return(-1);
#else
    Elf32_Ehdr file_exec;
    int        fd;
    int        tmode = 0;
    int        emode = 0;

   int currentArch = -1;

#if defined ( VXWORKS_55 ) || defined ( VXWORKS_54 )
#if CPU_PPC
    currentArch = EM_PPC;
#elif CPU_PENTIUM
    currentArch = EM_386;
#else
#error "Unsupported CPU type"
#endif
#else
#if CPU_PPC
    currentArch = EM_PPC;
#elif ( _VX_CPU_FAMILY==_VX_I80X86 )
    currentArch = EM_386;
#else
#error "Unsupported CPU type"
#endif
#endif


    fd = open(path, O_WRONLY, 0);
    if (fd != ERROR) {
        tmode = W_OK | F_OK;
        emode = 1;
        close(fd);
    }
    fd = open (path, O_RDONLY, 0);
    if (fd != ERROR) {
        tmode = tmode | R_OK | F_OK;
        emode = 1;
        if (read (fd, (char *)&file_exec, sizeof (file_exec)) == sizeof (file_exec)) {
            if ((strncmp (ELFMAG,file_exec.e_ident,4) == 0) && 
		(file_exec.e_machine == currentArch)) {
                tmode = tmode | X_OK ;
            }
        }
        close(fd);
    }
    if ((amode & tmode) == amode) {
        if ((amode == 0) && (emode == 0)) {
            return(-1);
        } else {
            return(0);
        }
    } else {
        return(-1);
    }
#endif
}

char *
strdup(
    const char *s1)
{
   char *str;
   if (s1 != (char *)NULL) {
       str = (char *)malloc(strlen(s1) + 1 );
       os_strncpy (str,s1,strlen(s1));
       str[strlen(s1)] = '\0';
       return (str);
   } else {
      return (NULL);
   } 

}



const char pwd_static[] = "VXWORKS";
struct passwd *
getpwuid(
    int uid)
{
    static struct passwd glb_passwd;
    glb_passwd.pw_name = (char *)pwd_static;
    glb_passwd.pw_passwd = (char *)pwd_static;
    glb_passwd.pw_age = (char *)pwd_static;
    glb_passwd.pw_comment = (char *)pwd_static;
    glb_passwd.pw_gecos = (char *)pwd_static;
    glb_passwd.pw_dir = (char *)pwd_static;
    glb_passwd.pw_shell = (char *)pwd_static;
    glb_passwd.pw_uid = 0;
    glb_passwd.pw_gid = 0;
    return (&glb_passwd);
}


/* used for dbt programs */
int
start_prog(
    char *executable_file,
    char *name,
    char *arguments,
    int prio)
{
    os_procAttr procAttr;
    os_procId Id;

    procAttr.schedClass = OS_SCHED_REALTIME;
    procAttr.lockPolicy = OS_LOCK_DEFAULT;
    procAttr.schedPriority = prio;
    procAttr.userCred.uid = 0;
    procAttr.userCred.gid = 0;

    return(os_procCreate(executable_file, name, arguments, &procAttr, &Id));
}


#ifdef HRTIME
extern unsigned int sysBusClkUs(void);

static long long
current_time (void)
{
    unsigned int        lowcounter, highcounter;
    unsigned long long  time;

    sysGetTimeBase (&lowcounter, &highcounter);
    time = highcounter;
    time <<= 32;
    time |= lowcounter;

    return (time);
}
#endif

#if defined VXWORKS_55 || defined VXWORKS_54
void
gettimeofday(struct timeval *tp, void * dummy)
{
#ifdef HRTIME
    tp->tv_sec = 0;
    tp->tv_usec = (current_time() / sysBusClkUs());
#else
    struct timespec tv;
    clock_gettime (CLOCK_REALTIME, &tv);
    if (tv.tv_nsec) tp->tv_usec =
           tv.tv_nsec/1000;
    tp->tv_sec  = tv.tv_sec;
#endif
}
#endif
