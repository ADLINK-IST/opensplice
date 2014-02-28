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
#include "c_base.h"
#include "c_stringSupport.h"
#include "q_expr.h"
#include "v_kernel.h"
#include "v__kernel.h"
#include "v__observable.h"
#include "v_entity.h"
#include "v_participant.h"
#include "v_partition.h"
#include "v_publisher.h"
#include "v_subscriber.h"
#include "v_cfElement.h"
#include "v__writer.h"
#include "v_reader.h"
#include "v_entry.h"
#include "v_time.h"
#include "v_dataReaderEntry.h"
#include "v_serviceManager.h"
#include "v_groupSet.h"
#include "v_status.h"
#include "v_handle.h"
#include "v_dataReader.h"
#include "v__topic.h"
#include "v_topicQos.h"
#include "v_publisherQos.h"
#include "v_subscriberQos.h"
#include "v_writerQos.h"
#include "v_readerQos.h"
#include "v__builtin.h"
#include "v_statistics.h"
#include "v_public.h"
#include "v_kernelStatistics.h"
#include "v_kernelQos.h"
#include "v_configuration.h"
#include "v__spliced.h"
#include "v__leaseManager.h"
#include "v__crc.h"
#include "os.h"
#include "os_report.h"
#include "os_uniqueNodeId.h"
#include "v__policy.h"
#include "v__partition.h"

#include "os.h"

#define __ERROR(m) printf(m); printf("\n");

#define V_KERNEL_MAX_SAMPLES_WARN_LEVEL_DEF 5000
#define V_KERNEL_MAX_SAMPLES_WARN_LEVEL_MIN 1
#define V_KERNEL_MAX_INSTANCES_WARN_LEVEL_DEF 5000
#define V_KERNEL_MAX_INSTANCES_WARN_LEVEL_MIN 1
#define V_KERNEL_MAX_SAMPLES_PER_INSTANCES_WARN_LEVEL_DEF 5000
#define V_KERNEL_MAX_SAMPLES_PER_INSTANCES_WARN_LEVEL_MIN 1

static void
v_loadWarningLevels(
    v_kernel kernel,
    v_configuration config);

v_object
v_new(
    v_kernel kernel,
    c_type type)
{
    v_object o;
    c_type t;
    c_long i;

    assert(C_TYPECHECK(kernel,v_kernel));
    assert(C_TYPECHECK(type,c_type));

    if (type == NULL) {
        return NULL;
    }
    o = c_new(type);
    if (o) {
        o->kernel = kernel;
        t = type;
        while (t != NULL) {
            for (i=0;i<K_TYPECOUNT;i++) {
                if (t == kernel->type[i]) {
                    o->kind = i;
                    return o;
                }
            }
            if (c_baseObject(t)->kind == M_CLASS) {
                t = c_type(c_class(t)->extends);
            } else {
                t = NULL;
            }
        }
        o->kind = K_OBJECT;
    } else {
        OS_REPORT(OS_ERROR,
                  "v_new",0,
                  "Failed to create kernel object.");
        assert(FALSE);
    }
    return o;
}

v_object
v_objectNew(
    v_kernel kernel,
    v_kind kind)
{
    v_object o;

    assert(C_TYPECHECK(kernel,v_kernel));

    o = c_new(v_kernelType(kernel,kind));
    if (o) {
        assert(C_TYPECHECK(o,v_object));
        o->kind = kind;
        o->kernel = kernel;
    } else {
        OS_REPORT(OS_ERROR,
                  "v_objectNew",0,
                  "Failed to create kernel object.");
        assert(FALSE);
    }
    return o;
}

v_kernel
v_kernelAttach(
    c_base base,
    const c_char *name)
{
    v_kernel kernel = NULL;
    os_uint32 attachCount;

    if (name == NULL) {
        OS_REPORT(OS_ERROR,
                  "v_kernelAttach",0,
                  "Failed to lookup kernel, specified kernel name = <NULL>");
    } else {
        kernel = c_lookup(base,name);
        if (kernel != NULL) {
            if (c_checkType(kernel,"v_kernel") != kernel) {
                c_free(kernel);
                kernel = NULL;
                OS_REPORT_1(OS_ERROR,
                            "v_kernelAttach",0,
                            "Object '%s' is apparently not of type 'v_kernel'",
                            name);
            } else {
                attachCount = pa_increment(&kernel->userCount);
                if(attachCount == 1){
                    /* Result of the attach may NEVER be 1, as that would mean that an
                     * attach to an unreferenced kernel succeeded. If it happens, undo
                     * increment and free reference to returned kernel. */
                    pa_decrement(&kernel->userCount);
                    c_free(kernel);
                    kernel = NULL;
                    OS_REPORT_1(OS_ERROR,
                                "v_kernelAttach",0,
                                "Operation aborted: Object '%s' is apparently an "
                                "unreferenced kernel object.",
                                name);
                }
            }
        }
    }

    return kernel;
}

void
v_kernelDetach(
    v_kernel k)
{
    os_uint32 attachCount;

    assert(C_TYPECHECK(k,v_kernel));

    attachCount = pa_decrement(&k->userCount);
    /* Assert on zero-boundary crossing */
    assert(attachCount + 1 > attachCount);
}

c_long
v_kernelUserCount(
    v_kernel k)
{
    assert(C_TYPECHECK(k,v_kernel));
    return k->userCount;
}

/*
 * v_kernelNetworkCount is used to request the number of network services
 * that the kernel's configuration expects.  Currently this value is
 * determined once during kernel initialisation, but could be extended to
 * support the concept of dynamically started/stopped/restarted services,
 * This would allow the node to adapt to the changes in the configuration.
 */
c_long
v_kernelNetworkCount(
    v_kernel k)
{
    assert(C_TYPECHECK(k,v_kernel));
    return k->networkServiceCount;
}

void
v_kernelSetNetworkCount(
    v_kernel k,
    c_long count)
{
    assert(C_TYPECHECK(k,v_kernel));
    k->networkServiceCount = count;
}

