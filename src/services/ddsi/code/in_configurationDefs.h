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
#ifndef IN_CONFIGURATIONDEFS_H
#define IN_CONFIGURATIONDEFS_H

#define INCF_SEP "/"
#define INCF_ROOT(par)           INCF_ROOT_##par
#define INCF_SUBROOT(root, par)  root INCF_SEP par
#define INCF_NAME(par)           INCF_NAME_##par
#define INCF_ATTRIB(par)         INCF_ATTRIB_##par
#define INCF_DEF(par)            INCF_DEF_##par
#define INCF_MIN(par)            INCF_MIN_##par
#define INCF_MAX(par)            INCF_MAX_##par

#define INCF_SIMPLE_PARAM(type,root,par) \
    in_configurationGet##type##Parameter(root, INCF_NAME_##par, INCF_DEF_##par)

#define INCF_DEFAULTED_PARAM(type,root,par,def) \
    in_configurationGet##type##Parameter(root, INCF_NAME_##par, def)

#define INCF_SIMPLE_ATTRIB(type,root,attr) \
    in_configurationGet##type##Attribute(root, INCF_ATTRIB_##attr, INCF_DEF_##attr, INCF_DEF_##attr)

#define INCF_DEFAULTED_ATTRIB(type,root,attr,defNoElmt, defNoAttrib) \
    in_configurationGet##type##Attribute(root, INCF_ATTRIB_##attr, defNoElmt, defNoAttrib)

#define INCF_SIMPLE_SUBPARAM(type,root,sub,par) \
    in_configurationGet##type##Parameter(root, INCF_ROOT_##sub INCF_SEP INCF_NAME_##par, INCF_DEF_##par)

#define INCF_DEFAULTED_SUBPARAM(type,root,sub,par,def) \
    in_configurationGet##type##Parameter(root, INCF_ROOT_##sub INCF_SEP INCF_NAME_##par, def)

#define INCF_BROADCAST_EXPR       "broadcast"
#define INCF_FIRSTAVAILABLE_EXPR  "first available"


#define INCF_ROOT_NetworkingService     "DDSIService"
#define INCF_ROOT_Domain                "Domain"


/* ----------------------------- General /----------------------------------- */
#define INCF_ROOT_General               "General"

/* ------------------ Channels/NetworkInterfaceAddress ---------------------- */
#define INCF_NAME_Interface             "NetworkInterfaceAddress"     /* string */
#define INCF_DEF_Interface              INCF_FIRSTAVAILABLE_EXPR

/* ----------------------------- Channels ----------------------------------- */
#define INCF_ROOT_Channels              "Channels"
/* ------------------------- Channels/Channel ------------------------------- */
#define INCF_ROOT_Channel              INCF_SUBROOT(INCF_ROOT_Channels, "Channel")
#define INCF_ATTRIB_ChannelName        "name"
#define INCF_ATTRIB_priority           "priority"                      /* ulong */
#define INCF_DEF_priority              (0)
#define INCF_ATTRIB_reliable           "reliable"                    /* boolean */
#define INCF_DEF_reliable              (FALSE)
#define INCF_ATTRIB_default            "default"                     /* boolean */
#define INCF_DEF_default               (FALSE)
#define INCF_ATTRIB_enabled            "enabled"                     /* boolean */
#define INCF_DEF_enabled               (TRUE)

/* -------------------- Channels/Channel/QueueSize -------------------------- */
#define INCF_NAME_QueueSize           "QueueSize"                      /* ulong */
#define INCF_DEF_QueueSize            (4000U)
#define INCF_MIN_QueueSize            (1U)

/* --------------------- Channels/Channel/PortNr ---------------------------- */
#define INCF_NAME_PortNr              "PortNr"                        /* ushort */
#define INCF_DEF_PortNr               (3367U)
#define INCF_MIN_PortNr               (1U)
#define INCF_MAX_PortNr               (65535U)

/* ----------------- Channels/Channel/AdminQueueSize ------------------------ */
#define INCF_NAME_AdminQueueSize      "AdminQueueSize"                 /* ulong */
#define INCF_DEF_AdminQueueSize       (4000U)
#define INCF_MIN_AdminQueueSize       (400U)

