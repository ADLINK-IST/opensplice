/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "os_diag.h"
#include "os_time.h"
#include "os_stdlib.h"
#include "os_process.h"
#include "os_thread.h"
#include "os_abstract.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#ifdef VXWORKS_RTP
#include <wrapper/wrapperHostLib.h>
#endif

#define OS_DIAGSERVICES_MAX	(1)

typedef struct {
    void (*os_diagService)(
    os_IDiagService_s diagServiceContext,
    const char *cluster,
    os_diagLevel level,
    const char *description,
    va_list args);
    os_IDiagService_s os_diagContext;
} os_diagServiceType;

static os_diagServiceType os_diagServices[OS_DIAGSERVICES_MAX] = {
    { 0, 0 } };

static os_int32 os_diagServicesCount = 0;

static const char *os_diagLevelText [] = {
    "Function Entry",
    "Function Exit",
    "Flow Trace"
};

static void
os_defaultDiag (
    const char   *cluster,
    os_diagLevel  level,
    const char   *description,
    va_list 	  args)
{
    os_timeW ostime;
    char extended_description[512];
    char procIdentity[64];
    char threadIdentity[64];
    char node[64];

    ostime = os_timeWGet();

    os_vsnprintf (extended_description, sizeof(extended_description)-1, description, args);
    extended_description [sizeof(extended_description)-1] = '\0';

    os_gethostname (node, sizeof(node));
    os_threadFigureIdentity (threadIdentity, sizeof (threadIdentity)-1);
    threadIdentity [sizeof (threadIdentity)-1] = '\0';
    os_procFigureIdentity (procIdentity, sizeof (procIdentity)-1);
    procIdentity [sizeof (procIdentity)-1] = '\0';

    printf (
	"Diag - %s:%s:%s - T:%" PA_PRItime " L:%s C:%s D:%s\n",
        node,
        procIdentity,
        threadIdentity,
        OS_TIMEW_PRINT(ostime),
	os_diagLevelText[level],
	cluster,
	extended_description);
}

void
os_diag (
    const char   *diag_clusters,
    const char   *cluster,
    os_diagLevel  level,
    const char   *description,
    ...)
{
    va_list args;
    os_int32 i;

    if (strstr (diag_clusters, cluster) == (char *)0) {
	return;
    }
    va_start (args, description);
    if (os_diagServicesCount != 0) {
	for (i = 0; i < OS_DIAGSERVICES_MAX; i++) {
	    if (os_diagServices[i].os_diagContext != 0) {
		os_diagServices[i].os_diagService (os_diagServices[i].os_diagContext,
		    cluster, level, description, args);
	    }
	}
    } else {
	os_defaultDiag (cluster, level, description, args);
    }
    va_end (args);
}

os_int32
os_registerDiagService (
    os_IDiagService_s diagServiceContext,
    void (*diagService)(
        os_IDiagService_s diagServiceContext,
        const char       *cluster,
        os_diagLevel      level,
        const char       *description,
        va_list           args)
    )
{
    os_int32 i;

    for (i = 0; i < OS_DIAGSERVICES_MAX; i++) {
        if (os_diagServices[i].os_diagContext == 0) {
	    os_diagServices[i].os_diagContext = diagServiceContext;
	    os_diagServices[i].os_diagService = diagService;
	    os_diagServicesCount++;
	    return os_diagServicesCount;
	}
    }
    return -1;
}

os_int32
os_unregisterDiagService (
    os_IDiagService_s diagServiceContext)
{
    os_int32 i;

    for (i = 0; i < OS_DIAGSERVICES_MAX; i++) {
        if (os_diagServices[i].os_diagContext == diagServiceContext) {
	    os_diagServices[i].os_diagContext = 0;
	    os_diagServices[i].os_diagService = 0;
	    os_diagServicesCount--;
	    return os_diagServicesCount;
	}
    }
    return -1;
}
