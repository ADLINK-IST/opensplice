/**
 * TODO:/todo
 * 1. Check consistency between namespaces. Overlap between them is not a
 *    valid situation.
 */
#include "d_configuration.h"
#include "d__configuration.h"
#include "d__durability.h"
#include "d_durability.h"
#include "d_nameSpace.h"
#include "d_table.h"
#include "d_misc.h"
#include "d_object.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_stdlib.h"
#include "v_builtin.h"

d_configuration
d_configurationNew(
    d_durability service,
    const c_char* serviceName,
    c_long domainId)
{
    d_configuration config;
    u_cfElement   cfg;
    c_iter        iter;
    u_cfElement   element, found, domainElement;
    c_char*       attrValue;
    c_bool        success;

    config = d_configuration(os_malloc(C_SIZEOF(d_configuration)));
    d_objectInit(d_object(config), D_CONFIGURATION, d_configurationDeinit);

    if(config != NULL){
        cfg = u_participantGetConfiguration(u_participant(d_durabilityGetService(service)));

        if (cfg != NULL) {
            iter = u_cfElementXPath(cfg, "Domain");

            if(c_iterLength(iter) > 1){
                OS_REPORT_1(OS_WARNING, D_CONTEXT, 0,
                "%d Domain configurations found.",
                c_iterLength(iter));
            } else if(c_iterLength(iter) == 0){
                OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                "No Domain configuration found. Applying defaults...");
            }
            domainElement = NULL;
            element = u_cfElement(c_iterTakeFirst(iter));

            while(element) {
                if(domainElement){
                   u_cfElementFree(domainElement);
                }
                domainElement = element;
                element = u_cfElement(c_iterTakeFirst(iter));
            }
            c_iterFree(iter);

            iter  = u_cfElementXPath(cfg, "DurabilityService");

            if(c_iterLength(iter) > 1){
                OS_REPORT_2(OS_WARNING, D_CONTEXT, 0,
                "%d DurabilityService configurations found for serviceName '%s'.",
                c_iterLength(iter), serviceName);
            } else if(c_iterLength(iter) == 0){
                OS_REPORT_1(OS_WARNING, D_CONTEXT, 0,
                "No DurabilityService configurations found for serviceName '%s'. Applying defaults...",
                serviceName);
            }

            found = NULL;
            element = u_cfElement(c_iterTakeFirst(iter));

            while(element) {
                success = u_cfElementAttributeStringValue(element, "name", &attrValue);

                if(success == TRUE){
                    if(strcmp(serviceName, attrValue) == 0){
                        if(found){
                            u_cfElementFree(found);
                        }
                        found = element;
                        element = NULL;
                    }
                    os_free(attrValue);
                }
                if(element){
                    u_cfElementFree(element);
                }
                element = u_cfElement(c_iterTakeFirst(iter));
            }
            d_configurationInit(config, service, domainElement, found);

            if(found){
                u_cfElementFree(found);
            }
            if(domainElement){
                u_cfElementFree(domainElement);
            }
            c_iterFree(iter);
            u_cfElementFree(cfg);
        }
    }
    return config;
}

void
d_configurationDeinit(
    d_object object)
{
    d_configuration configuration;
    d_nameSpace ns;
    c_char* name;

    assert(d_objectIsValid(object, D_CONFIGURATION) == TRUE);

    if(object){
        configuration = d_configuration(object);

        if(configuration->persistentStoreDirectory){
            os_free(configuration->persistentStoreDirectory);
        }
        if(configuration->nameSpaces){
            ns = d_nameSpace(c_iterTakeFirst(configuration->nameSpaces));

            while(ns){
                d_nameSpaceFree(ns);
                ns = d_nameSpace(c_iterTakeFirst(configuration->nameSpaces));
            }
            c_iterFree(configuration->nameSpaces);
        }
        if(configuration->networkServiceNames){
            d_tableFree(configuration->networkServiceNames);
        }
        if(configuration->services){
            name = (c_char*)(c_iterTakeFirst(configuration->services));

            while(name){
                os_free(name);
                name = (c_char*)(c_iterTakeFirst(configuration->services));
            }
            c_iterFree(configuration->services);
            configuration->services = NULL;
        }
        if(configuration->publisherName){
            os_free(configuration->publisherName);
            configuration->publisherName = NULL;
        }
        if(configuration->subscriberName){
            os_free(configuration->subscriberName);
            configuration->subscriberName = NULL;
        }
        if(configuration->partitionName){
            os_free(configuration->partitionName);
            configuration->partitionName = NULL;
        }
        if(configuration->tracingOutputFileName){
            if( (strcmp(configuration->tracingOutputFileName, "stdout") != 0) &&
                (strcmp(configuration->tracingOutputFileName, "stderr") != 0))
            {
                if(configuration->tracingOutputFile){
                    fclose(configuration->tracingOutputFile);
                    configuration->tracingOutputFile = NULL;
                }
            }
            os_free(configuration->tracingOutputFileName);
            configuration->tracingOutputFileName = NULL;
        }
    }
}

void
d_configurationFree(
    d_configuration configuration)
{
    assert(d_objectIsValid(d_object(configuration), D_CONFIGURATION) == TRUE);

    if(configuration){
        d_objectFree(d_object(configuration), D_CONFIGURATION);
    }
}