/* ----------------- Channels/Channel/GroupQueueSize ------------------------ */
#define INCF_NAME_GroupQueueSize      "GroupQueueSize"                 /* ulong */
#define INCF_DEF_GroupQueueSize       (2000U)
#define INCF_MIN_GroupQueueSize       (100U)

/* -------------------- Channels/Channel/Resolution ------------------------- */
#define INCF_NAME_Resolution          "Resolution"                    /* ushort */
#define INCF_DEF_Resolution           (10U)
#define INCF_MIN_Resolution           (1U)

/* -------------------- Channels/Channel/Multicast -------------------------- */
#define INCF_ROOT_Multicast           "Multicast"
/* The enabled attribute and default value is defined under Channels/Channel  */

/* ---------------- Channels/Channel/Multicast/Address ---------------------- */
#define INCF_NAME_Address             "Address"                       /* string */
#define INCF_DEF_Address              "239.255.0.1"  /* Default Address specificied by DDSi */

/* -------------- Channels/Channel/Multicast/TimeToLive --------------------- */
#define INCF_NAME_TimeToLive          "TimeToLive"                     /* ulong */
#define INCF_DEF_TimeToLive           (32U)
#define INCF_MIN_TimeToLive           (1U)
#define INCF_MAX_TimeToLive           (255U)

/* ----------------- Channels/Channel/Receiving|Sending/ReportInterval ------------------ */
#define INCF_NAME_ReportInterval      "ReportInterval"                 /* ulong */
#define INCF_DEF_ReportInterval       (100U)
#define INCF_MIN_ReportInterval       (10U)

/* ----------------------- Channels/Channel/Receiving ------------------------------ */
#define INCF_ROOT_Rx                  "Receiving"

/* ------------------ Channels/Channel/Receiving/Scheduling ------------------------ */
#define INCF_ROOT_Scheduling         "Scheduling"

/* --------------- Channels/Channel/Receiving/Scheduling/Class --------------------- */
#define INCF_NAME_Class               "Class"                         /* string */
#define INCF_DEF_Class                "default"

/* --------------- Channels/Channel/Receiving/Scheduling/Priority ------------------ */
#define INCF_NAME_Priority            "Priority"                        /* long */
#define INCF_DEF_Priority             (0x8000000)
#define INCF_ATTRIB_PriorityKind      "priority_kind"                 /* string */
#define INCF_DEF_PriorityKind         "default"

/* ---------------- Channels/Channel/Receiving/ReceiveBufferSize ------------------- */
#define INCF_NAME_ReceiveBufferSize   "ReceiveBufferSize"              /* ulong */
#define INCF_DEF_ReceiveBufferSize    (1000000U)

/* ------------------ Channels/Channel/Receiving/DefragBufferSize ---------------------- */
#define INCF_NAME_DefragBufferSize        "DefragBufferSize"                   /* ulong */
#define INCF_DEF_DefragBufferSize         (100000U)
#define INCF_MIN_DefragBufferSize         (500U)

/* ----------------- Channels/Channel/Receiving/SMPOptimization ------------------ */
#define INCF_ROOT_SMPOptimization     "SMPOptimization"

/* ----------------------- Channels/Channel/Sending ------------------------------ */
#define INCF_ROOT_Tx                  "Sending"

/* ------------------ Channels/Channel/Sending/Scheduling ------------------------ */
/* The scheduling root element is already defined under Channels/Channel/Receiving   */

/* ------------------ Channels/Channel/Receiving/FragmentSize ---------------------- */
#define INCF_NAME_FragmentSize        "FragmentSize"                   /* ulong */
#define INCF_DEF_FragmentSize         (1300U)
#define INCF_MIN_FragmentSize         (200U)

/* ------------------- Channels/Channel/Sending/MaxRetries ----------------------- */
#define INCF_NAME_MaxRetries          "MaxRetries"                     /* ulong */
#define INCF_DEF_MaxRetries           (100U)
#define INCF_MIN_MaxRetries           (1U)

