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

#ifndef D__DURABILITY_H
#define D__DURABILITY_H

#include "d__types.h"
#include "u_user.h"
#include "os_thread.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_durability){
    C_EXTENDS(d_object);
    u_service        service;
    u_serviceManager serviceManager;
    d_configuration  configuration;
    d_admin          admin;
    
    d_serviceState   state;
    os_threadId      leaseThread;
    os_threadId      statusThread;
    c_bool           splicedRunning;
};

typedef enum d_connectivity_s {
    D_CONNECTIVITY_UNDETERMINED,
    D_CONNECTIVITY_OK,
    D_CONNECTIVITY_INCOMPATIBLE_STATE,
    D_CONNECTIVITY_INCOMPATIBLE_DATA_MODEL
} d_connectivity;

#define D_OSPL_NODE "__NODE"
#define D_OSPL_BUILTIN_PARTITION "BUILT-IN PARTITION__"

d_durability    d_durabilityNew                     (const c_char* uri, 
                                                     const c_char* serviceName,
                                                     c_long domainId);

void            d_durabilityInit                    (d_durability durability);

void            d_durabilityDeinit                  (d_object object);

d_connectivity  d_durabilityDetermineConnectivity   (d_durability durability);

void            d_durabilityHandlePersistentInitial (d_durability durability);

void            d_durabilityFree                    (d_durability durability);

c_bool          d_durabilityArgumentsProcessing     (int argc,
                                                     char *argv[],
                                                     c_char **uri,
                                                     c_char **serviceName);

void            d_durabilityWatchSpliceDaemon       (v_serviceStateKind spliceDaemonState,
                                                     c_voidp usrData);

c_voidp         d_durabilityUpdateLease             (c_voidp args);

c_voidp         d_durabilityNotifyStatus            (c_voidp args);

void            d_durabilityLoadModule              (v_entity entity,
                                                     c_voidp args);

#if defined (__cplusplus)
}
#endif

#endif /* D__DURABILITY_H */
