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

#ifndef D_CONFIGURATION_H
#define D_CONFIGURATION_H

#include "d__types.h"
#include "d_object.h"
#include "u_user.h"
#include "v_kernel.h"
#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_configuration){
    C_EXTENDS(d_object);
    c_bool        builtinTopicsEnabled;
    v_duration    livelinessExpiryTime;      /* ExpiryTime (duration */
    c_float       livelinessExpiry;          /* ExpiryTime (float) */
    os_time       livelinessUpdateInterval;  /* ExpiryTime * update_factor */
    os_threadAttr livelinessScheduling;      /* Not implemented yet.*/
    os_time       heartbeatUpdateInterval;
    os_time       heartbeatExpiryTime;
    c_float       heartbeatExpiry;
    os_threadAttr heartbeatScheduling;
    os_time       timingInitialWaitPeriod;
    c_ulong       networkMaxWaitCount;
    os_time       networkMaxWaitTime;
    d_table       networkServiceNames;
    c_iter        services;
    os_time       networkSampleResendTimeRange;
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
    c_char*       persistentKVStoreStorageType;
    c_char*       persistentKVStoreStorageParameters;
    os_time       persistentStoreSleepTime;
    os_time       persistentStoreSessionTime;
    c_ulong       persistentQueueSize;
    os_threadAttr persistentScheduling;
    c_address     persistentMMFStoreAddress;
    c_size        persistentMMFStoreSize;
    c_iter        nameSpaces;
    c_iter        policies;
    os_time       startTime;
    c_ulong       persistentUpdateInterval;
    c_ulong       persistentThreadCount;
    v_duration    latencyBudget;
    c_long        transportPriority;
    v_duration    heartbeatLatencyBudget;
    c_long        heartbeatTransportPriority;
    v_duration    alignerLatencyBudget;
    c_long        alignerTransportPriority;
    os_threadAttr alignerScheduling;
    os_threadAttr aligneeScheduling;
    os_time       initialRequestCombinePeriod;
    os_time       operationalRequestCombinePeriod;
    c_bool        timeAlignment;
    d_name        role;
    os_time       timeToWaitForAligner;
};

#define d_configuration(d) ((d_configuration)(d))

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
d_configuration d_configurationNew                  (d_durability service,
                                                     const c_char* serviceName,
                                                     c_long domainId);

/**
 * Frees the supplied configuration.
 *
 * @param configuration The configuration to free.
 */
void            d_configurationFree                 (d_configuration configuration);


c_bool          d_configurationInNameSpace              (d_nameSpace ns,
                                                         d_partition partition,
                                                         d_topic topic,
                                                         c_bool aligner);

#if defined (__cplusplus)
}
#endif

#endif /* D_CONFIGURATION_H */