void
d_configurationInit(
    d_configuration config,
    d_durability durability,
    u_cfElement domainElement,
    u_cfElement element)
{
    d_nameSpace ns;

    if(config != NULL){
        /** First apply all defaults. */
        d_printTimedEvent(durability, D_LEVEL_FINER,
                            D_THREAD_MAIN, "Initializing configuration...\n");

        config->persistentStoreDirectory    = NULL;
        config->persistentStoreMode         = D_STORE_TYPE_XML;
        config->partitionName               = NULL;
        config->publisherName               = NULL;
        config->subscriberName              = NULL;
        config->tracingOutputFile           = NULL;
        config->tracingOutputFileName       = NULL;
        config->tracingSynchronous          = FALSE;
        config->networkServiceNames         = d_tableNew(strcmp, os_free);
        config->services                    = c_iterNew(NULL);
        config->tracingVerbosityLevel       = D_LEVEL_INFO;
        config->nameSpaces                  = NULL;
        config->tracingTimestamps           = TRUE;
        config->tracingRelativeTimestamps   = FALSE;
        config->timeAlignment               = TRUE;
        config->startTime                   = os_timeGet();
        config->networkMaxWaitCount         = D_DEFAULT_NETWORK_MAX_WAITCOUNT;

        d_configurationSetTime(&(config->heartbeatExpiryTime), D_DEFAULT_HEARTBEAT_EXPIRY_TIME);
        config->heartbeatExpiry = D_DEFAULT_HEARTBEAT_EXPIRY_TIME;
        d_configurationSetTime(&(config->heartbeatUpdateInterval), D_DEFAULT_HEARTBEAT_UPDATE_INTERVAL*D_DEFAULT_HEARTBEAT_EXPIRY_TIME);
        d_configurationSetTime(&(config->livelinessUpdateInterval), D_DEFAULT_LIVELINESS_UPDATE_INTERVAL*D_DEFAULT_HEARTBEAT_EXPIRY_TIME);
        config->livelinessExpiry            = D_DEFAULT_LIVELINESS_EXPIRY_TIME;
        d_configurationSetDuration(&(config->livelinessExpiryTime), D_DEFAULT_LIVELINESS_EXPIRY_TIME);

        os_threadAttrInit(&config->livelinessScheduling);
        os_threadAttrInit(&config->heartbeatScheduling);
        os_threadAttrInit(&config->persistentScheduling);
        os_threadAttrInit(&config->alignerScheduling);
        os_threadAttrInit(&config->aligneeScheduling);

        d_configurationSetTimingInitialWaitPeriod               (config, D_DEFAULT_TIMING_INITIAL_WAIT_PERIOD);
        d_configurationSetNetworkWaitForAttachmentMaxWaitCount  (config, D_DEFAULT_NETWORK_MAX_WAITCOUNT);
        d_configurationSetNetworkWaitForAttachmentMaxWaitTime   (config, D_DEFAULT_NETWORK_MAX_WAITTIME);
        d_configurationSetNetworkResendTimeRange                (config, D_DEFAULT_NETWORK_SAMPLE_RESEND_RANGE);
        d_configurationSetPublisherName                         (config, D_PUBLISHER_NAME);
        d_configurationSetSubscriberName                        (config, D_SUBSCRIBER_NAME);
        d_configurationSetPartitionName                         (config, D_PARTITION_NAME);
        d_configurationSetTracingOutputFile                     (config, NULL);
        d_configurationSetPersistentStoreSleepTime              (config, D_DEFAULT_PERSISTENT_STORE_SLEEP_TIME);
        d_configurationSetPersistentStoreSessionTime            (config, D_DEFAULT_PERSISTENT_STORE_SESSION_TIME);
        d_configurationSetPersistentQueueSize                   (config, D_DEFAULT_PERSISTENT_QUEUE_SIZE);
        d_configurationSetOptimizeUpdateInterval                (config, D_DEFAULT_OPTIMIZE_INTERVAL);

        d_configurationSetInitialRequestCombinePeriod           (config, D_DEFAULT_INITIAL_REQUEST_COMBINE_PERIOD);
        d_configurationSetOperationalRequestCombinePeriod       (config, D_DEFAULT_OPERATIONAL_REQUEST_COMBINE_PERIOD);
        d_configurationSetLatencyBudget                         (config, D_DEFAULT_LATENCY_BUDGET);
        d_configurationSetHeartbeatLatencyBudget                (config, D_DEFAULT_LATENCY_BUDGET);
        d_configurationSetAlignmentLatencyBudget                (config, D_DEFAULT_LATENCY_BUDGET);
        d_configurationSetTransportPriority                     (config, D_DEFAULT_TRANSPORT_PRIORITY);
        d_configurationSetHeartbeatTransportPriority            (config, D_DEFAULT_TRANSPORT_PRIORITY);
        d_configurationSetAlignmentTransportPriority            (config, D_DEFAULT_TRANSPORT_PRIORITY);

        durability->configuration = config;

        if(element){
            d_printTimedEvent(durability, D_LEVEL_CONFIG, D_THREAD_MAIN,
                "Configuration defaults applied. Now searching for actual one...\n");

            if(domainElement){
                d_configurationValueFloat  (config, domainElement, "Lease/ExpiryTime/#text", d_configurationSetLivelinessExpiryTime);
                d_configurationSetLivelinessUpdateFactor(config, domainElement, "Lease/ExpiryTime", "update_factor");
            } else {
                OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                    "No Domain configuration found. Applying default Lease...");
            }
            d_configurationValueString (config, element, "Watchdog/Scheduling/Class/#text", d_configurationSetLivelinessSchedulingClass);
            d_configurationValueLong   (config, element, "Watchdog/Scheduling/Priority/#text", d_configurationSetLivelinessSchedulingPriority);

            d_configurationValueString (config, element, "Persistent/StoreDirectory/#text", d_configurationSetPersistentStoreDirectory);

            d_configurationValueString (config, element, "Persistent/Scheduling/Class/#text", d_configurationSetPersistentSchedulingClass);
            d_configurationValueLong   (config, element, "Persistent/Scheduling/Priority/#text", d_configurationSetPersistentSchedulingPriority);

            d_configurationValueFloat  (config, element, "Persistent/StoreSleepTime/#text", d_configurationSetPersistentStoreSleepTime);
            d_configurationValueULong  (config, element, "Persistent/QueueSize/#text", d_configurationSetPersistentQueueSize);
            d_configurationValueString (config, element, "Persistent/StoreMode/#text", d_configurationSetPersistentStoreMode);
            d_configurationValueULong  (config, element, "Persistent/StoreOptimizeInterval/#text", d_configurationSetOptimizeUpdateInterval);
            d_configurationValueFloat  (config, element, "Persistent/StoreSessionTime/#text", d_configurationSetPersistentStoreSessionTime);

            d_configurationValueString (config, element, "EntityNames/Partition/#text", d_configurationSetPartitionName);
            d_configurationValueString (config, element, "EntityNames/Publisher/#text", d_configurationSetPublisherName);
            d_configurationValueString (config, element, "EntityNames/Subscriber/#text", d_configurationSetSubscriberName);

            d_configurationAttrValueBoolean (config, element, "Tracing", "synchronous", d_configurationSetTracingSynchronous);
            d_configurationValueString      (config, element, "Tracing/Verbosity/#text", d_configurationSetTracingVerbosity);
            d_configurationValueString      (config, element, "Tracing/OutputFile/#text", d_configurationSetTracingOutputFile);
            d_configurationValueBoolean     (config, element, "Tracing/Timestamps/#text", d_configurationSetTracingTimestamps);
            d_configurationSetTracingRelativeTimestamps(config, element, "Tracing/Timestamps", "absolute");

            d_configurationValueFloat  (config, element, "Network/Heartbeat/ExpiryTime/#text", d_configurationSetHeartbeatExpiryTime);
            d_configurationSetHeartbeatUpdateFactor(config, element,"Network/Heartbeat/ExpiryTime", "update_factor");
            d_configurationValueString (config, element, "Network/Heartbeat/Scheduling/Class/#text", d_configurationSetHeartbeatSchedulingClass);
            d_configurationValueLong   (config, element, "Network/Heartbeat/Scheduling/Priority/#text", d_configurationSetHeartbeatSchedulingPriority);

            d_configurationValueFloat  (config, element, "Network/InitialDiscoveryPeriod/#text", d_configurationSetTimingInitialWaitPeriod);
            d_configurationValueBoolean(config, element, "Network/Alignment/TimeAlignment/#text", d_configurationSetTimeAlignment);
            d_configurationValueFloat  (config, element, "Network/Alignment/RequestCombinePeriod/Initial/#text", d_configurationSetInitialRequestCombinePeriod);
            d_configurationValueFloat  (config, element, "Network/Alignment/RequestCombinePeriod/Operational/#text", d_configurationSetOperationalRequestCombinePeriod);
            d_configurationValueString (config, element, "Network/Alignment/AlignerScheduling/Class/#text", d_configurationSetAlignerSchedulingClass);
            d_configurationValueLong   (config, element, "Network/Alignment/AlignerScheduling/Priority/#text", d_configurationSetAlignerSchedulingPriority);
            d_configurationValueString (config, element, "Network/Alignment/AligneeScheduling/Class/#text", d_configurationSetAligneeSchedulingClass);
            d_configurationValueLong   (config, element, "Network/Alignment/AligneeScheduling/Priority/#text", d_configurationSetAligneeSchedulingPriority);

            d_configurationAttrValueLong(config, element, "Network", "transport_priority", d_configurationSetTransportPriority);
            /*Take over default transport priority first...*/
            d_configurationAttrValueLong(config, element, "Network", "transport_priority", d_configurationSetHeartbeatTransportPriority);
            d_configurationAttrValueLong(config, element, "Network", "transport_priority", d_configurationSetAlignmentTransportPriority);
            /*Now override with actual values if available.*/
            d_configurationAttrValueLong(config, element, "Network/Heartbeat", "transport_priority", d_configurationSetHeartbeatTransportPriority);
            d_configurationAttrValueLong(config, element, "Network/Alignment", "transport_priority", d_configurationSetAlignmentTransportPriority);

            d_configurationAttrValueFloat(config, element, "Network", "latency_budget", d_configurationSetLatencyBudget);
            /*Take over default latency budget first...*/
            d_configurationAttrValueFloat(config, element, "Network", "latency_budget", d_configurationSetHeartbeatLatencyBudget);
            d_configurationAttrValueFloat(config, element, "Network", "latency_budget", d_configurationSetAlignmentLatencyBudget);
            /*Now override with actual values if available.*/
            d_configurationAttrValueFloat(config, element, "Network/Heartbeat", "latency_budget", d_configurationSetHeartbeatLatencyBudget);
            d_configurationAttrValueFloat(config, element, "Network/Alignment", "latency_budget", d_configurationSetAlignmentLatencyBudget);

            d_configurationValueFloat  (config, element, "Network/ResendTimeRange/#text", d_configurationSetNetworkResendTimeRange);
            d_configurationSetNetworkWaitForAttachment(config, element, "Network/WaitForAttachment", "ServiceName/#text");

            config->nameSpaces = d_configurationResolveNameSpaces(element, "NameSpaces/NameSpace");
        } else {
            d_printTimedEvent(durability, D_LEVEL_CONFIG, D_THREAD_MAIN,
                "Configuration defaults applied. No actual one found...\n");
        }

        if(c_iterLength(config->nameSpaces) == 0) {
            ns = d_nameSpaceNew("NoName", D_ALIGNEE_INITIAL_AND_ALIGNER, D_DURABILITY_ALL);
            d_nameSpaceAddElement(ns, "all", "*", "*");
            config->nameSpaces = c_iterInsert(config->nameSpaces, ns);
        }

        d_configurationReport(config, durability);
    }
}

