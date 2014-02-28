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

#ifndef D__CONFIGURATION_H
#define D__CONFIGURATION_H

#include "d__types.h"
#include "u_user.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

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

#define D_MINIMUM_NETWORK_MAX_WAITTIME              ((float)0.01e0)
#define D_DEFAULT_NETWORK_MAX_WAITTIME              ((float)2.0e0)
#define D_MAXIMUM_NETWORK_MAX_WAITTIME              ((float)10.0e0)

#define D_MINIMUM_NETWORK_SAMPLE_RESEND_RANGE       ((float)0.2e0)
#define D_DEFAULT_NETWORK_SAMPLE_RESEND_RANGE       ((float)0.7e0)
#define D_MAXIMUM_NETWORK_SAMPLE_RESEND_RANGE       ((float)2.0e0)

#define D_PUBLISHER_NAME                            "durabilityPublisher"
#define D_SUBSCRIBER_NAME                           "durabilitySubscriber"
#define D_PARTITION_NAME                            "durabilityPartition"

#define D_DEFAULT_TRACING_OUTFILE                   "durability.log"

#define D_MINIMUM_PERSISTENT_STORE_SLEEP_TIME       ((float)0.5e0)
#define D_DEFAULT_PERSISTENT_STORE_SLEEP_TIME       ((float)2.0e0)
#define D_MAXIMUM_PERSISTENT_STORE_SLEEP_TIME       ((float)10.0e0)

#define D_MINIMUM_PERSISTENT_STORE_SESSION_TIME     ((float)5.0e0)
#define D_DEFAULT_PERSISTENT_STORE_SESSION_TIME     ((float)20.0e0)
#define D_MAXIMUM_PERSISTENT_STORE_SESSION_TIME     ((float)60.0e0)

#define D_MINIMUM_PERSISTENT_QUEUE_SIZE             (0)
#define D_DEFAULT_PERSISTENT_QUEUE_SIZE             (0)
#define D_MAXIMUM_PERSISTENT_QUEUE_SIZE             (10000)

#define D_MINIMUM_PERSISTENT_SMP_COUNT              (1)
#define D_DEFAULT_PERSISTENT_SMP_COUNT              (1)

#define D_MINIMUM_PERSISTENT_MMF_STORE_SIZE         (1048567)

#define D_MINIMUM_OPTIMIZE_INTERVAL                 (10)
#define D_DEFAULT_OPTIMIZE_INTERVAL                 (0)
#define D_MAXIMUM_OPTIMIZE_INTERVAL                 (1000000000)

#define D_MINIMUM_TIME_TO_WAIT_FOR_ALIGNER          ((float)0.0e0)
#define D_DEFAULT_TIME_TO_WAIT_FOR_ALIGNER          ((float)1.0e0)
#define D_MAXIMUM_TIME_TO_WAIT_FOR_ALIGNER          ((float)1.0e0)

#define D_DEFAULT_ROLE                              "DefaultRole"

int            d_configurationInit                             (d_configuration config,
                                                                 d_durability durability,
                                                                 u_cfElement domainElement,
                                                                 u_cfElement element);

void            d_configurationDeinit                           (d_object object);

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

void            d_configurationSetLatencyBudget                         (d_configuration config,
                                                                         c_float seconds);

void            d_configurationSetHeartbeatLatencyBudget                (d_configuration config,
                                                                         c_float seconds);

void            d_configurationSetAlignmentLatencyBudget                (d_configuration config,
                                                                         c_float seconds);

void            d_configurationSetTransportPriority                     (d_configuration config,
                                                                         c_long prio);

void            d_configurationSetHeartbeatTransportPriority            (d_configuration config,
                                                                         c_long prio);

void            d_configurationSetAlignmentTransportPriority            (d_configuration config,
                                                                         c_long prio);

void            d_configurationSetNetworkWaitForAttachmentMaxWaitCount  (d_configuration config,
                                                                         c_ulong maxWaits);

void            d_configurationSetNetworkWaitForAttachmentMaxWaitTime   (d_configuration config,
                                                                         c_float maxWaits);

void            d_configurationSetNetworkWaitForAttachment              (d_configuration config,
                                                                         u_cfElement  elementParent,
                                                                         const c_char* attachName,
                                                                         const c_char* serviceName);

void            d_configurationSetNetworkResendTimeRange                (d_configuration config,
                                                                         c_float seconds);

void            d_configurationSetPublisherName             (d_configuration  config,
                                                             const c_char * publisherName);

void            d_configurationSetSubscriberName            (d_configuration  config,
                                                             const c_char * subscriberName);

void            d_configurationSetPartitionName             (d_configuration  config,
                                                             const c_char * partitionName);

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

void            d_configurationSetPersistentMMFStoreAddress	(d_configuration  config,
                                                            c_address address);

void            d_configurationSetPersistentMMFStoreSize    (d_configuration  config,
                                                            c_size size);

void            d_configurationSetPersistentQueueSize       (d_configuration config,
                                                             c_ulong size);

void            d_configurationSetPersistentStoreSleepTime  (d_configuration config,
                                                             c_float seconds);

void            d_configurationSetPersistentStoreSessionTime(d_configuration config,
                                                             c_float seconds);

void            d_configurationSetPersistentSMPCount        (d_configuration config,
                                                             c_ulong count);

void            d_configurationSetBuiltinTopicsEnabled      (d_configuration config,
                                                             c_bool enabled);

void            d_configurationSetDuration                  (v_duration * timeOut,
                                                             c_float seconds );

void            d_configurationSetTime                      (os_time * timeOut,
                                                             c_float seconds );

c_iter          d_configurationResolvePolicies              (u_cfElement  elementParent,
                                                             const c_char * policyName);

c_iter          d_configurationResolveNameSpaces            (d_configuration config,
                                                             u_cfElement  elementParent,
                                                             const c_char * nameSpaceName);

void            d_configurationResolvePartition             (d_nameSpace  nameSpace,
                                                             u_cfElement  element,
                                                             c_char *     name,
                                                             const c_char * tag,
                                                             const c_char * topic);

void            d_configurationResolvePartitionTopic        (d_nameSpace  nameSpace,
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
void            d_configurationValueMemAddr					(d_configuration configuration,
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

#if defined (__cplusplus)
}
#endif

#endif /* D__CONFIGURATION_H */