c_bool
isServiceRequestedServiceKind(
    c_char * serviceDefinitionToSearch,
    c_char * requestedServiceName,
    v_configuration config
)
{
    c_iter iter;
    v_cfElement root;
    v_cfElement serviceElement;
    c_value serviceNameValue;

    c_bool result = FALSE;

    assert(config != NULL);
    assert(C_TYPECHECK(config,v_configuration));

    root = v_configurationGetRoot(config);
    iter = v_cfElementXPath(root, serviceDefinitionToSearch);

    while(c_iterLength(iter) > 0)
    {
        serviceElement = v_cfElement(c_iterTakeFirst(iter));
        if (serviceElement)
        {
            serviceNameValue = v_cfElementAttributeValue(serviceElement, "name");
            if(serviceNameValue.kind == V_STRING)
            {
               if (strcmp (serviceNameValue.is.String, requestedServiceName) == 0)
               {
                   result = TRUE;
                   break;
               }
            }
        }
    }

    if(iter)
    {
        c_iterFree(iter);
    }
    if(root)
    {
        c_free(root);
    }

    return result;
}

void
v_loadNetworkCount(
    v_kernel kernel,
    v_configuration config)
{
    c_iter iter;
    v_cfElement root;
    v_cfElement serviceElement;
    c_value serviceNameValue;
    c_value serviceEnabledValue;

    c_long networkServiceCount = 0;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(config != NULL);
    assert(C_TYPECHECK(config,v_configuration));

    /* For each of Service instantiations in the XML configuration, obtain
     * the "name" attribute.  Then search through each of the networking
     * service definitions checking each "name" attribute.  If these match,
     * then the service IS a networking service and the networkServiceCount
     * can be incremented.
     */

    root = v_configurationGetRoot(config);
    iter = v_cfElementXPath(root, "Domain/Service");

    while(c_iterLength(iter) > 0)
    {
        serviceElement = v_cfElement(c_iterTakeFirst(iter));
        if (serviceElement)
        {
            /* Only consider 'enabled' services */
            serviceEnabledValue = v_cfElementAttributeValue(serviceElement, "enabled");
            if(serviceEnabledValue.kind == V_STRING)
            {
                if(os_strcasecmp (serviceEnabledValue.is.String, "false") == 0)
                {
                    /* if the service is disabled continue to the next one */
                    continue;
                }
            }

            serviceNameValue = v_cfElementAttributeValue(serviceElement, "name");
            if(serviceNameValue.kind == V_STRING)
            {
                /* now need to match this service name to one of the service definition blocks,
                 * and if ddsi2 or native networking then it is a "networking" service
                 */
                if (isServiceRequestedServiceKind ("DDSI2Service", serviceNameValue.is.String, config) ||
                    isServiceRequestedServiceKind ("DDSI2EService", serviceNameValue.is.String, config) ||
                    isServiceRequestedServiceKind ("NetworkService", serviceNameValue.is.String, config) ||
                    isServiceRequestedServiceKind ("SNetworkService", serviceNameValue.is.String, config) ||
                    /* RnR is not really a networkingservice but it uses a networkReader (see OSPL-1142) */
                    isServiceRequestedServiceKind ("RnRService", serviceNameValue.is.String, config))
                {
                    networkServiceCount++;
                }
            }
        }
    }

    if(iter)
    {
        c_iterFree(iter);
    }
    if(root)
    {
        c_free(root);
    }

    v_kernelSetNetworkCount (kernel, networkServiceCount);
}


static c_iter
getDurabilityServiceNames(
    v_kernel _this)
{
    c_iter services, iter;
    v_cfElement root;
    v_cfElement serviceElement;
    c_value serviceNameValue;
    c_value serviceEnabledValue;

    root = v_configurationGetRoot(_this->configuration);
    iter = v_cfElementXPath(root, "Domain/Service");
    services = c_iterNew(NULL);

    while(c_iterLength(iter) > 0){
        serviceElement = v_cfElement(c_iterTakeFirst(iter));

        if (serviceElement){
            /* Only consider 'enabled' services */
            serviceEnabledValue = v_cfElementAttributeValue(serviceElement, "enabled");

            if(serviceEnabledValue.kind == V_STRING){
                if(os_strcasecmp (serviceEnabledValue.is.String, "false") == 0){
                    /* if the service is disabled continue to the next one */
                    continue;
                }
            }
            serviceNameValue = v_cfElementAttributeValue(serviceElement, "name");

            if(serviceNameValue.kind == V_STRING){
                /* now need to match this service name to one of the service
                 * definition blocks, and if it is a durability service
                 */
                if (isServiceRequestedServiceKind("DurabilityService",
                        serviceNameValue.is.String, _this->configuration)){
                    services = c_iterAppend(
                            services, serviceNameValue.is.String);
                }
            }
        }
    }

    if(iter){
        c_iterFree(iter);
    }
    if(root){
        c_free(root);
    }
    return services;
}