static void
d_configurationNameSpacesCombine(
    c_voidp object,
    c_voidp userData)
{
    d_nameSpace ns;
    c_char* result;
    c_char* partitions;

    const c_char* alignee;
    const c_char* kind;

    ns = d_nameSpace(object);
    result = (c_char*)userData;
    partitions = d_nameSpaceGetPartitions(ns);

    switch(d_nameSpaceGetDurabilityKind(ns)){
        case D_DURABILITY_ALL:
            kind = "ALL";
            break;
        case D_DURABILITY_PERSISTENT:
            kind = "PERSISTENT";
            break;
        case D_DURABILITY_TRANSIENT:
            kind = "TRANSIENT";
            break;
        default:
            kind = "NOT_VALID";
            assert(FALSE);
            break;
    }
    switch(d_nameSpaceGetAlignmentKind(ns)){
        case D_ALIGNEE_INITIAL_AND_ALIGNER:
            alignee = "INITIAL_AND_ALIGNER";
            break;
        case D_ALIGNEE_INITIAL:
            alignee = "INITIAL";
            break;
        case D_ALIGNEE_LAZY:
            alignee = "LAZY";
            break;
        case D_ALIGNEE_ON_REQUEST:
            alignee = "ON_REQUEST";
            break;
        default:
            alignee = "<<UNKNOWN>>";
            assert(FALSE);
            break;
    }
    sprintf(result,
                "%s" \
                "    - NameSpace:\n" \
                "        - AlignmentKind  : %s\n" \
                "        - DurabilityKind : %s\n" \
                "        - Partitions     : %s\n",
                result, alignee, kind, partitions);

    os_free(partitions);

    return;
}

static c_bool
isBuiltinGroup(
    d_partition partition,
    d_topic topic)
{
    c_bool result = FALSE;
    assert(partition);
    assert(topic);

    if(strcmp(partition, V_BUILTIN_PARTITION) == 0){
        if( (strcmp(topic, V_PARTICIPANTINFO_NAME) == 0) ||
            (strcmp(topic, V_TOPICINFO_NAME) == 0) ||
            (strcmp(topic, V_PUBLICATIONINFO_NAME) == 0) ||
            (strcmp(topic, V_SUBSCRIPTIONINFO_NAME) == 0))
        {
            result = TRUE;
        }
    }
    return result;
}

static c_bool
d_configurationServiceNamesCombine(
    c_char* serviceName,
    c_voidp userData)
{
    c_char* result;

    result = (c_char*)userData;

    sprintf(result, "%s    - %s\n", result, serviceName);

    return TRUE;
}

