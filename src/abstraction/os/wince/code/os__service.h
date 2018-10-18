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
#ifndef OS_WIN32__SERVICE_H__
#define OS_WIN32__SERVICE_H__

#include "os_defs.h"
#include "os_time.h"
#include "os_mutex.h"
#include "os_cond.h"

#define OS_SERVICE_ENTITY_NAME_MAX       32
#define OS_SERVICE_SEM_NAME_PREFIX       "osplSem"
#define OS_SERVICE_EVENT_NAME_PREFIX     "osplEv" 
#define OS_SERVICE_MUTEX_NAME_PREFIX     "osplMTX"

enum os_servicemsg_kind {
    OS_SRVMSG_UNDEFINED,
    OS_SRVMSG_CREATE_EVENT,
    OS_SRVMSG_DESTROY_EVENT,
    OS_SRVMSG_CREATE_SEMAPHORE,
    OS_SRVMSG_DESTROY_SEMAPHORE,
    OS_SRVMSG_GET_TIME,
    OS_SRVMSG_TERMINATE,
    OS_SRVMSG_COUNT
};

struct os_servicemsg {
    os_result result;
    enum os_servicemsg_kind kind;
    long privateMessageQueueId;
    long lifecycleId;
    union {
        long id;
        struct _ospl_time {
            os_timeM start_time;
            LONGLONG time_offset;
        } time;
    } _u;
};

void
os_serviceInitialiseMsgQueueData(void);

void
os_serviceFinaliseMsgQueueData(void);

long
os_serviceGetProcessId(void);

os_result
requestResponseFromServiceQueue
(
    struct os_servicemsg * request,
    struct os_servicemsg * reply,
    char * writeMsgQueueName
);

os_char *
os_servicePipeName(void);

os_char *
os_createPipeNameFromMutex(os_mutex *mutex);

os_char *
os_createPipeNameFromCond(os_cond *cond);

os_char *
os_constructPipeName(const os_char * name);

#endif /* OS_WIN32__SERVICE_H__ */