v_result
v_kernelWaitForDurabilityAvailability(
    v_kernel _this,
    c_time timeout)
{
    c_iter durabilityServices;
    c_string serviceName;
    v_serviceManager serviceManager;
    c_time expiryTime;
    v_serviceStateKind state;
    v_result result;
    c_time sleepTime;

    expiryTime = c_timeAdd(v_timeGet(), timeout);

    /* Set sleepTime to 100 ms, unless provided timeout is less than that.*/
    sleepTime.seconds = 0;
    sleepTime.nanoseconds = 100 * 1000 * 1000; /*100ms*/

    if(c_timeCompare(timeout, sleepTime) == C_LT){
        sleepTime = timeout;
    }
    serviceManager = v_getServiceManager(_this);
    durabilityServices = getDurabilityServiceNames(_this);
    serviceName = (c_string)c_iterTakeFirst(durabilityServices);

    /* If no durability services have been configured, return pre-condition
     * not met.
     */
    if(!serviceName){
        result = V_RESULT_PRECONDITION_NOT_MET;
    } else {
        result = V_RESULT_OK;
    }

    while(serviceName && (result == V_RESULT_OK)){
        state = v_serviceManagerGetServiceStateKind(serviceManager, serviceName);

        switch(state){
        case STATE_NONE:
            /* Fall-through on purpose */
        case STATE_INITIALISING:
            /* Wait some time if still within timeout. */
            if(c_timeCompare(expiryTime, v_timeGet()) == C_GT){
                c_timeNanoSleep(sleepTime);
            } else {
                result = V_RESULT_TIMEOUT;
            }
            break;
        case STATE_OPERATIONAL:
            /* Result is ok, move on to next durability service. */
            break;
        case STATE_INCOMPATIBLE_CONFIGURATION:
            /* The durability service could not match its configuration,
             * e.g., because no aligner could be found within the specified
             * time to wait for an aligner. In this case fall-through on
             * purpose and return PRECONDITION_NOT_MET to the readers that
             * called this method.
             */
        case STATE_DIED:
            /*Fall-through on purpose */
        case STATE_TERMINATING:
            /*Fall-through on purpose */
        case STATE_TERMINATED:
            result = V_RESULT_PRECONDITION_NOT_MET;
            break;
        }
        /* Only if operational, move on to the next service. */
        if(state == STATE_OPERATIONAL){
            serviceName = (c_string)c_iterTakeFirst(durabilityServices);
        }
    }
    c_iterFree(durabilityServices);

    return result;
}


