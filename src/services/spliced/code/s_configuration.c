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
#include "s_configuration.h"
#include "report.h"

#include "u_service.h"

#include "os.h"

/*********************** static functions *************************/
static void
s_configurationSetDuration(
    v_duration *timeOut,
    c_float    seconds)
{
    os_time tmp;

    tmp = os_realToTime(seconds);

    timeOut->seconds     = tmp.tv_sec;
    timeOut->nanoseconds = tmp.tv_nsec;
}

static void
s_configurationSetTime(
    os_time *timeOut,
    c_float seconds)
{
    *timeOut = os_realToTime(seconds);
}

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

    s_configurationSetTime(&(config->serviceTerminatePeriod), sec);
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
    os_time expiryTime;

    if (update_factor < S_CFG_HEARTBEAT_UPDATE_FACTOR_MINIMUM) {
        update_factor = S_CFG_HEARTBEAT_UPDATE_FACTOR_MINIMUM;
    }

    if (update_factor > S_CFG_HEARTBEAT_UPDATE_FACTOR_MAXIMUM) {
        update_factor = S_CFG_HEARTBEAT_UPDATE_FACTOR_MAXIMUM;
    }

    expiryTime.tv_sec = config->heartbeatExpiryTime.seconds;
    expiryTime.tv_nsec = config->heartbeatExpiryTime.nanoseconds;
    update_factor = update_factor * (c_float)os_timeToReal(expiryTime);
    s_configurationSetDuration(&(config->heartbeatUpdateInterval), update_factor);
}

/**
 * Sets the transport priority of the Heartbeat writer
 * @param config The configuration struct to store the transport priority in
 * @param prio The transport priority
 */
void
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
void
s_configurationSetHeartbeatSchedulingClass(
    s_configuration config,
    const c_char* class)
{
    if (config->heartbeatScheduling == NULL) {
        config->heartbeatScheduling = os_malloc(sizeof(os_threadAttr));
        if (config->heartbeatScheduling) {
            os_threadAttrInit(config->heartbeatScheduling);
            config->heartbeatScheduling->stackSize = 512*1024; /* 512KB */
        }
    }

    if (config->heartbeatScheduling) {
        if (os_strcasecmp(class, "Timeshare") == 0) {
            config->heartbeatScheduling->schedClass = OS_SCHED_TIMESHARE;
        } else if (os_strcasecmp(class, "Realtime") == 0) {
            config->heartbeatScheduling->schedClass = OS_SCHED_REALTIME;
        } else {
            config->heartbeatScheduling->schedClass = OS_SCHED_DEFAULT;
        }
    }
}

/**
 * Sets the scheduling priority of the Heartbeat manager.
 * @param config The configuration struct to store the scheduling class in
 * @param priority The scheduling priority
 */
