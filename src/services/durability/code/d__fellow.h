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
#ifndef D__FELLOW_H
#define D__FELLOW_H

#include "d__types.h"
#include "d_lock.h"
#include "os_mutex.h"
#include "d_fellow.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_fellow){
    C_EXTENDS(d_lock);
    d_networkAddress address;
    d_serviceState state;
    d_communicationState communicationState;
    d_name role;
    d_timestamp lastStatusReport;
    d_table groups;
    d_table nameSpaces;
    c_ulong requestCount;
    c_long expectedGroupCount;
    c_ulong expectedNameSpaces;
    c_bool groupsRequested;
};

void    d_fellowDeinit      (d_object object);

#if defined (__cplusplus)
}
#endif

#endif /* D__FELLOW_H */
