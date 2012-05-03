/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#ifndef NW_CONFIGURATIONDEFS_H
#define NW_CONFIGURATIONDEFS_H

#include "os_socket.h"

#define NWCF_SEP "/"
#define NWCF_ROOT(par)           NWCF_ROOT_##par
#define NWCF_SUBROOT(root, par)  root NWCF_SEP par
#define NWCF_NAME(par)           NWCF_NAME_##par
#define NWCF_ATTRIB(par)         NWCF_ATTRIB_##par
#define NWCF_DEF(par)            NWCF_DEF_##par
#define NWCF_MIN(par)            NWCF_MIN_##par
#define NWCF_MAX(par)            NWCF_MAX_##par

#define NWCF_SIMPLE_PARAM(type,root,par) \
    nw_configurationGet##type##Parameter(root, NWCF_NAME_##par, NWCF_DEF_##par)

#define NWCF_DEFAULTED_PARAM(type,root,par,def) \
    nw_configurationGet##type##Parameter(root, NWCF_NAME_##par, def)

#define NWCF_SIMPLE_ATTRIB(type,root,attr) \
    nw_configurationGet##type##Attribute(root, NWCF_ATTRIB_##attr, NWCF_DEF_##attr, NWCF_DEF_##attr)

#define NWCF_DEFAULTED_ATTRIB(type,root,attr,defNoElmt, defNoAttrib) \
    nw_configurationGet##type##Attribute(root, NWCF_ATTRIB_##attr, defNoElmt, defNoAttrib)

#define NWCF_SIMPLE_SUBPARAM(type,root,sub,par) \
    nw_configurationGet##type##Parameter(root, NWCF_ROOT_##sub NWCF_SEP NWCF_NAME_##par, NWCF_DEF_##par)

#define NWCF_DEFAULTED_SUBPARAM(type,root,sub,par,def) \
    nw_configurationGet##type##Parameter(root, NWCF_ROOT_##sub NWCF_SEP NWCF_NAME_##par, def)

#define NWCF_BROADCAST_EXPR       "broadcast"
#define NWCF_FIRSTAVAILABLE_EXPR  "first available"


#define NWCF_ROOT_NetworkingService     "NetworkService"
#define NWCF_ROOT_Domain                "Domain"


/* ----------------------------- General /----------------------------------- */
#define NWCF_ROOT_General               "General"

/* ------------------ General/NetworkInterfaceAddress ---------------------- */
#define NWCF_NAME_Interface             "NetworkInterfaceAddress"     /* string */
#define NWCF_DEF_Interface              NWCF_FIRSTAVAILABLE_EXPR
#define NWCF_ATTRIB_forced              "forced"                     /* boolean */
#define NWCF_DEF_forced                 (FALSE)
#define NWCF_ATTRIB_ipv6                "ipv6"                     /* boolean */
#define NWCF_DEF_ipv6                   (FALSE)
/* ------------------ General/Reconnection ---------------------- */
#define NWCF_NAME_Reconnection          "Reconnection"                /* string */
#define NWCF_ATTRIB_allowed             "allowed"                      /* ulong */
#define NWCF_DEF_allowed                (0)
/* ----------------------------- Channels ----------------------------------- */
#define NWCF_ROOT_Channels              "Channels"
/* ------------------------- Channels/Channel ------------------------------- */
#define NWCF_ROOT_Channel              NWCF_SUBROOT(NWCF_ROOT_Channels, "Channel")
#define NWCF_ATTRIB_ChannelName        "name"
#define NWCF_ATTRIB_priority           "priority"                      /* ulong */
#define NWCF_DEF_priority              (0)
#define NWCF_ATTRIB_reliable           "reliable"                    /* boolean */
#define NWCF_DEF_reliable              (FALSE)
#define NWCF_ATTRIB_default            "default"                     /* boolean */
#define NWCF_DEF_default               (FALSE)
#define NWCF_ATTRIB_enabled            "enabled"                     /* boolean */
#define NWCF_DEF_enabled               (TRUE)

/* -------------------- Channels/Channel/QueueSize -------------------------- */
#define NWCF_NAME_QueueSize           "QueueSize"                      /* ulong */
#define NWCF_DEF_QueueSize            (400U)
#define NWCF_MIN_QueueSize            (1U)