v_kernel
v_kernelNew(
    c_base base,
    const c_char *name,
    v_kernelQos qos)
{
    v_kernel kernel;
    v_kernelStatistics kernelStatistics;
    v_spliced sd;

    kernel = c_lookup(base,name);
    if (kernel != NULL) {
        assert(C_TYPECHECK(kernel,v_kernel));
        kernel->userCount++;
        return kernel;
    }

    loadkernelModule(base);

    kernel = (v_kernel)c_new(c_resolve(base,"kernelModule::v_kernel"));
    if (!kernel) {
        OS_REPORT(OS_ERROR,
                  "v_kernelNew",0,
                  "Failed to allocate kernel.");
        return NULL;
    }
    v_objectKind(kernel) = K_KERNEL;
    v_object(kernel)->kernel = (c_voidp)kernel;
    kernel->handleServer = v_handleServerNew(base);

#define INITTYPE(k,t,l) k->type[l] = c_resolve(base,#t)

    INITTYPE(kernel,kernelModule::v_kernel,             K_KERNEL);
    INITTYPE(kernel,kernelModule::v_participant,        K_PARTICIPANT);
    INITTYPE(kernel,kernelModule::v_waitset,            K_WAITSET);
    INITTYPE(kernel,kernelModule::v_condition,          K_CONDITION);
    INITTYPE(kernel,kernelModule::v_query,              K_QUERY);
    INITTYPE(kernel,kernelModule::v_dataReaderQuery,    K_DATAREADERQUERY);
    INITTYPE(kernel,kernelModule::v_dataViewQuery,      K_DATAVIEWQUERY);
    INITTYPE(kernel,kernelModule::v_dataView,           K_DATAVIEW);
    INITTYPE(kernel,kernelModule::v_dataViewSample,     K_DATAVIEWSAMPLE);
    INITTYPE(kernel,kernelModule::v_dataViewInstance,   K_DATAVIEWINSTANCE);
    INITTYPE(kernel,kernelModule::v_projection,         K_PROJECTION);
    INITTYPE(kernel,kernelModule::v_mapping,            K_MAPPING);
    INITTYPE(kernel,kernelModule::v_topic,              K_TOPIC);
    INITTYPE(kernel,kernelModule::v_message,            K_MESSAGE);
    INITTYPE(kernel,kernelModule::v_transaction,        K_TRANSACTION);
    INITTYPE(kernel,kernelModule::v_dataReaderInstance, K_DATAREADERINSTANCE);
    INITTYPE(kernel,kernelModule::v_purgeListItem,      K_PURGELISTITEM);
    INITTYPE(kernel,kernelModule::v_groupPurgeItem,     K_GROUPPURGEITEM);
    INITTYPE(kernel,kernelModule::v_dataReaderSample,   K_READERSAMPLE);
    INITTYPE(kernel,kernelModule::v_publisher,          K_PUBLISHER);
    INITTYPE(kernel,kernelModule::v_subscriber,         K_SUBSCRIBER);
    INITTYPE(kernel,kernelModule::v_partition,             K_DOMAIN);
    INITTYPE(kernel,kernelModule::v_partitionInterest,     K_DOMAININTEREST);
    INITTYPE(kernel,kernelModule::v_partitionAdmin,        K_DOMAINADMIN);
    INITTYPE(kernel,kernelModule::v_reader,             K_READER);
    INITTYPE(kernel,kernelModule::v_writer,             K_WRITER);
    INITTYPE(kernel,kernelModule::v_writerGroup,        K_WRITERGROUP);
    INITTYPE(kernel,kernelModule::v_group,              K_GROUP);
    INITTYPE(kernel,kernelModule::v_groupInstance,      K_GROUPINSTANCE);
    INITTYPE(kernel,kernelModule::v_groupSample,        K_GROUPSAMPLE);
    INITTYPE(kernel,kernelModule::v_groupCacheItem,     K_GROUPCACHEITEM);
    INITTYPE(kernel,kernelModule::v_cache,              K_CACHE);
    INITTYPE(kernel,kernelModule::v_entry,              K_ENTRY);
    INITTYPE(kernel,kernelModule::v_dataReaderEntry,    K_DATAREADERENTRY);
    INITTYPE(kernel,kernelModule::v_groupAction,        K_GROUPACTION);
    INITTYPE(kernel,kernelModule::v_groupStream,        K_GROUPSTREAM);
    INITTYPE(kernel,kernelModule::v_groupQueue,         K_GROUPQUEUE);
    INITTYPE(kernel,kernelModule::v_groupQueueSample,   K_GROUPQUEUESAMPLE);
    INITTYPE(kernel,kernelModule::v_dataReader,         K_DATAREADER);
    INITTYPE(kernel,kernelModule::v_deliveryService,    K_DELIVERYSERVICE);
    INITTYPE(kernel,kernelModule::v_deliveryServiceEntry, K_DELIVERYSERVICEENTRY);
    INITTYPE(kernel,kernelModule::v_index,              K_INDEX);
    INITTYPE(kernel,kernelModule::v_filter,             K_FILTER);
    INITTYPE(kernel,kernelModule::v_readerStatus,       K_READERSTATUS);
    INITTYPE(kernel,kernelModule::v_writerStatus,       K_WRITERSTATUS);
    INITTYPE(kernel,kernelModule::v_partitionStatus,    K_DOMAINSTATUS);
    INITTYPE(kernel,kernelModule::v_topicStatus,        K_TOPICSTATUS);
    INITTYPE(kernel,kernelModule::v_subscriberStatus,   K_SUBSCRIBERSTATUS);
    INITTYPE(kernel,kernelModule::v_status,             K_PUBLISHERSTATUS);
    INITTYPE(kernel,kernelModule::v_status,             K_PARTICIPANTSTATUS);
    INITTYPE(kernel,kernelModule::v_kernelStatus,       K_KERNELSTATUS);
    INITTYPE(kernel,kernelModule::v_readerStatistics,   K_READERSTATISTICS);
    INITTYPE(kernel,kernelModule::v_writerStatistics,   K_WRITERSTATISTICS);
    INITTYPE(kernel,kernelModule::v_queryStatistics,    K_QUERYSTATISTICS);
    INITTYPE(kernel,kernelModule::v_lease,              K_LEASE);
    INITTYPE(kernel,kernelModule::v_leaseAction,        K_LEASEACTION);
    INITTYPE(kernel,kernelModule::v_serviceManager,     K_SERVICEMANAGER);
    INITTYPE(kernel,kernelModule::v_service,            K_SERVICE);
    INITTYPE(kernel,kernelModule::v_serviceState,       K_SERVICESTATE);
    INITTYPE(kernel,kernelModule::v_networking,         K_NETWORKING);
    INITTYPE(kernel,kernelModule::v_durability,         K_DURABILITY);
    INITTYPE(kernel,kernelModule::v_cmsoap,             K_CMSOAP);
    INITTYPE(kernel,kernelModule::v_rnr,                K_RNR);
    INITTYPE(kernel,kernelModule::v_leaseManager,       K_LEASEMANAGER);
    INITTYPE(kernel,kernelModule::v_groupSet,           K_GROUPSET);
    INITTYPE(kernel,kernelModule::v_proxy,              K_PROXY);
    INITTYPE(kernel,kernelModule::v_waitsetEvent,       K_WAITSETEVENT);
    INITTYPE(kernel,kernelModule::v_waitsetEventHistoryDelete,  K_WAITSETEVENTHISTORYDELETE);
    INITTYPE(kernel,kernelModule::v_waitsetEventHistoryRequest, K_WAITSETEVENTHISTORYREQUEST);
    INITTYPE(kernel,kernelModule::v_waitsetEventPersistentSnapshot, K_WAITSETEVENTPERSISTENTSNAPSHOT);
    INITTYPE(kernel,kernelModule::v_waitsetEventConnectWriter, K_WAITSETEVENTCONNECTWRITER);
    INITTYPE(kernel,kernelModule::v_writerSample,       K_WRITERSAMPLE);
    INITTYPE(kernel,kernelModule::v_writerInstance,     K_WRITERINSTANCE);
    INITTYPE(kernel,kernelModule::v_writerInstanceTemplate, K_WRITERINSTANCETEMPLATE);
    INITTYPE(kernel,kernelModule::v_writerCacheItem,    K_WRITERCACHEITEM);
    /* Networking types */
    INITTYPE(kernel,kernelModule::v_networkReader,      K_NETWORKREADER);
    INITTYPE(kernel,kernelModule::v_networkReaderEntry, K_NETWORKREADERENTRY);
    INITTYPE(kernel,kernelModule::v_networkMessage,     K_NETWORKMESSAGE);
    INITTYPE(kernel,kernelModule::v_networkMapEntry,    K_NETWORKMAPENTRY);

    INITTYPE(kernel,kernelModule::v_spliced,            K_SPLICED);
    INITTYPE(kernel,kernelModule::v_configuration,      K_CONFIGURATION);
    INITTYPE(kernel,kernelModule::v_registration,       K_REGISTRATION);

    INITTYPE(kernel,kernelModule::v_historicalDataRequest,K_HISTORICALDATAREQUEST);
    INITTYPE(kernel,kernelModule::v_persistentSnapshotRequest,K_PERSISTENTSNAPSHOTREQUEST);
    INITTYPE(kernel,kernelModule::v_pendingDisposeElement,K_PENDINGDISPOSEELEMENT);

#undef INITTYPE


    kernel->pendingDisposeList =
       c_listNew(v_kernelType(kernel, K_PENDINGDISPOSEELEMENT ));
    c_mutexInit(&kernel->pendingDisposeListMutex, SHARED_MUTEX);

    kernelStatistics = v_kernelStatisticsNew(kernel);
    v_observableInit(v_observable(kernel),
                     V_KERNEL_VERSION,
                     v_statistics(kernelStatistics),
                     TRUE);
    c_lockInit(&kernel->lock,SHARED_LOCK);
    kernel->qos = v_kernelQosNew(kernel, qos);
    {
        /* Fill GID with 'random' value */
        memset(&kernel->GID, 0, sizeof(kernel->GID));
        kernel->GID.systemId = os_uniqueNodeIdGet();
    }
    kernel->participants = c_setNew(v_kernelType(kernel,K_PARTICIPANT));
    kernel->partitions = c_tableNew(v_kernelType(kernel,K_DOMAIN),"name");
    kernel->topics = c_tableNew(v_kernelType(kernel,K_TOPIC),"name");
    kernel->groupSet = v_groupSetNew(kernel);
    kernel->serviceManager = v_serviceManagerNew(kernel);
    kernel->livelinessLM = v_leaseManagerNew(kernel);
    kernel->configuration = NULL;
    kernel->userCount = 1;
    kernel->networkServiceCount = 0;
    kernel->transactionCount = 0;
    kernel->splicedRunning = TRUE;
    kernel->maxSamplesWarnLevel = V_KERNEL_MAX_SAMPLES_WARN_LEVEL_DEF;
    kernel->maxSamplesWarnShown = FALSE;
    kernel->maxSamplesPerInstanceWarnLevel = V_KERNEL_MAX_SAMPLES_PER_INSTANCES_WARN_LEVEL_DEF;
    kernel->maxSamplesPerInstanceWarnShown = FALSE;
    kernel->maxInstancesWarnLevel = V_KERNEL_MAX_INSTANCES_WARN_LEVEL_DEF;
    kernel->maxInstancesWarnShown = FALSE;
    kernel->enabledStatisticsCategories =
        c_listNew(c_resolve(base, "kernelModule::v_statisticsCategory"));

    c_mutexInit(&kernel->sharesMutex, SHARED_MUTEX);
    kernel->shares = c_tableNew(v_kernelType(kernel,K_SUBSCRIBER),
                                "qos.share.name");

    kernel->crc = v_crcNew(kernel, V_CRC_KEY);
    kernel->builtin = v_builtinNew(kernel);

    kernel->deliveryService = NULL;

    sd = v_splicedNew(kernel);
    c_free(sd);


    ospl_c_bind(kernel,name);

    return kernel;
}