void
s_configurationSetHeartbeatSchedulingPriority(
    s_configuration config,
    c_long priority)
{
    if (config->heartbeatScheduling == NULL) {
        config->heartbeatScheduling = os_malloc(sizeof(os_threadAttr));
        if (config->heartbeatScheduling) {
            os_threadAttrInit(config->heartbeatScheduling);
            config->heartbeatScheduling->stackSize = 512*1024; /* 512KB */
        }
    }

    if (config->heartbeatScheduling) {
        config->heartbeatScheduling->schedPriority = priority;
    }
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
    os_time leasePeriod;
    if (update_factor < S_CFG_LEASE_UPDATE_FACTOR_MINIMUM) {
        update_factor = S_CFG_LEASE_UPDATE_FACTOR_MINIMUM;
    }

    if (update_factor > S_CFG_LEASE_UPDATE_FACTOR_MAXIMUM) {
        update_factor = S_CFG_LEASE_UPDATE_FACTOR_MAXIMUM;
    }

    leasePeriod.tv_sec = config->leasePeriod.seconds;
    leasePeriod.tv_nsec = config->leasePeriod.nanoseconds;
    update_factor = update_factor * (c_float)os_timeToReal(leasePeriod);
    s_configurationSetDuration(&(config->leaseRenewalPeriod), update_factor);
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

static void
s_configurationSetKernelManagerSchedulingClass( s_configuration config,
                                                const c_char* class)
{
   s_configurationSetSchedulingClass( &config->kernelManagerScheduling, class );
}

static void
s_configurationSetLeaseRenewSchedulingClass(
    s_configuration config,
    const c_char* class)
{
    s_configurationSetSchedulingClass( &config->leaseRenewScheduling, class );
}

static void
s_configurationSetDomainName(
    s_configuration config,
    const c_char* domainName)
{
    config->domainName = os_strdup(domainName);
}

static void
s_configurationSetLeaseRenewSchedulingPriority(
    s_configuration config,
    c_long priority)
{
    config->leaseRenewScheduling.schedPriority = priority;
}


static void s_configurationSetKernelManagerSchedulingPriority(
   s_configuration config,
   c_long priority)
{
    config->kernelManagerScheduling.schedPriority = priority;
}

static void s_configurationSetGCSchedulingClass( s_configuration config,
                                                 const c_char* class)
{
   s_configurationSetSchedulingClass( &config->garbageCollectorScheduling,
                                      class );
}

static void
s_configurationSetGCSchedulingPriority(
    s_configuration config,
    c_long priority)
{
    config->garbageCollectorScheduling.schedPriority = priority;
}

static void
s_configurationSetResendManagerSchedulingClass(
    s_configuration config,
    const c_char* class)
{
   s_configurationSetSchedulingClass( &config->resendManagerScheduling, class );
}

static void
s_configurationSetResendManagerSchedulingPriority(
    s_configuration config,
    c_long priority)
{
    config->resendManagerScheduling.schedPriority = priority;
}

static void s_configurationSetCandMCommandSchedulingClass( s_configuration config,
                                                        const c_char* class)
{
   s_configurationSetSchedulingClass( &config->cAndMCommandScheduling, class );
}

static void
s_configurationSetCandMCommandSchedulingPriority( s_configuration config,
                                                  c_long priority)
{
    config->cAndMCommandScheduling.schedPriority = priority;
}


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
    if (config != NULL) {
        /** First apply defaults for tracing */
        config->tracingOutputFile           = NULL;
        config->tracingOutputFileName       = NULL;
        config->tracingSynchronous          = FALSE;
        config->tracingVerbosityLevel       = S_RPTLEVEL_NONE;
        config->tracingTimestamps           = TRUE;
        config->tracingRelativeTimestamps   = FALSE;
        config->startTime                   = os_timeGet();
        config->domainName                  = NULL;

        config->enableCandMCommandThread    = TRUE;

        /*s_printTimedEvent(daemon, S_RPTLEVEL_FINER, S_THREAD_MAIN, "Initializing configuration...\n");*/
        s_configurationSetTime(&(config->serviceTerminatePeriod), S_CFG_SERVICETERMINATEPERIOD_DEFAULT);
        s_configurationSetLeasePeriod(config, S_CFG_LEASE_EXPIRYTIME_DEFAULT);
        s_configurationSetLeaseRenewalPeriod(config, S_CFG_LEASE_UPDATE_FACTOR_DEFAULT);

        s_configurationSetHeartbeatExpiryTime(config, S_CFG_HEARTBEAT_EXPIRYTIME_DEFAULT);
        s_configurationSetHeartbeatUpdateInterval(config, S_CFG_HEARTBEAT_UPDATE_FACTOR_DEFAULT);

        s_configurationSetHeartbeatTransportPriority(config, S_CFG_HEARTBEAT_TRANSPORT_PRIORITY_DEFAULT);

        /* Apply defaults to rest of configuration */
        os_threadAttrInit(&config->kernelManagerScheduling);
        config->kernelManagerScheduling.stackSize = 512*1024; /* 512KB */

        os_threadAttrInit(&config->garbageCollectorScheduling);
        config->garbageCollectorScheduling.stackSize = 512*1024; /* 512KB */

        os_threadAttrInit(&config->resendManagerScheduling);
        config->resendManagerScheduling.stackSize = 512*1024; /* 512KB */

        os_threadAttrInit(&config->cAndMCommandScheduling);
        config->cAndMCommandScheduling.stackSize = 512*1024; /* 512KB */

        os_threadAttrInit(&config->leaseRenewScheduling);
        config->leaseRenewScheduling.stackSize = 512*1024; /* 512KB */

        config->heartbeatScheduling = NULL;
    }
}

static void
s_configurationDeinit(
    s_configuration config)
{
    assert(config);
    if (config) {
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
            if(config->domainName)
            {
                os_free(config->domainName);
                config->domainName = NULL;
            }
        }
        os_free(config->heartbeatScheduling);
    }
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

/************* public **********************/

s_configuration
s_configurationNew(void)
{
    s_configuration config;

    config = s_configuration(os_malloc(C_SIZEOF(s_configuration)));
    if (config != NULL) {
        s_configurationInit(config);
    }
    return config;
}