/* --------------------- Channels/Channel/PortNr ---------------------------- */
#define NWCF_NAME_PortNr              "PortNr"                        /* ushort */
#define NWCF_DEF_PortNr               (3367U)
#define NWCF_MIN_PortNr               (1U)
#define NWCF_MAX_PortNr               (65535U)

/* ----------------- Channels/Channel/AdminQueueSize ------------------------ */
#define NWCF_NAME_AdminQueueSize      "AdminQueueSize"                 /* ulong */
#define NWCF_DEF_AdminQueueSize       (4000U)
#define NWCF_MIN_AdminQueueSize       (400U)

/* ----------------- Channels/Channel/GroupQueueSize ------------------------ */
#define NWCF_NAME_GroupQueueSize      "GroupQueueSize"                 /* ulong */
#define NWCF_DEF_GroupQueueSize       (2000U)
#define NWCF_MIN_GroupQueueSize       (100U)

/* -------------------- Channels/Channel/Resolution ------------------------- */
#define NWCF_NAME_Resolution          "Resolution"                    /* ushort */
#define NWCF_DEF_Resolution           (10U)
#define NWCF_MIN_Resolution           (1U)

/* -------------------- Channels/Channel/Multicast -------------------------- */
#define NWCF_ROOT_Multicast           "Multicast"
/* The enabled attribute and default value is defined under Channels/Channel  */

/* ---------------- Channels/Channel/Cipher ---------------------- */
#define NWCF_NAME_Cipher             "Cipher"                       /* string */
#define NWCF_DEF_Cipher              "disabled"

/* ---------------- Channels/Channel/CipherKey ---------------------- */
#define NWCF_NAME_CipherKey          "CipherKey"                   /* string */
#define NWCF_DEF_CipherKey           ""

/* ---------------- Channels/Channel/Multicast/Address ---------------------- */
#define NWCF_NAME_Address             "Address"                       /* string */
#define NWCF_DEF_Address              "230.0.0.1"

/* -------------- Channels/Channel/Multicast/TimeToLive --------------------- */
#define NWCF_NAME_TimeToLive          "TimeToLive"                     /* ulong */
#define NWCF_DEF_TimeToLive           (32U)
#define NWCF_MIN_TimeToLive           (1U)
#define NWCF_MAX_TimeToLive           (255U)

/* ----------------- Channels/Channel/Receiving|Sending/ReportInterval ------------------ */
#define NWCF_NAME_ReportInterval      "ReportInterval"                 /* ulong */
#define NWCF_DEF_ReportInterval       (100U)
#define NWCF_MIN_ReportInterval       (10U)

/* ----------------------- Channels/Channel/Receiving ------------------------------ */
#define NWCF_ROOT_Rx                  "Receiving"

/* ------------------ Channels/Channel/Receiving/Scheduling ------------------------ */
#define NWCF_ROOT_Scheduling         "Scheduling"

/* --------------- Channels/Channel/Receiving/Scheduling/Class --------------------- */
#define NWCF_NAME_Class               "Class"                         /* string */
#define NWCF_DEF_Class                "default"

/* --------------- Channels/Channel/Receiving/Scheduling/Priority ------------------ */
#define NWCF_NAME_Priority            "Priority"                        /* long */
#define NWCF_DEF_Priority             (0x8000000)
#define NWCF_ATTRIB_PriorityKind      "priority_kind"                 /* string */
#define NWCF_DEF_PriorityKind         "default"

/* ---------------- Channels/Channel/Receiving/PacketRetentionPeriod ------------------- */
#define NWCF_NAME_PacketRetentionPeriod   "PacketRetentionPeriod"              /* ulong */
#ifndef NWCF_DEF_PacketRetentionPeriod
#define NWCF_DEF_PacketRetentionPeriod    (0U)
#endif
/* ---------------- Channels/Channel/Receiving/ReliabilityRecoveryPeriod ------------------- */
#define NWCF_NAME_ReliabilityRecoveryPeriod   "ReliabilityRecoveryPeriod"              /* ulong */
#ifndef NWCF_DEF_ReliabilityRecoveryPeriod
#define NWCF_DEF_ReliabilityRecoveryPeriod    (1000U)
#endif


/* ---------------- Channels/Channel/Receiving/ReceiveBufferSize ------------------- */
#define NWCF_NAME_ReceiveBufferSize   "ReceiveBufferSize"              /* ulong */
#ifndef NWCF_DEF_ReceiveBufferSize
#define NWCF_DEF_ReceiveBufferSize    (1000000U)
#endif