v_partition
v_addPartition(
    v_kernel kernel,
    v_partition partition)
{
    v_partition found;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(partition != NULL);
    assert(C_TYPECHECK(partition,v_partition));

    c_lockWrite(&kernel->lock);
    found = c_insert(kernel->partitions,partition);
    c_lockUnlock(&kernel->lock);

    if (found != partition) {
        assert(v_objectKind(found) == K_DOMAIN);
        if (v_objectKind(found) != K_DOMAIN) {
            partition = NULL;
        } else {
            partition = found;
        }
    }
    return partition;
}

v_partition
v_removePartition(
    v_kernel kernel,
    v_partition partition)
{
    v_partition found;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(partition != NULL);
    assert(C_TYPECHECK(partition,v_partition));

    c_lockWrite(&kernel->lock);
    found = c_remove(kernel->partitions,partition,NULL,NULL);
    c_lockUnlock(&kernel->lock);
    if (found == NULL) {
        return NULL;
    }
    if (v_objectKind(found) != K_DOMAIN) {
        return NULL;
    }
    return found;
}

v_topic
v__addTopic(
    v_kernel kernel,
    v_topic topic)
{
    v_topic found;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(topic != NULL);
    assert(C_TYPECHECK(topic,v_topic));

    c_lockWrite(&kernel->lock);
    found = c_insert(kernel->topics,topic);
    c_lockUnlock(&kernel->lock);

    if (found != topic) {
        if (v_objectKind(found) != K_TOPIC) {
            return NULL;
        }
        topic = found;
    }
    return topic;
}

v_topic
v_removeTopic(
    v_kernel kernel,
    v_topic topic)
{
    v_topic found;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(topic != NULL);
    assert(C_TYPECHECK(topic,v_topic));

    c_lockWrite(&kernel->lock);
    found = c_remove(kernel->topics,topic,NULL,NULL);
    c_lockUnlock(&kernel->lock);
    if (found == NULL) {
        return NULL;
    }
    if (v_objectKind(found) != K_TOPIC) {
        return NULL;
    }
    return found;
}

v_participant
v_addParticipant(
    v_kernel kernel,
    v_participant p)
{
    v_participant found;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(p != NULL);
    assert(C_TYPECHECK(p,v_participant));

    c_lockWrite(&kernel->lock);
    found = c_insert(kernel->participants,p);
    c_lockUnlock(&kernel->lock);

    return found;
}

v_participant
v_removeParticipant(
    v_kernel kernel,
    v_participant p)
{
    v_participant found;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(p != NULL);
    assert(C_TYPECHECK(p,v_participant));

    c_lockWrite(&kernel->lock);
    found = c_remove(kernel->participants,p,NULL,NULL);
    c_lockUnlock(&kernel->lock);

    return found;
}

void
v_lockShares(
    v_kernel kernel)
{
    c_mutexLock(&kernel->sharesMutex);
}
void
v_unlockShares(
    v_kernel kernel)
{
    c_mutexUnlock(&kernel->sharesMutex);
}

v_entity
v_addShareUnsafe(
    v_kernel kernel,
    v_entity e)
{
    v_entity found;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    found = c_insert(kernel->shares,e);

    return found;
}

v_entity
v_removeShare(
    v_kernel kernel,
    v_entity e)
{
    v_entity found;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    v_lockShares(kernel);
    found = c_remove(kernel->shares,e,NULL,NULL);
    v_unlockShares(kernel);

    return found;
}

c_iter
v_resolveShare(
    v_kernel kernel,
    const c_char *name)
{
    c_iter list;
    c_collection q;
    q_expr expr;
    c_value params[1];

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    expr = (q_expr)q_parse("name like %0");
    params[0] = c_stringValue((char *)name);
    q = c_queryNew(kernel->shares,expr,params);
    q_dispose(expr);
    v_lockShares(kernel);
    list = ospl_c_select(q,0);
    v_unlockShares(kernel);
    c_free(q);
    return list;
}


