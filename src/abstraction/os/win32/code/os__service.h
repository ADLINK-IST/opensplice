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
#ifndef OS_WIN32__SERVICE_H__
#define OS_WIN32__SERVICE_H__

#include "os_defs.h"
#include "os_time.h"
#include "os_mutex.h"
#include "os_cond.h"

#define OS_SERVICE_ENTITY_NAME_MAX      32
#define OS_SERVICE_SEM_NAME_PREFIX      "osplSem"
#define OS_SERVICE_EVENT_NAME_PREFIX    "osplEv"
#define OS_SERVICE_MUTEX_NAME_PREFIX    "osplMTX"
#define OS_SERVICE_GLOBAL_NAME_PREFIX   "Global\\"

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
    union {
        long id;
        struct _ospl_time {
            os_time start_time;
            LONGLONG time_offset;
        } time;
    } _u;
};

os_char *
os_servicePipeName(void);

os_char *
os_createPipeNameFromMutex(os_mutex *mutex);

os_char *
os_createPipeNameFromCond(os_cond *cond);

os_char *
os_constructPipeName(const os_char * name);

#endif /* OS_WIN32__SERVICE_H__ */
