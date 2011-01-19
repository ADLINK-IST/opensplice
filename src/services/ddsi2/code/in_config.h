/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2010 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef IN_CONFIG_H_
#define IN_CONFIG_H_

#include "os_classbase.h"
#include "os_iterator.h"
#include "os_heap.h"
#include "os_socket.h"
#include "u_participant.h"

typedef enum in_level {
    IN_LEVEL_FINEST, IN_LEVEL_FINER, IN_LEVEL_FINE,
    IN_LEVEL_CONFIG, IN_LEVEL_INFO,
    IN_LEVEL_WARNING, IN_LEVEL_SEVERE, IN_LEVEL_NONE
} in_level;

OS_CLASS(in_config);

OS_STRUCT(in_config)
{
    os_char* networkAddressString;
    FILE* tracingOutputFile;
    c_char* tracingOutputFileName;
    c_bool tracingTimestamps;
    c_bool tracingRelativeTimestamps;
    in_level tracingVerbosityLevel;
    os_char* peers;
    c_bool allowMulticast;
    c_bool enableMulticastLoopback;
    int domainId;
    int participantIndex;
    c_bool coexistWithNativeNetworking;
    os_char* enableUndocFeatures;
};

in_config
in_configNew(
    u_participant participant,
    const c_char* serviceName);

void
in_configFree(
    in_config config);

#define in_config(c) ((in_config)(c))

void
in_printf(
    in_level level,
    const char * eventText,
    ...);

int
in_printfRtps(
    const char* eventText,
    va_list args);

#endif /* IN_CONFIG_H_ */