/* ------------------ Channels/Channel/Sending/MaxBurstSize ---------------------- */
#define INCF_NAME_MaxBurstSize        "MaxBurstSize"                   /* ulong */
#define INCF_DEF_MaxBurstSize         (200000U) /* 200 KB/ResolutionTick */
#define INCF_MIN_MaxBurstSize         (1024U)

/* ------------------ Channels/Channel/Sending/ThrottleThreshold ---------------------- */
#define INCF_NAME_ThrottleThreshold   "ThrottleThreshold"                   /* ulong */
#define INCF_DEF_ThrottleThreshold    (50U)
#define INCF_MIN_ThrottleThreshold    (2U)

/* ------------------ Channels/Channel/Sending/ThrottleLimit ---------------------- */
#define INCF_NAME_ThrottleLimit        "ThrottleLimit"                   /* ulong */
#define INCF_DEF_ThrottleLimit         (10240U) /* 10 KB/ResolutionTick */
#define INCF_MIN_ThrottleLimit         (1024U)

/* ----------------- Channels/Channel/Sending/RecoveryFactor --------------------- */
#define INCF_NAME_RecoveryFactor      "RecoveryFactor"                 /* ulong */
#define INCF_DEF_RecoveryFactor       (3U)
#define INCF_MIN_RecoveryFactor       (2U)

/* ----------------- Channels/Channel/Sending/DiffServField ----------------- */
#define INCF_NAME_DiffServField       "DiffServField"                  /* ulong */
#define INCF_DEF_DiffServField        (0)

/* --------------------- Channels/Channel/Discovery ------------------------- */
#define INCF_ROOT_DiscoveryChannel    "Discovery"
/* The enabled attribute and default value is defined under Channels/Channel  */

/* ------------- Channels/Channel/Discovery/Sending/Interval ---------------- */
#define INCF_NAME_Interval            "Interval"                       /* ulong */
#define INCF_DEF_Interval             (1000U)
#define INCF_MIN_Interval             (100U)

/* ----------- Channels/Channel/Discovery/Sending/SafetyFactor -------------- */
#define INCF_NAME_SafetyFactor        "SafetyFactor"                   /* float */
#define INCF_DEF_SafetyFactor         (0.9F)
#define INCF_MIN_SafetyFactor         (0.2F)
#define INCF_MAX_SafetyFactor         (1.0F)

/* ------------- Channels/Channel/Discovery/Sending/SalvoSize --------------- */
#define INCF_NAME_SalvoSize           "SalvoSize"                      /* ulong */
#define INCF_DEF_SalvoSize            (3U)
#define INCF_MIN_SalvoSize            (1U)

/* -------- Channels/Channel/Discovery/Receiving/DeathDetectionCount -------- */
#define INCF_NAME_DeathDetectionCount "DeathDetectionCount"            /* ulong */
#define INCF_DEF_DeathDetectionCount  (6U)
#define INCF_MIN_DeathDetectionCount  (1U)


/* --------------------------- Partitioning --------------------------------- */
#define INCF_ROOT_Partitioning        "Partitioning"

/* ------------------------- Partitioning/GlobalPartition ------------------- */
#define INCF_ROOT_GlobalPartition      INCF_SUBROOT(INCF_ROOT_Partitioning, "GlobalPartition")
#define INCF_DEF_GlobalAddress         INCF_BROADCAST_EXPR

/* ------------------------- Partitioning/Partitions ------------------------ */
#define INCF_ROOT_INPartitions         INCF_SUBROOT(INCF_ROOT_Partitioning, "NetworkPartitions")
#define INCF_ROOT_INPartition          INCF_SUBROOT(INCF_ROOT_INPartitions, "NetworkPartition")
#define INCF_ATTRIB_INPartitionName    "Name"
#define INCF_ATTRIB_INPartitionAddress "Address"
#define INCF_ATTRIB_Connected          "Connected"
#define INCF_DEF_INPartitionAddress    ""
#define INCF_DEF_Connected             (TRUE)

/* ------------------------- Partitioning/PartitionMappings ----------------- */
#define INCF_ROOT_PartitionMappings   INCF_SUBROOT(INCF_ROOT_Partitioning, "PartitionMappings")
#define INCF_ROOT_PartitionMapping    INCF_SUBROOT(INCF_ROOT_PartitionMappings, "PartitionMapping")
#define INCF_ATTRIB_DCPSPartitionTopic "DCPSPartitionTopic"
#define INCF_ATTRIB_NetworkPartition  "NetworkPartition"