void
s_configurationFree(
    s_configuration config)
{
    assert(config);

    if (config) {
        s_configurationDeinit(config);
        os_free(config);
    }
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
        iter = u_cfElementXPath(domain,
                                "Statistics/Category[@enabled='true']");
        node = u_cfNode(c_iterTakeFirst(iter));
        while (node != NULL) {
            if (u_cfNodeKind(node) == V_CFELEMENT) {
                attribute = u_cfElementAttribute(u_cfElement(node), "name");
                if (attribute) {
                    u_cfAttributeStringValue(attribute, &name);
                    if (name) {
                        u_serviceEnableStatistics(u_service(splicedGetService(daemon)), name);
                        os_free(name);
                    }
                    u_cfAttributeFree(attribute);
                }
            }
            u_cfNodeFree(node);
            node = u_cfNode(c_iterTakeFirst(iter));
        }
        c_iterFree(iter);
        s_configurationValueFloat(config, domain, "ServiceTerminatePeriod/#text", s_configurationSetServiceTerminatePeriod);
        s_configurationValueFloat(config, domain, "Lease/ExpiryTime/#text", s_configurationSetLeasePeriod);
        s_configurationAttrValueFloat(config, domain, "Lease/ExpiryTime", "update_factor", s_configurationSetLeaseRenewalPeriod);
        s_configurationValueString (config, domain, "Watchdog/Scheduling/Class/#text", s_configurationSetLeaseRenewSchedulingClass);
        s_configurationValueLong   (config, domain, "Watchdog/Scheduling/Priority/#text", s_configurationSetLeaseRenewSchedulingPriority);

        /* Heartbeat */
        config->heartbeatExpiryTime.seconds = config->leasePeriod.seconds;
        config->heartbeatExpiryTime.nanoseconds = config->leasePeriod.nanoseconds;

        config->heartbeatUpdateInterval.seconds = config->leaseRenewalPeriod.seconds;
        config->heartbeatUpdateInterval.nanoseconds = config->leaseRenewalPeriod.nanoseconds;
    }

    if (dcfg) {
        s_configurationAttrValueBoolean (config, dcfg, "Tracing", "synchronous", s_configurationSetTracingSynchronous);
        s_configurationValueString      (config, dcfg, "Tracing/Verbosity/#text", s_configurationSetTracingVerbosity);
        s_configurationValueString      (config, dcfg, "Tracing/OutputFile/#text", s_configurationSetTracingOutputFile);
        s_configurationValueBoolean     (config, dcfg, "Tracing/Timestamps/#text", s_configurationSetTracingTimestamps);
        s_configurationSetTracingRelativeTimestamps(config, dcfg, "Tracing/Timestamps", "absolute");

        /* Kernelmanager */
        s_configurationValueString(config, dcfg, "KernelManager/Scheduling/Class/#text", s_configurationSetKernelManagerSchedulingClass);
        s_configurationValueLong(config, dcfg, "KernelManager/Scheduling/Priority/#text", s_configurationSetKernelManagerSchedulingPriority);

         /* GarbageCollector */
         s_configurationValueString(config, dcfg, "GarbageCollector/Scheduling/Class/#text", s_configurationSetGCSchedulingClass);
         s_configurationValueLong(config, dcfg, "GarbageCollector/Scheduling/Priority/#text", s_configurationSetGCSchedulingPriority);

         /* ResendManager */
         s_configurationValueString(config, dcfg, "ResendManager/Scheduling/Class/#text", s_configurationSetResendManagerSchedulingClass);
         s_configurationValueLong(config, dcfg, "ResendManager/Scheduling/Priority/#text", s_configurationSetResendManagerSchedulingPriority);

         /* Heartbeat */
         s_configurationAttrValueLong(config, dcfg, "Heartbeat", "transport_priority", s_configurationSetHeartbeatTransportPriority);

         s_configurationValueFloat(config, dcfg, "Heartbeat/ExpiryTime/#text", s_configurationSetHeartbeatExpiryTime);
         s_configurationAttrValueFloat(config, dcfg, "Heartbeat/ExpiryTime", "update_factor", s_configurationSetHeartbeatUpdateInterval);

         s_configurationValueString(config, dcfg, "Heartbeat/Scheduling/Class/#text", s_configurationSetHeartbeatSchedulingClass);
         s_configurationValueLong(config, dcfg, "Heartbeat/Scheduling/Priority/#text", s_configurationSetHeartbeatSchedulingPriority);

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
                        s_configurationValueLong(config, dcfg, "ControlAndMonitoringCommandReceiver/Scheduling/Priority/#text", s_configurationSetResendManagerSchedulingPriority);
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
