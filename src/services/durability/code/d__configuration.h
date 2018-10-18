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

#ifndef D__CONFIGURATION_H
#define D__CONFIGURATION_H

#include "d__types.h"
#include "u_user.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_configuration validity.
 * Because d_configuration is a concrete class typechecking is required.
 */
#define             d_configurationIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_CONFIGURATION)

/**
 * \brief The d_configuration cast macro.
 *
 * This macro casts an object to a d_configuration object.
 */
#define d_configuration(_this) ((d_configuration)(_this))

/* Configuration defaults */
#define D_MINIMUM_LIVELINESS_EXPIRY_TIME            ((float) 0.2e0)
#define D_DEFAULT_LIVELINESS_EXPIRY_TIME            ((float)10.0e0)
#define D_MAXIMUM_LIVELINESS_EXPIRY_TIME            ((float)2147483647.2147483647)

#define D_MINIMUM_LIVELINESS_UPDATE_FACTOR          ((float)0.01e0)
#define D_DEFAULT_LIVELINESS_UPDATE_FACTOR          ((float)0.2e0)
#define D_MAXIMUM_LIVELINESS_UPDATE_FACTOR          ((float)1.0e0)

#define D_MINIMUM_HEARTBEAT_UPDATE_FACTOR           ((float)0.1e0)
#define D_DEFAULT_HEARTBEAT_UPDATE_FACTOR           ((float)0.25e0)
#define D_MAXIMUM_HEARTBEAT_UPDATE_FACTOR           ((float)0.9e0)

#define D_MINIMUM_HEARTBEAT_EXPIRY_TIME             ((float)0.2e0)
#define D_DEFAULT_HEARTBEAT_EXPIRY_TIME             ((float)4.0e0)
#define D_MAXIMUM_HEARTBEAT_EXPIRY_TIME             ((float)2147483647.2147483647)

#define D_MINIMUM_TIMING_INITIAL_WAIT_PERIOD        ((float)0.10e0)
#define D_DEFAULT_TIMING_INITIAL_WAIT_PERIOD        ((float)3.000e0)
#define D_MAXIMUM_TIMING_INITIAL_WAIT_PERIOD        ((float)60.000e0)

#define D_MINIMUM_INITIAL_REQUEST_COMBINE_PERIOD    ((float)0.01e0)
#define D_DEFAULT_INITIAL_REQUEST_COMBINE_PERIOD    ((float)0.5e0)
#define D_MAXIMUM_INITIAL_REQUEST_COMBINE_PERIOD    ((float)5.0e0)

#define D_MINIMUM_OPERATIONAL_REQUEST_COMBINE_PERIOD ((float)0.01e0)
#define D_DEFAULT_OPERATIONAL_REQUEST_COMBINE_PERIOD ((float)0.01e0)
#define D_MAXIMUM_OPERATIONAL_REQUEST_COMBINE_PERIOD ((float)5.0e0)

#define D_MINIMUM_LATENCY_BUDGET                    ((float)0.0e0)
#define D_DEFAULT_LATENCY_BUDGET                    ((float)0.0e0)
#define D_MAXIMUM_LATENCY_BUDGET                    ((float)2147483647.2147483647)

#define D_MINIMUM_TRANSPORT_PRIORITY                (0)
#define D_DEFAULT_TRANSPORT_PRIORITY                (0)
#define D_MAXIMUM_TRANSPORT_PRIORITY                (2147483647)

#define D_MINIMUM_NETWORK_MAX_WAITCOUNT             (1)
#define D_DEFAULT_NETWORK_MAX_WAITCOUNT             (200)
#define D_MAXIMUM_NETWORK_MAX_WAITCOUNT             (1000)

#define D_PUBLISHER_NAME                            "durabilityPublisher"
#define D_SUBSCRIBER_NAME                           "durabilitySubscriber"
#define D_PARTITION_NAME                            "durabilityPartition"
#define D_CLIENT_DURABILITY_PARTITION_NAME          D_PARTITION_NAME
#define D_CLIENT_DURABILITY_PUBLISHER_NAME          "clientDurabilityPublisher"
#define D_CLIENT_DURABILITY_SUBSCRIBER_NAME         "clientDurabilitySubscriber"
#define D_DURABILITY_BUILTIN_SUBSCRIBER_NAME        "durabilityBuiltinSubscriber"