/* ------------------------------ Tracing ----------------------------------- */
#define INCF_ROOT_Tracing             "Tracing"
/* The enabled attribute and default value is defined under Channels/Channel  */

/* ------------------------- Tracing/OutputFile ----------------------------- */
#define INCF_NAME_OutputFile           "OutputFile"                   /* string */
#define INCF_DEF_OutputFile            "stdout"

/* ------------------------- Tracing/TimeStamps ----------------------------- */
#define INCF_NAME_Timestamps           "Timestamps"                  /* boolean */
#define INCF_DEF_Timestamps            (TRUE)
#define INCF_ATTRIB_absolute           "absolute"                    /* boolean */
#define INCF_DEF_absolute              (TRUE)

/* ------------------------- Tracing/Categories ----------------------------- */
#define INCF_ROOT_Categories           INCF_SUBROOT(INCF_ROOT_Tracing, "Categories")

/* --------------------- Tracing/Categories/Default ------------------------- */
#define INCF_NAME_Default              "Default"
#define INCF_DEF_Default               0

/* ------------------ Tracing/Categories/Configuration ---------------------- */
#define INCF_NAME_Configuration        "Configuration"
/* ------------------ Tracing/Categories/Construction ----------------------- */
#define INCF_NAME_Construction         "Construction"
/* ------------------ Tracing/Categories/Destruction ------------------------ */
#define INCF_NAME_Destruction          "Destruction"
/* -------------------- Tracing/Categories/Mainloop ------------------------- */
#define INCF_NAME_Mainloop             "Mainloop"
/* --------------------- Tracing/Categories/Groups -------------------------- */
#define INCF_NAME_Groups               "Groups"
/* ---------------------- Tracing/Categories/Send --------------------------- */
#define INCF_NAME_Send                 "Send"
/* -------------------- Tracing/Categories/Receive -------------------------- */
#define INCF_NAME_Receive              "Receive"
/* --------------------- Tracing/Categories/Test --------------------------- */
#define INCF_NAME_Test                 "Test"
/* ------------------ Tracing/Categories/Receive/Test ----------------------- */
#define INCF_NAME_DiscoveryTracing     "Discovery"

/* ------------------------------ Reporting --------------------------------- */
#define INCF_ROOT_Reporting            "Reporting"
/* The enabled attribute and default value is defined under Channels/Channel  */

/* --------------------------- Reporting/Level ------------------------------ */
#define INCF_NAME_Level                "Level"                        /* string */

/* --------------------------- Reporting/Events ----------------------------- */
#define INCF_NAME_Events               "Events"                      /* boolean */

/*--------------------------- Reporting/Periodic ---------------------------- */
#define INCF_NAME_Periodic             "Periodic"                    /* boolean */

/* --------------------------- Reporting/OneShot ---------------------------- */
#define INCF_NAME_OneShot              "OneShot"                     /* boolean */



/* ------------------------------- WatchDog --------------------------------- */
#define INCF_ROOT_WatchDog             "WatchDog"
/* structure Scheduling is defined under Channels/Channel/Receiving           */



/* ------------------- Profiling (For internal use only) -------------------- */
#define INCF_ROOT_Profiling            "Profiling"

/* ------------------------- Profiling/ProfOutFile -------------------------- */
#define INCF_NAME_ProfOutFile          "OutFile"                      /* string */
#define INCF_DEF_ProfOutFile           "stdout"

/* ------------------------- Profiling/ProfDefault -------------------------- */
#define INCF_ROOT_ProfDefault          INCF_SUBROOT(INCF_ROOT_Profiling, "ProfDefault")

/* ------------------- Profiling/ProfDefault/DoProfiling -------------------- */
#define INCF_NAME_DoProfiling          "DoProfiling"                 /* boolean */
#define INCF_DEF_DoProfiling           (FALSE)