void
d_configurationReport(
    d_configuration config,
    d_durability durability)
{
    const c_char* class, *class2;
    const c_char* pstoreDir;
    const c_char* pstoreMode;
    const c_char* verbosity;
    const c_char* timestamps;
    const c_char* timeAlignment;
    const c_char* relativeTimestamps;
    c_char serviceNames [512];
    c_char ns [4096];

    d_printTimedEvent(durability, D_LEVEL_CONFIG, D_THREAD_MAIN, "Configuration:\n");

    switch(config->livelinessScheduling.schedClass){
        case OS_SCHED_DEFAULT:
            class = "DEFAULT";
            break;
        case OS_SCHED_REALTIME:
            class = "REALTIME";
            break;
        case OS_SCHED_TIMESHARE:
            class = "TIMESHARE";
            break;
        default:
            assert(FALSE);
            class = "UNKNOWN";
            break;
    }
    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Liveliness.ExpiryTime                       : %d.%9.9d\n" \
            "- Liveliness.UpdateInterval                   : %d.%9.9d\n" \
            "- Liveliness.Scheduling.Class                 : %s\n" \
            "- Liveliness.Scheduling.Priority              : %d\n"
            , config->livelinessExpiryTime.seconds
            , config->livelinessExpiryTime.nanoseconds
            , config->livelinessUpdateInterval.tv_sec
            , config->livelinessUpdateInterval.tv_nsec
            , class
            , config->livelinessScheduling.schedPriority);


    if(!(config->persistentStoreDirectory)){
        pstoreDir = "NULL";
    } else {
        pstoreDir = config->persistentStoreDirectory;
    }
    switch(config->persistentStoreMode){
        case D_STORE_TYPE_XML:
            pstoreMode = "XML";
            break;
        case D_STORE_TYPE_BIG_ENDIAN:
            pstoreMode = "BIG ENDIAN";
            break;
        case D_STORE_TYPE_UNKNOWN:
            pstoreMode = "UNKNOWN";
            break;
        default:
            assert(FALSE);
            pstoreMode = "UNKNOWN";
            break;
    }

    switch(config->persistentScheduling.schedClass){
        case OS_SCHED_DEFAULT:
            class = "DEFAULT";
            break;
        case OS_SCHED_REALTIME:
            class = "REALTIME";
            break;
        case OS_SCHED_TIMESHARE:
            class = "TIMESHARE";
            break;
        default:
            assert(FALSE);
            class = "UNKNOWN";
            break;
    }

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Persistent.StoreDirectory                   : %s\n" \
            "- Persistent.StoreMode                        : %s\n" \
            "- Persistent.QueueSize                        : %u\n" \
            "- Persistent.StoreSleepTime                   : %d.%9.9d\n" \
            "- Persistent.StoreSessionTime                 : %d.%9.9d\n" \
            "- Persistent.StoreOptimizeInterval            : %d\n" \
            "- Persistent.Scheduling.Class                 : %s\n" \
            "- Persistent.Scheduling.Priority              : %d\n" \
            , pstoreDir
            , pstoreMode
            , config->persistentQueueSize
            , config->persistentStoreSleepTime.tv_sec
            , config->persistentStoreSleepTime.tv_nsec
            , config->persistentStoreSessionTime.tv_sec
            , config->persistentStoreSessionTime.tv_nsec
            , config->persistentUpdateInterval
            , class
            , config->persistentScheduling.schedPriority);

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- EntityNames.Publisher                       : %s\n" \
            "- EntityNames.Subscriber                      : %s\n" \
            "- EntityNames.Partition                       : %s\n"
            , config->publisherName
            , config->subscriberName
            , config->partitionName);

    switch(config->tracingVerbosityLevel){
        case D_LEVEL_NONE:
            verbosity = "NONE";
            break;
        case D_LEVEL_SEVERE:
            verbosity = "SEVERE";
            break;
        case D_LEVEL_WARNING:
            verbosity = "WARNING";
            break;
        case D_LEVEL_CONFIG:
            verbosity = "CONFIG";
            break;
        case D_LEVEL_INFO:
            verbosity = "INFO";
            break;
        case D_LEVEL_FINE:
            verbosity = "FINE";
            break;
        case D_LEVEL_FINER:
            verbosity = "FINER";
            break;
        case D_LEVEL_FINEST:
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
    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Tracing.Verbosity                           : %s\n" \
            "- Tracing.OutputFile                          : %s\n" \
            "- Tracing.Timestamps                          : %s\n" \
            "- Tracing.RelativeTimestamps                  : %s\n"
            , verbosity
            , config->tracingOutputFileName
            , timestamps
            , relativeTimestamps);

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Network.InitialDiscoveryPeriod              : %d.%d\n" \
            "- Network.LatencyBudget                       : %d.%9.9d\n" \
            "- Network.TransportPriority                   : %d\n"
            , config->timingInitialWaitPeriod.tv_sec
            , config->timingInitialWaitPeriod.tv_nsec
            , config->latencyBudget.seconds
            , config->latencyBudget.nanoseconds
            , config->transportPriority);

    switch(config->heartbeatScheduling.schedClass){
        case OS_SCHED_DEFAULT:
            class = "DEFAULT";
            break;
        case OS_SCHED_REALTIME:
            class = "REALTIME";
            break;
        case OS_SCHED_TIMESHARE:
            class = "TIMESHARE";
            break;
        default:
            assert(FALSE);
            class = "UNKNOWN";
            break;
    }

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Network.Heartbeat.ExpiryTime                : %d.%9.9d\n" \
            "- Network.Heartbeat.UpdateInterval            : %d.%9.9d\n" \
            "- Network.Heartbeat.LatencyBudget             : %d.%9.9d\n" \
            "- Network.Heartbeat.TransportPriority         : %d\n" \
            "- Network.Heartbeat.Scheduling.Class          : %s\n" \
            "- Network.Heartbeat.Scheduling.Priority       : %d\n" \
            , config->heartbeatExpiryTime.tv_sec
            , config->heartbeatExpiryTime.tv_nsec
            , config->heartbeatUpdateInterval.tv_sec
            , config->heartbeatUpdateInterval.tv_nsec
            , config->heartbeatLatencyBudget.seconds
            , config->heartbeatLatencyBudget.nanoseconds
            , config->heartbeatTransportPriority
            , class
            , config->heartbeatScheduling.schedPriority
            );

    switch(config->alignerScheduling.schedClass){
        case OS_SCHED_DEFAULT:
            class = "DEFAULT";
            break;
        case OS_SCHED_REALTIME:
            class = "REALTIME";
            break;
        case OS_SCHED_TIMESHARE:
            class = "TIMESHARE";
            break;
        default:
            assert(FALSE);
            class = "UNKNOWN";
            break;
    }

    switch(config->aligneeScheduling.schedClass){
        case OS_SCHED_DEFAULT:
            class2 = "DEFAULT";
            break;
        case OS_SCHED_REALTIME:
            class2 = "REALTIME";
            break;
        case OS_SCHED_TIMESHARE:
            class2 = "TIMESHARE";
            break;
        default:
            assert(FALSE);
            class2 = "UNKNOWN";
                break;
        }

    if(config->timeAlignment == TRUE){
        timeAlignment = "TRUE";
    } else {
        timeAlignment = "FALSE";
    }

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Network.TimeAlignment                       : %s\n" \
            "- Network.Alignment.RequestCombine.Initial    : %d.%9.9d\n" \
            "- Network.Alignment.RequestCombine.Operational: %d.%9.9d\n" \
            "- Network.Alignment.LatencyBudget             : %d.%9.9d\n" \
            "- Network.Alignment.TransportPriority         : %d\n"  \
            "- Network.Alignment.AlignerScheduling.Class   : %s\n"  \
            "- Network.Alignment.AlignerScheduling.Priority: %d\n"  \
            "- Network.Alignment.AligneeScheduling.Class   : %s\n"  \
            "- Network.Alignment.AligneeScheduling.Priority: %d\n"  \
            , timeAlignment
            , config->initialRequestCombinePeriod.tv_sec
            , config->initialRequestCombinePeriod.tv_nsec
            , config->operationalRequestCombinePeriod.tv_sec
            , config->operationalRequestCombinePeriod.tv_nsec
            , config->alignerLatencyBudget.seconds
            , config->alignerLatencyBudget.nanoseconds
            , config->alignerTransportPriority
            , class
            , config->alignerScheduling.schedPriority
            , class2
            , config->aligneeScheduling.schedPriority);

    serviceNames[0] = '\0';
    d_tableWalk(config->networkServiceNames, d_configurationServiceNamesCombine, &serviceNames);

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Network.ResendTimeRange                     : %d.%9.9d\n" \
            "- Network.WaitForAttachment.MaxWaitCount      : %u\n" \
            "- Network.WaitForAttachment.ServiceNames      : \n%s"
            , config->networkSampleResendTimeRange.tv_sec
            , config->networkSampleResendTimeRange.tv_nsec
            , config->networkMaxWaitCount
            , serviceNames
            );

    ns[0] = '\0';
    c_iterWalk(config->nameSpaces, d_configurationNameSpacesCombine, &ns);

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- NameSpaces                                  :\n%s"
            , ns
            );
}

void
d_configurationSetLivelinessExpiryTime(
    d_configuration config,
    c_float seconds)
{
    c_float sec;

    sec = seconds;

    if (sec < D_MINIMUM_LIVELINESS_EXPIRY_TIME) {
        sec = D_MINIMUM_LIVELINESS_EXPIRY_TIME;
    }
    if (sec > D_MAXIMUM_LIVELINESS_EXPIRY_TIME) {
        sec = D_MAXIMUM_LIVELINESS_EXPIRY_TIME;
    }
    config->livelinessExpiry = sec;
    d_configurationSetDuration(&(config->livelinessExpiryTime), sec);
}

void
d_configurationSetLivelinessUpdateFactor(
    d_configuration config,
    u_cfElement element,
    const c_char* expiryTimePath,
    const c_char* updateFactorName)
{
    c_float sec;
    u_cfElement expiryElement;
    c_iter elements;
    c_bool success;

    elements = u_cfElementXPath(element, expiryTimePath);

    if(elements){
        expiryElement = u_cfElement(c_iterTakeFirst(elements));

        while(expiryElement){
            success = u_cfElementAttributeFloatValue(expiryElement, updateFactorName, &sec);

            if(success == TRUE){
                if (sec < D_MINIMUM_LIVELINESS_UPDATE_INTERVAL) {
                    sec = D_MINIMUM_LIVELINESS_UPDATE_INTERVAL;
                }
                if (sec > D_MAXIMUM_LIVELINESS_UPDATE_INTERVAL) {
                    sec = D_MAXIMUM_LIVELINESS_UPDATE_INTERVAL;
                }
                sec = config->livelinessExpiry * sec;
                d_configurationSetTime(&(config->livelinessUpdateInterval), sec);
            }
            u_cfElementFree(expiryElement);
            expiryElement = u_cfElement(c_iterTakeFirst(elements));
        }
        c_iterFree(elements);
    }
}

void
d_configurationSetLivelinessSchedulingClass(
    d_configuration config,
    const c_char* class)
{
    if(os_strcasecmp(class, "Timeshare") == 0){
        config->livelinessScheduling.schedClass = OS_SCHED_TIMESHARE;
    } else if(os_strcasecmp(class, "Realtime") == 0){
        config->livelinessScheduling.schedClass = OS_SCHED_REALTIME;
    } else {
        config->livelinessScheduling.schedClass = OS_SCHED_DEFAULT;
    }
}

void
d_configurationSetLivelinessSchedulingPriority(
    d_configuration config,
    c_long priority)
{
    config->livelinessScheduling.schedPriority = priority;
}

void
d_configurationSetHeartbeatSchedulingClass(
    d_configuration config,
    const c_char* class)
{
    if(os_strcasecmp(class, "Timeshare") == 0){
        config->heartbeatScheduling.schedClass = OS_SCHED_TIMESHARE;
    } else if(os_strcasecmp(class, "Realtime") == 0){
        config->heartbeatScheduling.schedClass = OS_SCHED_REALTIME;
    } else {
        config->heartbeatScheduling.schedClass = OS_SCHED_DEFAULT;
    }
}

void
d_configurationSetHeartbeatSchedulingPriority(
    d_configuration config,
    c_long priority)
{
    config->heartbeatScheduling.schedPriority = priority;
}