#define D_DEFAULT_TRACING_OUTFILE                   "durability.log"

#define D_MINIMUM_PERSISTENT_STORE_SLEEP_TIME       ((float)0.0e0)
#define D_DEFAULT_PERSISTENT_STORE_SLEEP_TIME       ((float)0.0e0)
#define D_MAXIMUM_PERSISTENT_STORE_SLEEP_TIME       ((float)10.0e0)

#define D_MINIMUM_PERSISTENT_STORE_SESSION_TIME     ((float)0.001e0)
#define D_DEFAULT_PERSISTENT_STORE_SESSION_TIME     ((float)20.0e0)
#define D_MAXIMUM_PERSISTENT_STORE_SESSION_TIME     ((float)60.0e0)

#define D_MINIMUM_PERSISTENT_QUEUE_SIZE             (0)
#define D_DEFAULT_PERSISTENT_QUEUE_SIZE             (0)
#define D_MAXIMUM_PERSISTENT_QUEUE_SIZE             (10000)

#define D_DEFAULT_PERSISTENT_KV_COMPRESSION_ENABLED (FALSE)
#define D_DEFAULT_PERSISTENT_KV_COMPRESSION_ALGORITHM "lzf"

#define D_MINIMUM_OPTIMIZE_INTERVAL                 (10)
#define D_DEFAULT_OPTIMIZE_INTERVAL                 (0)
#define D_MAXIMUM_OPTIMIZE_INTERVAL                 (1000000000)

#define D_MINIMUM_TIME_TO_WAIT_FOR_ALIGNER          ((float)0.0e0)
#define D_DEFAULT_TIME_TO_WAIT_FOR_ALIGNER          ((float)1.0e0)
#define D_MAXIMUM_TIME_TO_WAIT_FOR_ALIGNER          ((float)1.0e0)

#define D_MINIMUM_SERVICE_TERMINATE_PERIOD          ((float)0.0e0)
#define D_DEFAULT_SERVICE_TERMINATE_PERIOD          ((float)10.0e0)
#define D_MAXIMUM_SERVICE_TERMINATE_PERIOD          ((float)60.0e0)

#define D_DEFAULT_TERMINATE_REMOVAL_PERIOD          ((float)5.0e0)

#define D_DEFAULT_ROLE                              "DefaultRole"

#define D_DEFAULT_EQUALITY_CHECK                    (FALSE)

#define D_MINIMUM_MASTER_PRIORITY                   (0)
#define D_DEFAULT_MASTER_PRIORITY                   (255)
#define D_MAXIMUM_MASTER_PRIORITY                   (255)

#define D_MINIMUM_MASTER_ELECTION_WAIT_TIME         ((float)0.0e0)
#define D_DEFAULT_MASTER_ELECTION_WAIT_TIME         ((float)0.0e0)
#define D_MAXIMUM_MASTER_ELECTION_WAIT_TIME         ((float)10.0e0)

#define IS_LEGACY_MASTER_SELECTION(priority)        ((priority == D_MAXIMUM_MASTER_PRIORITY))

#define D_MINIMUM_MAJORITY_VOTING_THRESHOLD         (5)
#define D_DEFAULT_MAJORITY_VOTING_THRESHOLD         (D_MINIMUM_MAJORITY_VOTING_THRESHOLD)
#define D_MAXIMUM_MAJORITY_VOTING_THRESHOLD         (50)

#define D_DEFAULT_DURABLE_POLICY_CACHE              (TRUE)

#define D_DEFAULT_THREAD_LIVELINESS_REPORT_PERIOD   ((float)0.0e0)

/* Master selection algorithm */
#define D_USE_LEGACY_ALGORITHM                      (0)
#define D_USE_MASTER_PRIORITY_ALGORITHM             (1)

/* Mask settings for additional tracing */
#define D_TRACE_PRIO_QUEUE         (0x0001U << 0) /*  1 */
#define D_TRACE_GROUP              (0x0001U << 1) /*  2 */
#define D_TRACE_CLIENT_DURABILITY  (0x0001U << 2) /*  4 */
#define D_TRACE_GROUP_HASH         (0x0001U << 3) /*  8 */
#define D_TRACE_MASTER_SELECTION   (0x0001U << 4) /* 16 */
#define D_TRACE_COMBINE_REQUESTS   (0x0001U << 5) /* 32 */
#define D_TRACE_CHAINS             (0x0001U << 6) /* 64 */
#define D_TRACE_NONE               (0U)
#define D_TRACE_ALL                (~0U)