/* ------------------ Channels/Channel/Receiving/DefragBufferSize ---------------------- */
#define NWCF_NAME_DefragBufferSize        "DefragBufferSize"                   /* ulong */
#define NWCF_DEF_DefragBufferSize         (100000U)
#define NWCF_DEF_DefragBufferSizeBestEffort (5000U)
#define NWCF_MIN_DefragBufferSize         (500U)

/* ----------------- Channels/Channel/Receiving/SMPOptimization ------------------ */
#define NWCF_ROOT_SMPOptimization     "SMPOptimization"

/* ------------------ Channels/Channel/Receiving/MaxReliabBacklog ---------------------- */
#define NWCF_NAME_MaxReliabBacklog        "MaxReliabBacklog"                   /* ulong */
#define NWCF_DEF_MaxReliabBacklog         (1000U)
#define NWCF_MIN_MaxReliabBacklog         (100U)

/* ------------------ Channels/Channel/Receiving/CrcCheck ---------------------- */
#define NWCF_NAME_CrcCheck			      "CrcCheck"                   /* ubool */
#define NWCF_DEF_CrcCheck         		  (FALSE)


/* ----------------------- Channels/Channel/Sending ------------------------------ */
#define NWCF_ROOT_Tx                  "Sending"

/* ------------------ Channels/Channel/Sending/Scheduling ------------------------ */
/* The scheduling root element is already defined under Channels/Channel/Receiving   */

/* ------------------ Channels/Channel/Receiving/FragmentSize ---------------------- */
#define NWCF_NAME_FragmentSize        "FragmentSize"                   /* ulong */
#define NWCF_DEF_FragmentSize         (1300U)
#define NWCF_MIN_FragmentSize         (200U)
#define NWCF_MAX_FragmentSize         (4294967295U)                    /* max int */

/* ------------------- Channels/Channel/Sending/MaxRetries ----------------------- */
#define NWCF_NAME_MaxRetries          "MaxRetries"                     /* ulong */
#define NWCF_DEF_MaxRetries           (100U)
#define NWCF_MIN_MaxRetries           (1U)

/* ------------------ Channels/Channel/Sending/MaxBurstSize ---------------------- */
#define NWCF_NAME_MaxBurstSize        "MaxBurstSize"                           /* ulong */
#define NWCF_DEF_MaxBurstSize         (0x3FFFFFFFU)               /* 1 GB/ResolutionTick*/
#define NWCF_MAX_MaxBurstSize         (0x3FFFFFFFU)               /* 1 GB/ResolutionTick*/
#define NWCF_MIN_MaxBurstSize         (1024U)

/* ------------------ Channels/Channel/Sending/ThrottleThreshold ---------------------- */
#define NWCF_NAME_ThrottleThreshold   "ThrottleThreshold"                   /* ulong */
#define NWCF_DEF_ThrottleThreshold    (50U)
#define NWCF_MIN_ThrottleThreshold    (2U)

/* ------------------ Channels/Channel/Sending/ThrottleLimit ---------------------- */
#define NWCF_NAME_ThrottleLimit        "ThrottleLimit"                   /* ulong */
#define NWCF_DEF_ThrottleLimit         (10240U) /* 10 KB/ResolutionTick */
#define NWCF_MIN_ThrottleLimit         (128U)

/* ----------------- Channels/Channel/Sending/RecoveryFactor --------------------- */
#define NWCF_NAME_RecoveryFactor      "RecoveryFactor"                 /* ulong */
#define NWCF_DEF_RecoveryFactor       (3U)
#define NWCF_MIN_RecoveryFactor       (2U)

/* ----------------- Channels/Channel/Sending/DontRoute --------------------- */
/* The default if TRUE because in older OpenSplice versions, there was no
 * configuration option and it was always set to true... */
#define NWCF_NAME_DontRoute           "DontRoute"                      /* bool */
#define NWCF_DEF_DontRoute            (TRUE)

/* ----------------- Channels/Channel/Sending/DontFrag ---------------------- */
#define NWCF_NAME_DontFragment        "DontFragment"                   /* bool */
#define NWCF_DEF_DontFragment         (FALSE)

/* ----------------- Channels/Channel/Sending/DiffServField ----------------- */
#define NWCF_NAME_DiffServField       "DiffServField"                  /* ulong */
#define NWCF_DEF_DiffServField        (0)

