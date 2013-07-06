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

#ifndef D__GROUPLOCALLISTENER_H
#define D__GROUPLOCALLISTENER_H

#include "d__types.h"
#include "d__listener.h"
#include "u_user.h"
#include "v_group.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define D_FLOOR_SEQUENCE_NUMBER  (-1)

C_CLASS(d_groupIncomplete);

C_STRUCT(d_groupIncomplete){
    d_group dgroup;
    v_group vgroup;
};

#define d_groupIncomplete(g) ((d_groupIncomplete)(g))

C_STRUCT(d_groupLocalListener){
    C_EXTENDS(d_listener);
    c_bool initialGroupsAdministrated;
    c_long lastSequenceNumber;
    os_mutex masterLock;
    d_eventListener fellowListener;
    d_eventListener nameSpaceListener;
    d_sampleChainListener sampleChainListener;
    d_waitsetEntity waitsetData;
    d_actionQueue actionQueue;
    d_actionQueue masterMonitor;
};

struct findAligner{
    d_fellow fellow;
    d_group group;
};

void        d_groupLocalListenerInit                    (d_groupLocalListener listener,
                                                         d_subscriber subscriber);

void        d_groupLocalListenerDeinit                  (d_object object);

c_ulong     d_groupLocalListenerAction                  (u_dispatcher o,
                                                         u_waitsetEvent event,
                                                         c_voidp usrData);

void        d_groupLocalListenerInitLocalGroups         (v_entity e,
                                                         c_voidp args);

c_ulong     d_groupLocalListenerNewGroupLocalAction     (u_dispatcher o,
                                                         c_ulong event,
                                                         c_voidp userData);

void        d_groupLocalListenerHandleNewGroupsLocal    (v_entity entity,
                                                         c_voidp args);

c_bool      d_groupLocalFindAlignerFellow               (d_fellow fellow,
                                                         c_voidp userData);

void        d_groupLocalListenerInitPersistent          (d_nameSpace nameSpace,
                                                         c_voidp userData);

c_bool      d_groupLocalListenerFindNameSpaceFellow     (d_fellow fellow,
                                                         c_voidp userData);

c_bool      findBestQualityForNS                        (d_group group,
                                                         c_voidp userData);

#if defined (__cplusplus)
}
#endif

#endif /* D__GROUPLOCALLISTENER_H */