C_STRUCT(d_configuration){
    C_EXTENDS(d_object);
    c_bool        builtinTopicsEnabled;
    os_duration   livelinessExpiryTime;      /* ExpiryTime (duration */
    c_float       livelinessExpiry;          /* ExpiryTime (float) */
    os_duration   livelinessUpdateInterval;  /* ExpiryTime * update_factor */
    os_threadAttr livelinessScheduling;      /* Not implemented yet.*/
    os_duration   heartbeatUpdateInterval;
    os_duration   heartbeatExpiryTime;
    c_float       heartbeatExpiry;
    os_threadAttr heartbeatScheduling;
    os_duration   timingInitialWaitPeriod;
    c_ulong       networkMaxWaitCount;
    os_duration   networkMaxWaitTime;
    d_table       networkServiceNames;
    c_iter        services;
    c_char*       publisherName;
    c_char*       subscriberName;
    c_char*       partitionName;
    FILE*         tracingOutputFile;
    c_char*       tracingOutputFileName;
    c_bool        tracingSynchronous;
    c_bool        tracingTimestamps;
    c_bool        tracingRelativeTimestamps;
    d_level       tracingVerbosityLevel;
    c_char*       persistentStoreDirectory;
    d_storeType   persistentStoreMode;
    c_ulong       persistentStoreStackSize;
    c_char*       persistentKVStoreStorageType;
    c_char*       persistentKVStoreStorageParameters;
    c_bool        persistentKVStoreCompressionEnabled;
    d_compressionKV persistentKVStoreCompressionAlgorithm;
    os_duration   persistentStoreSleepTime;
    os_duration   persistentStoreSessionTime;
    c_ulong       persistentQueueSize;
    os_threadAttr persistentScheduling;
    c_iter        nameSpaces;
    c_iter        policies;
    c_iter        filters;
    c_iter        durablePolicies;
    os_timeW      startWTime;
    os_timeM      startMTime;
    os_timeE      startETime;
    c_ulong       persistentUpdateInterval;
    os_duration   latencyBudget;
    c_long        transportPriority;
    os_duration   heartbeatLatencyBudget;
    c_long        heartbeatTransportPriority;
    os_duration   alignerLatencyBudget;
    c_long        alignerTransportPriority;
    os_threadAttr alignerScheduling;
    os_threadAttr aligneeScheduling;
    os_duration   initialRequestCombinePeriod;
    os_duration   operationalRequestCombinePeriod;
    c_bool        timeAlignment;
    d_name        role;
    os_duration   timeToWaitForAligner;
    c_bool        mustAlignBuiltinTopics;           /* if TRUE durability will align builtin topics */
    c_bool        waitForRemoteReaders;             /* if TRUE durability will send its
                                                     * nameSpaces to a fellow only if
                                                     * ALL durability readers of the
                                                     * fellow have been discovered.
                                                     * The same holds for response to client durability requests.
                                                     */
    c_float       serviceTerminate;                 /* Amount of time the domain should wait for other services to terminate */
    os_duration   serviceTerminatePeriod;
    os_duration   terminateRemovalPeriod;           /* Period to ignore messages from the fellow after it has terminated */
    c_bool        clientDurabilityEnabled;          /* if TRUE durability is able to provide historical data when asked for by a client durability service */
    c_char*       clientDurabilityPartitionName;    /* client durability partition name */
    c_char*       clientDurabilityPublisherName;    /* client durability publisher name */
    c_char*       clientDurabilitySubscriberName;   /* client durability subscriber name */
    c_bool        deadlockDetection;
    c_bool        capabilitySupport;                /* if TRUE the capability interface is supported; default is TRUE.
                                                     * Capability support can be disabled by setting environment var
                                                     * OSPL_DURABILITY_CAPABILITY_SUPPORT to 0
                                                     */
    c_bool        capabilityGroupHash;              /* if TRUE the ability to calculate a hash of a data set for a group is enabled */
    c_bool        capabilityEOTSupport;             /* if TRUE the durability service support the ability to align EOT messages */
    c_bool        capabilityY2038Ready;             /* if TRUE the durability service can advertise times beyond 2038 */
    c_ulong       traceMask;                        /* mask to indicate what traces to log, only effective when verbosity level is set to D_LEVEL_FINEST */

    os_uint32     seqnum_initval;                   /* initial sequence number value (0 by default, can be changed using environment variable OSPL_DURABILITY_SEQNUM_INITIAL_VALUE */
    c_ulong       capabilityMasterSelection;        /* Indicator for master selection algorithm to use */

    c_bool        masterSelectionLegacy;            /* Use legacy mode */
    os_duration   masterElectionWaitTime;           /* Time to wait before selecting a master, defaults to 0 */
    c_ulong       majorityVotingThreshold;          /* Number of rounds before ending up in majority voting, defaults to D_MINIMUM_MAJORITY_VOTING_THRESHOLD */
    c_ulong       initialMergeStateValue;
    os_duration   threadLivelinessReportPeriod;     /* Period to report thread liveliness, 0 when infinite */
};