void
d_configurationSetPersistentSchedulingClass(
    d_configuration config,
    const c_char* class)
{
    if(os_strcasecmp(class, "Timeshare") == 0){
        config->persistentScheduling.schedClass = OS_SCHED_TIMESHARE;
    } else if(os_strcasecmp(class, "Realtime") == 0){
        config->persistentScheduling.schedClass = OS_SCHED_REALTIME;
    } else {
        config->persistentScheduling.schedClass = OS_SCHED_DEFAULT;
    }
}

void
d_configurationSetPersistentSchedulingPriority(
    d_configuration config,
    c_long priority)
{
    config->persistentScheduling.schedPriority = priority;
}

void
d_configurationSetAlignerSchedulingClass(
    d_configuration config,
    const c_char* class)
{
    if(os_strcasecmp(class, "Timeshare") == 0){
        config->alignerScheduling.schedClass = OS_SCHED_TIMESHARE;
    } else if(os_strcasecmp(class, "Realtime") == 0){
        config->alignerScheduling.schedClass = OS_SCHED_REALTIME;
    } else {
        config->alignerScheduling.schedClass = OS_SCHED_DEFAULT;
    }
}

void
d_configurationSetAlignerSchedulingPriority(
    d_configuration config,
    c_long priority)
{
    config->alignerScheduling.schedPriority = priority;
}

void
d_configurationSetAligneeSchedulingClass(
    d_configuration config,
    const c_char* class)
{
    if(os_strcasecmp(class, "Timeshare") == 0){
        config->aligneeScheduling.schedClass = OS_SCHED_TIMESHARE;
    } else if(os_strcasecmp(class, "Realtime") == 0){
        config->aligneeScheduling.schedClass = OS_SCHED_REALTIME;
    } else {
        config->aligneeScheduling.schedClass = OS_SCHED_DEFAULT;
    }
}

void
d_configurationSetAligneeSchedulingPriority(
    d_configuration config,
    c_long priority)
{
    config->aligneeScheduling.schedPriority = priority;
}

void
d_configurationSetHeartbeatUpdateFactor(
    d_configuration config,
    u_cfElement element,
    const c_char* expiryTimePath,
    const c_char* updateFactorName)
{
    c_float sec;
    u_cfElement expiryElement;
    c_iter elements;
    c_bool success;

    elements = u_cfElementXPath(element, expiryTimePath);

    if(elements){
        expiryElement = u_cfElement(c_iterTakeFirst(elements));

        while(expiryElement){
            success = u_cfElementAttributeFloatValue(expiryElement, updateFactorName, &sec);

            if(success == TRUE){
                if (sec < D_MINIMUM_HEARTBEAT_UPDATE_INTERVAL) {
                    sec = D_MINIMUM_HEARTBEAT_UPDATE_INTERVAL;
                }
                if (sec > D_MAXIMUM_HEARTBEAT_UPDATE_INTERVAL) {
                    sec = D_MAXIMUM_HEARTBEAT_UPDATE_INTERVAL;
                }
                sec = config->heartbeatExpiry * sec;
                d_configurationSetTime(&(config->heartbeatUpdateInterval), sec);
            }
            u_cfElementFree(expiryElement);
            expiryElement = u_cfElement(c_iterTakeFirst(elements));
        }
        c_iterFree(elements);
    }
}

void
d_configurationSetHeartbeatExpiryTime(
    d_configuration config,
    c_float seconds)
{
    c_float sec;

    sec = seconds;

    if (sec < D_MINIMUM_HEARTBEAT_EXPIRY_TIME) {
        sec = D_MINIMUM_HEARTBEAT_EXPIRY_TIME;
    }
    if (sec > D_MAXIMUM_HEARTBEAT_EXPIRY_TIME) {
        sec = D_MAXIMUM_HEARTBEAT_EXPIRY_TIME;
    }
    d_configurationSetTime(&(config->heartbeatExpiryTime), sec);
    config->heartbeatExpiry     = sec;
}

void
d_configurationSetTimingInitialWaitPeriod(
    d_configuration config,
    c_float seconds)
{
    c_float sec;

    sec = seconds;

    if (sec < D_MINIMUM_TIMING_INITIAL_WAIT_PERIOD) {
        sec = D_MINIMUM_TIMING_INITIAL_WAIT_PERIOD;
    }
    if (sec > D_MAXIMUM_TIMING_INITIAL_WAIT_PERIOD) {
        sec = D_MAXIMUM_TIMING_INITIAL_WAIT_PERIOD;
    }
    d_configurationSetTime(&(config->timingInitialWaitPeriod), sec);
}

void
d_configurationSetLatencyBudget(
    d_configuration config,
    c_float seconds)
{
    c_float sec;

    sec = seconds;

    if (sec < D_MINIMUM_LATENCY_BUDGET) {
        sec = D_MINIMUM_LATENCY_BUDGET;
    }
    if (sec > D_MAXIMUM_LATENCY_BUDGET) {
        sec = D_MAXIMUM_LATENCY_BUDGET;
    }
    d_configurationSetDuration(&(config->latencyBudget), sec);
}

void
d_configurationSetHeartbeatLatencyBudget(
    d_configuration config,
    c_float seconds)
{
    c_float sec;

    sec = seconds;

    if (sec < D_MINIMUM_LATENCY_BUDGET) {
        sec = D_MINIMUM_LATENCY_BUDGET;
    }
    if (sec > D_MAXIMUM_LATENCY_BUDGET) {
        sec = D_MAXIMUM_LATENCY_BUDGET;
    }
    d_configurationSetDuration(&(config->heartbeatLatencyBudget), sec);
}

void
d_configurationSetAlignmentLatencyBudget(
    d_configuration config,
    c_float seconds)
{
    c_float sec;

    sec = seconds;

    if (sec < D_MINIMUM_LATENCY_BUDGET) {
        sec = D_MINIMUM_LATENCY_BUDGET;
    }
    if (sec > D_MAXIMUM_LATENCY_BUDGET) {
        sec = D_MAXIMUM_LATENCY_BUDGET;
    }
    d_configurationSetDuration(&(config->alignerLatencyBudget), sec);
}


void
d_configurationSetTransportPriority(
    d_configuration config,
    c_long prio)
{
    c_long p;

    p = prio;

    if (p < D_MINIMUM_TRANSPORT_PRIORITY) {
        p = D_MINIMUM_TRANSPORT_PRIORITY;
    }
    if (p > D_MAXIMUM_TRANSPORT_PRIORITY) {
        p = D_MAXIMUM_TRANSPORT_PRIORITY;
    }
    config->transportPriority = p;
}

void
d_configurationSetHeartbeatTransportPriority(
    d_configuration config,
    c_long prio)
{
    c_long p;

    p = prio;

    if (p < D_MINIMUM_TRANSPORT_PRIORITY) {
        p = D_MINIMUM_TRANSPORT_PRIORITY;
    }
    if (p > D_MAXIMUM_TRANSPORT_PRIORITY) {
        p = D_MAXIMUM_TRANSPORT_PRIORITY;
    }
    config->heartbeatTransportPriority = p;
}

void
d_configurationSetAlignmentTransportPriority(
    d_configuration config,
    c_long prio)
{
    c_long p;

    p = prio;

    if (p < D_MINIMUM_TRANSPORT_PRIORITY) {
        p = D_MINIMUM_TRANSPORT_PRIORITY;
    }
    if (p > D_MAXIMUM_TRANSPORT_PRIORITY) {
        p = D_MAXIMUM_TRANSPORT_PRIORITY;
    }
    config->alignerTransportPriority = p;
}

void
d_configurationSetInitialRequestCombinePeriod(
    d_configuration config,
    c_float seconds)
{
    c_float sec;

    sec = seconds;

    if (sec < D_MINIMUM_INITIAL_REQUEST_COMBINE_PERIOD) {
        sec = D_MINIMUM_INITIAL_REQUEST_COMBINE_PERIOD;
    }
    if (sec > D_MAXIMUM_INITIAL_REQUEST_COMBINE_PERIOD) {
        sec = D_MAXIMUM_INITIAL_REQUEST_COMBINE_PERIOD;
    }
    d_configurationSetTime(&(config->initialRequestCombinePeriod), sec);
}