/* --------------------- Channels/Channel/Discovery ------------------------- */
#define NWCF_ROOT_DiscoveryChannel    "Discovery"
/* The enabled attribute and default value is defined under Channels/Channel  */

/* ------------- Channels/Channel/Discovery/Scope --------------------------- */
#define NWCF_ATTRIB_Scope            "Scope"                       /* string */
#define NWCF_DEF_Scope               ("")

/* ------------- Channels/Channel/Discovery/ProbeList --------------------------- */
#define NWCF_NAME_ProbeList         "ProbeList"                       /* string */
#define NWCF_DEF_Scope             ("")

/* ------------- Channels/Channel/Discovery/Sending/Interval ---------------- */
#define NWCF_NAME_Interval            "Interval"                       /* ulong */
#define NWCF_DEF_Interval             (333U)
#define NWCF_MIN_Interval             (100U)

/* ----------- Channels/Channel/Discovery/Sending/SafetyFactor -------------- */
#define NWCF_NAME_SafetyFactor        "SafetyFactor"                   /* float */
#define NWCF_DEF_SafetyFactor         (0.9F)
#define NWCF_MIN_SafetyFactor         (0.2F)
#define NWCF_MAX_SafetyFactor         (1.0F)

/* ------------- Channels/Channel/Discovery/Sending/SalvoSize --------------- */
#define NWCF_NAME_SalvoSize           "SalvoSize"                      /* ulong */
#define NWCF_DEF_SalvoSize            (3U)
#define NWCF_MIN_SalvoSize            (1U)

/* -------- Channels/Channel/Discovery/Receiving/DeathDetectionCount -------- */
#define NWCF_NAME_DeathDetectionCount "DeathDetectionCount"            /* ulong */
#define NWCF_DEF_DeathDetectionCount  (6U)
#define NWCF_MIN_DeathDetectionCount  (1U)


/* --------------------------- Partitioning --------------------------------- */
#define NWCF_ROOT_Partitioning        "Partitioning"

/* ------------------------- Partitioning/GlobalPartition ------------------- */
#define NWCF_ROOT_GlobalPartition      NWCF_SUBROOT(NWCF_ROOT_Partitioning, "GlobalPartition")
#define NWCF_DEF_GlobalAddress         "localhost"

/* ------------------------- Partitioning/Partitions ------------------------ */
#define NWCF_ROOT_NWPartitions         NWCF_SUBROOT(NWCF_ROOT_Partitioning, "NetworkPartitions")
#define NWCF_ROOT_NWPartition          NWCF_SUBROOT(NWCF_ROOT_NWPartitions, "NetworkPartition")
#define NWCF_ATTRIB_NWPartitionName    "Name"
#define NWCF_ATTRIB_NWPartitionAddress "Address"
#define NWCF_ATTRIB_Connected          "Connected"
#define NWCF_ATTRIB_Compression        "Compression"
#define NWCF_ATTRIB_NWSecurity         "Security"
#define NWCF_ATTRIB_NWSecurityPolicy   "SecurityProfile"
#define NWCF_ATTRIB_Authentication     "Authentication"
#define NWCF_ATTRIB_X509Authentication "X509Authentication"
#define NWCF_ATTRIB_Credentials        "Credentails"
#define NWCF_ATTRIB_AccessControl      "AccessControl"
#define NWCF_ATTRIB_MulticastTimeToLive "MulticastTimeToLive"
#define NWCF_DEF_NWPartitionAddress    ""
#define NWCF_DEF_Connected             (TRUE)
#define NWCF_DEF_Compression           (FALSE)
#define NWCF_DEF_NWSecurityPolicy      (NULL)
#define NWCF_DEF_MulticastTimeToLive   (32U)

/* ------------------------- Partitioning/PartitionMappings ----------------- */
#define NWCF_ROOT_PartitionMappings   NWCF_SUBROOT(NWCF_ROOT_Partitioning, "PartitionMappings")
#define NWCF_ROOT_PartitionMapping    NWCF_SUBROOT(NWCF_ROOT_PartitionMappings, "PartitionMapping")
#define NWCF_ATTRIB_DCPSPartitionTopic "DCPSPartitionTopic"
#define NWCF_ATTRIB_NetworkPartition  "NetworkPartition"
#define NWCF_ROOT_IgnoredPartitions   NWCF_SUBROOT(NWCF_ROOT_Partitioning, "IgnoredPartitions")
#define NWCF_ROOT_IgnoredPartition    NWCF_SUBROOT(NWCF_ROOT_IgnoredPartitions, "IgnoredPartition")
#define NWCF_ATTRIB_DCPSPartitionTopic "DCPSPartitionTopic"


