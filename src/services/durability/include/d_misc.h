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

#ifndef D_MISC_H
#define D_MISC_H

#include "os.h"
#include "d__types.h"
#include "c_time.h"
#include "v_kernel.h"
#include "d_durability.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define D_CONTEXT_DURABILITY    "Durability Service"
#define RR_MALLOC_FAILED        "Malloc failed for '%s'\n"

#define D_THREAD_MAIN                        "mainThread"
#define D_THREAD_UNSPECIFIED                 "unspecified"
#define D_THREAD_GROUP_LOCAL_LISTENER        "groupLocalListener"
#define D_THREAD_GROUP_REMOTE_LISTENER       "groupRemoteListener"
#define D_THREAD_GROUPS_REQUEST_LISTENER     "groupsRequestListener"
#define D_THREAD_SAMPLE_CHAIN_LISTENER       "sampleChainListener"
#define D_THREAD_SAMPLE_REQUEST_LISTENER     "sampleRequestListener"
#define D_THREAD_NAMESPACES_REQUEST_LISTENER "nameSpacesRequestListener"
#define D_THREAD_NAMESPACES_LISTENER         "nameSpacesListener"
#define D_THREAD_STATUS_LISTENER             "statusListener"
#define D_THREAD_PERISTENT_DATA_LISTENER     "persistentDataListener"
#define D_THREAD_DELETE_DATA_LISTENER        "deleteDataListener"
#define D_THREAD_SPLICED_LISTENER            "splicedListener"
#define D_THREAD_READER_LISTENER             "readerListener"
#define D_THREAD_RESEND_QUEUE            	 "resendQueue"
#define D_THREAD_GROUP_CREATION              "groupCreationQueue"

void    d_free   ( c_voidp allocated );
c_voidp d_malloc ( os_uint32 size, const char * errorText );

void                d_conv_os2c_time            (c_time *time,os_time *osTime);

void                d_printTimedEvent           (d_durability durability,
                                                 d_level level,
                                                 const char* thread,
                                                 const char * eventText,
                                                 ...);

void                d_printEvent                (d_durability durability,
                                                 d_level level,
                                                 const char * eventText,
                                                 ...);

void                d_reportLocalGroup          (d_durability durability,
                                                 const char *thread,
                                                 v_group group);

c_base              d_findBase                  (d_durability durability);

c_bool              d_patternMatch              (const char* str,
                                                 const char* pattern);

#if defined (__cplusplus)
}
#endif

#endif /* D_MISC_H */