/* ------------------- Profiling/ProfDefault/ReportLaps --------------------- */
#define INCF_NAME_ReportLaps           "ReportLaps"                    /* ulong */
#define INCF_DEF_ReportLaps            (100)

/* --------------------------- Profiling/Bridge ----------------------------- */
#define INCF_ROOT_Bridge               INCF_SUBROOT(INCF_ROOT_Profiling, "Bridge")
/* The DoProfiling and ReportLaps elements are defined under Profiling/Default*/

/* ---------------------------- Profiling/Plug ------------------------------ */
#define INCF_ROOT_Plug                 INCF_SUBROOT(INCF_ROOT_Profiling, "Plug")
/* The DoProfiling and ReportLaps elements are defined under Profiling/Default*/

/* ----------------------- Profiling/Fragmentation -------------------------- */
#define INCF_ROOT_Fragmentation        INCF_SUBROOT(INCF_ROOT_Profiling, "Fragmentation")
/* The DoProfiling and ReportLaps elements are defined under Profiling/Default*/

/* ------------------------ Profiling/NetworkStack -------------------------- */
#define INCF_ROOT_NetworkStack         INCF_SUBROOT(INCF_ROOT_Profiling, "NetworkStack")
/* The DoProfiling and ReportLaps elements are defined under Profiling/Default*/

/* ------------------------ Profiling/Serialization ------------------------- */
#define INCF_ROOT_Serialization        INCF_SUBROOT(INCF_ROOT_Profiling, "Serialization")
/* The DoProfiling and ReportLaps elements are defined under Profiling/Default*/

/* DDS */
#define INCF_ROOT_DDS         "OpenSplice"
/* --------------------------------- Domain --------------------------------- */
#define INCF_ROOT_Domain              "Domain"
/* ------------------------------ Domain/Lease ------------------------------ */
#define INCF_ROOT_Lease               INCF_SUBROOT(INCF_ROOT_Domain, "Lease")
/* ------------------------ Domain/Lease/ExpiryTime ------------------------- */
#define INCF_ROOT_ExpiryTime          INCF_SUBROOT(INCF_ROOT_Lease, "ExpiryTime")
#define INCF_NAME_ExpiryTime          "ExpiryTime"       /* float */
#define INCF_DEF_ExpiryTime           (5.0F)

#define INCF_ATTRIB_update_factor     "update_factor"    /* float */
#define INCF_MIN_update_factor         (0.1F)
#define INCF_MAX_update_factor         (1.0F)
#define INCF_DEF_update_factor         (0.5F)

#define INCF_ROOT_Debugging "Debugging"

#define INCF_NAME_WaitForDebugger "WaitForDebugger" /* ulong */
#define INCF_DEF_WaitForDebugger 0

#define INCF_NAME_UseLoopback "UseLoopback" /* boolean */
#define INCF_DEF_UseLoopback (FALSE)
#define INCF_NAME_UseComplementPartitions "UseComplementPartitions" /* boolean */
#define INCF_DEF_UseComplementPartitions (FALSE)

#define INCF_ROOT_Lossy INCF_SUBROOT(INCF_ROOT_Debugging, "Lossy")
#define INCF_ROOT_LossySending INCF_SUBROOT(INCF_ROOT_Lossy, "Sending")
#define INCF_ROOT_LossyReceiving INCF_SUBROOT(INCF_ROOT_Lossy, "Receiving")
#define INCF_NAME_BeLossy "BeLossy" /* boolean */
#define INCF_DEF_BeLossy (FALSE)

#define INCF_NAME_Threshold "Threshold" /* ulong */
#define INCF_DEF_Threshold (5)
#define INCF_MIN_Threshold (1)

#define INCF_NAME_NoPacking "NoPacking" /* boolean */
#define INCF_DEF_NoPacking (FALSE)

#define INCF_NAME_Bool "Bool"
#define INCF_DEF_Bool (FALSE)

#define INCF_NAME_Long "Long"
#define INCF_DEF_Long (1)

#define INCF_NAME_ULong "ULong"
#define INCF_DEF_ULong (0U)

#define INCF_NAME_Float "Float"
#define INCF_DEF_Float (3.14F)

#define INCF_NAME_String "String"
#define INCF_DEF_String "Hello world"

#endif