/* ------------------------------ Tracing ----------------------------------- */
#define NWCF_ROOT_Tracing             "Tracing"
/* The enabled attribute and default value is defined under Channels/Channel  */

/* ------------------------- Tracing/OutputFile ----------------------------- */
#define NWCF_NAME_OutputFile           "OutputFile"                   /* string */
#define NWCF_DEF_OutputFile            "stdout"

/* ------------------------- Tracing/TimeStamps ----------------------------- */
#define NWCF_NAME_Timestamps           "Timestamps"                  /* boolean */
#define NWCF_DEF_Timestamps            (TRUE)
#define NWCF_ATTRIB_absolute           "absolute"                    /* boolean */
#define NWCF_DEF_absolute              (TRUE)

/* ------------------------- Tracing/Categories ----------------------------- */
#define NWCF_ROOT_Categories           NWCF_SUBROOT(NWCF_ROOT_Tracing, "Categories")

/* --------------------- Tracing/Categories/Default ------------------------- */
#define NWCF_NAME_Default              "Default"
#define NWCF_DEF_Default               0

/* ------------------ Tracing/Categories/Configuration ---------------------- */
#define NWCF_NAME_Configuration        "Configuration"
/* ------------------ Tracing/Categories/Construction ----------------------- */
#define NWCF_NAME_Construction         "Construction"
/* ------------------ Tracing/Categories/Destruction ------------------------ */
#define NWCF_NAME_Destruction          "Destruction"
/* -------------------- Tracing/Categories/Mainloop ------------------------- */
#define NWCF_NAME_Mainloop             "Mainloop"
/* --------------------- Tracing/Categories/Groups -------------------------- */
#define NWCF_NAME_Groups               "Groups"
/* ---------------------- Tracing/Categories/Send --------------------------- */
#define NWCF_NAME_Send                 "Send"
/* -------------------- Tracing/Categories/Receive -------------------------- */
#define NWCF_NAME_Receive              "Receive"
/* --------------------- Tracing/Categories/Test --------------------------- */
#define NWCF_NAME_Test                 "Test"
/* ------------------ Tracing/Categories/Receive/Test ----------------------- */
#define NWCF_NAME_DiscoveryTracing     "Discovery"

/* ------------------------------ Reporting --------------------------------- */
#define NWCF_ROOT_Reporting            "Reporting"
/* The enabled attribute and default value is defined under Channels/Channel  */

/* --------------------------- Reporting/Level ------------------------------ */
#define NWCF_NAME_Level                "Level"                        /* string */

/* --------------------------- Reporting/Events ----------------------------- */
#define NWCF_NAME_Events               "Events"                      /* boolean */

/*--------------------------- Reporting/Periodic ---------------------------- */
#define NWCF_NAME_Periodic             "Periodic"                    /* boolean */

/* --------------------------- Reporting/OneShot ---------------------------- */
#define NWCF_NAME_OneShot              "OneShot"                     /* boolean */



/* ------------------------------- WatchDog --------------------------------- */
#define NWCF_ROOT_WatchDog             "WatchDog"
/* structure Scheduling is defined under Channels/Channel/Receiving           */



/* ------------------- Profiling (For internal use only) -------------------- */
#define NWCF_ROOT_Profiling            "Profiling"

/* ------------------------- Profiling/ProfOutFile -------------------------- */
#define NWCF_NAME_ProfOutFile          "OutFile"                      /* string */
#define NWCF_DEF_ProfOutFile           "stdout"

/* ------------------------- Profiling/ProfDefault -------------------------- */
#define NWCF_ROOT_ProfDefault          NWCF_SUBROOT(NWCF_ROOT_Profiling, "ProfDefault")

/* ------------------- Profiling/ProfDefault/DoProfiling -------------------- */
#define NWCF_NAME_DoProfiling          "DoProfiling"                 /* boolean */
#define NWCF_DEF_DoProfiling           (FALSE)

