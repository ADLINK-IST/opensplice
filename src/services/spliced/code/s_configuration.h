/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef S_CONFIGURATION_H
#define S_CONFIGURATION_H

#include "report.h"
#include "s_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define S_CFG_SERVICETERMINATEPERIOD_MINIMUM       (0.0F)

#if defined(INTEGRITY)
#define S_CFG_SERVICETERMINATEPERIOD_DEFAULT       (30.0F)
#else
#define S_CFG_SERVICETERMINATEPERIOD_DEFAULT       (10.0F)
#endif
#define S_CFG_SERVICETERMINATEPERIOD_MAXIMUM       (60.0F)

#define S_CFG_HEARTBEAT_EXPIRYTIME_MINIMUM         (0.2F)
#define S_CFG_HEARTBEAT_EXPIRYTIME_DEFAULT         (10.0F)

#define S_CFG_HEARTBEAT_UPDATE_FACTOR_MINIMUM      (0.01F)
#define S_CFG_HEARTBEAT_UPDATE_FACTOR_DEFAULT      (0.2F)
#define S_CFG_HEARTBEAT_UPDATE_FACTOR_MAXIMUM      (1.0F)

#define S_CFG_HEARTBEAT_TRANSPORT_PRIORITY_MINIMUM (0)
#define S_CFG_HEARTBEAT_TRANSPORT_PRIORITY_DEFAULT (0)
#define S_CFG_HEARTBEAT_TRANSPORT_PRIORITY_MAXIMUM (2147483647)

#define S_CFG_LEASE_EXPIRYTIME_MINIMUM             (0.2F)
#define S_CFG_LEASE_EXPIRYTIME_DEFAULT             (10.0F)

#define S_CFG_LEASE_UPDATE_FACTOR_MINIMUM          (0.01F)
#define S_CFG_LEASE_UPDATE_FACTOR_DEFAULT          (0.2F)
#define S_CFG_LEASE_UPDATE_FACTOR_MAXIMUM          (1.0F)

#define S_CFG_REQUEST_PARTITION_DEFAULT            "durabilityPartition"
#define S_CFG_DURABLE_POLICY_CACHE                 TRUE

#define S_CFG_STACKSIZE_DEFAULT                    (524288) /* 512 KB*/


C_STRUCT(s_configuration)
{
    FILE*          tracingOutputFile;
    c_char*        tracingOutputFileName;
    c_bool         tracingSynchronous;
    c_bool         tracingTimestamps;
    c_bool         tracingRelativeTimestamps;
    s_reportlevel  tracingVerbosityLevel;
    os_timeM       startTimeMonotonic;
    os_duration    serviceTerminatePeriod;

    os_duration    heartbeatExpiryTime;
    os_duration    heartbeatUpdateInterval;
    os_threadAttr* heartbeatAttribute;
    c_long         heartbeatTransportPriority;

    os_duration    leasePeriod;        /* ExpiryTime */
    os_duration    leaseRenewalPeriod; /* ExpiryTime * ExpiryTime@update_factor */
    os_threadAttr  kernelManagerAttribute;
    os_threadAttr  garbageCollectorAttribute;
    os_threadAttr  resendManagerAttribute;
    os_threadAttr  cAndMCommandAttribute;
    c_bool         enableCandMCommandThread;
    os_threadAttr  leaseRenewAttribute;
    os_threadAttr  shmMonitorAttribute;
    os_char* domainName;
    os_char *partition;
    c_iter durablePolicies;
};

#define s_configuration(config) ((s_configuration)(config))

s_configuration s_configurationNew(void);
void s_configurationFree(s_configuration config);

void
s_configurationRead(
    s_configuration config,
    spliced daemon);

#if defined (__cplusplus)
}
#endif

#endif /* S_CONFIGURATION_H */