c_iter
v_resolveParticipants(
    v_kernel kernel,
    const c_char *name)
{
    c_iter list;
    c_collection q;
    q_expr expr;
    c_value params[1];

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    expr = (q_expr)q_parse("name like %0");
    params[0] = c_stringValue((char *)name);
    q = c_queryNew(kernel->participants,expr,params);
    q_dispose(expr);
    c_lockRead(&kernel->lock);
    list = ospl_c_select(q,0);
    c_lockUnlock(&kernel->lock);
    c_free(q);
    return list;
}

c_iter
v_resolvePartitions (
    v_kernel kernel,
    const c_char *name)
{
    c_iter list;
    c_collection q;
    q_expr expr;
    c_value params[1];

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    expr = (q_expr)q_parse("name like %0");
    params[0] = c_stringValue((char *)name);
    q = c_queryNew(kernel->partitions,expr,params);
    q_dispose(expr);
    c_lockRead(&kernel->lock);
    list = ospl_c_select(q,0);
    c_lockUnlock(&kernel->lock);
    c_free(q);
    return list;
}

c_iter
v_resolveTopics(
    v_kernel kernel,
    const c_char *name)
{
    c_iter list;
    c_collection q;
    q_expr expr;
    c_value params[1];

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    expr = (q_expr)q_parse("name like %0");
    params[0] = c_stringValue((char *)name);
    q = c_queryNew(kernel->topics,expr,params);
    q_dispose(expr);
    c_lockRead(&kernel->lock);
    list = ospl_c_select(q,0);
    c_lockUnlock(&kernel->lock);
    c_free(q);
    return list;
}


static c_bool
alwaysFalse(
    c_object found,
    c_object requested,
    c_voidp arg)
{
    v_topic *topicFound = (v_topic *)arg;

    OS_UNUSED_ARG(requested);
    assert(topicFound != NULL);
    assert(*topicFound == NULL); /* Out param */

    *topicFound = v_topic(c_keep(found));
    return FALSE;
}


v_topic
v_lookupTopic(
    v_kernel kernel,
    const char *name)
{
    v_topic topicFound;
    C_STRUCT(v_topic) dummyTopic;
    c_base base = c_getBase(c_object(kernel));

    /* Create a dummy topic for look-up */
    memset(&dummyTopic, 0, sizeof(dummyTopic));
    ((v_entity)(&dummyTopic))->name = c_stringNew(base,name);
    topicFound = NULL;
    c_lockRead(&kernel->lock);
    /* This does not remove anything because the alwaysFalse function always
     * returns false */
    c_remove(kernel->topics, &dummyTopic, alwaysFalse, &topicFound);
    c_lockUnlock(&kernel->lock);
    c_free(((v_entity)(&dummyTopic))->name);

    return topicFound;
}

v_serviceManager
v_getServiceManager(
    v_kernel kernel)
{
    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    return kernel->serviceManager;
}

v_configuration
v_getConfiguration(
    v_kernel kernel)
{
    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    return kernel->configuration;
}

v_configuration
v_setConfiguration(
    v_kernel kernel,
    v_configuration config)
{
    v_configuration old;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(config != NULL);
    assert(C_TYPECHECK(config,v_configuration));

    old = kernel->configuration;
    kernel->configuration = config;
    c_keep(kernel->configuration);
    v_loadWarningLevels(kernel, config);
    v_loadNetworkCount(kernel, config);

    if (old != NULL) {
        c_free(old);

    }
    return old;
}

void
v_loadWarningLevels(
    v_kernel kernel,
    v_configuration config)
{
    c_iter iter;
    v_cfData elementData = NULL;
    c_value value;
    v_cfElement root;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(config != NULL);
    assert(C_TYPECHECK(config,v_configuration));

    root = v_configurationGetRoot(config);
    /* load max samples warn level */
    iter = v_cfElementXPath(root, "Domain/ResourceLimits/MaxSamples/WarnAt/#text");
    while(c_iterLength(iter) > 0)
    {
        elementData = v_cfData(c_iterTakeFirst(iter));
    }
    if(iter)
    {
        c_iterFree(iter);
    }
    if(elementData)/* aka the last one from the previous while loop */
    {
        value = v_cfDataValue(elementData);
        sscanf(value.is.String, "%u", &kernel->maxSamplesWarnLevel);
        if(kernel->maxSamplesWarnLevel < V_KERNEL_MAX_SAMPLES_WARN_LEVEL_MIN)
        {
            kernel->maxSamplesWarnLevel = V_KERNEL_MAX_SAMPLES_WARN_LEVEL_MIN;
        }
    }
    elementData = NULL;
    /* load max instances warn level */
    iter = v_cfElementXPath(root, "Domain/ResourceLimits/MaxInstances/WarnAt/#text");
    while(c_iterLength(iter) > 0)
    {
        elementData = v_cfData(c_iterTakeFirst(iter));
    }
    if(iter)
    {
        c_iterFree(iter);
    }
    if(elementData)/* aka the last one from the previous while loop */
    {
        value = v_cfDataValue(elementData);
        sscanf(value.is.String, "%u", &kernel->maxInstancesWarnLevel);
        if(kernel->maxInstancesWarnLevel < V_KERNEL_MAX_INSTANCES_WARN_LEVEL_MIN)
        {
            kernel->maxInstancesWarnLevel = V_KERNEL_MAX_INSTANCES_WARN_LEVEL_MIN;
        }
    }
    elementData = NULL;
    /* load max samples per instances warn level */
    iter = v_cfElementXPath(root, "Domain/ResourceLimits/MaxSamplesPerInstance/WarnAt/#text");
    while(c_iterLength(iter) > 0)
    {
        elementData = v_cfData(c_iterTakeFirst(iter));
    }
    if(iter)
    {
        c_iterFree(iter);
    }
    if(elementData)/* aka the last one from the previous while loop */
    {
        value = v_cfDataValue(elementData);
        sscanf(value.is.String, "%u", &kernel->maxSamplesPerInstanceWarnLevel);
        if(kernel->maxSamplesPerInstanceWarnLevel < V_KERNEL_MAX_SAMPLES_PER_INSTANCES_WARN_LEVEL_MIN)
        {
            kernel->maxSamplesPerInstanceWarnLevel = V_KERNEL_MAX_SAMPLES_PER_INSTANCES_WARN_LEVEL_MIN;
        }

    }
}