void
d_configurationSetOperationalRequestCombinePeriod(
    d_configuration config,
    c_float seconds)
{
    c_float sec;

    sec = seconds;

    if (sec < D_MINIMUM_OPERATIONAL_REQUEST_COMBINE_PERIOD) {
        sec = D_MINIMUM_OPERATIONAL_REQUEST_COMBINE_PERIOD;
    }
    if (sec > D_MAXIMUM_OPERATIONAL_REQUEST_COMBINE_PERIOD) {
        sec = D_MAXIMUM_OPERATIONAL_REQUEST_COMBINE_PERIOD;
    }
    d_configurationSetTime(&(config->operationalRequestCombinePeriod), sec);
}

void
d_configurationSetNetworkWaitForAttachmentMaxWaitCount(
    d_configuration config,
    c_ulong maxWaits )
{
    config->networkMaxWaitCount = maxWaits;

    if (config->networkMaxWaitCount < D_MINIMUM_NETWORK_MAX_WAITCOUNT) {
        config->networkMaxWaitCount = D_MINIMUM_NETWORK_MAX_WAITCOUNT;
    }
    if (config->networkMaxWaitCount > D_MAXIMUM_NETWORK_MAX_WAITCOUNT) {
        config->networkMaxWaitCount = D_MAXIMUM_NETWORK_MAX_WAITCOUNT;
    }
    d_configurationSetNetworkWaitForAttachmentMaxWaitTime(config, (c_float)config->networkMaxWaitCount/100); /*10 ms*/
}

void
d_configurationSetNetworkWaitForAttachmentMaxWaitTime(
    d_configuration config,
    c_float maxWaits)
{
    c_float networkMaxWaitTime = maxWaits;

    if (networkMaxWaitTime < D_MINIMUM_NETWORK_MAX_WAITTIME) {
        networkMaxWaitTime = D_MINIMUM_NETWORK_MAX_WAITTIME;
    }
    if (networkMaxWaitTime > D_MAXIMUM_NETWORK_MAX_WAITTIME) {
        networkMaxWaitTime = D_MAXIMUM_NETWORK_MAX_WAITTIME;
    }
    config->networkMaxWaitTime = os_realToTime(networkMaxWaitTime);

    return;
}

void
d_configurationSetNetworkWaitForAttachment(
    d_configuration config,
    u_cfElement  elementParent,
    const c_char* attachName,
    const c_char* service)
{
    c_iter      attachIter, iter;
    u_cfElement element;
    u_cfData    data;
    c_ulong     maxWaitCount;
    c_bool      success;
    c_float     maxWaitTime;
    c_char*     serviceName;

    attachIter = u_cfElementXPath(elementParent, attachName);
    element = u_cfElement(c_iterTakeFirst(attachIter));

    if(element){
        success = u_cfElementAttributeULongValue(element, "maxWaitCount", &maxWaitCount);

        if(success){
            d_configurationSetNetworkWaitForAttachmentMaxWaitCount(config, maxWaitCount);
        }
        success = u_cfElementAttributeFloatValue(element, "maxWaitTime", &maxWaitTime);

        if(success){
            d_configurationSetNetworkWaitForAttachmentMaxWaitTime(config, maxWaitTime);
        }
        iter = u_cfElementXPath(element, service);
        data = u_cfData(c_iterTakeFirst(iter));

        while(data) {
            success = u_cfDataStringValue(data, &serviceName);

            if (success == TRUE) {
               d_tableInsert(config->networkServiceNames, serviceName);
               config->services = c_iterInsert(config->services, os_strdup(serviceName));
            }
            u_cfDataFree(data);
            data = u_cfData(c_iterTakeFirst(iter));
        }
        c_iterFree(iter);
        u_cfElementFree(element);

        element = u_cfElement(c_iterTakeFirst(attachIter));

        while(element){
            u_cfElementFree(element);
            element = u_cfElement(c_iterTakeFirst(attachIter));
        }
    }
    c_iterFree(attachIter);
}

void
d_configurationSetNetworkResendTimeRange(
    d_configuration config,
    c_float seconds )
{
    c_float sec;

    sec = seconds;

    if (sec < D_MINIMUM_NETWORK_SAMPLE_RESEND_RANGE) {
        sec = D_MINIMUM_NETWORK_SAMPLE_RESEND_RANGE;
    }
    if (sec > D_MAXIMUM_NETWORK_SAMPLE_RESEND_RANGE) {
        sec = D_MAXIMUM_NETWORK_SAMPLE_RESEND_RANGE;
    }
    d_configurationSetTime(&(config->networkSampleResendTimeRange), sec);
}

void
d_configurationSetPublisherName(
    d_configuration  config,
    const c_char * publisherName)
{
    if (config) {
        if (publisherName != NULL) {
            if(config->publisherName) {
                d_free(config->publisherName);
                config->publisherName = NULL;
            }
            config->publisherName = os_strdup(publisherName);
        }
    }
}

void
d_configurationSetSubscriberName(
    d_configuration  config,
    const c_char * subscriberName)
{
    if (config) {
        if (subscriberName != NULL) {
            if(config->subscriberName) {
                d_free(config->subscriberName);
                config->subscriberName = NULL;
            }
            config->subscriberName = os_strdup(subscriberName);
        }
    }
}

void
d_configurationSetPartitionName(
    d_configuration  config,
    const c_char * partitionName)
{
    if (config) {
        if (partitionName != NULL) {
            if(config->partitionName) {
                d_free(config->partitionName);
                config->partitionName = NULL;
            }
            config->partitionName = os_strdup(partitionName);
        }
    }
}

void
d_configurationSetTracingSynchronous(
    d_configuration config,
    const c_bool synchronous)
{
    config->tracingSynchronous = synchronous;
}