struct durablePolicy {
    char *obtain;
    c_bool cache;
};


/**
 * Allocates a new configuration.
 * The configuration is read from the user layer service of the supplied
 * d_durability object.
 *
 * @param service The object to resolve the user layer service from. This
 *                contains the configuration that was read when creating it.
 * @param serviceName The name of the service. This is used to locate the
 *                    correct configuration entity in the configuration file.
 *                    The 'name' attribute of the durability service in the
 *                    configuration must match this parameter.
 * @return The read configuration. The configuration items that were not in
 *         the configuration file have the default value.
 */
d_configuration d_configurationNew                              (d_durability service,
                                                                 const c_char* serviceName,
                                                                 c_long domainId);

/**
 * Frees the supplied configuration.
 *
 * @param configuration The configuration to free.
 */
void            d_configurationFree                             (d_configuration config);

c_bool          d_configurationInNameSpace                      (d_nameSpace ns,
                                                                 d_partition partition,
                                                                 d_topic topic,
                                                                 c_bool aligner);

int            d_configurationInit                              (d_configuration config,
                                                                 u_cfElement cfg,
                                                                 d_durability durability,
                                                                 u_cfElement domainElement,
                                                                 u_cfElement element);

void            d_configurationDeinit                           (d_configuration config);

void            d_configurationSetLivelinessExpiryTime          (d_configuration config,
                                                                 c_float seconds);

void            d_configurationSetLivelinessUpdateFactor        (d_configuration config,
                                                                 u_cfElement element,
                                                                 const c_char* expiryTimePath,
                                                                 const c_char* updateFactorName);

void            d_configurationSetLivelinessSchedulingClass     (d_configuration config,
                                                                 const c_char* class);

void            d_configurationSetLivelinessSchedulingPriority  (d_configuration config,
                                                                 c_long priority);

void            d_configurationSetPersistentSchedulingClass     (d_configuration config,
                                                                 const c_char* class);

void            d_configurationSetPersistentSchedulingPriority  (d_configuration config,
                                                                 c_long priority);

void            d_configurationSetHeartbeatSchedulingClass      (d_configuration config,
                                                                 const c_char* class);

void            d_configurationSetHeartbeatSchedulingPriority   (d_configuration config,
                                                                 c_long priority);

void            d_configurationSetAlignerSchedulingClass        (d_configuration config,
                                                                 const c_char* class);

void            d_configurationSetAlignerSchedulingPriority     (d_configuration config,
                                                                 c_long priority);

void            d_configurationSetPersistentSchedulingStackSize (d_configuration config,
                                                                 c_ulong stackSize);

void            d_configurationSetAligneeSchedulingClass        (d_configuration config,
                                                                 const c_char* class);

void            d_configurationSetAligneeSchedulingPriority     (d_configuration config,
                                                                 c_long priority);

void            d_configurationSetHeartbeatExpiryTime           (d_configuration config,
                                                                 c_float seconds);

void            d_configurationSetHeartbeatUpdateFactor         (d_configuration config,
                                                                 u_cfElement element,
                                                                 const c_char* expiryTimePath,
                                                                 const c_char* updateFactorName);

void            d_configurationSetTimingInitialWaitPeriod       (d_configuration config,
                                                                 c_float seconds);

void            d_configurationSetInitialRequestCombinePeriod           (d_configuration config,
                                                                         c_float seconds);

void            d_configurationSetOperationalRequestCombinePeriod       (d_configuration config,
                                                                         c_float seconds);

void            d_configurationSetMasterElectionWaitTime                (d_configuration config,
                                                                         c_float seconds);