void
v_checkMaxInstancesWarningLevel(
    v_kernel _this,
    c_ulong count)
{
    if(count >= _this->maxInstancesWarnLevel && !_this->maxInstancesWarnShown)
    {
        OS_REPORT_2(OS_WARNING,
            "v_checkMaxInstancesWarningLevel",0,
            "The number of instances '%d' has surpassed the "
            "warning level of '%d' instances.",
            count,
            _this->maxInstancesWarnLevel);
        _this->maxInstancesWarnShown = TRUE;
    }
}

void
v_checkMaxSamplesWarningLevel(
    v_kernel _this,
    c_ulong count)
{
    if(count >= _this->maxSamplesWarnLevel && !_this->maxSamplesWarnShown)
    {
        OS_REPORT_2(OS_WARNING,
            "v_checkMaxSamplesWarningLevel",0,
            "The number of samples '%d' has surpassed the "
            "warning level of '%d' samples.",
            count,
            _this->maxSamplesWarnLevel);
        _this->maxSamplesWarnShown = TRUE;
    }
}

void
v_checkMaxSamplesPerInstanceWarningLevel(
    v_kernel _this,
    c_ulong count)
{
    if(count >= _this->maxSamplesPerInstanceWarnLevel && !_this->maxSamplesPerInstanceWarnShown)
    {
        OS_REPORT_2(OS_WARNING,
            "v_checkMaxSamplesPerInstanceWarningLevel",0,
            "The number of samples per instance '%d' has surpassed the "
            "warning level of '%d' samples per instance.",
            count,
            _this->maxSamplesPerInstanceWarnLevel);
        _this->maxSamplesPerInstanceWarnShown = TRUE;
    }
}

/* --------------------------- Builtin topic methods ------------------------ */

void
v_writeBuiltinTopic(
    v_kernel k,
    enum v_infoId id,
    v_message msg)
{
    v_writer writer;

    if (msg != NULL) {
        if (k->builtin != NULL) {
            writer = v_builtinWriterLookup(k->builtin,id);
            if (writer != NULL) {
                /* No need to fill writerGID, this is done by the writer */
                v_writerWrite(writer,msg,v_timeGet(),NULL);
            }
        }
    }
}

void
v_writeDisposeBuiltinTopic(
    v_kernel k,
    enum v_infoId id,
    v_message msg)
{
    v_writer writer;

    if (msg != NULL) {
        if (k->builtin != NULL) {
            writer = v_builtinWriterLookup(k->builtin,id);
            if (writer != NULL) {
                /* No need to fill writerGID, this is done by the writer */
                v_writerWriteDispose(writer,msg,v_timeGet(),NULL);
            }
        }
    }
}

void
v_unregisterBuiltinTopic(
    v_kernel k,
    enum v_infoId id,
    v_message msg)
{
    v_writer writer;

    if (msg != NULL) {
        writer = v_builtinWriterLookup(k->builtin,id);
        if (writer != NULL) {
            /* No need to fill writerGID, this is done by the writer */
            v_writerUnregister(writer,msg,v_timeGet(),NULL);
        }
    }
}


/* Statistics enabling/disabling */

static v_statisticsCategory
v_statisticsCategoryNew(
    v_kernel k,
    const char *categoryName)
{
    c_base base;
    v_statisticsCategory result;

    base = c_getBase(k);
    result = c_stringNew(base, categoryName);
    return result;
}

static c_bool
v_statisticsCategoryCompareAction(
    c_object o,
    c_voidp arg)
{
    c_bool result;
    v_statisticsCategory category = o;
    char *toCompare = arg;

    if (strcmp(category, toCompare) == 0) {
        result = FALSE; /* Stop the action */
    } else {
        result = TRUE; /* Continue the action */
    }
    return result;
}


static c_bool
isEnabledStatisticsUnlocked(
    v_kernel k,
    const char *categoryName)
{
    c_bool result;

    result = !c_readAction(k->enabledStatisticsCategories,
        v_statisticsCategoryCompareAction, (c_voidp)categoryName);

    return result;
}


c_bool
v_isEnabledStatistics(
    v_kernel k,
    const char *categoryName)
{
    c_bool result;

    c_lockRead(&k->lock);
    result = isEnabledStatisticsUnlocked(k, categoryName);
    c_lockUnlock(&k->lock);

    return result;
}


void
v_enableStatistics(
    v_kernel k,
    const char *categoryName)
{
    v_statisticsCategory category;

    c_lockWrite(&k->lock);
    /* Only add this if it is not yet enabled */
    if (!isEnabledStatisticsUnlocked(k, categoryName)) {
        category = v_statisticsCategoryNew(k, categoryName);
        c_append(k->enabledStatisticsCategories, category);
    }
    c_lockUnlock(&k->lock);
}


void
v_disableStatistics(
    v_kernel k,
    const char *categoryName)
{
    /* Not yet implemented */
    /* Here, we will have to walk over all entities and forward the disable
     * request. The entity itself has to switch off the corresponding
     * statistics admin */
    OS_UNUSED_ARG(k);
    OS_UNUSED_ARG(categoryName);
}

c_bool
v_kernelCheckHandleServer (
    v_kernel k,
    c_address serverId)
{
   return ((c_address)k->handleServer == serverId);
}