/* ------------------- Profiling/ProfDefault/ReportLaps --------------------- */
#define NWCF_NAME_ReportLaps           "ReportLaps"                    /* ulong */
#define NWCF_DEF_ReportLaps            (100)

/* --------------------------- Profiling/Bridge ----------------------------- */
#define NWCF_ROOT_Bridge               NWCF_SUBROOT(NWCF_ROOT_Profiling, "Bridge")
/* The DoProfiling and ReportLaps elements are defined under Profiling/Default*/

/* ---------------------------- Profiling/Plug ------------------------------ */
#define NWCF_ROOT_Plug                 NWCF_SUBROOT(NWCF_ROOT_Profiling, "Plug")
/* The DoProfiling and ReportLaps elements are defined under Profiling/Default*/

/* ----------------------- Profiling/Fragmentation -------------------------- */
#define NWCF_ROOT_Fragmentation        NWCF_SUBROOT(NWCF_ROOT_Profiling, "Fragmentation")
/* The DoProfiling and ReportLaps elements are defined under Profiling/Default*/

/* ------------------------ Profiling/NetworkStack -------------------------- */
#define NWCF_ROOT_NetworkStack         NWCF_SUBROOT(NWCF_ROOT_Profiling, "NetworkStack")
/* The DoProfiling and ReportLaps elements are defined under Profiling/Default*/

/* ------------------------ Profiling/Serialization ------------------------- */
#define NWCF_ROOT_Serialization        NWCF_SUBROOT(NWCF_ROOT_Profiling, "Serialization")
/* The DoProfiling and ReportLaps elements are defined under Profiling/Default*/

/* DDS */
#define NWCF_ROOT_DDS         "OpenSplice"
/* --------------------------------- Domain --------------------------------- */
#define NWCF_ROOT_Domain              "Domain"
/* ------------------------------ Domain/Lease ------------------------------ */
#define NWCF_ROOT_Lease               NWCF_SUBROOT(NWCF_ROOT_Domain, "Lease")
/* ------------------------ Domain/Lease/ExpiryTime ------------------------- */
#define NWCF_ROOT_ExpiryTime          NWCF_SUBROOT(NWCF_ROOT_Lease, "ExpiryTime")
#define NWCF_NAME_ExpiryTime          "ExpiryTime"       /* float */
#define NWCF_DEF_ExpiryTime           (5.0F)

#define NWCF_ATTRIB_update_factor     "update_factor"    /* float */
#define NWCF_MIN_update_factor         (0.1F)
#define NWCF_MAX_update_factor         (1.0F)
#define NWCF_DEF_update_factor         (0.5F)

#define NWCF_ROOT_Debugging "Debugging"

#define NWCF_NAME_WaitForDebugger "WaitForDebugger" /* ulong */
#define NWCF_DEF_WaitForDebugger 0

#define NWCF_NAME_UseLoopback "UseLoopback" /* boolean */
#define NWCF_DEF_UseLoopback (FALSE)
#define NWCF_NAME_UseComplementPartitions "UseComplementPartitions" /* boolean */
#define NWCF_DEF_UseComplementPartitions (FALSE)

#define NWCF_ROOT_Lossy NWCF_SUBROOT(NWCF_ROOT_Debugging, "Lossy")
#define NWCF_ROOT_LossySending NWCF_SUBROOT(NWCF_ROOT_Lossy, "Sending")
#define NWCF_ROOT_LossyReceiving NWCF_SUBROOT(NWCF_ROOT_Lossy, "Receiving")
#define NWCF_NAME_BeLossy "BeLossy" /* boolean */
#define NWCF_DEF_BeLossy (FALSE)

#define NWCF_NAME_Threshold "Threshold" /* ulong */
#define NWCF_DEF_Threshold (5)
#define NWCF_MIN_Threshold (1)

#define NWCF_NAME_NoPacking "NoPacking" /* boolean */
#define NWCF_DEF_NoPacking (FALSE)

#define NWCF_NAME_Bool "Bool"
#define NWCF_DEF_Bool (FALSE)

#define NWCF_NAME_Long "Long"
#define NWCF_DEF_Long (1)

#define NWCF_NAME_ULong "ULong"
#define NWCF_DEF_ULong (0U)

#define NWCF_NAME_Float "Float"
#define NWCF_DEF_Float (3.14F)

#define NWCF_NAME_String "String"
#define NWCF_DEF_String "Hello world"

#endif /* NW_CONFIGURATIONDEFS_H */