void            d_configurationSetLatencyBudget                         (d_configuration config,
                                                                         c_float seconds);

void            d_configurationSetHeartbeatLatencyBudget                (d_configuration config,
                                                                         c_float seconds);

void            d_configurationSetAlignmentLatencyBudget                (d_configuration config,
                                                                         c_float seconds);

void            d_configurationSetMajorityVotingThreshold               (d_configuration config,
                                                                         c_ulong threshold);

void            d_configurationSetTransportPriority                     (d_configuration config,
                                                                         c_long prio);

void            d_configurationSetHeartbeatTransportPriority            (d_configuration config,
                                                                         c_long prio);

void            d_configurationSetAlignmentTransportPriority            (d_configuration config,
                                                                         c_long prio);

void            d_configurationSetNetworkWaitForAttachmentMaxWaitCount  (d_configuration config,
                                                                         c_ulong maxWaits);

void            d_configurationSetNetworkWaitForAttachment              (d_configuration config,
                                                                         u_cfElement  elementParent,
                                                                         const c_char* attachName,
                                                                         const c_char* serviceName);

void            d_configurationSetNetworkResendTimeRange                (d_configuration config,
                                                                         c_float seconds);

void            d_configurationSetClientDurabilityEnabled               (d_configuration config,
                                                                         const c_bool enabled);

void            d_configurationSetPublisherName             (d_configuration  config,
                                                             const c_char * publisherName);

void            d_configurationSetSubscriberName            (d_configuration  config,
                                                             const c_char * subscriberName);

void            d_configurationSetPartitionName             (d_configuration  config,
                                                             const c_char * partitionName);

void            d_configurationSetClientDurabilityPartitionName         (d_configuration  config,
                                                                         const c_char * partitionName);

void            d_configurationSetClientDurabilityPublisherName         (d_configuration  config,
                                                                         const c_char * publisherName);

void            d_configurationSetClientDurabilitySubscriberName        (d_configuration  config,
                                                                         const c_char * subscriberName);

void            d_configurationSetTracingSynchronous        (d_configuration config,
                                                             const c_bool synchronous);

void            d_configurationSetTracingOutputFile         (d_configuration config,
                                                             const c_char* value);

void            d_configurationSetTracingTimestamps         (d_configuration  config,
                                                             c_bool useTimestamp);

void            d_configurationSetTimeAlignment             (d_configuration  config,
                                                             c_bool alignment);

void            d_configurationSetTimeToWaitForAligner      (d_configuration  config,
                                                             c_float seconds);

void            d_configurationSetTracingRelativeTimestamps (d_configuration config,
                                                             u_cfElement element,
                                                             const c_char* timestampsPath,
                                                             const c_char* absoluteName);

void            d_configurationSetTracingVerbosity          (d_configuration config,
                                                             const c_char* value);


void            d_configurationSetPersistentStoreDirectory  (d_configuration config,
                                                             const c_char* storePath);


void            d_configurationSetPersistentStoreMode       (d_configuration  config,
                                                             const c_char * storeModeName);

void            d_configurationResolvePersistentKVConfig    (d_configuration config,
                                                             u_cfElement elementParent,
                                                             const c_char *elementName);

void            d_configurationSetPersistentKVStorageParameters (d_configuration  config,
                                                                 const c_char * parameters);

void            d_configurationResolvePersistentKVCompression (d_configuration config,
                                                               u_cfElement elementParent,
                                                               const c_char *elementName);

void            d_configurationSetPersistentKVCompressionEnabled (d_configuration config,
                                                                  c_bool enabled);

void            d_configurationSetPersistentKVCompressionAlgorithm (d_configuration config,
                                                                    const c_char *algorithm);

void            d_configurationSetPersistentQueueSize       (d_configuration config,
                                                             c_ulong size);

void            d_configurationSetPersistentStoreSleepTime  (d_configuration config,
                                                             c_float seconds);

void            d_configurationSetPersistentStoreSessionTime(d_configuration config,
                                                             c_float seconds);

void            d_configurationSetBuiltinTopicsEnabled      (d_configuration config,
                                                             c_bool enabled);

void            d_configurationSetDuration                  (os_duration *duration,
                                                             c_float seconds );

c_iter          d_configurationResolvePolicies              (u_cfElement  elementParent,
                                                             const c_char * policyName);

