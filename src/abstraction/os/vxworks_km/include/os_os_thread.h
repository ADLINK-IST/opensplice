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
#ifndef OS_VXWORKS_THREAD_H
#define OS_VXWORKS_THREAD_H

#include <vxWorks.h>
#include "os_typebase.h"
#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define THREAD_VALIDITY_RUNNING (0x74687264) /* thread running */
#define THREAD_VALIDITY_EXIT    (0x65786974) /* exit    */
#define THREAD_VALIDITY_READ    (0x4d48414d) /* status is read */
typedef enum {
    thread_ValInvalid = 0,
    thread_ValInit    = 1,
    thread_ValRunning = THREAD_VALIDITY_RUNNING,
    thread_ValExit    = THREAD_VALIDITY_EXIT,
    thread_ValRead    = THREAD_VALIDITY_READ
} thread_Validity;

#define os_threadContextData(o)         ((os_threadContextData)o)
OS_CLASS(os_threadContextData);
OS_STRUCT(os_threadContextData) {
    thread_Validity  threadValidity;
    os_os_int32         threadAttrPrio;
    char             *threadName;
    os_os_int32         threadTaskId;
    void             *threadExitStatus;
    os_os_int32         procId;
    os_os_uint32        *threadMem;
    SEM_ID           threadDataSem;
    SEM_ID           threadExitTrigger;
};

extern os_threadContextData threadContextData;
extern os_threadContextData id_none;

typedef os_threadContextData os_os_threadId;

#define OS_THREAD_ID_NONE id_none

os_result os_threadNew(const char *name);
char * os_threadGetName();
void * os_threadCheckExitValue(os_threadContextData process_threadContextData);
void os_threadDispose();
os_result os_threadAwaitTermination(os_threadContextData process_threadContextData);
void os_threadMemFree(os_os_int32 index);
os_threadContextData os_threadInit(const char *name);
os_result os_threadAddTaskVar(os_os_int32 taskid, os_threadContextData process_threadContextData);
os_result os_threadDeleteTaskVar(os_os_int32 taskid, os_threadContextData process_threadContextData);

void
os_threadSetValidity (os_threadContextData process_threadContextData,
		      thread_Validity Validity);

void os_threadHookThreadInit(os_procContextData process_procContextData,
			     os_os_int32 threadTaskId);

os_result os_hookthreadNew(const char *name,
			   os_procContextData process_procContextData,
			   os_os_int32 taskid);
#if defined (__cplusplus)
}
#endif

#endif /* OS_VXWORKS_THREAD_H */
