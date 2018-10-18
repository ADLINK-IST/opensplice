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
#ifndef D__FELLOW_H
#define D__FELLOW_H

#include "d__types.h"
#include "d__lock.h"
#include "d__table.h"
#include "os_mutex.h"
#include "os_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* Durability reader flags.
 * These flags can be used to determine what functionality is
 * offered by a fellow
 */
#define D_NAMESPACES_READER_FLAG             (0x0001U << 0) /*    1 */
#define D_NAMESPACESREQUEST_READER_FLAG      (0x0001U << 1) /*    2 */
#define D_STATUS_READER_FLAG                 (0x0001U << 2) /*    4 */
#define D_GROUPSREQUEST_READER_FLAG          (0x0001U << 3) /*    8 */
#define D_DELETEDATA_READER_FLAG             (0x0001U << 4) /*   16 */
#define D_NEWGROUP_READER_FLAG               (0x0001U << 5) /*   32 */
#define D_SAMPLREQUEST_READER_FLAG           (0x0001U << 6) /*   64 */
#define D_SAMPLECHAIN_READER_FLAG            (0x0001U << 7) /*   128 */
#define D_CAPABILITY_READER_FLAG             (0x0001U << 8) /*   256 */

#define D_BASIC_DURABILITY_READER_FLAGS  \
    D_NAMESPACES_READER_FLAG | D_NAMESPACESREQUEST_READER_FLAG | D_STATUS_READER_FLAG |  \
    D_GROUPSREQUEST_READER_FLAG | D_DELETEDATA_READER_FLAG | D_NEWGROUP_READER_FLAG  |   \
    D_SAMPLREQUEST_READER_FLAG | D_SAMPLECHAIN_READER_FLAG

typedef enum d_communicationState_s {
    D_COMMUNICATION_STATE_UNKNOWN                 = 0,
    D_COMMUNICATION_STATE_APPROVED                = 1,
    D_COMMUNICATION_STATE_INCOMPATIBLE_STATE      = 2,
    D_COMMUNICATION_STATE_INCOMPATIBLE_DATA_MODEL = 3,
    D_COMMUNICATION_STATE_TERMINATED              = 4
} d_communicationState;

/* Translate fellow state to text */
extern char* d_fellowState_text[];
#define d_fellowStateText(state) d_fellowState_text[state]

typedef enum d_fellowAlignStatus_s {
    D_ALIGN_FALSE,
    D_ALIGN_TRUE,
    D_ALIGN_UNKNOWN
} d_fellowAlignStatus;


/**
 * Macro that checks the d_fellow validity.
 * Because d_fellow is a concrete class typechecking is required.
 */
#define             d_fellowIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_FELLOW)

/**
 * \brief The d_fellow cast macro.
 *
 * This macro casts an object to a d_fellow object.
 */
#define d_fellow(_this) ((d_fellow)(_this))

/**
 * Descriptor to hold all capabilities of the fellow.
 */
struct capabilityDescriptor {
    c_bool groupHash;
    c_bool EOTSupport;
    c_bool Y2038Ready;
    c_ulong masterSelection;
    c_ulong incarnation;
};

C_STRUCT(d_fellow){
    C_EXTENDS(d_lock);
    d_networkAddress address;
    d_serviceState state;
    d_communicationState communicationState;
    d_name role;
    os_timeM lastStatusReport;
    os_uint32 lastSeqNum;
    d_table groups;
    d_table nameSpaces;
    c_ulong requestCount;
    c_long expectedGroupCount;
    c_ulong expectedNameSpaces;
    c_bool groupsRequested;     /* Indicates whether I have requested groups from the fellow */
    c_bool isConfirmed;         /* TRUE if this fellow is confirmed, FALSE otherwise. */
    c_bool hasConfirmed;        /* TRUE if this fellow has become confirmed but no nameSpaces have been requested yet from this fellow. */
    c_bool recentlyTerminated;  /* Indicates if the message that this fellow has recently terminated has been printed or not. */
    os_timeM removalTime;       /* Time (monotonic) when the fellow is removed */
    c_bool capabilitySupport;   /* Indicates if the fellow supports the capability interface or not */
    c_long heartbeatDisposeCount;
    c_ulong readers;            /* Bit pattern to indicate which durability readers are discovered of the fellow */
    c_ulong requiredReaders;    /* Bit pattern to indicate which durability readers MUST BE discovered before the fellow is considered responsive */
    c_bool recently_joined;     /* Indicator if the fellow has recently joined */
    c_bool responsive;          /* Indicates if a namespacerequest has been sent to the fellow */
    c_bool has_requested_namespaces; /* Indicates if the fellow has requested namespaces from me */
    struct capabilityDescriptor capabilities;  /* Indicates the capabilities received from this fellow */
    c_ulong incarnation;        /* The incarnation that I advertise to my fellow */
};

d_fellow                d_fellowNew                     (d_networkAddress address,
                                                         d_serviceState state,
                                                         c_bool isConfirmed);

void                    d_fellowDeinit                  (d_fellow fellow);

void                    d_fellowFree                    (d_fellow fellow);

d_serviceState          d_fellowGetState                (d_fellow fellow);

void                    d_fellowUpdateStatus            (d_fellow fellow,
                                                         d_serviceState state,
                                                         os_uint32 seqnum);

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

os_timeM                d_fellowGetLastStatusReport     (d_fellow fellow);

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
                                                         c_long count);

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
                                                         d_durabilityKind kind,
                                                         c_ulong *masterPriority);

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

d_name                  d_fellowGetRole                 (d_fellow fellow);

c_bool                  d_fellowIsConfirmed             (d_fellow fellow);

void                    d_fellowSetConfirmed            (d_fellow fellow,
                                                         c_bool confirmed);

c_bool                  d_fellowHasConfirmed            (d_fellow fellow);


c_bool                  d_fellowHasRecentlyTerminated   (d_fellow fellow);

c_long                  d_fellowSetLastDisposeCount     (d_fellow fellow,
                                                         c_long disposeCount);

c_long                  d_fellowGetLastDisposeCount     (d_fellow fellow);

void                    d_fellowSetCapabilitySupport    (d_fellow fellow,
                                                         c_bool capabilitySupport);

c_bool                  d_fellowHasCapabilitySupport    (d_fellow fellow);

c_bool                  d_fellowIsResponsive            (d_fellow fellow,
                                                         c_bool waitForRemoteReaders);

void                    d_fellowAddReader               (d_fellow fellow,
                                                         c_ulong reader);

void                    d_fellowRemoveReader            (d_fellow fellow,
                                                         c_ulong reader);

c_bool                  d_fellowHasDiscoveredReaders    (d_fellow fellow,
                                                         c_ulong mask,
                                                         c_bool waitForRemoteReaders);

void                    d_fellowSetCapability           (d_fellow fellow,
                                                         d_capability capability);

c_bool                  d_fellowHasCapabilityGroupHash  (d_fellow fellow);

c_bool                  d_fellowHasCapabilityEOTSupport (d_fellow fellow);

c_bool                  d_fellowHasCapabilityY2038Ready (d_fellow fellow);

void                    d_fellowSendNSRequest           (d_fellow fellow);

void                    d_fellowCheckSendCapabilities   (d_fellow fellow, c_bool initial);

os_uint32               d_fellowGetLastSeqNum           (d_fellow fellow);
c_bool                  d_fellowHasRecentlyJoined       (d_fellow fellow);

void                    d_fellowCheckInitialResponsiveness(d_fellow fellow);

#if defined (__cplusplus)
}
#endif

#endif /* D__FELLOW_H */