c_iter          d_configurationResolveNameSpaces            (d_configuration config,
                                                             u_cfElement  elementParent,
                                                             const c_char * nameSpaceName,
                                                             int * resolveResult);

c_iter          d_configurationResolveFilters               (d_configuration config,
                                                             u_cfElement  elementParent,
                                                             const c_char *filterElement,
                                                             d_durability durability);

int            d_configurationResolvePartition             (d_nameSpace  nameSpace,
                                                             u_cfElement  element,
                                                             c_char *     name,
                                                             const c_char * tag,
                                                             const c_char * topic);

int            d_configurationResolvePartitionTopic        (d_nameSpace  nameSpace,
                                                             u_cfElement  element,
                                                             c_char *     name,
                                                             const c_char * tag);

void            d_configurationAttrValueLong                (d_configuration configuration,
                                                             u_cfElement  element,
                                                             const char * tag,
                                                             const char * attr,
                                                             void (* const setAction)(d_configuration config, c_long longValue));

void            d_configurationAttrValueULong               (d_configuration configuration,
                                                             u_cfElement  element,
                                                             const char * tag,
                                                             const char * attr,
                                                             void (* const setAction)(d_configuration config, c_ulong longValue));

void            d_configurationAttrValueFloat               (d_configuration configuration,
                                                             u_cfElement  element,
                                                             const char * tag,
                                                             const char * attr,
                                                             void (* const setAction)(d_configuration config, c_float floatValue));

void            d_configurationAttrValueBoolean             (d_configuration configuration,
                                                             u_cfElement  element,
                                                             const char * tag,
                                                             const char * attr,
                                                             void (* const setAction)(d_configuration config, c_bool boolValue));

void            d_configurationValueULong                   (d_configuration configuration,
                                                             u_cfElement  element,
                                                             const char * tag,
                                                             void (* const setAction)(d_configuration config, c_ulong longValue));

void            d_configurationValueSize                   (d_configuration configuration,
                                                             u_cfElement  element,
                                                             const char * tag,
                                                             void (* const setAction)(d_configuration config, c_size size));

void            d_configurationValueLong                    (d_configuration configuration,
                                                             u_cfElement  element,
                                                             const char * tag,
                                                             void (* const setAction)(d_configuration config, c_long longValue));

void            d_configurationValueFloat                   (d_configuration configuration,
                                                             u_cfElement  element,
                                                             const char * tag,
                                                             void (* const setAction)(d_configuration config, c_float floatValue));

void            d_configurationValueString                  (d_configuration configuration,
                                                             u_cfElement  element,
                                                             const char * tag,
                                                             void (* const setAction)(d_configuration config, const c_char* str));
void            d_configurationValueMemAddr                 (d_configuration configuration,
                                                             u_cfElement  element,
                                                             const char * tag,
                                                             void (* const setAction)(d_configuration config, c_address addr));

void            d_configurationValueBoolean                 (d_configuration configuration,
                                                             u_cfElement  element,
                                                             const char * tag,
                                                             void         (* const setAction)(d_configuration config, c_bool str));

void            d_configurationSetOptimizeUpdateInterval    (d_configuration config,
                                                             c_ulong size);

void            d_configurationReport                       (d_configuration config,
                                                             d_durability durability);

void            d_configurationSetRole                      (d_configuration config,
                                                             const c_char* role);

void            d_configurationSetServiceTerminatePeriod    (d_configuration config,
                                                             const c_float seconds);

void            d_configurationSetThreadLivelinessReportPeriod (d_configuration config,
                                                             const c_float seconds);

void            d_configurationSetDeadlockDetection         (d_configuration config,
                                                             c_bool deadlockDetection);

void            d_configurationSetY2038Ready                (d_configuration config,
                                                             const c_char *y2038Ready);

int             d_configurationResolveFilterPartitionTopic  (d_filter filter,
                                                             u_cfElement elementParent,
                                                             const c_char *name,
                                                             const c_char *tag);

c_iter          d_configurationResolveDurablePolicies       (u_cfElement  elementParent,
                                                             const c_char * policy);

c_bool          d_configurationCheckFilterInNameSpace       (d_configuration config, d_nameSpace nameSpace);

d_policy        d_configurationFindPolicyForNameSpace       (d_configuration config,
                                                             const char * nameSpaceName);


#if defined (__cplusplus)
}
#endif

#endif /* D__CONFIGURATION_H */
