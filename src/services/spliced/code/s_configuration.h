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
#ifndef S_CONFIGURATION_H
#define S_CONFIGURATION_H

#include "report.h"
#include "s_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define S_CFG_SERVICETERMINATEPERIOD_MINIMUM (0.0F)
#define S_CFG_SERVICETERMINATEPERIOD_DEFAULT (10.0F)
#define S_CFG_SERVICETERMINATEPERIOD_MAXIMUM (60.0F)

#define S_CFG_LEASEPERIOD_MINIMUM            (0.2F)
#define S_CFG_LEASEPERIOD_DEFAULT            (60.0F)

#define S_CFG_LEASERENEWALPERIOD_MINIMUM     (0.05F)
#define S_CFG_LEASERENEWALPERIOD_DEFAULT     (0.1F)
#define S_CFG_LEASERENEWALPERIOD_MAXIMUM     (0.9F)

C_STRUCT(s_configuration)
{
    FILE*         tracingOutputFile;
    c_char*       tracingOutputFileName;
    c_bool        tracingSynchronous;
    c_bool        tracingTimestamps;
    c_bool        tracingRelativeTimestamps;
    s_reportlevel tracingVerbosityLevel;
    os_time       startTime;
    os_time       serviceTerminatePeriod;
    v_duration    leasePeriod;
    v_duration    leaseRenewalPeriod;
    os_threadAttr kernelManagerScheduling;
    os_threadAttr garbageCollectorScheduling;
    os_threadAttr resendManagerScheduling;
    os_threadAttr cAndMCommandScheduling;
    c_bool        enableCandMCommandThread;
    os_threadAttr leaseRenewScheduling;
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
