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

#ifndef D_FELLOW_H
#define D_FELLOW_H

#include "d__types.h"
#include "os_mutex.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_fellow(f) ((d_fellow)(f))

typedef enum d_communicationState_s {
    D_COMMUNICATION_STATE_UNKNOWN = 0,
    D_COMMUNICATION_STATE_APPROVED = 1,
    D_COMMUNICATION_STATE_INCOMPATIBLE_STATE = 2,
    D_COMMUNICATION_STATE_INCOMPATIBLE_DATA_MODEL = 3,
    D_COMMUNICATION_STATE_TERMINATED = 4
} d_communicationState;

/* Translate fellow state to text */
extern char* d_fellowState_text[];
#define d_fellowStateText(state) d_fellowState_text[state]


typedef enum d_fellowAlignStatus_s {
    D_ALIGN_FALSE,
    D_ALIGN_TRUE,
    D_ALIGN_UNKNOWN
} d_fellowAlignStatus;

d_fellow                d_fellowNew                     (d_networkAddress address,
                                                         d_serviceState state);

void                    d_fellowFree                    (d_fellow fellow);

d_serviceState          d_fellowGetState                (d_fellow fellow);

void                    d_fellowUpdateStatus            (d_fellow fellow,
                                                         d_serviceState state,
                                                         d_timestamp timestamp);

c_bool                  d_fellowAddGroup                (d_fellow fellow,
                                                         d_group group);
                                             
d_group                 d_fellowGetGroup                (d_fellow fellow,
                                                         const c_char* partition,
                                                         const c_char* topic,
                                                         d_durabilityKind kind);

d_group                 d_fellowRemoveGroup             (d_fellow fellow,
                                                         d_group group);

int                     d_fellowCompare                 (d_fellow fellow1,
                                                         d_fellow fellow2);

c_bool                  d_fellowGroupWalk               (d_fellow fellow,
                                                         c_bool ( * action ) (
                                                            d_group group, 
                                                            c_voidp userData),
                                                         c_voidp userData);

d_timestamp             d_fellowGetLastStatusReport     (d_fellow fellow);

d_networkAddress        d_fellowGetAddress              (d_fellow fellow);

void                    d_fellowSetAddress              (d_fellow fellow,
                                                         d_networkAddress address);

d_communicationState    d_fellowGetCommunicationState   (d_fellow fellow);

void                    d_fellowSetCommunicationState   (d_fellow fellow,
                                                         d_communicationState state);

c_bool                  d_fellowSetGroupsRequested      (d_fellow fellow);

c_bool                  d_fellowGetGroupsRequested      (d_fellow fellow);

c_bool                  d_fellowAddNameSpace            (d_fellow fellow,
                                                         d_nameSpace nameSpace);

c_bool                  d_fellowAreNameSpacesComplete   (d_fellow fellow);

c_ulong                 d_fellowNameSpaceCount          (d_fellow fellow);

c_bool                  d_fellowNameSpaceWalk           (d_fellow fellow,
                                                         c_bool ( * action ) (
                                                            d_nameSpace nameSpace, 
                                                            c_voidp userData),
                                                         c_voidp userData);

d_nameSpace             d_fellowGetNameSpace            (d_fellow fellow,
                                                         d_nameSpace template);

void                    d_fellowRequestAdd              (d_fellow fellow);

void                    d_fellowRequestRemove           (d_fellow fellow);

c_ulong                 d_fellowRequestCountGet         (d_fellow fellow);

void                    d_fellowRequestCountSet         (d_fellow fellow,
                                                         c_ulong count);

void                    d_fellowSetExpectedGroupCount   (d_fellow fellow,
                                                         c_ulong count);

c_long                  d_fellowGetExpectedGroupCount   (d_fellow fellow);

void                    d_fellowSetExpectedNameSpaces   (d_fellow fellow,
                                                         c_ulong count);

c_ulong                 d_fellowGetExpectedNameSpaces   (d_fellow fellow);

c_ulong                 d_fellowGetGroupCount           (d_fellow fellow);

c_bool                  d_fellowIsCompleteForGroup      (d_fellow fellow,
                                                         const c_char* partition,
                                                         const c_char* topic,
                                                         d_durabilityKind kind);

d_fellowAlignStatus     d_fellowIsAlignerForGroup       (d_fellow fellow,
                                                         const c_char* partition,
                                                         const c_char* topic,
                                                         d_durabilityKind kind);

d_fellowAlignStatus     d_fellowIsAlignerForNameSpace   (d_fellow fellow,
                                                         d_nameSpace nameSpace);

c_bool                  d_fellowIsGroupInNameSpaces     (d_fellow fellow,
                                                         const c_char* partition,
                                                         const c_char* topic,
                                                         d_durabilityKind kind);

c_bool                  d_fellowHasGroup                (d_fellow fellow,
                                                         const c_char* partition,
                                                         const c_char* topic,
                                                         d_durabilityKind kind);

void                    d_fellowClearMaster             (d_fellow fellow,
                                                         d_networkAddress master);

void                    d_fellowSetRole                 (d_fellow fellow,
                                                         d_name role);

d_name                d_fellowGetRole                 (d_fellow fellow);

#if defined (__cplusplus)
}
#endif

#endif /* D_FELLOW_H */
