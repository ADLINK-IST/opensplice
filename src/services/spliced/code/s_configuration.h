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
#ifndef S_CONFIGURATION_H
#define S_CONFIGURATION_H

#include "report.h"
#include "s_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define S_CFG_SERVICETERMINATEPERIOD_MINIMUM       (0.0F)
#define S_CFG_SERVICETERMINATEPERIOD_DEFAULT       (10.0F)
#define S_CFG_SERVICETERMINATEPERIOD_MAXIMUM       (60.0F)

#define S_CFG_HEARTBEAT_EXPIRYTIME_MINIMUM         (0.2F)
#define S_CFG_HEARTBEAT_EXPIRYTIME_DEFAULT         (10.0F)

#define S_CFG_HEARTBEAT_UPDATE_FACTOR_MINIMUM      (0.01F)
#define S_CFG_HEARTBEAT_UPDATE_FACTOR_DEFAULT      (0.2F)
#define S_CFG_HEARTBEAT_UPDATE_FACTOR_MAXIMUM      (1.0F)

#if 0
#define S_CFG_HEARTBEAT_LATENCY_BUDGET_MINIMUM     (0.0F)
#define S_CFG_HEARTBEAT_LATENCY_BUDGET_DEFAULT     (0.0F)
#define S_CFG_HEARTBEAT_LATENCY_BUDGET_MAXIMUM     (2147483647.2147483647F)
#endif

#define S_CFG_HEARTBEAT_TRANSPORT_PRIORITY_MINIMUM (0)
#define S_CFG_HEARTBEAT_TRANSPORT_PRIORITY_DEFAULT (0)
#define S_CFG_HEARTBEAT_TRANSPORT_PRIORITY_MAXIMUM (2147483647)

#define S_CFG_LEASE_EXPIRYTIME_MINIMUM             (0.2F)
#define S_CFG_LEASE_EXPIRYTIME_DEFAULT             (10.0F)

#define S_CFG_LEASE_UPDATE_FACTOR_MINIMUM          (0.01F)
#define S_CFG_LEASE_UPDATE_FACTOR_DEFAULT          (0.2F)
#define S_CFG_LEASE_UPDATE_FACTOR_MAXIMUM          (1.0F)

C_STRUCT(s_configuration)
{
    FILE*          tracingOutputFile;
    c_char*        tracingOutputFileName;
    c_bool         tracingSynchronous;
    c_bool         tracingTimestamps;
    c_bool         tracingRelativeTimestamps;
    s_reportlevel  tracingVerbosityLevel;
    os_time        startTime;
    os_time        serviceTerminatePeriod;

    v_duration     heartbeatExpiryTime;
    v_duration     heartbeatUpdateInterval;
    os_threadAttr* heartbeatScheduling;
    c_long         heartbeatTransportPriority;

    v_duration     leasePeriod;        /* ExpiryTime */
    v_duration     leaseRenewalPeriod; /* ExpiryTime * ExpiryTime@update_factor */
    os_threadAttr  kernelManagerScheduling;
    os_threadAttr  garbageCollectorScheduling;
    os_threadAttr  resendManagerScheduling;
    os_threadAttr  cAndMCommandScheduling;
    c_bool         enableCandMCommandThread;
    os_threadAttr  leaseRenewScheduling;
    os_char* domainName;
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
