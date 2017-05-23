/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include "s_configuration.h"
#include "report.h"

#include "u_service.h"

#include "vortex_os.h"

#include "v_durabilityClient.h"

/*********************** static functions *************************/
static void
s_configurationSetDuration(
    os_duration *timeOut,
    c_float seconds)
{
    *timeOut = os_realToDuration(seconds);
}

#if 1
/* Configuration retrieve functions */
static void
s_configurationAttrValueLong(
    s_configuration configuration,
    u_cfElement     element,
    const char      *tag,
    const char      *attr,
    void            (* const setAction)(s_configuration config, c_long longValue))
{
    c_iter   iter;
    u_cfElement e;
    u_cfAttribute a;
    c_long   longValue;
    c_bool   found;

    iter = u_cfElementXPath(element, tag);
    e = u_cfElement(c_iterTakeFirst(iter));

    while (e != NULL) {
        a = u_cfElementAttribute(e, attr);

        if (a) {
            found = u_cfAttributeLongValue(a, &longValue);
            /* QAC EXPECT 2100; */
            if (found == TRUE) {
                setAction(configuration, longValue);
            }
            u_cfAttributeFree(a);
        }
        u_cfElementFree(e);
        e = u_cfElement(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}
#endif

static void
s_configurationAttrValueFloat(
    s_configuration configuration,
    u_cfElement     element,
    const char      *tag,
    const char      *attr,
    void            (* const setAction)(s_configuration config, c_float floatValue))
{
    c_iter   iter;
    u_cfElement e;
    u_cfAttribute a;
    c_float   floatValue;
    c_bool   found;

    iter = u_cfElementXPath(element, tag);
    e = u_cfElement(c_iterTakeFirst(iter));

    while (e != NULL) {
        a = u_cfElementAttribute(e, attr);

        if (a) {
            found = u_cfAttributeFloatValue(a, &floatValue);
            /* QAC EXPECT 2100; */
            if (found == TRUE) {
                setAction(configuration, floatValue);
            }
            u_cfAttributeFree(a);
        }
        u_cfElementFree(e);
        e = u_cfElement(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}

static void
s_configurationAttrValueString(
    s_configuration configuration,
    u_cfElement     element,
    const char      *tag,
    const char      *attr,
    void            (* const setAction)(s_configuration config, const c_char *strValue))
{
    c_iter   iter;
    u_cfElement e;
    u_cfAttribute a;
    c_char   *strValue;
    c_bool   found;

    iter = u_cfElementXPath(element, tag);
    e = u_cfElement(c_iterTakeFirst(iter));

    while (e != NULL) {
        a = u_cfElementAttribute(e, attr);

        if (a) {
            found = u_cfAttributeStringValue(a, &strValue);
            /* QAC EXPECT 2100; */
            if (found == TRUE) {
                setAction(configuration, strValue);
                os_free(strValue);
            }
            u_cfAttributeFree(a);
        }
        u_cfElementFree(e);
        e = u_cfElement(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}

static void
s_configurationAttrValueBoolean(
    s_configuration configuration,
    u_cfElement     element,
    const char      *tag,
    const char      *attr,
    void            (* const setAction)(s_configuration config, c_bool boolValue))
{
    c_iter   iter;
    u_cfElement e;
    u_cfAttribute a;
    c_bool   boolValue, found;

    iter = u_cfElementXPath(element, tag);
    e = u_cfElement(c_iterTakeFirst(iter));

    while (e != NULL) {
        a = u_cfElementAttribute(e, attr);

        if (a) {
            found = u_cfAttributeBoolValue(a, &boolValue);
            /* QAC EXPECT 2100; */
            if (found == TRUE) {
                setAction(configuration, boolValue);
            }
            u_cfAttributeFree(a);
        }
        u_cfElementFree(e);
        e = u_cfElement(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}

static void
s_configurationValueLong(
    s_configuration configuration,
    u_cfElement     element,
    const char      *tag,
    void            (* const setAction)(s_configuration config, c_long longValue))
{
    c_iter   iter;
    u_cfData data;
    c_long   longValue;
    c_bool   found;

    iter = u_cfElementXPath(element, tag);
    data = u_cfData(c_iterTakeFirst(iter));

    while (data != NULL) {
        found = u_cfDataLongValue(data, &longValue);
        /* QAC EXPECT 2100; */
        if (found == TRUE) {
            setAction(configuration, longValue);
        }
        u_cfDataFree(data);
        data = u_cfData(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}

#if 0
static void
s_configurationValueULong(
    s_configuration configuration,
    u_cfElement     element,
    const char      *tag,
    void            (* const setAction)(s_configuration config, c_ulong longValue))
{
    c_iter   iter;
    u_cfData data;
    c_long   longValue;
    c_ulong  ulongValue;
    c_bool   found;

    iter = u_cfElementXPath(element, tag);
    data = u_cfData(c_iterTakeFirst(iter));
    while (data != NULL) {
        found = u_cfDataLongValue(data, &longValue);
        /* QAC EXPECT 2100; */
        if (found == TRUE) {
            if (longValue < 0) {
                longValue = -longValue;
                if (longValue < 0) {
                    longValue++;
                    longValue = -longValue;
                }
            }
            ulongValue = (c_ulong)longValue;
            setAction(configuration, ulongValue);
        }
        u_cfDataFree(data);
        data = u_cfData(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}
#endif

#if 0
static void
s_configurationValueSize(
    s_configuration configuration,
    u_cfElement     element,
    const char      *tag,
    void            (* const setAction)(s_configuration config, c_size size))
{
    c_iter   iter;
    u_cfData data;
    c_size  size;
    c_bool   found;

    iter = u_cfElementXPath(element, tag);
    data = u_cfData(c_iterTakeFirst(iter));
    while (data != NULL) {
        found = u_cfDataSizeValue(data, &size);
        /* QAC EXPECT 2100; */
        if (found == TRUE) {
            setAction(configuration, size);
        }
        u_cfDataFree(data);
        data = u_cfData(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}
#endif

static void
s_configurationValueFloat(
    s_configuration configuration,
    u_cfElement     element,
    const c_char    *tag,
    void            (* const setAction)(s_configuration config, c_float floatValue))
{
    c_iter   iter;
    u_cfData data;
    c_bool   found;
    c_float  floatValue;

    iter = u_cfElementXPath(element, tag);
    data = u_cfData(c_iterTakeFirst(iter));

    while (data != NULL) {
        found = u_cfDataFloatValue(data, &floatValue);

        if (found == TRUE) {
            setAction(configuration, floatValue);
        }
        u_cfDataFree(data);
        data = u_cfData(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}

static void
s_configurationValueBoolean(
    s_configuration configuration,
    u_cfElement     element,
    const char      *tag,
    void            (* const setAction)(s_configuration config, c_bool str))
{
    c_iter   iter;
    u_cfData data;
    c_bool   found;
    c_bool   b;

    iter = u_cfElementXPath(element, tag);
    data = u_cfData(c_iterTakeFirst(iter));

    while (data) {
        found = u_cfDataBoolValue(data, &b);
        /* QAC EXPECT 2100; */
        if (found == TRUE) {
            setAction(configuration, b);
        }
        u_cfDataFree(data);
        data = u_cfData(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}

static void
s_configurationValueString(
    s_configuration configuration,
    u_cfElement     element,
    const char      *tag,
    void            (* const setAction)(s_configuration config, const c_char * str) )
{
    c_iter   iter;
    u_cfData data;
    c_bool   found;
    c_char *   str;

    iter = u_cfElementXPath(element, tag);
    data = u_cfData(c_iterTakeFirst(iter));

    while (data) {
        found = u_cfDataStringValue(data, &str);
        /* QAC EXPECT 2100; */
        if (found == TRUE) {
            setAction(configuration, str);
            os_free(str);
        }
        u_cfDataFree(data);
        data = u_cfData(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}

/* Set functions */
static void
s_configurationSetServiceTerminatePeriod(
    s_configuration config,
    c_float sec)
{
    if (sec < S_CFG_SERVICETERMINATEPERIOD_MINIMUM) {
        sec = S_CFG_SERVICETERMINATEPERIOD_MINIMUM;
    }
    if (sec > S_CFG_SERVICETERMINATEPERIOD_MAXIMUM) {
        sec = S_CFG_SERVICETERMINATEPERIOD_MAXIMUM;
    }

    s_configurationSetDuration(&(config->serviceTerminatePeriod), sec);
}

/**
 * Sets the heartbeat expiry time-period. After this call the heartbeat-
 * period will be set to MAX(S_CFG_HEARBEAT_EXPIRYTIME_MINIMUM, expiryTime).
 * @param config The configuration struct to store the heartbeat expiry time in
 * @param expiryTime The ExpiryTime of the heartbeat.
 */
static void
s_configurationSetHeartbeatExpiryTime(
    s_configuration config,
    c_float expiryTime)
{
    if (expiryTime < S_CFG_HEARTBEAT_EXPIRYTIME_MINIMUM) {
        expiryTime = S_CFG_HEARTBEAT_EXPIRYTIME_MINIMUM;
    }

    s_configurationSetDuration(&(config->heartbeatExpiryTime), expiryTime);
}

/**
 * Sets the heartbeat update interval, which is defined by
 * Heartbeat.ExpiryTime * Heartbeat.ExpiryTime@update_factor.
 * After this call the renewal period will be set to
 *  Heartbeat.ExpiryTime * MIN(S_CFG_HEARTBEAT_UPDATE_FACTOR_MAXIMUM,
 * MAX(S_CFG_HEARTBEAT_UPDATE_FACTOR_MINIMUM, update_factor)).
 * @param config The configuration struct to store the heartbeat update interval in
 * @param update_factor The update-factor that needs to be applied to the ExpiryTime
 */
static void
s_configurationSetHeartbeatUpdateInterval(
    s_configuration config,
    c_float update_factor)
{
    if (update_factor < S_CFG_HEARTBEAT_UPDATE_FACTOR_MINIMUM) {
        update_factor = S_CFG_HEARTBEAT_UPDATE_FACTOR_MINIMUM;
    }

    if (update_factor > S_CFG_HEARTBEAT_UPDATE_FACTOR_MAXIMUM) {
        update_factor = S_CFG_HEARTBEAT_UPDATE_FACTOR_MAXIMUM;
    }

    config->heartbeatUpdateInterval = os_durationMul(config->heartbeatExpiryTime, update_factor);
}

static void s_configurationSetSchedulingClass( os_threadAttr *tattr,
                                               const c_char* class )
{
   if (os_strcasecmp(class, "Timeshare") == 0)
   {
      tattr->schedClass = OS_SCHED_TIMESHARE;
   }
   else if (os_strcasecmp(class, "Realtime") == 0)
   {
      tattr->schedClass = OS_SCHED_REALTIME;
   }
   else
   {
      tattr->schedClass = OS_SCHED_DEFAULT;
   }
}

static void s_configurationSetStackSize( os_threadAttr *tattr,
                                         const c_long stackSize )
{
    tattr->stackSize = (os_uint32)stackSize;
}

/**
 * Sets the transport priority of the Heartbeat writer
 * @param config The configuration struct to store the transport priority in
 * @param prio The transport priority
 */
static void
s_configurationSetHeartbeatTransportPriority(
    s_configuration config,
    c_long prio)
{
    if (prio < S_CFG_HEARTBEAT_TRANSPORT_PRIORITY_MINIMUM) {
        prio = S_CFG_HEARTBEAT_TRANSPORT_PRIORITY_MINIMUM;
    }
    if (prio > S_CFG_HEARTBEAT_TRANSPORT_PRIORITY_MAXIMUM) {
        prio = S_CFG_HEARTBEAT_TRANSPORT_PRIORITY_MAXIMUM;
    }
    config->heartbeatTransportPriority = prio;
}

/**
 * Sets the scheduling class of the Heartbeat manager.
 * @param config The configuration struct to store the scheduling class in
 * @param class The scheduling class
 */
static void
s_configurationSetHeartbeatSchedulingClass(
    s_configuration config,
    const c_char* class)
{
    if (config->heartbeatAttribute == NULL) {
         config->heartbeatAttribute = os_malloc(sizeof(*config->heartbeatAttribute));
         os_threadAttrInit(config->heartbeatAttribute);
         s_configurationSetStackSize(config->heartbeatAttribute,S_CFG_STACKSIZE_DEFAULT);
    }
    s_configurationSetSchedulingClass(config->heartbeatAttribute, class );
}

/**
 * Sets the scheduling priority of the Heartbeat manager.
 * @param config The configuration struct to store the scheduling class in
 * @param priority The scheduling priority
 */
static void
s_configurationSetHeartbeatSchedulingPriority(
    s_configuration config,
    c_long priority)
{
    if (config->heartbeatAttribute == NULL) {
         config->heartbeatAttribute = os_malloc(sizeof(*config->heartbeatAttribute));
         os_threadAttrInit(config->heartbeatAttribute);
         s_configurationSetStackSize(config->heartbeatAttribute,S_CFG_STACKSIZE_DEFAULT);
    }
    config->heartbeatAttribute->schedPriority = priority;
}


/**
 * Sets the lease-period, which is the ExpiryTime. After this call the lease-
 * period will be set to MAX(S_CFG_LEASE_EXPIRYTIME_MINIMUM, expiryTime).
 * @param config The configuration struct to store the renewal-period in
 * @param expiryTime The ExpiryTime of the lease.
 */
static void
s_configurationSetLeasePeriod(
    s_configuration config,
    c_float expiryTime)
{
    if (expiryTime < S_CFG_LEASE_EXPIRYTIME_MINIMUM) {
        expiryTime = S_CFG_LEASE_EXPIRYTIME_MINIMUM;
    }

    s_configurationSetDuration(&(config->leasePeriod), expiryTime);
}

/**
 * Sets the lease-renewal-period, which is defined by
 * ExpiryTime * ExpiryTime@update_factor. After this call the renewal period
 * will be set to ExpiryTime * MIN(S_CFG_LEASE_UPDATE_FACTOR_MAXIMUM,
 * MAX(S_CFG_LEASE_UPDATE_FACTOR_MINIMUM, update_factor)).
 * @param config The configuration struct to store the renewal-period in
 * @param update_factor The update-factor that needs to be applied to the ExpiryTime
 */
static void
s_configurationSetLeaseRenewalPeriod(
    s_configuration config,
    c_float update_factor)
{
    if (update_factor < S_CFG_LEASE_UPDATE_FACTOR_MINIMUM) {
        update_factor = S_CFG_LEASE_UPDATE_FACTOR_MINIMUM;
    }

    if (update_factor > S_CFG_LEASE_UPDATE_FACTOR_MAXIMUM) {
        update_factor = S_CFG_LEASE_UPDATE_FACTOR_MAXIMUM;
    }

    config->leaseRenewalPeriod = os_durationMul(config->leasePeriod, update_factor);
}



static void
s_configurationSetshmMonitorStackSize( s_configuration config,
                                       const c_long stackSize)
{
    s_configurationSetStackSize( &config->shmMonitorAttribute, stackSize );
}

static void
s_configurationSetKernelManagerStackSize( s_configuration config,
                                       const c_long stackSize)
{
    s_configurationSetStackSize( &config->kernelManagerAttribute, stackSize );
}

static void
s_configurationSetLeaseRenewStackSize( s_configuration config,
                                       const c_long stackSize)
{
    s_configurationSetStackSize( &config->leaseRenewAttribute, stackSize );
}

static void
s_configurationSetGCStackSize( s_configuration config,
                                       const c_long stackSize)
{
    s_configurationSetStackSize( &config->garbageCollectorAttribute, stackSize );
}

static void
s_configurationSetResendManagerStackSize( s_configuration config,
                                       const c_long stackSize)
{
    s_configurationSetStackSize( &config->resendManagerAttribute, stackSize );
}

static void
s_configurationSetCandMCommandStackSize( s_configuration config,
                                       const c_long stackSize)
{
    s_configurationSetStackSize( &config->cAndMCommandAttribute, stackSize );
}

static void
s_configurationSetHeartbeatStackSize( s_configuration config,
                                       const c_long stackSize)
{
    if (config->heartbeatAttribute == NULL) {
         config->heartbeatAttribute = os_malloc(sizeof(*config->heartbeatAttribute));
         os_threadAttrInit(config->heartbeatAttribute);
    }
    s_configurationSetStackSize(config->heartbeatAttribute, stackSize );
}



static void
s_configurationSetshmMonitorSchedulingClass( s_configuration config,
                                                const c_char* class)
{
   s_configurationSetSchedulingClass( &config->shmMonitorAttribute, class );
}

static void
s_configurationSetKernelManagerSchedulingClass( s_configuration config,
                                                const c_char* class)
{
   s_configurationSetSchedulingClass( &config->kernelManagerAttribute, class );
}

static void
s_configurationSetLeaseRenewSchedulingClass(
    s_configuration config,
    const c_char* class)
{
    s_configurationSetSchedulingClass( &config->leaseRenewAttribute, class );
}


static void
s_configurationSetPartition(
    s_configuration config,
    const c_char* partition)
{
    if (config->partition) {
        os_free(config->partition);
    }
    config->partition = os_strdup(partition);
}


static void
s_configurationSetDomainName(
    s_configuration config,
    const c_char* domainName)
{
    if (config->domainName) {
        os_free(config->domainName);
    }
    config->domainName = os_strdup(domainName);
}

static void s_configurationSetshmMonitorSchedulingPriority(
   s_configuration config,
   c_long priority)
{
    config->shmMonitorAttribute.schedPriority = priority;
}

static void
s_configurationSetLeaseRenewSchedulingPriority(
    s_configuration config,
    c_long priority)
{
    config->leaseRenewAttribute.schedPriority = priority;
}


static void s_configurationSetKernelManagerSchedulingPriority(
   s_configuration config,
   c_long priority)
{
    config->kernelManagerAttribute.schedPriority = priority;
}

static void s_configurationSetGCSchedulingClass( s_configuration config,
                                                 const c_char* class)
{
   s_configurationSetSchedulingClass( &config->garbageCollectorAttribute,
                                      class );
}

static void
s_configurationSetGCSchedulingPriority(
    s_configuration config,
    c_long priority)
{
    config->garbageCollectorAttribute.schedPriority = priority;
}

static void
s_configurationSetResendManagerSchedulingClass(
    s_configuration config,
    const c_char* class)
{
   s_configurationSetSchedulingClass( &config->resendManagerAttribute, class );
}

static void
s_configurationSetResendManagerSchedulingPriority(
    s_configuration config,
    c_long priority)
{
    config->resendManagerAttribute.schedPriority = priority;
}

static void s_configurationSetCandMCommandSchedulingClass( s_configuration config,
                                                        const c_char* class)
{
   s_configurationSetSchedulingClass( &config->cAndMCommandAttribute, class );
}

#if 1
static void
s_configurationSetCandMCommandSchedulingPriority( s_configuration config,
                                                  c_long priority)
{
    config->cAndMCommandAttribute.schedPriority = priority;
}
#endif

static void
s_configurationSetTracingSynchronous(
    s_configuration config,
    const c_bool synchronous)
{
    config->tracingSynchronous = synchronous;
}

static void
s_configurationSetTracingOutputFile(
    s_configuration config,
    const c_char *value)
{
    if (value) {
        if (config->tracingOutputFileName) {
            if ((os_strcasecmp(config->tracingOutputFileName, "stdout") != 0) &&
                (os_strcasecmp(config->tracingOutputFileName, "stderr") != 0)) {
                if  (config->tracingOutputFile) {
                    fclose(config->tracingOutputFile);
                    config->tracingOutputFile = NULL;
                }
            }
            os_free(config->tracingOutputFileName);
            config->tracingOutputFileName = NULL;
        }

        if (os_strcasecmp(value, "stdout") == 0) {
            config->tracingOutputFileName = os_strdup("stdout");
            config->tracingOutputFile = stdout; /* default */
        } else {
            if (os_strcasecmp(value, "stderr") == 0) {
                config->tracingOutputFileName = os_strdup("stderr");
                config->tracingOutputFile = stderr;
            } else {
                char * filename = os_fileNormalize(value);
                config->tracingOutputFile = fopen(filename, "a");
                config->tracingOutputFileName = os_strdup(value);
                os_free(filename);
            }
        }
    }
}

static void
s_configurationSetTracingTimestamps(
    s_configuration  config,
    c_bool useTimestamp)
{
    if (config) {
        config->tracingTimestamps = useTimestamp;
    }
}

static void
s_configurationSetTracingRelativeTimestamps(
    s_configuration config,
    u_cfElement element,
    const c_char* timestampsPath,
    const c_char* absoluteName)
{
    u_cfElement timestampsElement;
    c_iter elements;
    c_bool success, absolute;

    elements = u_cfElementXPath(element, timestampsPath);

    if(elements){
        timestampsElement = u_cfElement(c_iterTakeFirst(elements));

        while(timestampsElement){
            success = u_cfElementAttributeBoolValue(timestampsElement, absoluteName, &absolute);

            if(success == TRUE){
                config->tracingRelativeTimestamps = (!absolute);
            }
            u_cfElementFree(timestampsElement);
            timestampsElement = u_cfElement(c_iterTakeFirst(elements));
        }
        c_iterFree(elements);
    }
}

static void
s_configurationSetTracingVerbosity(
    s_configuration config,
    const c_char* value)
{
    if (value) {
        if (os_strcasecmp(value, "SEVERE") == 0) {
            config->tracingVerbosityLevel = S_RPTLEVEL_SEVERE;
        } else if (os_strcasecmp(value, "WARNING") == 0) {
            config->tracingVerbosityLevel = S_RPTLEVEL_WARNING;
        } else if (os_strcasecmp(value, "INFO") == 0) {
            config->tracingVerbosityLevel = S_RPTLEVEL_INFO;
        } else if (os_strcasecmp(value, "CONFIG") == 0) {
            config->tracingVerbosityLevel = S_RPTLEVEL_CONFIG;
        } else if (os_strcasecmp(value, "FINE") == 0) {
            config->tracingVerbosityLevel = S_RPTLEVEL_FINE;
        } else if (os_strcasecmp(value, "FINER") == 0) {
            config->tracingVerbosityLevel = S_RPTLEVEL_FINER;
        } else if (os_strcasecmp(value, "FINEST") == 0) {
            config->tracingVerbosityLevel = S_RPTLEVEL_FINEST;
        } else if (os_strcasecmp(value, "NONE") == 0) {
            config->tracingVerbosityLevel = S_RPTLEVEL_NONE;
        }
    }
}
/* Init/deinit */
static void
s_configurationInit(
    s_configuration config)
{
    assert(config != NULL);

    /** First apply defaults for tracing */
    config->tracingOutputFile           = NULL;
    config->tracingOutputFileName       = NULL;
    config->tracingSynchronous          = FALSE;
    config->tracingVerbosityLevel       = S_RPTLEVEL_NONE;
    config->tracingTimestamps           = TRUE;
    config->tracingRelativeTimestamps   = FALSE;
    config->startTimeMonotonic          = os_timeMGet();
    config->domainName                  = NULL;
    config->partition                   = NULL;
    config->enableCandMCommandThread    = TRUE;
    config->durablePolicies             = NULL;

    /*s_printTimedEvent(daemon, S_RPTLEVEL_FINER, S_THREAD_MAIN, "Initializing configuration...\n");*/
    s_configurationSetDuration(&(config->serviceTerminatePeriod), S_CFG_SERVICETERMINATEPERIOD_DEFAULT);
    s_configurationSetLeasePeriod(config, S_CFG_LEASE_EXPIRYTIME_DEFAULT);
    s_configurationSetLeaseRenewalPeriod(config, S_CFG_LEASE_UPDATE_FACTOR_DEFAULT);

    s_configurationSetHeartbeatExpiryTime(config, S_CFG_HEARTBEAT_EXPIRYTIME_DEFAULT);
    s_configurationSetHeartbeatUpdateInterval(config, S_CFG_HEARTBEAT_UPDATE_FACTOR_DEFAULT);

    s_configurationSetHeartbeatTransportPriority(config, S_CFG_HEARTBEAT_TRANSPORT_PRIORITY_DEFAULT);

    s_configurationSetPartition(config, S_CFG_REQUEST_PARTITION_DEFAULT);

    /* Apply defaults to rest of configuration */
    os_threadAttrInit(&config->kernelManagerAttribute);
    s_configurationSetStackSize(&config->kernelManagerAttribute,S_CFG_STACKSIZE_DEFAULT);

    os_threadAttrInit(&config->garbageCollectorAttribute);
    s_configurationSetStackSize(&config->garbageCollectorAttribute,S_CFG_STACKSIZE_DEFAULT);

    os_threadAttrInit(&config->resendManagerAttribute);
    s_configurationSetStackSize(&config->resendManagerAttribute,S_CFG_STACKSIZE_DEFAULT);

    os_threadAttrInit(&config->cAndMCommandAttribute);
    s_configurationSetStackSize(&config->cAndMCommandAttribute,S_CFG_STACKSIZE_DEFAULT);

    os_threadAttrInit(&config->leaseRenewAttribute);
    s_configurationSetStackSize(&config->leaseRenewAttribute,S_CFG_STACKSIZE_DEFAULT);

    os_threadAttrInit(&config->shmMonitorAttribute);
    s_configurationSetStackSize(&config->shmMonitorAttribute,S_CFG_STACKSIZE_DEFAULT);

    config->heartbeatAttribute = NULL;
}

static void
s_configurationDeinit(
    s_configuration config)
{
    assert(config);

    if (config->tracingOutputFileName) {
        if( (strcmp(config->tracingOutputFileName, "stdout") != 0) &&
            (strcmp(config->tracingOutputFileName, "stderr") != 0)) {
            if (config->tracingOutputFile) {
                fclose(config->tracingOutputFile);
                config->tracingOutputFile = NULL;
            }
        }
        os_free(config->tracingOutputFileName);
        config->tracingOutputFileName = NULL;
    }
    if(config->domainName) {
        os_free(config->domainName);
        config->domainName = NULL;
    }
    if (config->partition) {
        os_free(config->partition);
        config->partition = NULL;
    }
    if (config->durablePolicies) {
        struct durablePolicy *dp;
        while ((dp = (struct durablePolicy *)c_iterTakeFirst(config->durablePolicies)) != NULL) {
            os_free(dp->obtain);
            os_free(dp);
        }
        c_iterFree(config->durablePolicies);
    }
    os_free(config->heartbeatAttribute);

    return;
}

/* report functions */
static void
s_configurationReport(
    s_configuration config,
    spliced daemon)
{
    const c_char* verbosity;
    const c_char* timestamps;
    const c_char* relativeTimestamps;

    switch(config->tracingVerbosityLevel){
    case S_RPTLEVEL_NONE:
        verbosity = "NONE";
    break;
    case S_RPTLEVEL_SEVERE:
        verbosity = "SEVERE";
    break;
    case S_RPTLEVEL_WARNING:
        verbosity = "WARNING";
    break;
    case S_RPTLEVEL_CONFIG:
        verbosity = "CONFIG";
    break;
    case S_RPTLEVEL_INFO:
        verbosity = "INFO";
    break;
    case S_RPTLEVEL_FINE:
        verbosity = "FINE";
    break;
    case S_RPTLEVEL_FINER:
        verbosity = "FINER";
    break;
    case S_RPTLEVEL_FINEST:
        verbosity = "FINEST";
    break;
    default:
        assert(FALSE);
        verbosity = "UNKNOWN";
    break;
    }
    if(config->tracingTimestamps == TRUE){
        timestamps = "TRUE";
    } else {
        timestamps = "FALSE";
    }
    if(config->tracingRelativeTimestamps == TRUE){
        relativeTimestamps = "TRUE";
    } else {
        relativeTimestamps = "FALSE";
    }
    s_printEvent(daemon, S_RPTLEVEL_CONFIG,
            "- Tracing.Verbosity                           : %s\n" \
            "- Tracing.OutputFile                          : %s\n" \
            "- Tracing.Timestamps                          : %s\n" \
            "- Tracing.RelativeTimestamps                  : %s\n"
            , verbosity
            , config->tracingOutputFileName
            , timestamps
            , relativeTimestamps);
}


static c_iter
s_configurationResolveDurablePolicies(
    u_cfElement  elementParent,
    const c_char *policy)
{
    c_iter iter, result;
    u_cfElement element;
    c_bool found;
    c_bool isCache = FALSE;
    c_char *obtain;
    c_char *cache;
    struct durablePolicy *dp;

    result = c_iterNew(NULL);
    iter = u_cfElementXPath(elementParent, policy);
    element = (u_cfElement)c_iterTakeFirst(iter);
    while (element) {
        /* Parse obtain attribute */
        found = u_cfElementAttributeStringValue(element, "obtain", &obtain);
        if (found) {
            /* Parse cache attribute */
            found = u_cfElementAttributeStringValue(element, "cache", &cache);
            if (found){
                isCache = (os_strcasecmp(cache, "TRUE") == 0) ? TRUE : FALSE;
                os_free(cache);
            } else {
                isCache = S_CFG_DURABLE_POLICY_CACHE;   /* default cache */
            }
            /* Add to durablePolicy struct */
            dp = (struct durablePolicy *)os_malloc(sizeof(struct durablePolicy));
            dp->obtain = os_strdup(obtain);
            dp->cache = isCache;
            result = c_iterAppend(result, dp);
            os_free(obtain);
        }
        u_cfElementFree(element);
        element = (u_cfElement)c_iterTakeFirst(iter);
    } /* while */
    c_iterFree(iter);
    return result;
}



/************* public **********************/

s_configuration
s_configurationNew(void)
{
    s_configuration config;
    config = os_malloc(sizeof *config);
    s_configurationInit(config);
    return config;
}

void
s_configurationFree(
    s_configuration config)
{
    assert(config);

    s_configurationDeinit(config);
    os_free(config);
}

void
s_configurationRead(
    s_configuration config,
    spliced daemon)
{
    u_cfElement root;
    u_cfElement element;
    u_cfElement domain;
    u_cfElement dcfg;
    u_cfAttribute attribute;
    u_cfNode node;
    c_iter iter;
    c_char *name;
    c_bool enabled;
    c_bool success;

    root = u_participantGetConfiguration(u_participant(splicedGetService(daemon)));
    if (root) {
        iter = u_cfElementXPath(root, "Domain");
        domain = u_cfElement(c_iterTakeFirst(iter));
        element = u_cfElement(c_iterTakeFirst(iter));
        while (element) {
            u_cfElementFree(element);
            element = u_cfElement(c_iterTakeFirst(iter));
        }
        c_iterFree(iter);

        iter = u_cfElementXPath(root, "Domain/Daemon");
        dcfg = u_cfElement(c_iterTakeFirst(iter));
        element = u_cfElement(c_iterTakeFirst(iter));
        while (element) {
            u_cfElementFree(element);
            element = u_cfElement(c_iterTakeFirst(iter));
        }
        c_iterFree(iter);
    } else {
        domain = NULL;
        dcfg = NULL;
    }

    if (domain) {
        s_configurationValueString (config, domain, "Name/#text", s_configurationSetDomainName);
        /* Enable statistics */
        iter = u_cfElementXPath(domain,"Statistics/Category");
        node = u_cfNode(c_iterTakeFirst(iter));

        while (node != NULL) {
            if (u_cfNodeKind(node) == V_CFELEMENT) {
                /* Get the (optional) "enabled"-attribute of the category.
                * If not present then "enabled=true" is assumed.
                * Only if enabled="false" the category must not be checked. */
                enabled = TRUE;
                success = u_cfElementAttributeBoolValue(u_cfElement(node), "enabled", &enabled);
                if (!success || enabled) {
                    attribute = u_cfElementAttribute(u_cfElement(node), "name");
                    if (attribute) {
                        u_cfAttributeStringValue(attribute, &name);
                        if (name) {
                            u_domainEnableStatistics(u_participantDomain(u_participant(splicedGetService(daemon))), name);
                            os_free(name);
                        }
                        u_cfAttributeFree(attribute);
                    }
                }
            }
            u_cfNodeFree(node);
            node = u_cfNode(c_iterTakeFirst(iter));
        }
        c_iterFree(iter);
        s_configurationValueFloat(config, domain, "ServiceTerminatePeriod/#text", s_configurationSetServiceTerminatePeriod);
        s_configurationValueFloat(config, domain, "Lease/ExpiryTime/#text", s_configurationSetLeasePeriod);
        s_configurationAttrValueFloat(config, domain, "Lease/ExpiryTime", "update_factor", s_configurationSetLeaseRenewalPeriod);
        s_configurationAttrValueString (config, domain, "DurablePolicies", "partition", s_configurationSetPartition);
        config->durablePolicies = s_configurationResolveDurablePolicies(domain, "DurablePolicies/Policy");

        /* Heartbeat */
        config->heartbeatExpiryTime = config->leasePeriod;
        config->heartbeatUpdateInterval = config->leaseRenewalPeriod;
    }

    if (dcfg) {
        s_configurationAttrValueBoolean (config, dcfg, "Tracing", "synchronous", s_configurationSetTracingSynchronous);
        s_configurationValueString      (config, dcfg, "Tracing/Verbosity/#text", s_configurationSetTracingVerbosity);
        s_configurationValueString      (config, dcfg, "Tracing/OutputFile/#text", s_configurationSetTracingOutputFile);
        s_configurationValueBoolean     (config, dcfg, "Tracing/Timestamps/#text", s_configurationSetTracingTimestamps);
        s_configurationSetTracingRelativeTimestamps(config, dcfg, "Tracing/Timestamps", "absolute");

        s_configurationValueString (config, dcfg, "Watchdog/Scheduling/Class/#text", s_configurationSetLeaseRenewSchedulingClass);
        s_configurationValueLong (config, dcfg, "Watchdog/Scheduling/Priority/#text", s_configurationSetLeaseRenewSchedulingPriority);
        s_configurationValueLong (config, dcfg, "Watchdog/StackSize/#text", s_configurationSetLeaseRenewStackSize);

        /* Kernelmanager */
        s_configurationValueString(config, dcfg, "KernelManager/Scheduling/Class/#text", s_configurationSetKernelManagerSchedulingClass);
        s_configurationValueLong(config, dcfg, "KernelManager/Scheduling/Priority/#text", s_configurationSetKernelManagerSchedulingPriority);
        s_configurationValueLong(config, dcfg, "KernelManager/StackSize/#text", s_configurationSetKernelManagerStackSize);
         /* GarbageCollector */
         s_configurationValueString(config, dcfg, "GarbageCollector/Scheduling/Class/#text", s_configurationSetGCSchedulingClass);
         s_configurationValueLong(config, dcfg, "GarbageCollector/Scheduling/Priority/#text", s_configurationSetGCSchedulingPriority);
         s_configurationValueLong(config, dcfg, "GarbageCollector/StackSize/#text", s_configurationSetGCStackSize);

         /* ResendManager */
         s_configurationValueString(config, dcfg, "ResendManager/Scheduling/Class/#text", s_configurationSetResendManagerSchedulingClass);
         s_configurationValueLong(config, dcfg, "ResendManager/Scheduling/Priority/#text", s_configurationSetResendManagerSchedulingPriority);
         s_configurationValueLong(config, dcfg, "ResendManager/StackSize/#text", s_configurationSetResendManagerStackSize);
         /* shmMonitor */
         s_configurationValueString(config, dcfg, "shmMonitor/Scheduling/Class/#text", s_configurationSetshmMonitorSchedulingClass);
         s_configurationValueLong(config, dcfg, "shmMonitor/Scheduling/Priority/#text", s_configurationSetshmMonitorSchedulingPriority);
         s_configurationValueLong(config, dcfg, "shmMonitor/StackSize/#text", s_configurationSetshmMonitorStackSize);

         /* Heartbeat */
         s_configurationAttrValueLong(config, dcfg, "Heartbeat", "transport_priority", s_configurationSetHeartbeatTransportPriority);

         s_configurationValueFloat(config, dcfg, "Heartbeat/ExpiryTime/#text", s_configurationSetHeartbeatExpiryTime);
         s_configurationAttrValueFloat(config, dcfg, "Heartbeat/ExpiryTime", "update_factor", s_configurationSetHeartbeatUpdateInterval);

         s_configurationValueString(config, dcfg, "Heartbeat/Scheduling/Class/#text", s_configurationSetHeartbeatSchedulingClass);
         s_configurationValueLong(config, dcfg, "Heartbeat/Scheduling/Priority/#text", s_configurationSetHeartbeatSchedulingPriority);
         s_configurationValueLong(config, dcfg, "Heartbeat/StackSize/#text", s_configurationSetHeartbeatStackSize);

         /* Control and Monitoring Command Receiver */
         iter = u_cfElementXPath(domain,
                                 "ControlAndMonitoringCommandReceiver[@enabled='false']");
         node = u_cfNode(c_iterTakeFirst(iter));
         /* Enabled unless explictly disabled in config */
         config->enableCandMCommandThread=(!node);
         if ( node != NULL )
         {
            u_cfNodeFree(node);
         }
         c_iterFree(iter);

         if ( config->enableCandMCommandThread )
         {
            iter = u_cfElementXPath(domain,
                                    "ControlAndMonitoringCommandReceiver[@enabled='true']");
            node = u_cfNode(c_iterTakeFirst(iter));
            while (node != NULL)
            {
               if (u_cfNodeKind(node) == V_CFELEMENT)
               {
                  attribute = u_cfElementAttribute(u_cfElement(node), "name");
                  if (attribute)
                  {
                     u_cfAttributeStringValue(attribute, &name);
                     if (name)
                     {
                        s_configurationValueString(config, dcfg, "ControlAndMonitoringCommandReceiver/Scheduling/Class/#text", s_configurationSetCandMCommandSchedulingClass);
                        s_configurationValueLong(config, dcfg, "ControlAndMonitoringCommandReceiver/Scheduling/Priority/#text", s_configurationSetCandMCommandSchedulingPriority);
                        s_configurationValueLong(config, dcfg, "ControlAndMonitoringCommandReceiver/StackSize/#text", s_configurationSetCandMCommandStackSize);
                        os_free(name);
                     }
                     u_cfAttributeFree(attribute);
                  }
               }
               u_cfNodeFree(node);
               node = u_cfNode(c_iterTakeFirst(iter));
            }
            c_iterFree(iter);

         }
    }
    /* Finally report the configuration */
    s_configurationReport(config, daemon);

    if (domain) {
        u_cfElementFree(domain);
    }
    if (dcfg) {
        u_cfElementFree(dcfg);
    }

    if (root) {
        u_cfElementFree(root);
    }
}