v_persistentSnapshotRequest
v_persistentSnapshotRequestNew(
    v_kernel kernel,
    const c_char* partition_expression,
    const c_char* topic_expression,
    const c_char* uri)
{
    v_persistentSnapshotRequest request;
    c_base base;

    request = c_new(v_kernelType(kernel,K_PERSISTENTSNAPSHOTREQUEST));
    if(request)
    {
        base = c_getBase(kernel);
        if(partition_expression)
        {
            request->partitionExpr = c_stringNew(base, partition_expression);
        }
        if(topic_expression)
        {
            request->topicExpr = c_stringNew(base, topic_expression);
        }
        if(uri)
        {
            request->uri = c_stringNew(base, uri);
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "v_kernel::v_persistentSnapshotRequest",0,
                  "Failed to create v_persistentSnapshotRequest object.");
        assert(FALSE);
    }

    return request;
}

v_result
v_kernelCreatePersistentSnapshot(
    v_kernel _this,
    const c_char * partition_expression,
    const c_char * topic_expression,
    const c_char * uri)
{
    v_result result = V_RESULT_OK;
    C_STRUCT(v_event) event;
    v_persistentSnapshotRequest request;

    request = v_persistentSnapshotRequestNew(_this, partition_expression, topic_expression, uri);
    if(request)
    {
        event.kind = V_EVENT_PERSISTENT_SNAPSHOT;
        event.source = v_publicHandle(v_public(_this));
        event.userData = request;
        v_observableNotify(v_observable(_this),&event);
    } else
    {
        result = V_RESULT_OUT_OF_MEMORY;
    }
    return result;
}

c_ulong
v_kernelGetTransactionId (
    v_kernel _this)
{
    c_ulong id = pa_increment(&_this->transactionCount);
    while ((id % 256) == 0) {
        /* the value '0' is reserved to specify 'no-transaction' and
         * the transactionId should not exceed 256. The statement
         * id % 256 covers both these cases.
         */
        id = pa_increment(&_this->transactionCount);
    }
    return (id % 256);
}

/*
 * ES, dds1576: This method consults the configuration info stored in the kernel
 * to determine the access policy for this partition
 */
v_accessMode
v_kernelPartitionAccessMode(
    v_kernel _this,
    v_partitionPolicy partition)
{
    v_configuration config;
    v_cfElement root;
    v_cfElement element;
    c_iter iter;
    v_accessMode retVal = V_ACCESS_MODE_UNDEFINED;
    c_value expression;
    c_value accessMode;
    c_iter partitionsSplit;
    c_char* partitionName;

    config = v_getConfiguration(_this);
    if(config)
    {
        root = v_configurationGetRoot(config);
        /* Iterate over all partitionAccess elements */
        iter = v_cfElementXPath(root, "Domain/PartitionAccess");
        while(c_iterLength(iter) > 0)
        {
            element = v_cfElement(c_iterTakeFirst(iter));
            /* Get the partition expression value, it should be a string */
            expression = v_cfElementAttributeValue(element, "partition_expression");
            if(expression.kind == V_STRING)
            {
                /* iterate over partitions, if one matches, exit and return */
                partitionsSplit = v_partitionPolicySplit(partition);
                while(c_iterLength(partitionsSplit) > 0)
                {
                    partitionName = (c_char*)c_iterTakeFirst(partitionsSplit);
                    if(v_partitionStringMatchesExpression(partitionName, expression.is.String))
                    {
                        /* The partition matches the expression.*/
                        accessMode = v_cfElementAttributeValue(element, "access_mode");
                        if(accessMode.kind == V_STRING)
                        {
                            /* A correct solution space can be realized between multiple
                             * expressions having an AND relationship by specifying the
                             * following rules R&W=RW, R&N=N, W&N=N, RW&N=N.
                             */
                            switch(retVal)
                            {
                                case V_ACCESS_MODE_UNDEFINED: /* start state */
                                    if(strcmp(accessMode.is.String, "none") == 0)
                                    {
                                        retVal = V_ACCESS_MODE_NONE;
                                    } else if(strcmp(accessMode.is.String, "write") == 0)
                                    {
                                        retVal = V_ACCESS_MODE_WRITE;
                                    } else if(strcmp(accessMode.is.String, "read") == 0)
                                    {
                                        retVal = V_ACCESS_MODE_READ;
                                    } else if(strcmp(accessMode.is.String, "readwrite") == 0)
                                    {
                                        retVal = V_ACCESS_MODE_READ_WRITE;
                                    }
                                    break;
                                case V_ACCESS_MODE_WRITE:
                                    if(strcmp(accessMode.is.String, "read") == 0 ||
                                       strcmp(accessMode.is.String, "readwrite") == 0)
                                    {
                                        retVal = V_ACCESS_MODE_READ_WRITE;
                                    } else if(strcmp(accessMode.is.String, "none") == 0)
                                    {
                                        retVal = V_ACCESS_MODE_NONE;
                                    }
                                    break;
                                case V_ACCESS_MODE_READ:
                                    if(strcmp(accessMode.is.String, "write") == 0 ||
                                       strcmp(accessMode.is.String, "readwrite") == 0)
                                    {
                                        retVal = V_ACCESS_MODE_READ_WRITE;
                                    } else if(strcmp(accessMode.is.String, "none") == 0)
                                    {
                                        retVal = V_ACCESS_MODE_NONE;
                                    }
                                    break;
                                case V_ACCESS_MODE_READ_WRITE:
                                    if(strcmp(accessMode.is.String, "none") == 0)
                                    {
                                        retVal = V_ACCESS_MODE_NONE;
                                    }
                                    break;
                                default: /* case V_ACCESS_MODE_NONE > none always remains none */
                                    break;
                            }
                        }
                    }
                    os_free(partitionName);
                }
                c_iterFree(partitionsSplit);
            }
        }
        if(iter)
        {
            c_iterFree(iter);
        }
        if(root)
        {
            c_free(root);
        }
    }
    if(retVal == V_ACCESS_MODE_UNDEFINED)
    {
        /* No specific rights defined, fall back to default */
        retVal = V_ACCESS_MODE_READ_WRITE;
    }
    return retVal;
}