void
d_configurationSetTracingOutputFile(
    d_configuration config,
    const c_char* value)
{
    if(value){
        if(config->tracingOutputFileName){
            if( (os_strcasecmp(config->tracingOutputFileName, "stdout") != 0) &&
                (os_strcasecmp(config->tracingOutputFileName, "stderr") != 0))
            {
                if(config->tracingOutputFile){
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
        } else if (os_strcasecmp(value, "stderr") == 0) {
            config->tracingOutputFileName = os_strdup("stderr");
            config->tracingOutputFile = stderr;
        } else {
            char * filename = os_fileNormalize(value);
            config->tracingOutputFile = fopen(filename, "a");
            config->tracingOutputFileName = os_strdup(filename);
        }
    }
}

void
d_configurationSetTracingTimestamps(
    d_configuration  config,
    c_bool useTimestamp)
{
    if (config) {
        config->tracingTimestamps = useTimestamp;
    }
}

void
d_configurationSetTimeAlignment(
    d_configuration  config,
    c_bool alignment)
{
    if (config) {
        config->timeAlignment = alignment;
    }
}

void
d_configurationSetTracingRelativeTimestamps(
    d_configuration config,
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

void
d_configurationSetTracingVerbosity(
    d_configuration config,
    const c_char* value)
{
    if(value){
        if(os_strcasecmp(value, "SEVERE") == 0){
            config->tracingVerbosityLevel = D_LEVEL_SEVERE;
        } else if(os_strcasecmp(value, "WARNING") == 0){
            config->tracingVerbosityLevel = D_LEVEL_WARNING;
        } else if(os_strcasecmp(value, "INFO") == 0){
            config->tracingVerbosityLevel = D_LEVEL_INFO;
        } else if(os_strcasecmp(value, "CONFIG") == 0){
            config->tracingVerbosityLevel = D_LEVEL_CONFIG;
        } else if(os_strcasecmp(value, "FINE") == 0){
            config->tracingVerbosityLevel = D_LEVEL_FINE;
        } else if(os_strcasecmp(value, "FINER") == 0){
            config->tracingVerbosityLevel = D_LEVEL_FINER;
        } else if(os_strcasecmp(value, "FINEST") == 0){
            config->tracingVerbosityLevel = D_LEVEL_FINEST;
        } else if(os_strcasecmp(value, "NONE") == 0){
            config->tracingVerbosityLevel = D_LEVEL_NONE;
        }
    }
}

void
d_configurationSetPersistentStoreDirectory(
    d_configuration config,
    const c_char* storePath)
{
    if(config){
        if(config->persistentStoreDirectory != NULL){
            os_free(config->persistentStoreDirectory);
        }
        config->persistentStoreDirectory = os_fileNormalize(storePath);
    }
}

void
d_configurationSetPersistentStoreMode(
    d_configuration  config,
    const c_char * storeModeName)
{
    if (config) {
        if (storeModeName != NULL) {
            if(os_strcasecmp(storeModeName, "XML") == 0){
                config->persistentStoreMode = D_STORE_TYPE_XML;
            } else {
                config->persistentStoreMode = D_STORE_TYPE_XML;
            }
        }
    }
}

void
d_configurationSetPersistentStoreSleepTime(
    d_configuration config,
    c_float seconds)
{
    c_float sec;

    sec = seconds;

    if (sec < D_MINIMUM_PERSISTENT_STORE_SLEEP_TIME) {
        sec = D_MINIMUM_PERSISTENT_STORE_SLEEP_TIME;
    }
    if (sec > D_MAXIMUM_PERSISTENT_STORE_SLEEP_TIME) {
        sec = D_MAXIMUM_PERSISTENT_STORE_SLEEP_TIME;
    }
    d_configurationSetTime(&(config->persistentStoreSleepTime), sec);
}

void
d_configurationSetPersistentStoreSessionTime(
    d_configuration config,
    c_float seconds)
{
    c_float sec;

    sec = seconds;

    if (sec < D_MINIMUM_PERSISTENT_STORE_SESSION_TIME) {
        sec = D_MINIMUM_PERSISTENT_STORE_SESSION_TIME;
    }
    if (sec > D_MAXIMUM_PERSISTENT_STORE_SESSION_TIME) {
        sec = D_MAXIMUM_PERSISTENT_STORE_SESSION_TIME;
    }
    d_configurationSetTime(&(config->persistentStoreSessionTime), sec);
}

void
d_configurationSetPersistentQueueSize(
    d_configuration config,
    c_ulong size)
{
    config->persistentQueueSize = size;

    if (config->persistentQueueSize > D_MAXIMUM_PERSISTENT_QUEUE_SIZE) {
        config->persistentQueueSize = D_MAXIMUM_PERSISTENT_QUEUE_SIZE;
    }
}

void
d_configurationSetOptimizeUpdateInterval(
    d_configuration config,
    c_ulong size)
{
    config->persistentUpdateInterval = size;

    if ((config->persistentUpdateInterval < D_MINIMUM_OPTIMIZE_INTERVAL) && (config->persistentUpdateInterval != 0)) {
        config->persistentUpdateInterval = D_MINIMUM_OPTIMIZE_INTERVAL;
    }
    if (config->persistentUpdateInterval > D_MAXIMUM_OPTIMIZE_INTERVAL) {
        config->persistentUpdateInterval = D_MAXIMUM_OPTIMIZE_INTERVAL;
    }
}

void
d_configurationSetDuration(
    v_duration * timeOut,
    c_float        seconds )
{
    os_time tmp;

    tmp = os_realToTime(seconds);

    timeOut->seconds     = tmp.tv_sec;
    timeOut->nanoseconds = tmp.tv_nsec;
}

void
d_configurationSetTime(
    os_time *timeOut,
    c_float seconds )
{
    *timeOut = os_realToTime(seconds);
}

void
d_configurationResolvePartitionTopic(
    d_nameSpace  nameSpace,
    u_cfElement  element,
    c_char *     name,
    const c_char * tag,
    const c_char * topic )
{
    c_iter   iter;
    u_cfData data;
    c_char * partition;
    c_bool   found;

    iter = u_cfElementXPath(element, tag);
    data = u_cfData(c_iterTakeFirst(iter));

    while(data) {
        found = u_cfDataStringValue(data, &partition);

        if (found == TRUE) {
            d_nameSpaceAddElement(nameSpace, name, partition, topic);
            os_free(partition);
        }
        u_cfDataFree(data);
        data = u_cfData(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}

void
d_configurationResolvePartition(
    d_nameSpace nameSpace,
    u_cfElement element,
    c_char* name,
    const c_char* tag,
    const c_char* topic)
{
    c_iter iter, iter2;
    u_cfElement partitionElement;
    u_cfNode data;
    c_ulong size;
    c_bool found;
    c_char* partition;

    iter = u_cfElementXPath(element, tag);
    partitionElement = u_cfElement(c_iterTakeFirst(iter));

    while(partitionElement){
        iter2 = u_cfElementGetChildren(partitionElement);
        size = c_iterLength(iter2);

        if(size != 0){
            data = u_cfNode(c_iterTakeFirst(iter2));

            if(u_cfNodeKind(data) == V_CFDATA){
                found = u_cfDataStringValue(u_cfData(data), &partition);

                if (found == TRUE) {
                    d_nameSpaceAddElement(nameSpace, name, partition, topic);
                    os_free(partition);
                }
            }
            u_cfNodeFree(data);
        } else {
            d_nameSpaceAddElement(nameSpace, name, "", topic);
        }
        c_iterFree(iter2);
        u_cfElementFree(partitionElement);
        partitionElement = u_cfElement(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);
}

void
d_configurationAttrValueLong(
    d_configuration configuration,
    u_cfElement  element,
    const char * tag,
    const char * attr,
    void         (* const setAction)(d_configuration config, c_long longValue) )
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

        if(a){
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

void
d_configurationAttrValueFloat(
    d_configuration configuration,
    u_cfElement  element,
    const char * tag,
    const char * attr,
    void         (* const setAction)(d_configuration config, c_float floatValue) )
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

        if(a){
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

void
d_configurationAttrValueBoolean(
    d_configuration configuration,
    u_cfElement  element,
    const char * tag,
    const char * attr,
    void         (* const setAction)(d_configuration config, c_bool boolValue) )
{
    c_iter   iter;
    u_cfElement e;
    u_cfAttribute a;
    c_bool   boolValue, found;

    iter = u_cfElementXPath(element, tag);
    e = u_cfElement(c_iterTakeFirst(iter));

    while (e != NULL) {
        a = u_cfElementAttribute(e, attr);

        if(a){
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

void
d_configurationValueLong(
    d_configuration configuration,
    u_cfElement  element,
    const char * tag,
    void         (* const setAction)(d_configuration config, c_long longValue) )
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

void
d_configurationValueULong(
    d_configuration configuration,
    u_cfElement  element,
    const char * tag,
    void         (* const setAction)(d_configuration config, c_ulong longValue) )
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

void
d_configurationValueFloat(
    d_configuration configuration,
    u_cfElement  element,
    const c_char * tag,
    void         (* const setAction)(d_configuration config, c_float floatValue) )
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


void
d_configurationValueBoolean(
    d_configuration configuration,
    u_cfElement  element,
    const char * tag,
    void         (* const setAction)(d_configuration config, c_bool str) )
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

void
d_configurationValueString(
    d_configuration configuration,
    u_cfElement  element,
    const char * tag,
    void         (* const setAction)(d_configuration config, const c_char * str) )
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

c_bool
d_configurationGroupInInitialAligneeNS(
    d_configuration configuration,
    d_partition partition,
    d_topic topic,
    d_durabilityKind kind)
{
    d_nameSpace ns;
    c_bool inNameSpace;
    c_ulong count, i;

    assert(d_objectIsValid(d_object(configuration), D_CONFIGURATION) == TRUE);

    inNameSpace = FALSE;
    count       = c_iterLength(configuration->nameSpaces);

    for(i=0; (i<count) && (inNameSpace == FALSE); i++){
        ns = d_nameSpace(c_iterObject(configuration->nameSpaces, i));
        inNameSpace = d_configurationInNameSpace(ns, partition, topic, kind, FALSE);

        if(inNameSpace == TRUE){
            switch(d_nameSpaceGetAlignmentKind(ns)){
                case D_ALIGNEE_INITIAL_AND_ALIGNER:
                case D_ALIGNEE_INITIAL:
                    break;
                default:
                    inNameSpace = FALSE;
                    break;
            }

        }
    }
    if(inNameSpace == FALSE){
        inNameSpace = isBuiltinGroup(partition, topic);
    }
    return inNameSpace;
}

c_bool
d_configurationGroupInAligneeNS(
    d_configuration configuration,
    d_partition partition,
    d_topic topic,
    d_durabilityKind kind)
{
    d_nameSpace ns;
    c_bool inNameSpace;
    c_ulong count, i;

    assert(d_objectIsValid(d_object(configuration), D_CONFIGURATION) == TRUE);

    inNameSpace = FALSE;
    count       = c_iterLength(configuration->nameSpaces);

    for(i=0; (i<count) && (inNameSpace == FALSE); i++){
        ns = d_nameSpace(c_iterObject(configuration->nameSpaces, i));
        inNameSpace = d_configurationInNameSpace(ns, partition, topic, kind, FALSE);
    }
    if(inNameSpace == FALSE){
        inNameSpace = isBuiltinGroup(partition, topic);
    }
    return inNameSpace;
}

c_bool
d_configurationGroupInActiveAligneeNS(
    d_configuration configuration,
    d_partition partition,
    d_topic topic,
    d_durabilityKind kind)
{
    d_nameSpace ns;
    c_bool inNameSpace;
    c_ulong count, i;

    assert(d_objectIsValid(d_object(configuration), D_CONFIGURATION) == TRUE);

    inNameSpace = FALSE;
    count       = c_iterLength(configuration->nameSpaces);

    for(i=0; (i<count) && (inNameSpace == FALSE); i++){
        ns = d_nameSpace(c_iterObject(configuration->nameSpaces, i));
        inNameSpace = d_configurationInNameSpace(ns, partition, topic, kind, FALSE);

        if(inNameSpace){
            if(d_nameSpaceGetAlignmentKind(ns) == D_ALIGNEE_ON_REQUEST){
                inNameSpace = FALSE;
            }
        }
    }
    if(inNameSpace == FALSE){
        inNameSpace = isBuiltinGroup(partition, topic);
    }
    return inNameSpace;
}


c_bool
d_configurationGroupInAlignerNS(
    d_configuration configuration,
    d_partition partition,
    d_topic topic,
    d_durabilityKind kind)
{
    d_nameSpace ns;
    c_bool inNameSpace;
    c_ulong count, i;

    assert(d_objectIsValid(d_object(configuration), D_CONFIGURATION) == TRUE);

    inNameSpace = FALSE;
    count       = c_iterLength(configuration->nameSpaces);

    for(i=0; (i<count) && (inNameSpace == FALSE); i++){
        ns = d_nameSpace(c_iterObject(configuration->nameSpaces, i));
        inNameSpace = d_configurationInNameSpace(ns, partition, topic, kind, TRUE);
    }
    if(inNameSpace == FALSE){
        inNameSpace = isBuiltinGroup(partition, topic);
    }
    return inNameSpace;
}

c_bool
d_configurationInNameSpace(
    d_nameSpace ns,
    d_partition partition,
    d_topic topic,
    d_durabilityKind kind,
    c_bool aligner)
{
    c_bool result;
    d_durabilityKind dkind;

    result = FALSE;
    dkind = d_nameSpaceGetDurabilityKind(ns);

    switch(kind){
        case D_DURABILITY_PERSISTENT:
            switch(dkind){
                case D_DURABILITY_ALL:
                case D_DURABILITY_PERSISTENT:
                case D_DURABILITY_TRANSIENT:
                    result = TRUE;
                    break;
                default:
                    break;
            }
            break;
        case D_DURABILITY_TRANSIENT:
            switch(dkind){
                case D_DURABILITY_ALL:
                case D_DURABILITY_TRANSIENT:
                    result = TRUE;
                    break;
                default:
                    break;
            }
            break;
        case D_DURABILITY_ALL:
            result = TRUE;
            break;
        default:
            assert(FALSE);
            break;
    }
    if(result == TRUE){
        result = FALSE;

        if(d_nameSpaceIsIn(ns, partition, topic) == TRUE){
            if(aligner == TRUE) {
                if(d_nameSpaceIsAligner(ns) == TRUE){
                    result = TRUE;
                }
            } else {
                result = TRUE;
            }
        }
    }
    if(result == FALSE){
        result = isBuiltinGroup(partition, topic);
    }
    return result;
}

c_iter
d_configurationResolveNameSpaces(
    u_cfElement  elementParent,
    const c_char * nameSpaceName )
{
    c_iter      iter, result;
    u_cfElement element;
    c_char *    durabilityKind;
    c_char *    alignmentKind;
    c_bool      found;
    d_nameSpace ns;
    d_alignmentKind akind;
    d_durabilityKind dkind;

    result = c_iterNew(NULL);
    iter = u_cfElementXPath(elementParent, nameSpaceName);
    element = c_iterTakeFirst(iter);

    while (element) {
        found = u_cfElementAttributeStringValue(element, "durabilityKind", &durabilityKind);

        if(found){
            if(os_strcasecmp(durabilityKind, "TRANSIENT") == 0){
                dkind = D_DURABILITY_TRANSIENT;
            } else if(os_strcasecmp(durabilityKind, "TRANSIENT_LOCAL") == 0){
                dkind = D_DURABILITY_TRANSIENT_LOCAL;
            } else if(os_strcasecmp(durabilityKind, "PERSISTENT") == 0){
                dkind = D_DURABILITY_PERSISTENT;
            } else {
                dkind = D_DURABILITY_ALL;
            }
            os_free(durabilityKind);
        } else {
            dkind = D_DURABILITY_ALL;
        }

        found = u_cfElementAttributeStringValue(element, "alignmentKind", &alignmentKind);

        if(found){
            if(os_strcasecmp(alignmentKind, "ON_REQUEST") == 0){
                akind = D_ALIGNEE_ON_REQUEST;
            } else if(os_strcasecmp(alignmentKind, "INITIAL") == 0){
                akind = D_ALIGNEE_INITIAL;
            } else if(os_strcasecmp(alignmentKind, "LAZY") == 0){
                akind = D_ALIGNEE_LAZY;
            } else {
                akind = D_ALIGNEE_INITIAL_AND_ALIGNER;
            }
            os_free(alignmentKind);
        } else {
            akind = D_ALIGNEE_INITIAL_AND_ALIGNER;
        }

        ns = d_nameSpaceNew("NoName", akind, dkind);
        d_configurationResolvePartition(ns, element, "NoName", "Partition", "*");
        result = c_iterInsert(result, ns);

        u_cfElementFree(element);
        element = (u_cfElement)c_iterTakeFirst(iter);
    }
    c_iterFree(iter);

    return result;
}

d_nameSpace
d_configurationGetNameSpaceForGroup(
    d_configuration configuration,
    d_partition partition,
    d_topic topic,
    d_durabilityKind kind)
{
    d_nameSpace nameSpace;
    c_long i;
    d_durabilityKind dkind;
    assert(d_objectIsValid(d_object(configuration), D_CONFIGURATION) == TRUE);
    nameSpace = NULL;

    for(i=0; (i<c_iterLength(configuration->nameSpaces)) && (nameSpace == NULL); i++){
        nameSpace = d_nameSpace(c_iterObject(configuration->nameSpaces, i));

        if(d_nameSpaceIsIn(nameSpace, partition, topic) == TRUE){
            dkind = d_nameSpaceGetDurabilityKind(nameSpace);

            if(dkind != D_DURABILITY_ALL){
                if( (!((dkind == D_DURABILITY_TRANSIENT) && (kind == D_DURABILITY_PERSISTENT))) &&
                    (dkind != kind))
                {
                    nameSpace = NULL;
                }
            }
        } else {
            nameSpace = NULL;
        }
    }
    return nameSpace;
}
