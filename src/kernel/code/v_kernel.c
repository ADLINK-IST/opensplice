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
#include "c_base.h"
#include "c_stringSupport.h"
#include "q_expr.h"
#include "v_kernel.h"
#include "v__kernel.h"
#include "v__observable.h"
#include "v__entity.h"
#include "v__participant.h"
#include "v__service.h"
#include "v_partition.h"
#include "v_publisher.h"
#include "v_subscriber.h"
#include "v_cfElement.h"
#include "v__writer.h"
#include "v_reader.h"
#include "v_entry.h"
#include "v_dataReaderEntry.h"
#include "v_serviceManager.h"
#include "v_groupSet.h"
#include "v__groupStore.h"
#include "v__group.h"
#include "v_status.h"
#include "v_handle.h"
#include "v_dataReader.h"
#include "v_dataView.h"
#include "v__topic.h"
#include "v_topicQos.h"
#include "v_publisherQos.h"
#include "v_subscriberQos.h"
#include "v_writerQos.h"
#include "v_readerQos.h"
#include "v__builtin.h"
#include "v_public.h"
#include "v_kernelStatistics.h"
#include "v_kernelQos.h"
#include "v_configuration.h"
#include "v__spliced.h"
#include "v__leaseManager.h"
#include "v__transactionGroup.h"
#include "vortex_os.h"
#include "os_report.h"
#include "os_process.h"
#include "os_uniqueNodeId.h"
#include "v__policy.h"
#include "v__partition.h"
#include "v_instance.h"
#include "v__dataViewSample.h"
#include "v_typeRepresentation.h"
#include "sd_serializer.h"
#include "sd_serializerXMLTypeinfo.h"
#include "sd_typeInfoParser.h"
#include "v__processInfo.h"
#include "v__threadInfo.h"
#include "v_service.h"
#include "v_networking.h"
#include "v_messageQos.h"
#include "ut_trace.h"
#include "os_atomics.h"

#define __ERROR(m) printf(m); printf("\n");

#define V_KERNEL_MAX_SAMPLES_WARN_LEVEL_DEF 5000
#define V_KERNEL_MAX_SAMPLES_WARN_LEVEL_MIN 1
#define V_KERNEL_MAX_INSTANCES_WARN_LEVEL_DEF 5000
#define V_KERNEL_MAX_INSTANCES_WARN_LEVEL_MIN 1
#define V_KERNEL_MAX_SAMPLES_PER_INSTANCES_WARN_LEVEL_DEF 5000
#define V_KERNEL_MAX_SAMPLES_PER_INSTANCES_WARN_LEVEL_MIN 1
#define V_KERNEL_RETENTION_PERIOD_DEF 500u
#define V_KERNEL_RETENTION_PERIOD_MIN 1u

static v_result
v_loadWarningLevels(
    v_kernel kernel,
    v_configuration config);

static v_result
v_loadRetentionPeriod(
    v_kernel kernel,
    v_configuration config);

static v_result
v_loadDurabilitySupport(
    v_kernel kernel);

static v_result
v_loadTimeSpec(
    v_kernel kernel);

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
    o->kernel = kernel;
    t = type;
    while (t != NULL) {
        for (i=0;i<K_TYPECOUNT;i++) {
            if (t == kernel->type[i]) {
                o->kind = (enum v_kind) i;
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
    return o;
}

_Check_return_
_Ret_notnull_
_Pre_satisfies_(kind >= K_KERNEL && kind < K_TYPECOUNT)
v_object
v_objectNew(
    _In_ v_kernel kernel,
    _In_ v_kind kind)
{
    v_object o;

    assert(C_TYPECHECK(kernel,v_kernel));

    o = c_new(v_kernelType(kernel,kind));
    assert(C_TYPECHECK(o,v_object));
    o->kind = kind;
    o->kernel = kernel;
    return o;
}

_Check_return_
_Ret_maybenull_
v_object
v_objectNew_s(
    _In_ v_kernel kernel,
    _In_ v_kind kind)
{
    v_object o;

    assert(C_TYPECHECK(kernel,v_kernel));

    o = c_new_s(v_kernelType(kernel,kind));
    if (!o) {
        return NULL;
    }
    o->kind = kind;
    o->kernel = kernel;
    return o;
}

_Ret_z_
const char *
v_objectKindImage(
    _In_ v_object _this)
{
#define _CASE_(o) case o: return #o; break;
    switch (v_objectKind(_this)) {
        _CASE_(K_KERNEL);
        _CASE_(K_OBJECT);
        _CASE_(K_ENTITY);
        _CASE_(K_GROUPSET);
        _CASE_(K_STATUSCONDITION);
        _CASE_(K_OBJECTLOAN);
        _CASE_(K_OBJECTBUFFER);
        _CASE_(K_WAITSET);
        _CASE_(K_LISTENER);
        _CASE_(K_CONDITION);
        _CASE_(K_QUERY);
        _CASE_(K_DATAREADERQUERY);
        _CASE_(K_DATAVIEW);
        _CASE_(K_PROJECTION);
        _CASE_(K_MAPPING);
        _CASE_(K_FILTER);
        _CASE_(K_DEADLINEINSTANCE);
        _CASE_(K_DEADLINEINSTANCELIST);
        _CASE_(K_MESSAGE);
        _CASE_(K_MESSAGEEOT);
        _CASE_(K_EOTLISTELEMENT);
        _CASE_(K_TRANSACTIONADMIN);
        _CASE_(K_TRANSACTION);
        _CASE_(K_TRANSACTIONELEMENT);
        _CASE_(K_TRANSACTIONGROUPADMIN);
        _CASE_(K_TRANSACTIONGROUP);
        _CASE_(K_TRANSACTIONPUBLISHER);
        _CASE_(K_TRANSACTIONWRITER);
        _CASE_(K_TRANSACTIONGROUPWRITER);
        _CASE_(K_TRANSACTIONGROUPREADER);
        _CASE_(K_TRANSACTIONPENDING);
        _CASE_(K_WRITERINSTANCE);
        _CASE_(K_WRITERSAMPLE);
        _CASE_(K_WRITERCACHEITEM);
        _CASE_(K_GROUPINSTANCE);
        _CASE_(K_GROUPSAMPLE);
        _CASE_(K_GROUPCACHEITEM);
        _CASE_(K_DATAREADERINSTANCE);
        _CASE_(K_READERSAMPLE);
        _CASE_(K_DATAVIEWINSTANCE);
        _CASE_(K_DATAVIEWQUERY);
        _CASE_(K_DATAVIEWSAMPLE);
        _CASE_(K_ORDEREDINSTANCE);
        _CASE_(K_ORDEREDINSTANCESAMPLE);
        _CASE_(K_WRITERINSTANCETEMPLATE);
        _CASE_(K_TOPIC);
        _CASE_(K_TOPIC_ADAPTER);
        _CASE_(K_TYPEREPRESENTATION);
        _CASE_(K_PUBLISHER);
        _CASE_(K_SUBSCRIBER);
        _CASE_(K_DOMAIN);
        _CASE_(K_DOMAININTEREST);
        _CASE_(K_DOMAINADMIN);
        _CASE_(K_READER);
        _CASE_(K_WRITER);
        _CASE_(K_ENTRY);
        _CASE_(K_DATAREADERENTRY);
        _CASE_(K_DELIVERYSERVICEENTRY);
        _CASE_(K_GROUP);
        _CASE_(K_GROUPSTORE);
        _CASE_(K_WRITERGROUP);
        _CASE_(K_CACHE);
        _CASE_(K_GROUPACTION);
        _CASE_(K_GROUPSTREAM);
        _CASE_(K_GROUPQUEUE);
        _CASE_(K_GROUPQUEUESAMPLE);
        _CASE_(K_DATAREADER);
        _CASE_(K_DELIVERYSERVICE);
        _CASE_(K_PARTICIPANT);
        _CASE_(K_PURGELISTITEM);
        _CASE_(K_GROUPPURGEITEM);
        _CASE_(K_INDEX);
        _CASE_(K_READERSTATUS);
        _CASE_(K_WRITERSTATUS);
        _CASE_(K_PUBLISHERSTATUS);
        _CASE_(K_SUBSCRIBERSTATUS);
        _CASE_(K_DOMAINSTATUS);
        _CASE_(K_TOPICSTATUS);
        _CASE_(K_PARTICIPANTSTATUS);
        _CASE_(K_KERNELSTATUS);
        _CASE_(K_WRITERSTATISTICS);
        _CASE_(K_QUERYSTATISTICS);
        _CASE_(K_DATAREADERSTATISTICS);
        _CASE_(K_PROXY);
        _CASE_(K_WAITSETEVENT);
        _CASE_(K_LISTENEREVENT);
        _CASE_(K_WAITSETEVENTHISTORYDELETE);
        _CASE_(K_WAITSETEVENTHISTORYREQUEST);
        _CASE_(K_WAITSETEVENTPERSISTENTSNAPSHOT);
        _CASE_(K_WAITSETEVENTCONNECTWRITER);
        _CASE_(K_SERVICEMANAGER);
        _CASE_(K_SERVICE);
        _CASE_(K_SERVICESTATE);
        _CASE_(K_NETWORKING);
        _CASE_(K_DURABILITY);
        _CASE_(K_CMSOAP);
        _CASE_(K_RNR);
        _CASE_(K_NWBRIDGE);
        _CASE_(K_DBMSCONNECT);
        _CASE_(K_LEASEMANAGER);
        _CASE_(K_LEASE);
        _CASE_(K_LEASEACTION);
        _CASE_(K_SPLICED);
        _CASE_(K_CONFIGURATION);
        _CASE_(K_REGISTRATION);
        _CASE_(K_NETWORKREADER);
        _CASE_(K_NETWORKREADERENTRY);
        _CASE_(K_NETWORKMESSAGE);
        _CASE_(K_NETWORKMAPENTRY);
        _CASE_(K_HISTORICALDELETEREQUEST);
        _CASE_(K_HISTORICALDATAREQUEST);
        _CASE_(K_PERSISTENTSNAPSHOTREQUEST);
        _CASE_(K_PENDINGDISPOSEELEMENT);
        _CASE_(K_WRITEREOTSAMPLE);
        _CASE_(K_GID);
        _CASE_(K_TID);
        _CASE_(K_RXODATA);
        _CASE_(K_DURABILITYCLIENT);
        _CASE_(K_DURABILITYCLIENTEVENT);
        _CASE_(K_DURABILITYCLIENTSERVER);
        default: return "UNDEFINED"; break;
    }
#undef _CASE_
}

c_iter
v_kernelGetAttachedProcesses(
    v_kernel _this)
{
    c_iter processes;

    c_lockRead(&_this->lock);
    processes = ospl_c_select(_this->attachedProcesses, 0);
    c_lockUnlock(&_this->lock);

    return processes;
}

_Ret_notnull_
_Must_inspect_result_
v_processInfo
v_kernelGetOwnProcessInfoWeakRef (
    _Inout_ v_kernel _this)
{
    c_value keyValue;
    v_processInfo own;

    /* os_procId is modeled in the kernel odl as c_longlong */
    keyValue = c_longlongValue(os_procIdSelf());
    
    c_lockRead(&_this->lock);
    own = c_tableFindWeakRef(_this->attachedProcesses, &keyValue, 1);
    c_lockUnlock(&_this->lock);

    /* The process info for this process has to be available. It is needed to
     * in the kernel for a process to get access to the kernel and is retrieved
     * from the kernel too.
     */
    assert(own);
    return own;
}

_Ret_maybenull_
_Must_inspect_result_
v_processInfo
v_kernelGetProcessInfo(
    _Inout_ v_kernel _this,
    _In_ os_procId pid)
{
    c_value keyValue;
    v_processInfo info;

    /* os_procId is modeled in the kernel odl as c_longlong */
    keyValue = c_longlongValue(pid);

    c_lockRead(&_this->lock);
    info = c_tableFind(_this->attachedProcesses, &keyValue, 1);
    c_lockUnlock(&_this->lock);

    return info;
}

static v_processInfo
v__kernelRemoveProcessInfo(
    v_kernel _this,
    v_processInfo pi)
{
    v_processInfo info;

    assert(_this);
    assert(pi);

    c_lockWrite(&_this->lock);
    info = c_remove(_this->attachedProcesses, pi, NULL, NULL);
    c_lockUnlock(&_this->lock);

    return info;
}


v_kernel
v_kernelAttach(
    c_base base,
    const c_char *name,
    os_duration timeout,
    v_processInfo* procInfo)
{
    v_kernel kernel = NULL;
    v_processInfo procInfoSelf;
    os_timeM expiryTime;
    os_duration sleepTime = OS_DURATION_INIT(0,0);

    assert(base);
    assert(name);
    assert(procInfo);

    if (timeout > 0) {
        sleepTime = OS_DURATION_INIT(0, 100 * 1000 * 1000); /*100ms*/
        if(os_durationCompare(timeout, sleepTime) == OS_LESS){
            sleepTime = timeout;
        }
    }
    expiryTime = os_timeMAdd(os_timeMGet(), timeout);
    while ((kernel = c_lookup(base, name)) == NULL && os_timeMCompare(expiryTime, os_timeMGet()) == OS_MORE) {
        ospl_os_sleep(sleepTime);
    }
    if (kernel == NULL) {
        OS_REPORT(OS_ERROR, "v_kernelAttach", V_RESULT_ILL_PARAM, "Failed to lookup kernel with name '%s'", name);
        goto err_lookup;
    }
    if (c_checkType(kernel, "v_kernel") != kernel) {
        OS_REPORT(OS_ERROR, "v_kernelAttach", V_RESULT_ILL_PARAM, "Object with name '%s' is not of type 'v_kernel'.", name);
        goto err_kernelType;
    }
    if ((procInfoSelf = v_processInfoNew(kernel, os_procIdSelf())) == NULL){
        OS_REPORT(OS_ERROR, "v_kernelAttach", V_RESULT_INTERNAL_ERROR, "Failed to allocate v_processInfo within kernel.");
        goto err_processInfoNew;
    }

    while (((kernel->spliced == NULL) || v_serviceGetState((v_service)kernel->spliced) == STATE_INITIALISING) &&
            os_timeMCompare(expiryTime, os_timeMGet()) == OS_MORE)
    {
        ospl_os_sleep(sleepTime);
    }
    if (kernel->spliced == NULL || v_serviceGetState(v_service(kernel->spliced)) != STATE_OPERATIONAL) {
        if (os_timeMCompare(expiryTime, os_timeMGet()) == OS_MORE) {
            OS_REPORT(OS_ERROR, "v_kernelAttach",V_RESULT_ALREADY_DELETED,
                      "Attach process to Domain failed because Spliced is not running (anymore).");
        } else {
            OS_REPORT(OS_ERROR, "v_kernelAttach",V_RESULT_TIMEOUT,
                      "Attach process to Domain failed because Spliced did not become operational within the specified timeout.");
        }
        goto err_spliced_operational;
    }
    c_lockWrite(&kernel->lock);
    *procInfo = ospl_c_insert(kernel->attachedProcesses, procInfoSelf);
    c_lockUnlock(&kernel->lock);
    c_free(procInfoSelf);

    return kernel;

/* Error handling */
err_spliced_operational:
    c_free(procInfoSelf);
err_processInfoNew:
err_kernelType:
    c_free(kernel);
err_lookup:
    return NULL;
}

struct collectParticipantsArg {
    c_iter list;
    os_procId procId;
};

static c_bool
collectParticipants(
    c_object o,
    c_voidp arg)
{
    struct collectParticipantsArg *cpa = (struct collectParticipantsArg *)arg;

    if ((os_int) v_participant(o)->processId == cpa->procId) {
        cpa->list = c_iterInsert(cpa->list, c_keep(o));
    }
    return TRUE;
}

/* this is needed so that the group attachedServices and notInterestedServices list count
 * will be in sync with the v_kernelNetworkCount
 */
static c_bool
removeServiceFromGroup(
    c_object o,
    c_voidp args)
{
    v_group g = v_group(o);
    char * serviceName = (char *)args;
    v_groupRemoveAwareness(g,serviceName);
    return TRUE;
}

c_ulong
v_kernelMyProtectCount(
    v_kernel k)
{
    v_processInfo procInfo;
    os_procId procIntSelf;
    c_ulong protectCount = 0;

    procIntSelf = os_procIdSelf();

    procInfo = v_kernelGetProcessInfo(k, procIntSelf);
    if(procInfo){
        /* If the process is detaching itself, there should be no more
         * threads (not even waiting ones) in the kernel. The only valid
         * conclusion based on read access to the the atomically updated
         * protectCount in a live process (with at least 32-bit atomic
         * reads), is when it is guaranteed that protectCount will never
         * be increased anymore and the value is checked to be equal to
         * zero (which is the terminal state for the counter). Only then
         * is it a safe conclusion that there are no threads in SHM any-
         * more.
         */
        protectCount = pa_ld32 (&procInfo->protectCount) - pa_ld32 (&procInfo->blockedCount);

        if (protectCount != 0) {
            OS_REPORT(OS_ERROR, "v_kernelDetach", V_RESULT_PRECONDITION_NOT_MET,
                  "Detaching process %d (self) from kernel failed (domain %d), because there "
                   "are threads modifying shared resources (protectCount=%u, pc=%u, wc=%u, bc=%u).",
                   procIntSelf, procInfo->serial & V_KERNEL_THREAD_FLAG_DOMAINID,
                   protectCount,
                   pa_ld32 (&procInfo->protectCount),
                   pa_ld32 (&procInfo->waitCount),
                   pa_ld32 (&procInfo->blockedCount));
        }
    } else {
        OS_REPORT(OS_ERROR, "v_kernelProtectCountForProcess", 0,
                    "Failed to resolve process info record for my own process %d.", procIntSelf);
    }

    return protectCount;
}

v_result
v_kernelDetach(
    v_kernel k,
    os_procId procId)
{
    v_processInfo removed, procInfo;
    struct collectParticipantsArg cpa;
    v_participant p;
    v_kind kind;
    v_result result = V_RESULT_OK;
    c_string name;
    os_int procIntSelf;

    procIntSelf = os_procIdSelf();

    assert(k);
    assert(C_TYPECHECK(k,v_kernel));

    procInfo = v_kernelGetProcessInfo(k, procId);
    if(procInfo){
        c_ulong protectCount;
        if(procId == procIntSelf) {
            /* If the process is detaching itself, there should be no more
             * threads (not even waiting ones) in the kernel. The only valid
             * conclusion based on read access to the the atomically updated
             * protectCount in a live process (with at least 32-bit atomic
             * reads), is when it is guaranteed that protectCount will never
             * be increased anymore and the value is checked to be equal to
             * zero (which is the terminal state for the counter). Only then
             * is it a safe conclusion that there are no threads in SHM any-
             * more.
             */
            protectCount = pa_ld32 (&procInfo->protectCount) - pa_ld32 (&procInfo->blockedCount);
        } else {
            /* If the process is detaching a process that isn't running any-
             * more, the waiting threads don't have to be counted, since
             * these were sleeping (and thus not modifying SHM).
             */
            protectCount = pa_ld32 (&procInfo->protectCount) - (pa_ld32 (&procInfo->waitCount) + pa_ld32 (&procInfo->blockedCount));
        }

        if (protectCount != 0) {
            OS_REPORT(OS_ERROR, "v_kernelDetach", V_RESULT_PRECONDITION_NOT_MET,
                  "Detaching process %d%s from kernel failed (domain %d), because there "
                   "%s threads modifying shared resources (protectCount=%u, pc=%u, wc=%u, bc=%u).",
                   procId, procId == procIntSelf ? " (self)" : "",
                   procInfo->serial & V_KERNEL_THREAD_FLAG_DOMAINID,
                   procId == procIntSelf ? "are" : "were",
                   protectCount,
                   pa_ld32 (&procInfo->protectCount),
                   pa_ld32 (&procInfo->waitCount),
                   pa_ld32 (&procInfo->blockedCount));
            /* When the process did not cleanup it's resources then let the
             * shared memory monitor remove the process info of the terminated
             * process.
             */
            if(procId != procIntSelf) {
                removed = v__kernelRemoveProcessInfo(k, procInfo);
                assert(removed == procInfo);
                c_free(procInfo);
                /* Don't use v_processInfoFree(..); the mutexes aren't created by this process. */
                c_free(removed);
            }
            return V_RESULT_INTERNAL_ERROR;
        }
        cpa.procId = procId;
        cpa.list = NULL;
        c_lockRead(&k->lock);
        (void)c_walk(k->participants, collectParticipants, &cpa);
        c_lockUnlock(&k->lock);
        while ((p = v_participant(c_iterTakeFirst(cpa.list))) != NULL)
        {
            p->processIsZombie = procId != procIntSelf;
            kind = v_objectKind(v_object(p));
            switch (kind) {
            case K_SERVICE:
            case K_SPLICED:
            case K_NETWORKING:
            case K_DURABILITY:
            case K_NWBRIDGE:
            case K_CMSOAP:
            case K_RNR:
            case K_DBMSCONNECT:
                name = c_keep((c_object)v_serviceGetName(v_service(p)));
                if (name == NULL) {
                    name = c_keep(v_entityName(v_entity(p)));
                }
                if (name != NULL) {
                    v_groupSetWalk(k->groupSet, removeServiceFromGroup, name);
                }
                /* If the service has no name, it can't be in the group
                 * administration of attached/interested services either.
                 */

                (void)v_serviceChangeState(v_service(p),STATE_DIED);
                v_publicFree(v_public(p)); /* Deregister handle. */
                v_serviceFree(v_service(p)); /* Deinit service logic. */

                c_free(name);
            break;
            default:
                if (procId == procIntSelf) {
                    /* Deleting own entities is the normal case and therefore
                     * call v_publicFree() to deinit (unblock threads,
                     * invalidate handles and wait until the entity is release)
                     * and the free the entity.
                     */
                    v_publicFree(v_public(p));
                } else {
                    /* Garbage collecting others entities, the normal case
                     * doesn't work because the deinit hangs so best we can
                     * do is free resources, can do better.
                     */
                    v_participantFree(p);
                }
            break;
            }

            /* There are platforms on which it is impossible to destroy a
             * condition variable on which the process that disappeared was
             * blocking. We prefer to leak some participant related memory
             * to having to bring the entire domain down, so in case we know
             * that the disappeared process was waiting, we explicitly leak
             * our local reference.
             * NOTE: found->waitCount also includes threads that were just
             * sleeping, so we may leak more often than strictly needed.
             */
            if(procId == procIntSelf || pa_ld32 (&procInfo->waitCount) == 0){
                c_free(p);
            }
        }
        c_iterFree(cpa.list);

        removed = v__kernelRemoveProcessInfo(k, procInfo);
        assert(!removed || (removed == procInfo));
        c_free(procInfo);
        if(procId == procIntSelf) {
            v_processInfoFree(removed);
        } else {
            c_free(removed);
        }
    } else {
        result = V_RESULT_PRECONDITION_NOT_MET;
        OS_REPORT(OS_ERROR, "v_kernelDetach",result,
                    "Failed to resolve process info record for process %d.", procId);
    }
    return result;
}

c_ulong
v_kernelUserCount(
    v_kernel _this)
{
    c_ulong count;

    assert(_this);
    assert(C_TYPECHECK(_this,v_kernel));

    c_lockRead(&_this->lock);
    count = c_count(_this->attachedProcesses);
    c_lockUnlock(&_this->lock);

    return count;
}

/* v_kernelNetworkCount is used to request the number of network services
 * that the kernel's configuration expects.  Currently this value is
 * determined once during kernel initialisation, but could be extended to
 * support the concept of dynamically started/stopped/restarted services,
 * This would allow the node to adapt to the changes in the configuration.
 */
c_ulong
v_kernelNetworkCount(
    v_kernel k)
{
    assert(C_TYPECHECK(k,v_kernel));
    return k->networkServiceCount;
}

void
v_kernelSetNetworkCount(
    v_kernel k,
    c_ulong count)
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

static v_result
v_loadNetworkCount(
    v_kernel kernel,
    v_configuration config)
{
    v_result result = V_RESULT_OK;
    c_iter iter;
    v_cfElement root;
    v_cfElement serviceElement;
    c_value serviceNameValue;
    c_value serviceEnabledValue;

    c_ulong networkServiceCount = 0;

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
    return result;
}


static c_iter
getDurabilityServiceNames(
    v_kernel _this)
{
    c_iter services = NULL, iter;
    v_cfElement root;
    v_cfElement serviceElement;
    c_value serviceNameValue;
    c_value serviceEnabledValue;

    if (_this->configuration) {
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
    }
    return services;
}

v_result
v_kernelWaitForDurabilityAvailability(
    v_kernel _this,
    os_duration timeout)
{
    c_iter durabilityServices;
    c_string serviceName;
    v_serviceManager serviceManager;
    os_timeM expiryTime;
    v_serviceStateKind state;
    v_result result;
    os_duration sleepTime;

    expiryTime = os_timeMAdd(os_timeMGet(), timeout);

    /* Set sleepTime to 100 ms, unless provided timeout is less than that.*/
    sleepTime = OS_DURATION_INIT(0, 100 * 1000 * 1000); /*100ms*/

    if(os_durationCompare(timeout, sleepTime) == OS_LESS){
        sleepTime = timeout;
    }
    serviceManager = v_getServiceManager(_this);
    durabilityServices = getDurabilityServiceNames(_this);
    serviceName = (c_string)c_iterTakeFirst(durabilityServices);

    /* If no durability services have been configured, return pre-condition not met. */
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
            if(os_timeMCompare(expiryTime, os_timeMGet()) == OS_MORE){
                ospl_os_sleep(sleepTime);
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

static c_type init_kernel_type (c_base base, const char *module, const char *name)
{
    char tmp[256];
    c_type t;
    int n = snprintf (tmp, sizeof (tmp), "%s::%s", module, name);
    assert (n > 0 && (size_t)n < sizeof (tmp));
    (void) n;
    t = c_resolve (base, tmp);
    assert (t != NULL);
    return t;
}

static void init_kernel_type_cache (v_kernel kernel)
{
    struct ts { enum v_kind ix; const char *n; };
    static const struct ts tsi[] = {
        { K_KERNEL,                    "v_kernel" },
        { K_PARTICIPANT,               "v_participant" },
        { K_WAITSET,                   "v_waitset" },
        { K_LISTENER,                  "v_listener" },
        { K_QUERY,                     "v_query" },
        { K_DATAREADERQUERY,           "v_dataReaderQuery" },
        { K_DATAVIEWQUERY,             "v_dataViewQuery" },
        { K_DATAVIEW,                  "v_dataView" },
        { K_DATAVIEWSAMPLE,            "v_dataViewSample" },
        { K_DATAVIEWINSTANCE,          "v_dataViewInstance" },
        { K_PROJECTION,                "v_projection" },
        { K_MAPPING,                   "v_mapping" },
        { K_TOPIC,                     "v_topicImpl" },
        { K_TOPIC_ADAPTER,             "v_topicAdapter" },
        { K_TYPEREPRESENTATION,        "v_typeRepresentation" },
        { K_MESSAGE,                   "v_message" },
        { K_MESSAGEEOT,                "v_messageEOT" },
        { K_TRANSACTION,               "v_transaction" },
        { K_TRANSACTIONGROUP,          "v_transactionGroup" },
        { K_TRANSACTIONWRITER,         "v_transactionWriter" },
        { K_TRANSACTIONGROUPWRITER,    "v_transactionGroupWriter" },
        { K_TRANSACTIONGROUPREADER,    "v_transactionGroupReader" },
        { K_TRANSACTIONPUBLISHER,      "v_transactionPublisher" },
        { K_TRANSACTIONADMIN,          "v_transactionAdmin" },
        { K_TRANSACTIONGROUPADMIN,     "v_transactionGroupAdmin" },
        { K_TRANSACTIONELEMENT,        "v_transactionElement" },
        { K_DATAREADERINSTANCE,        "v_dataReaderInstance" },
        { K_PURGELISTITEM,             "v_purgeListItem" },
        { K_GROUPPURGEITEM,            "v_groupPurgeItem" },
        { K_READERSAMPLE,              "v_dataReaderSample" },
        { K_PUBLISHER,                 "v_publisher" },
        { K_SUBSCRIBER,                "v_subscriber" },
        { K_DOMAIN,                    "v_partition" },
        { K_DOMAININTEREST,            "v_partitionInterest" },
        { K_DOMAINADMIN,               "v_partitionAdmin" },
        { K_READER,                    "v_reader" },
        { K_OBJECTLOAN,                "v_objectLoan" },
        { K_OBJECTBUFFER,              "v_objectBuffer" },
        { K_WRITER,                    "v_writer" },
        { K_WRITERGROUP,               "v_writerGroup" },
        { K_GROUP,                     "v_group" },
        { K_GROUPSTORE,                "v_groupStore" },
        { K_GROUPINSTANCE,             "v_groupInstance" },
        { K_GROUPSAMPLE,               "v_groupSample" },
        { K_GROUPCACHEITEM,            "v_groupCacheItem" },
        { K_CACHE,                     "v_cache" },
        { K_ENTRY,                     "v_entry" },
        { K_DATAREADERENTRY,           "v_dataReaderEntry" },
        { K_GROUPACTION,               "v_groupAction" },
        { K_GROUPSTREAM,               "v_groupStream" },
        { K_GROUPQUEUE,                "v_groupQueue" },
        { K_GROUPQUEUESAMPLE,          "v_groupQueueSample" },
        { K_DATAREADER,                "v_dataReader" },
        { K_DELIVERYSERVICE,           "v_deliveryService" },
        { K_DELIVERYSERVICEENTRY,      "v_deliveryServiceEntry" },
        { K_INDEX,                     "v_index" },
        { K_FILTER,                    "v_filter" },
        { K_READERSTATUS,              "v_readerStatus" },
        { K_WRITERSTATUS,              "v_writerStatus" },
        { K_DOMAINSTATUS,              "v_partitionStatus" },
        { K_TOPICSTATUS,               "v_topicStatus" },
        { K_SUBSCRIBERSTATUS,          "v_subscriberStatus" },
        { K_PUBLISHERSTATUS,           "v_status" },
        { K_PARTICIPANTSTATUS,         "v_status" },
        { K_STATUSCONDITION,           "v_statusCondition" },
        { K_KERNELSTATUS,              "v_kernelStatus" },
        { K_DATAREADERSTATISTICS,      "v_dataReaderStatistics" },
        { K_WRITERSTATISTICS,          "v_writerStatistics" },
        { K_QUERYSTATISTICS,           "v_queryStatistics" },
        { K_LEASE,                     "v_lease" },
        { K_LEASEACTION,               "v_leaseAction" },
        { K_SERVICEMANAGER,            "v_serviceManager" },
        { K_SERVICE,                   "v_service" },
        { K_SERVICESTATE,              "v_serviceState" },
        { K_NETWORKING,                "v_networking" },
        { K_DURABILITY,                "v_durability" },
        { K_NWBRIDGE,                  "v_nwbridge" },
        { K_CMSOAP,                    "v_cmsoap" },
        { K_RNR,                       "v_rnr" },
        { K_DBMSCONNECT,               "v_dbmsconnect" },
        { K_LEASEMANAGER,              "v_leaseManager" },
        { K_GROUPSET,                  "v_groupSet" },
        { K_PROXY,                     "v_proxy" },
        { K_LISTENEREVENT,             "v_listenerEvent" },
        { K_WAITSETEVENT,              "v_waitsetEvent" },
        { K_WRITERSAMPLE,              "v_writerSample" },
        { K_WRITERINSTANCE,            "v_writerInstance" },
        { K_WRITERINSTANCETEMPLATE,    "v_writerInstanceTemplate" },
        { K_WRITERCACHEITEM,           "v_writerCacheItem" },
        { K_NETWORKREADER,             "v_networkReader" },
        { K_NETWORKREADERENTRY,        "v_networkReaderEntry" },
        { K_SPLICED,                   "v_spliced" },
        { K_CONFIGURATION,             "v_configuration" },
        { K_REGISTRATION,              "v_registration" },
        { K_HISTORICALDELETEREQUEST,   "v_historicalDeleteRequest" },
        { K_HISTORICALDATAREQUEST,     "v_historicalDataRequest" },
        { K_PERSISTENTSNAPSHOTREQUEST, "v_persistentSnapshotRequest" },
        { K_PENDINGDISPOSEELEMENT,     "v_pendingDisposeElement" },
        { K_WRITEREOTSAMPLE,           "v_writerEotSample" },
        { K_ORDEREDINSTANCESAMPLE,     "v_orderedInstanceSample" },
        { K_ORDEREDINSTANCE,           "v_orderedInstance" },
        { K_RXODATA,                   "v_rxoData" },
        { K_DURABILITYCLIENT,          "v_durabilityClient" },
        { K_DURABILITYCLIENTEVENT,     "v_durabilityClientEvent" },
        { K_DURABILITYCLIENTSERVER,    "v_durabilityClientServer" }
    };
    static const struct ts tsx[] = {
        { K_GID,                       "v_gid" },
        { K_TID,                       "v_tid" }
    };
    c_base const base = c_getBase (kernel);
    size_t i;
    for (i = 0; i < sizeof (tsi) / sizeof (tsi[0]); i++) {
        kernel->type[tsi[i].ix] = init_kernel_type (base, "kernelModuleI", tsi[i].n);
    }
    for (i = 0; i < sizeof (tsx) / sizeof (tsx[0]); i++) {
        kernel->type[tsx[i].ix] = init_kernel_type (base, "kernelModule", tsx[i].n);
    }
}


#define MILLION 1000000
_Check_return_
_Ret_maybenull_
_Success_(return != NULL)
v_kernel
v_kernelNew(
    _In_ c_base base,
    _In_z_ const c_char *name,
    _In_ v_kernelQos qos,
    _Outptr_ v_processInfo* procInfo)
{
    v_kernel kernel;
    os_duration retPeriod;
    c_type type;

    assert(1 <= qos->systemIdConfig.min && qos->systemIdConfig.min <= qos->systemIdConfig.max && qos->systemIdConfig.max <= 0x7fffffff);

    kernel = c_lookup(base,name);
    if (kernel != NULL) {
        v_processInfo procInfoSelf;

        assert(C_TYPECHECK(kernel,v_kernel));

        if ((procInfoSelf = v_processInfoNew(kernel, os_procIdSelf())) == NULL){
            OS_REPORT(OS_ERROR, "v_kernelAttach", V_RESULT_INTERNAL_ERROR, "Failed to allocate v_processInfo within kernel.");
            c_free(kernel);
            *procInfo = NULL;
            kernel = NULL;
        } else {
            c_lockWrite(&kernel->lock);
            *procInfo = ospl_c_insert(kernel->attachedProcesses, procInfoSelf);
            c_lockUnlock(&kernel->lock);

            c_free(procInfoSelf);
        }
        return kernel;
    }

    if (!loadkernelModuleI(base)) {
        OS_REPORT(OS_ERROR,
                  "v_kernelNew",V_RESULT_INTERNAL_ERROR,
                  "Failed to load kernel module.");
        return NULL;
    }

    type = c_resolve(base,"kernelModuleI::v_kernel");
    assert(type);
    kernel = c_new(type);
    c_free(type);
    v_objectKind(kernel) = K_KERNEL;
    v_object(kernel)->kernel = (c_voidp)kernel;
    kernel->handleServer = v_handleServerNew(base);

    init_kernel_type_cache (kernel);

    *procInfo = v_processInfoNew(kernel, os_procIdSelf());
    if(!*procInfo) {
        OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR, "Failed to allocate v_processInfo within kernel.");
        c_free(kernel);
        return NULL;
    }
    type = c_resolve(base, "kernelModuleI::v_processInfo");
    kernel->attachedProcesses = c_tableNew(type, "processId");
    c_free(type);
    (void) ospl_c_insert(kernel->attachedProcesses, *procInfo);
    c_free(*procInfo); /* procInfo should be returned to the caller without a refcount */

    kernel->pendingDisposeList = c_listNew(v_kernelType(kernel, K_PENDINGDISPOSEELEMENT ));
    (void)c_mutexInit(c_getBase(kernel), &kernel->pendingDisposeListMutex);

    v_entityInit(v_entity(kernel), name);
    c_lockInit(c_getBase(kernel), &kernel->lock);
    {
        /* Fill GID with 'random' value */
        memset(&kernel->GID, 0, sizeof(kernel->GID));
        kernel->GID.systemId = os_uniqueNodeIdGet(qos->systemIdConfig.min, qos->systemIdConfig.max, qos->systemIdConfig.entropySize, qos->systemIdConfig.entropy);
    }
    kernel->qos = v_kernelQosNew(kernel, qos);
    kernel->statistics = v_kernelStatisticsNew(kernel);
    kernel->spliced = NULL;
    kernel->participants = c_setNew(v_kernelType(kernel,K_PARTICIPANT));
    kernel->partitions = c_tableNew(v_kernelType(kernel,K_DOMAIN),"name");
    kernel->topics = c_tableNew(v_kernelType(kernel,K_TOPIC),"name");
    kernel->typeRepresentations = c_tableNew(v_kernelType(kernel,K_TYPEREPRESENTATION),"typeName,dataRepresentationId,typeHash.msb,typeHash.lsb");
    kernel->groupSet = v_groupSetNew(kernel);
    kernel->serviceManager = v_serviceManagerNew(kernel);
    kernel->livelinessLM = v_leaseManagerNew(kernel);
    kernel->configuration = NULL;
    kernel->networkServiceCount = 0;
    pa_st32 (&kernel->transactionCount, 0);
    kernel->transactionGroupAdmin = NULL;

    pa_st32(&kernel->accessCount, 0);
    kernel->accessBusy = FALSE;
    (void)c_mutexInit(c_getBase(kernel), &kernel->accessLock);
    c_condInit(c_getBase(kernel), &kernel->accessCond, &kernel->accessLock);

    kernel->splicedRunning = TRUE;
    kernel->maxSamplesWarnLevel = V_KERNEL_MAX_SAMPLES_WARN_LEVEL_DEF;
    kernel->maxSamplesWarnShown = FALSE;
    kernel->maxSamplesPerInstanceWarnLevel = V_KERNEL_MAX_SAMPLES_PER_INSTANCES_WARN_LEVEL_DEF;
    kernel->maxSamplesPerInstanceWarnShown = FALSE;
    kernel->maxInstancesWarnLevel = V_KERNEL_MAX_INSTANCES_WARN_LEVEL_DEF;
    kernel->maxInstancesWarnShown = FALSE;
    kernel->enabledStatisticsCategories = c_listNew(c_string_t(base));

    retPeriod = V_KERNEL_RETENTION_PERIOD_DEF * OS_DURATION_MILLISECOND;
    kernel->retentionPeriod = retPeriod;
    kernel->shares = c_tableNew(v_kernelType(kernel,K_SUBSCRIBER), "qos.share.v.name");
    kernel->deliveryService = NULL;

    kernel->durabilitySupport = FALSE;
    kernel->hasDurabilityService = FALSE;
    kernel->durabilityAligned = FALSE;
    pa_st32 (&kernel->purgeSuppressCount, 0);
    pa_st32 (&kernel->isolate, 0);

    return kernel;
}

void
v_kernelEnable(
    _Inout_ v_kernel kernel)
{
    kernel->builtin = v_builtinNew(kernel);
    kernel->spliced = v_service(v_splicedNew(kernel));
    ospl_c_bind(kernel, v_entityName(v_entity(kernel)));
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
    assert(v_objectKind(partition) == K_DOMAIN);

    c_lockWrite(&kernel->lock);
    found = ospl_c_insert(kernel->partitions,partition);
    if (found == partition) {
        found->addCount = 1;
    } else if (found != NULL) {
        found->addCount++;
        (void)c_keep(found);
    }
    c_lockUnlock(&kernel->lock);

    return found;
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
    if (partition->addCount == 0) {
        found = NULL;
    } else {
        partition->addCount--;
        if (partition->addCount == 0) {
            found = c_remove(kernel->partitions,partition,NULL,NULL);
            assert(found == partition);
        } else {
            /* keep required as found is a new reference to the partition */
            found = c_keep(partition);
        }
    }
    c_lockUnlock(&kernel->lock);

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
    assert(v_objectKind(topic) == K_TOPIC);

    c_lockWrite(&kernel->lock);
    found = ospl_c_insert(kernel->topics,topic);
    c_lockUnlock(&kernel->lock);

    return found;
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
    found = ospl_c_insert(kernel->participants,p);
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

v_typeRepresentation
v__addTypeRepresentation (
    v_kernel kernel,
    v_typeRepresentation tr)
{
    v_typeRepresentation found;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    c_lockWrite(&kernel->lock);
    found = ospl_c_insert(kernel->typeRepresentations, tr);
    c_lockUnlock(&kernel->lock);

    return found;
}

v_typeRepresentation
v__removeRepresentation (
    v_kernel kernel,
    v_typeRepresentation tr)
{
    v_typeRepresentation found;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    c_lockWrite(&kernel->lock);
    found = c_remove(kernel->typeRepresentations, tr, NULL, NULL);
    c_lockUnlock(&kernel->lock);

    return found;
}

static c_bool
alwaysFalseTypeRepresentation (
    c_object found,
    c_object requested,
    c_voidp arg)
{
    v_typeRepresentation *tr = (v_typeRepresentation *)arg;

    assert(tr != NULL);
    assert(*tr == NULL);

    OS_UNUSED_ARG(requested);

    *tr = v_typeRepresentation(c_keep(found));
    return FALSE;
}

v_typeRepresentation
v__lookupTypeRepresentation (
    v_kernel kernel,
    const c_string typeName,
    v_dataRepresentationId_t dataRepresentationId,
    const v_typeHash typeHash)
{
    v_typeRepresentation found = NULL;
    C_STRUCT(v_typeRepresentation) dummy;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(typeName != NULL);

    memset(&dummy, 0, sizeof(dummy));
    dummy.typeName = typeName;
    dummy.dataRepresentationId = dataRepresentationId;
    dummy.typeHash = typeHash;

    c_lockRead(&kernel->lock);
    /* This does not remove anything because the alwaysFalse function always returns false */
    c_remove(kernel->typeRepresentations, &dummy, alwaysFalseTypeRepresentation, &found);
    c_lockUnlock(&kernel->lock);

    return found;
}

v_subscriber
v_kernelAddSharedSubscriber(
    v_kernel kernel,
    v_subscriber subscriber)
{
    v_subscriber found;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(subscriber != NULL);
    assert(C_TYPECHECK(subscriber,v_subscriber));

    OSPL_LOCK(kernel);
    found = ospl_c_insert(kernel->shares,subscriber);
    if (found != subscriber) {
        found->shareCount++;
    }
    OSPL_UNLOCK(kernel);
    return found;
}

c_ulong
v_kernelRemoveSharedSubscriber(
    v_kernel kernel,
    v_subscriber subscriber)
{
    c_ulong count;
    v_subscriber found;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(subscriber != NULL);
    assert(C_TYPECHECK(subscriber,v_subscriber));

    OSPL_LOCK(kernel);
    count = --subscriber->shareCount;
    if (count == 0) {
        found = c_remove(kernel->shares,subscriber,NULL,NULL);
        c_free(found);
    }
    OSPL_UNLOCK(kernel);
    return count;
}


struct v_resolveServiceByTypeHelperArg {
    c_iter iter;
    v_serviceType type;
};

c_bool v_resolveServiceByTypeHelper(
    c_object o,
    c_voidp varg)

{
    v_participant participant = v_participant(o);
    struct v_resolveServiceByTypeHelperArg *arg = varg;

    if (c_instanceOf(participant, "v_service") && v_service(participant)->serviceType == arg->type) {
        arg->iter = c_iterAppend(arg->iter, c_keep(participant));
    }
    return TRUE;
}


c_iter
v_resolveServiceByServiceType(
   v_kernel kernel,
   v_serviceType type)
{
    struct v_resolveServiceByTypeHelperArg arg;
    arg.iter = NULL;
    arg.type = type;
    c_lockRead(&kernel->lock);
    (void)c_setWalk(kernel->participants, v_resolveServiceByTypeHelper, &arg);
    c_lockUnlock(&kernel->lock);
    return arg.iter;
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
    C_STRUCT(v_topicImpl) dummyTopic;
    c_base base = c_getBase(c_object(kernel));

    /* Create a dummy topic for look-up */
    memset(&dummyTopic, 0, sizeof(dummyTopic));
    ((v_entity)(&dummyTopic))->name = c_stringNew(base,name);
    topicFound = NULL;
    c_lockRead(&kernel->lock);
    /* This does not remove anything because the alwaysFalse function always returns false */
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

v_result
v_kernelConfigure(
    v_kernel kernel,
    v_configuration config)
{
    v_result result = V_RESULT_OK;
    v_configuration old;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(config != NULL);
    assert(C_TYPECHECK(config,v_configuration));

    old = kernel->configuration;
    kernel->configuration = c_keep(config);

    result = v_loadTimeSpec(kernel);
    if (result == V_RESULT_OK) {
        result = v_loadWarningLevels(kernel, config);
    }
    if (result == V_RESULT_OK) {
        result = v_loadNetworkCount(kernel, config);
    }
    if (result == V_RESULT_OK) {
        result = v_loadRetentionPeriod(kernel, config);
    }
    if (result == V_RESULT_OK) {
        result = v_loadDurabilitySupport(kernel);
    }

    if (old != NULL) {
        if (result != V_RESULT_OK) {
            /* rollback */
            (void)v_kernelConfigure(kernel, old);
        }
        c_free(old);
    }
    return result;
}

static v_result
v_loadTimeSpec(
    v_kernel kernel)
{
    v_result result = V_RESULT_OK;
    v_cfElement root;
    v_cfData data;
    c_iter iter;
    c_value attr_val;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel, v_kernel));

    root = v_configurationGetRoot(kernel->configuration);

    iter = v_cfElementXPath(root, "Domain/y2038Ready/#text");
    if (c_iterLength(iter) > 1) {
        result = V_RESULT_PRECONDITION_NOT_MET;
        OS_REPORT(OS_ERROR,
                  "v_kernel::v_loadTimeSpec",result,
                  "Configuration error: <y2038Ready> specified multiple times.");
    } else {
        data = v_cfData(c_iterTakeFirst(iter));
        if (data) {
            attr_val = v_cfDataValue(data);
            if (attr_val.kind == V_STRING && os_strcasecmp(attr_val.is.String, "true") == 0) {
                (void)ospl_c_bind(c_object(kernel), "y2038ready");
                c_baseSetY2038Ready(c_getBase(c_object(kernel)), TRUE);
            }
        }
    }
    c_iterFree(iter);
    c_free(root);
    return result;
}

static v_result
v_loadDurabilitySupport(
    v_kernel kernel)
{
    v_result result = V_RESULT_OK;
    v_cfElement root, elem;
    c_iter iter;
    c_value attr_val;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel, v_kernel));

    root = v_configurationGetRoot(kernel->configuration);

    /* Check if one of the enabled services is a durability-service */
    iter = v_cfElementXPath(root, "Domain/Service");
    elem = v_cfElement(c_iterTakeFirst(iter));
    while (elem) {
        attr_val = v_cfElementAttributeValue(elem, "enabled");
        if (attr_val.kind == V_UNDEFINED || os_strcasecmp(attr_val.is.String, "true") == 0) {
            attr_val = v_cfElementAttributeValue(elem, "name");
            assert(attr_val.kind == V_STRING);
            if (isServiceRequestedServiceKind("DurabilityService", attr_val.is.String, kernel->configuration)) {
                kernel->hasDurabilityService = kernel->durabilitySupport = TRUE;
                break;
            }
        }
        elem = v_cfElement(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);

    /* In case no durablity-service exists, check if client-durability is enabled */
    if (!kernel->durabilitySupport) {
        iter = v_cfElementXPath(root, "Domain/DurablePolicies/Policy");
        if (c_iterLength(iter) > 0) {
            kernel->durabilitySupport = TRUE;
        }
        c_iterFree(iter);
    }
    c_free(root);
    return result;
}

static v_result
v_loadWarningLevels(
    v_kernel kernel,
    v_configuration config)
{
    v_result result = V_RESULT_OK;
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
    return result;
}

#define MILLION 1000000
static v_result
v_loadRetentionPeriod(
    v_kernel kernel,
    v_configuration config)
{
    v_result result = V_RESULT_OK;
    c_iter iter;
    v_cfData elementData = NULL;
    c_value value;
    v_cfElement root;
    os_duration delay;
    c_ulong retPeriod;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));
    assert(config != NULL);
    assert(C_TYPECHECK(config,v_configuration));

    root = v_configurationGetRoot(config);
    /* load max samples warn level */
    iter = v_cfElementXPath(root, "Domain/RetentionPeriod/#text");
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
        (void)sscanf(value.is.String, "%u", &retPeriod);
        if(retPeriod < V_KERNEL_RETENTION_PERIOD_MIN)
        {
            retPeriod = V_KERNEL_RETENTION_PERIOD_MIN;
        }
        delay = retPeriod * OS_DURATION_MILLISECOND;
        kernel->retentionPeriod = delay;

    }
    elementData = NULL;
    return result;
}
#undef MILLION

void
v_checkMaxInstancesWarningLevel(
    v_kernel _this,
    c_ulong count)
{
    if(count >= _this->maxInstancesWarnLevel && !_this->maxInstancesWarnShown)
    {
        OS_REPORT(OS_API_INFO,
            "v_checkMaxInstancesWarningLevel", V_RESULT_OK,
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
        OS_REPORT(OS_API_INFO,
            "v_checkMaxSamplesWarningLevel", V_RESULT_OK,
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
        OS_REPORT(OS_API_INFO,
            "v_checkMaxSamplesPerInstanceWarningLevel", V_RESULT_OK,
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
                (void)v_writerWrite(writer,msg,os_timeWGet(),NULL);
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
                v_writerWriteDispose(writer,msg,os_timeWGet(),NULL);
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
            v_writerUnregister(writer,msg,os_timeWGet(),NULL);
        }
    }
}


/* Statistics enabling/disabling */

static c_bool
statisticsCategoryCompareAction(
    c_object o,
    c_voidp arg)
{
    c_bool result;
    c_string category = o;
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
        statisticsCategoryCompareAction, (c_voidp)categoryName);

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
    c_string category;
    c_base base;

    (void)c_lockWrite(&k->lock);
    /* Only add this if it is not yet enabled */
    if (!isEnabledStatisticsUnlocked(k, categoryName)) {
        base = c_getBase(k);
        category = c_stringNew(base, categoryName);
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
    /* TODO: we will have to walk over all entities and forward the disable
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

#define v_persistentSnapshotRequest(o) (C_CAST(o, v_persistentSnapshotRequest))

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
    c_base base;


    request = v_persistentSnapshotRequest(v_objectNew(_this, K_PERSISTENTSNAPSHOTREQUEST));
    if(request) {
        base = c_getBase(_this);
        if (partition_expression) {
            request->partitionExpr = c_stringNew(base, partition_expression);
        }
        if (topic_expression) {
            request->topicExpr = c_stringNew(base, topic_expression);
        }
        if (uri) {
            request->uri = c_stringNew(base, uri);
        }
        event.kind = V_EVENT_PERSISTENT_SNAPSHOT;
        event.source = v_observable(_this);
        event.data = request;
        event.handled = TRUE;

        OSPL_THROW_EVENT(_this, &event);
        c_free(request);
    } else {
        result = V_RESULT_OUT_OF_MEMORY;
        OS_REPORT(OS_ERROR,
                  "v_kernel::v_persistentSnapshotRequest",result,
                  "Failed to create v_persistentSnapshotRequest object.");
        assert(FALSE);
    }
    return result;
}

/* ES, dds1576: This method consults the configuration info stored in the kernel
 * to determine the access policy for this partition
 */
v_accessMode
v_kernelPartitionAccessMode(
    v_kernel _this,
    v_partitionPolicyI partition)
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

v_transactionGroupAdmin
v_kernelTransactionGroupAdmin(
    v_kernel _this)
{
    v_transactionGroupAdmin admin = NULL;

    c_lockWrite(&_this->lock);
    if (_this->transactionGroupAdmin == NULL) {
        _this->transactionGroupAdmin = v_transactionGroupAdminNew(v_object(_this));
        assert(_this->transactionGroupAdmin);
    }
    admin = _this->transactionGroupAdmin;
    c_lockUnlock(&_this->lock);
    return admin;
}

void
v_kernelGroupTransactionFlush(
    v_kernel _this,
    v_transactionAdmin admin)
{
    v_transactionGroupAdmin transactionGroupAdmin;
    assert(_this);

    c_lockRead(&_this->lock);
    transactionGroupAdmin = c_keep(_this->transactionGroupAdmin);
    c_lockUnlock(&_this->lock);

    if (transactionGroupAdmin) {
        if (v_kernelGroupTransactionTryLockAccess(_this)) {
            v_transactionGroupAdminFlushPending(transactionGroupAdmin, admin);
            v_kernelGroupTransactionUnlockAccess(_this);
        }
        c_free(transactionGroupAdmin);
    }
}

/**
 * \brief              This operation tries to obtain the access lock which
 *                     is used for group transactions
 *
 * \param _this      : The kernel this operation operates on.
 *
 * \return           : TRUE when access locked as result of this operation
 *                     FALSE operation was unable to lock access
 */
c_bool
v_kernelGroupTransactionTryLockAccess(
    v_kernel _this)
{
    c_bool result = FALSE;

    assert(v_objectKind(_this) == K_KERNEL);

    c_mutexLock(&_this->accessLock);
    if (pa_ld32(&_this->accessCount) > 0 || _this->accessBusy) {
        /* Unable to get lock */
    } else {
       result = _this->accessBusy = TRUE;
#ifndef NDEBUG
       _this->accessOwner = os_threadIdToInteger(os_threadIdSelf());
#endif
    }
    c_mutexUnlock(&_this->accessLock);

    return result;
}

void
v_kernelGroupTransactionUnlockAccess(
    v_kernel _this)
{
    assert(v_objectKind(_this) == K_KERNEL);

    c_mutexLock(&_this->accessLock);
    if (_this->accessBusy) {
        _this->accessBusy = FALSE;
#ifndef NDEBUG
        _this->accessOwner = 0;
#endif
        c_condBroadcast(&_this->accessCond);
    }
    c_mutexUnlock(&_this->accessLock);
}

void
v_kernelGroupTransactionBeginAccess(
    _Inout_ v_kernel _this)
{
    c_bool flush = FALSE;
    v_transactionGroupAdmin transactionGroupAdmin;

    c_mutexLock(&_this->accessLock);
    while (_this->accessBusy) {
        c_condWait(&_this->accessCond, &_this->accessLock);
    }
    if (pa_ld32(&_this->accessCount) == 0) {
        _this->accessBusy = TRUE;
        flush = _this->accessBusy;
    }
    (void)pa_inc32_nv(&_this->accessCount);
    c_mutexUnlock(&_this->accessLock);

    if (flush) {
        (void)c_lockRead(&_this->lock);
        transactionGroupAdmin = c_keep(_this->transactionGroupAdmin);
        c_lockUnlock(&_this->lock);

        if (transactionGroupAdmin) {
            v_transactionGroupAdminFlushPending(transactionGroupAdmin, NULL);
            c_free(transactionGroupAdmin);
        }
        c_mutexLock(&_this->accessLock);
        _this->accessBusy = FALSE;
        c_condBroadcast(&_this->accessCond);
        c_mutexUnlock(&_this->accessLock);
    }
}

void
v_kernelGroupTransactionEndAccess(
    _Inout_ v_kernel _this)
{
    c_bool flush;
    v_transactionGroupAdmin transactionGroupAdmin;

    /* To update access counts, the access lock must be acquired. */
    c_mutexLock(&_this->accessLock);

    assert(pa_ld32(&_this->accessCount) > 0);
    flush = (pa_dec32_nv(&_this->accessCount) == 0);
    if (flush) {
        _this->accessBusy = TRUE; /* must set to hold off others until after the flush. */
    }
    c_mutexUnlock(&_this->accessLock);

    if (flush) {
        (void)c_lockRead(&_this->lock);
        transactionGroupAdmin = c_keep(_this->transactionGroupAdmin);
        c_lockUnlock(&_this->lock);

        if (transactionGroupAdmin) {
            v_transactionGroupAdminFlushPending(transactionGroupAdmin, NULL);
            c_free(transactionGroupAdmin);
        }
        c_mutexLock(&_this->accessLock);
        _this->accessBusy = FALSE;
        c_condBroadcast(&_this->accessCond);
        c_mutexUnlock(&_this->accessLock);
    }
}

static c_bool
collectAllParticipants(
    c_object o,
    c_voidp arg)
{
    c_iter list = (c_iter)arg;
    list = c_iterInsert(list, c_keep(o));
    return TRUE;
}

/* This operation will update the kernel according to a newly discovered publication and
 * notify matching local subscriptions (synchronously).
 * The given message holds the builtin publication data describing the discovered writer.
 */
void
v_kernelNotifyPublication(
    v_kernel _this,
    v_message msg)
{
    c_iter list;
    v_participant participant;

    c_lockRead(&_this->lock);
    list = c_iterNew(NULL);
    (void)c_walk(_this->participants, collectAllParticipants, list);
    c_lockUnlock(&_this->lock);
    while ((participant = c_iterTakeFirst(list)) != NULL) {
        v_participantNotifyPublication(participant, msg);
        c_free(participant);
    }
    c_iterFree(list);
}

/* This operation will update the kernel according to a newly discovered subscription and
 * notify matching local publications (synchronously).
 * The given message holds the builtin subscription data describing the discovered reader.
 */
void
v_kernelNotifySubscription(
    v_kernel _this,
    v_message msg)
{
    c_iter list;
    v_participant participant;

    c_lockRead(&_this->lock);
    list = c_iterNew(NULL);
    (void)c_walk(_this->participants, collectAllParticipants, list);
    c_lockUnlock(&_this->lock);
    while ((participant = c_iterTakeFirst(list)) != NULL) {
        v_participantNotifySubscription(participant, msg);
        c_free(participant);
    }
    c_iterFree(list);
}

void
v_kernelNotifyCoherentPublication(
    v_kernel _this,
    v_message msg)
{
    c_iter list;
    c_ulong nrOfPartitions, i;
    v_group group;
    v_participant participant;
    struct v_publicationInfo *info;

    info = v_builtinPublicationInfoData(msg);

    /* transactionGroupAdmin only exists if there is interest for Group Coherence.
     * ignore notification if no interest exists.
     */
    if ((_this->transactionGroupAdmin) && (info->presentation.coherent_access)) {
        /* Notify all matching groups */
        nrOfPartitions = c_arraySize(info->partition.name);
        for (i=0; i<nrOfPartitions; i++) {
            list = v_groupSetLookup(_this->groupSet, info->partition.name[i], info->topic_name);
            while ((group = c_iterTakeFirst(list)) != NULL) {
                v_groupNotifyCoherentPublication(group, msg);
            }
            c_iterFree(list);
        }
    }

    if (info->presentation.access_scope == V_PRESENTATION_GROUP) {
        c_lockRead(&_this->lock);
        list = c_iterNew(NULL);
        (void)c_walk(_this->participants, collectAllParticipants, list);
        c_lockUnlock(&_this->lock);
        while ((participant = c_iterTakeFirst(list)) != NULL) {
            v_participantNotifyGroupCoherentPublication(participant, msg);
            c_free(participant);
        }
        c_iterFree(list);
    }
}

v_result
v_condWait (
    c_cond *cnd,
    c_mutex *mtx,
    const os_duration delay)
{
    v_result result = V_RESULT_OK;
    c_syncResult sr;

    v__kernelProtectWaitEnter(cnd, mtx);
    sr = c_condTimedWait(cnd, mtx, delay);
    v__kernelProtectWaitExit();

    switch(sr) {
        case SYNC_RESULT_TIMEOUT: result = V_RESULT_TIMEOUT; break;
        case SYNC_RESULT_BUSY:    result = V_RESULT_PRECONDITION_NOT_MET; break;
        case SYNC_RESULT_SUCCESS: result = V_RESULT_OK; break;
        default:                  result = V_RESULT_INTERNAL_ERROR; break;
    }

    return result;
}

v_result
v_kernel_load_xml_descriptor (
    v_kernel _this,
    const os_char *xml_descriptor)
{
    sd_serializer serializer;
    sd_serializedData serData;
    c_metaObject type;
    c_base base;
    v_result result;

    base = c_getBase(c_object(_this));
    if ( base ) {
        serializer = sd_serializerXMLTypeinfoNew(base, TRUE);
        serData = sd_serializerFromString(serializer, xml_descriptor);
        type = c_metaObject(sd_serializerDeserialize(serializer, serData));
        if (type != NULL) {
            c_free(type);
            result = V_RESULT_OK;
        } else {
            result = V_RESULT_ILL_PARAM;
        }
        sd_serializedDataFree(serData);
        sd_serializerFree(serializer);
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}

os_char *
v_kernel_get_xml_descriptor (
    v_kernel _this,
    const os_char *type_name)
{
    sd_serializer serializer;
    sd_serializedData serData;
    c_type type;
    c_base base = NULL;
    os_char *description = NULL;

    base = c_getBase(c_object(_this));
    if (base != NULL) {
        type = c_resolve(base, type_name);
        if ( type ) {
            serializer = sd_serializerXMLTypeinfoNew(base, FALSE);
            if ( serializer ) {
                serData = sd_serializerSerialize(serializer, (c_object)type);
                if ( serData ) {
                    description = sd_serializerToString(serializer, serData);
                    sd_serializedDataFree(serData);
                }
                sd_serializerFree(serializer);
            }
            c_free(type);
        }
    }
    return description;
}

c_type
v_kernel_lookup_type(
    v_kernel _this,
    const os_char *type_name)
{
    c_type type = NULL;
    c_base base = NULL;

    base = c_getBase(c_object(_this));
    if (base != NULL) {
        type = c_resolve(base, type_name);
    }
    return type;
}

c_bool
v_sampleMaskPass(
    v_sampleMask mask,
    const c_object sample)
{
    v_state sampleState;
    v_state instanceState;
    v_dataReaderInstance instance;
    v_sampleMask result = 0;
    c_bool pass = TRUE;

    if (sample) {
        sampleState = v_readerSampleState(v_readerSample(sample));
        if (v_stateTestOr(sampleState,L_READ | L_LAZYREAD)) {
            result = mask & V_MASK_READ_SAMPLE;
        } else {
            result = mask & V_MASK_NOT_READ_SAMPLE;
        }
        if (result != 0) {
            instance = v_readerSampleInstance(sample);
            instanceState = v_instanceState(instance);
            if (v_stateTestOr(instanceState,L_NEW | L_LAZYNEW)) {
                result = mask & V_MASK_NEW_VIEW;
            } else {
                result = mask & V_MASK_NOT_NEW_VIEW;
            }
        }
        if (result != 0) {
            if (v_objectKind(instance) == K_DATAVIEWINSTANCE) {
                instance = v_readerSampleInstance(v_dataViewSampleTemplate(sample)->sample);
                instanceState = v_instanceState(instance);
            }
            if (v_stateTest(instanceState,L_DISPOSED)) {
                result = mask & V_MASK_DISPOSED_INSTANCE;
            } else if (v_stateTest(instanceState,L_NOWRITERS)) {
                result = mask & V_MASK_NOWRITERS_INSTANCE;
            } else {
                result = mask & V_MASK_ALIVE_INSTANCE;
            }
        }
        pass = (result == 0 ? FALSE : TRUE);
    }
    return pass;
}

_Ret_z_
const os_char *
v_resultImage(
    _In_ v_result result)
{
    const os_char *image;

#define _CASE_(o) case o: image = #o; break;
    switch (result) {
    _CASE_(V_RESULT_UNDEFINED);
    _CASE_(V_RESULT_OK);
    _CASE_(V_RESULT_INTERRUPTED);
    _CASE_(V_RESULT_NOT_ENABLED);
    _CASE_(V_RESULT_OUT_OF_MEMORY);
    _CASE_(V_RESULT_INTERNAL_ERROR);
    _CASE_(V_RESULT_ILL_PARAM);
    _CASE_(V_RESULT_CLASS_MISMATCH);
    _CASE_(V_RESULT_DETACHING);
    _CASE_(V_RESULT_TIMEOUT);
    _CASE_(V_RESULT_INCONSISTENT_QOS);
    _CASE_(V_RESULT_IMMUTABLE_POLICY);
    _CASE_(V_RESULT_PRECONDITION_NOT_MET);
    _CASE_(V_RESULT_ALREADY_DELETED);
    _CASE_(V_RESULT_HANDLE_EXPIRED);
    _CASE_(V_RESULT_NO_DATA);
    _CASE_(V_RESULT_UNSUPPORTED);
    default:
        image = "Internal error: no image for illegal result value";
    break;
    }
    return image;
#undef _CASE_
}

const os_char *
v_writeResultImage(
    const v_writeResult result)
{
    const os_char *image;

#define _CASE_(o) case o: image = #o; break;
    switch (result) {
    _CASE_(V_WRITE_UNDEFINED);
    _CASE_(V_WRITE_SUCCESS);
    _CASE_(V_WRITE_SUCCESS_NOT_STORED);
    _CASE_(V_WRITE_REGISTERED);
    _CASE_(V_WRITE_UNREGISTERED);
    _CASE_(V_WRITE_PRE_NOT_MET);
    _CASE_(V_WRITE_ERROR);
    _CASE_(V_WRITE_TIMEOUT);
    _CASE_(V_WRITE_OUT_OF_RESOURCES);
    _CASE_(V_WRITE_REJECTED);
    _CASE_(V_WRITE_DUPLICATE);
    _CASE_(V_WRITE_COUNT);
    default:
        image = "Internal error: no image for illegal result value";
    break;
    }
    return image;
#undef _CASE_
}

os_int
v_resultToReturnCode(
    v_result result)
{
    os_int code = OS_RETCODE_ERROR;

    assert ((result & OS_RETCODE_ID_MASK) == OS_RETCODE_ID_V_RESULT);

    switch (result) {
        case V_RESULT_OK:
            code = OS_RETCODE_OK;
            break;
        case V_RESULT_INTERRUPTED:
            code = OS_RETCODE_ERROR;
            break;
        case V_RESULT_NOT_ENABLED: /* U_RESULT_NOT_INITIALISED */
            code = OS_RETCODE_ERROR;
            break;
        case V_RESULT_OUT_OF_MEMORY:
            code = OS_RETCODE_OUT_OF_RESOURCES;
            break;
        case V_RESULT_INTERNAL_ERROR:
            code = OS_RETCODE_ERROR;
            break;
        case V_RESULT_ILL_PARAM:
            code = OS_RETCODE_BAD_PARAMETER;
            break;
        case V_RESULT_CLASS_MISMATCH:
            code = OS_RETCODE_PRECONDITION_NOT_MET;
            break;
        case V_RESULT_DETACHING:
            code = OS_RETCODE_ALREADY_DELETED;
            break;
        case V_RESULT_TIMEOUT:
            code = OS_RETCODE_TIMEOUT;
            break;
        case V_RESULT_OUT_OF_RESOURCES:
            code = OS_RETCODE_OUT_OF_RESOURCES;
            break;
        case V_RESULT_INCONSISTENT_QOS:
            code = OS_RETCODE_INCONSISTENT_POLICY;
            break;
        case V_RESULT_IMMUTABLE_POLICY:
            code = OS_RETCODE_IMMUTABLE_POLICY;
            break;
        case V_RESULT_PRECONDITION_NOT_MET:
            code = OS_RETCODE_PRECONDITION_NOT_MET;
            break;
        case V_RESULT_ALREADY_DELETED:
            code = OS_RETCODE_ALREADY_DELETED;
            break;
        case V_RESULT_NO_DATA:
            code = OS_RETCODE_NO_DATA;
            break;
        case V_RESULT_UNSUPPORTED:
            code = OS_RETCODE_UNSUPPORTED;
            break;
        default:
            assert (result == V_RESULT_UNDEFINED);
            break;
    }

    return code;
}

os_int
v_writeResultToReturnCode(
    v_writeResult result)
{
    os_int code = OS_RETCODE_ERROR;

    assert ((result & OS_RETCODE_ID_MASK) == OS_RETCODE_ID_V_WRITE_RESULT);

    switch (result) {
        case V_WRITE_SUCCESS:
        case V_WRITE_SUCCESS_NOT_STORED:
        case V_WRITE_REGISTERED:
        case V_WRITE_UNREGISTERED:
            code = OS_RETCODE_OK;
            break;
        case V_WRITE_PRE_NOT_MET:
            code = OS_RETCODE_PRECONDITION_NOT_MET;
            break;
        case V_WRITE_ERROR:
            break;
        case V_WRITE_TIMEOUT:
            code = OS_RETCODE_TIMEOUT;
            break;
        case V_WRITE_OUT_OF_RESOURCES:
            code = OS_RETCODE_OUT_OF_RESOURCES;
            break;
        case V_WRITE_REJECTED:
        case V_WRITE_COUNT:
            break;
        default:
            assert (result == V_WRITE_UNDEFINED);
            break;
    }

    return code;
}

const os_char *
v_dataReaderResultImage(
    const v_dataReaderResult result)
{
    const os_char *image;

#define _CASE_(o) case o: image = #o; break;
    switch (result) {
    _CASE_(V_DATAREADER_INSERTED);
    _CASE_(V_DATAREADER_OUTDATED);
    _CASE_(V_DATAREADER_NOT_OWNER);
    _CASE_(V_DATAREADER_MAX_SAMPLES);
    _CASE_(V_DATAREADER_MAX_INSTANCES);
    _CASE_(V_DATAREADER_INSTANCE_FULL);
    _CASE_(V_DATAREADER_SAMPLE_LOST);
    _CASE_(V_DATAREADER_DUPLICATE_SAMPLE);
    _CASE_(V_DATAREADER_OUT_OF_MEMORY);
    _CASE_(V_DATAREADER_INTERNAL_ERROR);
    _CASE_(V_DATAREADER_UNDETERMINED);
    _CASE_(V_DATAREADER_FILTERED_OUT);
    _CASE_(V_DATAREADER_COUNT);
    default:
        image = "Internal error: no image for illegal result value";
    break;
    }
    return image;
#undef _CASE_
}

os_int
v_dataReaderResultToReturnCode(
    v_dataReaderResult result)
{
    os_int code = OS_RETCODE_ERROR;

    assert ((result & OS_RETCODE_ID_MASK) == OS_RETCODE_ID_V_DATAREADER_RESULT);

    switch (result) {
        case V_DATAREADER_INSERTED:
            code = OS_RETCODE_OK;
            break;
        case V_DATAREADER_OUTDATED:
            code = OS_RETCODE_PRECONDITION_NOT_MET;
            break;
        case V_DATAREADER_NOT_OWNER:
            code = OS_RETCODE_ILLEGAL_OPERATION;
            break;
        case V_DATAREADER_MAX_SAMPLES:
        case V_DATAREADER_MAX_INSTANCES:
        case V_DATAREADER_INSTANCE_FULL:
        case V_DATAREADER_DUPLICATE_SAMPLE:
            code = OS_RETCODE_PRECONDITION_NOT_MET;
            break;
        case V_DATAREADER_OUT_OF_MEMORY:
            code = OS_RETCODE_OUT_OF_RESOURCES;
            break;
        case V_DATAREADER_SAMPLE_LOST:
        case V_DATAREADER_INTERNAL_ERROR:
        case V_DATAREADER_FILTERED_OUT:
        case V_DATAREADER_COUNT:
            code = OS_RETCODE_ERROR;
            break;
        default:
            assert (result == V_DATAREADER_UNDETERMINED);
            break;
    }

    return code;
}

static void
v__kernelProtectTrace(
    v_processInfo _this,
    os_uint32 count,
    const char * oper)
{
#if UT_TRACE_PROTECTCOUNT
    UT_TRACE("[%d:%lu] %s %u (k:%p)\n",
             (int)_this->processId, os_threadIdToInteger(os_threadIdSelf()),
             oper, count, (void *)_this);
#else
    OS_UNUSED_ARG(_this);
    OS_UNUSED_ARG(count);
    OS_UNUSED_ARG(oper);
#endif
}

struct v__kernelThreadInfo {
    /* Counts the number of times this thread has invoked v_kernelProtect for
     * this domain. */
    os_uint32 myProtectCount;
    /* Maintains the state of this thread. */
    os_uint32 flags;
    /* The process specific record in SHM */
    v_processInfo pinfo;
    /* The thread specific record in SHM */
    v_threadInfo tinfo;
    /* Domain-ID + serial that pinfo (and thereby tinfo) corresponds to; needed
     * since we can't always dereference pinfo (in potentially detached SHM).
     * The pointer to pinfo may be reused as well. This is a caching mechanism
     * for the last accessed domain. Otherwise every protect needs to acquire
     * process-wide mutex to lookup the threadInfo-record. */
    c_ulong serial;
    /* Pointer to flag that, when non-zero, indicates that threads should block
     * on waking up from sleeps. */
    pa_uint32_t *block;
    os_uint32 blockmask;
    /* Process-private mutex used to deadlock a thread waiting in the kernel so
     * that the SHM can be unmapped without causing SIGSEGV's. */
    os_mutex *deadlock;
    /* Pointer to userdata returned on last v_kernelUnprotect(). */
    void *usrData;
};

static void
v__kernelThreadInfoFlags (
    _Inout_ struct v__kernelThreadInfo *threadMemInfo,
    _In_ os_uint32 mask,
    _In_ os_uint32 value)
{
    threadMemInfo->flags &= ~mask;
    threadMemInfo->flags |= (mask & value);

    v_threadInfoSetFlags(threadMemInfo->tinfo, threadMemInfo->flags);
}

_Ret_notnull_
static
struct v__kernelThreadInfo *
v__kernelThreadInfoGet(void)
{
    struct v__kernelThreadInfo *threadMemInfo;

    threadMemInfo = os_threadMemGet(OS_THREAD_PROCESS_INFO);
    if (!threadMemInfo) {
        /* This is the first time this thread enters the kernel, so create
         * thread specific memory to hold the domain specific processInfo record. */
        threadMemInfo = os_threadMemMalloc(OS_THREAD_PROCESS_INFO, sizeof *threadMemInfo, NULL, NULL);

        threadMemInfo->serial = 0;
        threadMemInfo->myProtectCount = 0;
        threadMemInfo->flags = 0;
        threadMemInfo->tinfo = NULL;
        v__kernelThreadInfoFlags(threadMemInfo, V_KERNEL_THREAD_FLAG_DOMAINID, (os_uint32)-1);
    }

    return threadMemInfo;
}

os_uint32
v_kernelThreadFlags (
    _In_ os_uint32 mask,
    _In_ os_uint32 value)
{
    os_uint32 oldFlags;
    struct v__kernelThreadInfo *threadMemInfo;

    threadMemInfo = v__kernelThreadInfoGet();

    oldFlags = threadMemInfo->flags;

    v__kernelThreadInfoFlags(threadMemInfo, mask, value);

    return oldFlags;
}

os_uint32
v_kernelThreadProtectCount (
    _In_ os_uint32 serial)
{
    struct v__kernelThreadInfo *threadMemInfo;

    threadMemInfo = v__kernelThreadInfoGet();

    if(threadMemInfo->serial == serial) {
        return threadMemInfo->myProtectCount;
    } else {
        assert(threadMemInfo->myProtectCount == 0);
        return 0;
    }
}

c_bool
v_kernelThreadInProtectedArea()
{
    c_bool result = FALSE;
    struct v__kernelThreadInfo *threadMemInfo;

    threadMemInfo = os_threadMemGet(OS_THREAD_PROCESS_INFO);
    if (threadMemInfo) {
        result = (threadMemInfo->myProtectCount > 0);
    }
    return result;
}

_Check_return_
v_result
v_kernelProtect(
    _Inout_ v_processInfo info,
    _In_ pa_uint32_t *block,
    _In_ os_uint32 blockmask,
    _Inout_ os_mutex *deadlock,
    _In_opt_ void *usrData)
{
    struct v__kernelThreadInfo *threadMemInfo;
    os_uint32 count;
    v_result result = V_RESULT_OK;

    assert(info);

    threadMemInfo = v__kernelThreadInfoGet();

    if(threadMemInfo->myProtectCount) {
        /* If the thread already has a protectCount set, then this MUST be for
         * the same domain.
         */
        assert(threadMemInfo->pinfo == info);
        assert(threadMemInfo->tinfo == v_processInfoGetThreadInfo(info, os_threadIdToInteger(os_threadIdSelf())));
        assert(threadMemInfo->serial == info->serial);
        assert(threadMemInfo->tinfo->serial == info->serial);
        assert(threadMemInfo->block == block);
        assert(threadMemInfo->blockmask == blockmask);
        assert(threadMemInfo->deadlock == deadlock);
        assert(threadMemInfo->usrData == usrData);
        assert((threadMemInfo->flags & V_KERNEL_THREAD_FLAG_DOMAINID) == (info->serial & V_KERNEL_THREAD_FLAG_DOMAINID));
    } else {
        /* Always cache info so it can be used in v_kernelUnprotect(),
         * v__kernelProtectWaitEnter() and v__kernelProtectWaitExit().
         */
        if(threadMemInfo->serial == 0 || threadMemInfo->serial != info->serial) {
            threadMemInfo->serial = info->serial;
            threadMemInfo->pinfo = info;
            threadMemInfo->tinfo = v_processInfoGetThreadInfo(info, os_threadIdToInteger(os_threadIdSelf()));
        }
        assert(threadMemInfo->tinfo);
        assert(threadMemInfo->tinfo->serial == threadMemInfo->pinfo->serial);
        threadMemInfo->block = block;
        threadMemInfo->blockmask = blockmask;
        threadMemInfo->deadlock = deadlock;
        threadMemInfo->usrData = usrData;
        assert((threadMemInfo->flags & V_KERNEL_THREAD_FLAG_DOMAINID) == 0xFF);
        v__kernelThreadInfoFlags(threadMemInfo, V_KERNEL_THREAD_FLAG_DOMAINID, info->serial);
    }

    threadMemInfo->myProtectCount++;
    threadMemInfo->tinfo->protectCount++;
    assert(threadMemInfo->myProtectCount == threadMemInfo->tinfo->protectCount);
    count = pa_inc32_nv(&threadMemInfo->pinfo->protectCount);
    v__kernelProtectTrace(threadMemInfo->pinfo, count, "Protect");

    return result;
}

_When_(mtx, _Requires_lock_held_(mtx->mtx))
_Pre_satisfies_((mtx && cnd) || (!mtx && !cnd))
void
v__kernelProtectWaitEnter(
    _In_opt_ c_cond *cnd,
    _Inout_opt_ c_mutex *mtx)
{
    struct v__kernelThreadInfo *threadMemInfo;
    os_uint32 count;

    assert(os_threadMemGet(OS_THREAD_PROCESS_INFO));

    threadMemInfo = v__kernelThreadInfoGet();

    v_threadInfoSetWaitInfo(threadMemInfo->tinfo, cnd, mtx);
    count = pa_add32_nv(&threadMemInfo->pinfo->waitCount, threadMemInfo->myProtectCount);
    v__kernelThreadInfoFlags(threadMemInfo, V_KERNEL_THREAD_FLAG_WAITING, V_KERNEL_THREAD_FLAG_WAITING);

    v__kernelProtectTrace(threadMemInfo->pinfo, count, "WaitEnter");
}

void
v__kernelProtectWaitExit(void)
{
    struct v__kernelThreadInfo *threadMemInfo;
    os_uint32 count;
    c_mutex *mtx;

    assert(os_threadMemGet(OS_THREAD_PROCESS_INFO));

    threadMemInfo = v__kernelThreadInfoGet();

    if(pa_ld32(threadMemInfo->block) & threadMemInfo->blockmask) {
        mtx = v_threadInfoGetAndClearWaitInfo(threadMemInfo->tinfo);
        if(mtx) {
            c_mutexUnlock(mtx);
        }
        v__kernelProtectTrace(threadMemInfo->pinfo, 0xFFFFFFFF, "BlockWait");
        (void) pa_add32_nv(&threadMemInfo->pinfo->blockedCount, threadMemInfo->myProtectCount);
        (void) pa_sub32_nv(&threadMemInfo->pinfo->waitCount, threadMemInfo->myProtectCount);

        os_mutexLock(threadMemInfo->deadlock); /* Will deadlock */
        /* This line is not reached; lock has to be deadlocked. */
        assert(FALSE);
        os_mutexUnlock(threadMemInfo->deadlock); /* For static analyzers */
    }

    count = pa_sub32_nv(&threadMemInfo->pinfo->waitCount, threadMemInfo->myProtectCount);
    v__kernelThreadInfoFlags(threadMemInfo, V_KERNEL_THREAD_FLAG_WAITING, 0);
    v_threadInfoSetWaitInfo(threadMemInfo->tinfo, NULL, NULL);
    v__kernelProtectTrace(threadMemInfo->pinfo, count + 1, "WaitExit");
}


/*
 * !!! Use this function very carefully. See documentation
 */
void
v_kernelProtectStrictReadOnlyEnter(void)
{
    struct v__kernelThreadInfo *threadMemInfo;
    os_uint32 count;

    assert(os_threadMemGet(OS_THREAD_PROCESS_INFO));

    threadMemInfo = v__kernelThreadInfoGet();

    count = pa_add32_nv(&threadMemInfo->pinfo->waitCount, threadMemInfo->myProtectCount);
    v__kernelThreadInfoFlags(threadMemInfo, V_KERNEL_THREAD_FLAG_WAITING, V_KERNEL_THREAD_FLAG_WAITING);

    v__kernelProtectTrace(threadMemInfo->pinfo, count, "CopyOutEnter");
}

/*
 * !!! Use this function very carefully. See documentation
 */
void
v_kernelProtectStrictReadOnlyExit(void)
{
    struct v__kernelThreadInfo *threadMemInfo;
    os_uint32 count;

    assert(os_threadMemGet(OS_THREAD_PROCESS_INFO));

    threadMemInfo = v__kernelThreadInfoGet();

    count = pa_sub32_nv(&threadMemInfo->pinfo->waitCount, threadMemInfo->myProtectCount);
    v__kernelThreadInfoFlags(threadMemInfo, V_KERNEL_THREAD_FLAG_WAITING, 0);

    v__kernelProtectTrace(threadMemInfo->pinfo, count + 1, "CopyOutEnter");
}

/*
 * This function unprotects the kernel
 *
 * return: NULL in case the protect count is lowered but has not yet reached 0
 *         other protect count has reached 0, v_kernelUnprotectFinalize should now
 *               be called to finalize the unprotect.
 */
_Ret_maybenull_
void *
v_kernelUnprotect(void)
{
    struct v__kernelThreadInfo *threadMemInfo;
    os_uint32 count;

    assert(os_threadMemGet(OS_THREAD_PROCESS_INFO));

    threadMemInfo = v__kernelThreadInfoGet();

    threadMemInfo->myProtectCount--;
    threadMemInfo->tinfo->protectCount--;
    assert(threadMemInfo->myProtectCount == threadMemInfo->tinfo->protectCount);

    if(threadMemInfo->myProtectCount == 0) {
        v__kernelThreadInfoFlags(threadMemInfo, V_KERNEL_THREAD_FLAG_DOMAINID, (os_uint32)-1);
        return threadMemInfo->usrData;
    } else {
        count = pa_dec32_nv(&threadMemInfo->pinfo->protectCount);
        v__kernelProtectTrace(threadMemInfo->pinfo, count + 1, "Unprotect");
        return NULL;
    }
}

/*
 * This function finalizes the kernel unprotect, it should only be called after the
 * v_kernelUnprotect function
 */
void
v_kernelUnprotectFinalize(
    void * usrData)
{
    struct v__kernelThreadInfo *threadMemInfo;
    os_uint32 count;

    if (usrData) {
        assert(os_threadMemGet(OS_THREAD_PROCESS_INFO));

        threadMemInfo = v__kernelThreadInfoGet();
        assert(threadMemInfo->usrData == usrData);
        assert(threadMemInfo->myProtectCount == threadMemInfo->tinfo->protectCount);

        count = pa_dec32_nv(&threadMemInfo->pinfo->protectCount);
        v__kernelProtectTrace(threadMemInfo->pinfo, count + 1, "Unprotect final");
    }
}

os_int32
v_kernelThreadInfoGetDomainId(void)
{
    struct v__kernelThreadInfo *threadMemInfo;

    threadMemInfo = os_threadMemGet(OS_THREAD_PROCESS_INFO);
    if (threadMemInfo) {
        return (threadMemInfo->serial & V_KERNEL_THREAD_FLAG_DOMAINID);
    }
    return -1;
}

v_spliced
v_kernelGetSpliced(
    v_kernel _this)
{
    v_spliced spliced;
    c_lockRead(&_this->lock);
    spliced = c_keep(_this->spliced);
    c_lockUnlock(&_this->lock);
    return spliced;
}

v_result
v_kernelDisposeAllData(
    v_kernel kernel,
    c_string partitionExpr,
    c_string topicExpr,
    os_timeW timestamp)
{
    v_result result = V_RESULT_OK;
    c_iter list;
    c_iter topicList = NULL;
    v_group group;
    v_topic topic;
    v_writeResult res;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    list = v_groupSetLookup(kernel->groupSet, partitionExpr, topicExpr);

    if (list)
    {
        if (c_iterLength(list) > 1) {
            topicList = c_iterNew(NULL);
        }

        group = c_iterTakeFirst(list);
        if ( group != NULL )
        {
           while (group)
           {
              res = v_groupDisposeAll( group, timestamp, 0 );
              if ( res != V_WRITE_SUCCESS )
              {
                 OS_REPORT(OS_WARNING, "kernel", V_RESULT_INTERNAL_ERROR,
                           "Dispose All Data failed due to internal error.");
                 result = V_RESULT_INTERNAL_ERROR;
              }

              topic = v_groupTopic( group );

              if (topicList) {
                  if (!c_iterContains(topicList, topic)){
                      topicList = c_iterAppend(topicList, c_keep(topic));
                  }
              } else {
                  v_topicNotifyAllDataDisposed( topic );
              }

              c_free(group);
              group = c_iterTakeFirst(list);
           }
           if (topicList) {
               topic = c_iterTakeFirst(topicList);
               while (topic) {
                   v_topicNotifyAllDataDisposed( topic );
                   c_free(topic);
                   topic = c_iterTakeFirst(topicList);
               }
               c_iterFree(topicList);
           }
        }
        else
        {
           /* Group does not exist yet, store the timestamp etc for when the group is created */
           v_pendingDisposeElement element = NULL;
           c_base base = c_getBase(c_object(kernel));
           int found = 0;
           c_ulong i;

           c_mutexLock(&kernel->pendingDisposeListMutex);
           for(i=0; i<c_listCount(kernel->pendingDisposeList); i++)
           {
              element = (v_pendingDisposeElement)c_readAt(kernel->pendingDisposeList, i);
              if ( !strcmp( element->disposeCmd.topicExpr, topicExpr)
                   && !strcmp( element->disposeCmd.partitionExpr, partitionExpr ))
              {
                 found = 1;

                 if ( os_timeWCompare( element->disposeTimestamp, timestamp ) == OS_LESS )
                 {
                    /* Already an older existing record for this partition
                     * and topic combination - update timestamp
                     */
                    element->disposeTimestamp = timestamp;
                 }
                 break;
              }
           }
           if ( !found )
           {
               v_pendingDisposeElement new;

               new = c_new( v_kernelType(kernel, K_PENDINGDISPOSEELEMENT ) );
               new->disposeCmd.topicExpr = c_stringNew(base, topicExpr);
               new->disposeCmd.partitionExpr = c_stringNew(base, partitionExpr);
               new->disposeTimestamp = timestamp;
               c_append( kernel->pendingDisposeList, new );
           }
           c_mutexUnlock(&kernel->pendingDisposeListMutex);
        }
    }
    c_iterFree(list);

    return result;
}

v_rxoData
v_kernel_rxoDataFromMessageQos(
    v_kernel kernel,
    const v_messageQos qos)
{
    v_rxoData rxo = NULL;

    rxo = v_rxoData(v_objectNew(kernel, K_RXODATA));

    rxo->durability.v.kind = v_messageQos_durabilityKind(qos);
    rxo->deadline.v.period = v_messageQos_getDeadlinePeriod(qos);
    rxo->latency.v.duration = v_messageQos_getLatencyPeriod(qos);
    rxo->liveliness.v.kind = v_messageQos_livelinessKind(qos);
    rxo->liveliness.v.lease_duration = v_messageQos_getLivelinessPeriod(qos);
    rxo->reliability.v.kind = v_messageQos_reliabilityKind(qos);
    rxo->orderby.v.kind = v_messageQos_orderbyKind(qos);
    rxo->ownership.v.kind = v_messageQos_ownershipKind(qos);

    return rxo;
}

v_rxoData
v_kernel_rxoDataFromPublicationInfo(
    v_kernel kernel,
    const struct v_publicationInfo *info)
{
    v_rxoData rxo = NULL;

    rxo = v_rxoData(v_objectNew(kernel, K_RXODATA));

    rxo->durability.v = info->durability;
    v_policyConvToInt_deadline(&rxo->deadline, &info->deadline);
    v_policyConvToInt_latency_budget(&rxo->latency, &info->latency_budget);
    v_policyConvToInt_liveliness(&rxo->liveliness, &info->liveliness);
    v_policyConvToInt_reliability(&rxo->reliability, &info->reliability);
    rxo->orderby.v = info->destination_order;
    rxo->ownership.v = info->ownership;

    return rxo;
}

v_rxoData
v_kernel_rxoDataFromReaderQos(
    v_kernel kernel,
    const v_readerQos qos)
{
    v_rxoData rxo = NULL;

    rxo = v_rxoData(v_objectNew(kernel, K_RXODATA));

    rxo->durability.v.kind  = qos->durability.v.kind;
    rxo->deadline.v.period  = qos->deadline.v.period;
    rxo->latency.v.duration = qos->latency.v.duration;
    rxo->liveliness.v.kind  = qos->liveliness.v.kind;
    rxo->liveliness.v.lease_duration = qos->liveliness.v.lease_duration;
    rxo->reliability.v.kind = qos->reliability.v.kind;
    rxo->orderby.v.kind     = qos->orderby.v.kind;
    rxo->ownership.v.kind   = qos->ownership.v.kind;

    return rxo;
}

#define latencyCompatible(offered,requested)\
    ((OS_DURATION_ISINFINITE((requested)->latency.v.duration)) ||\
     (os_durationCompare((offered)->latency.v.duration,(requested)->latency.v.duration) != OS_MORE))

#define deadlineCompatible(offered,requested)\
    ((OS_DURATION_ISINFINITE((requested)->deadline.v.period)) ||\
     (os_durationCompare((offered)->deadline.v.period,(requested)->deadline.v.period) != OS_MORE))

#define livelinessCompatible(offered,requested)\
    (((offered)->liveliness.v.kind >= (requested)->liveliness.v.kind) && \
     ((OS_DURATION_ISINFINITE((requested)->liveliness.v.lease_duration)) ||\
      (os_durationCompare((offered)->liveliness.v.lease_duration,(requested)->liveliness.v.lease_duration) != OS_MORE)))

c_bool
v_rxoDataCompatible(
    v_rxoData offered,
    v_rxoData requested)
{
    return ((offered->reliability.v.kind >= requested->reliability.v.kind) &&
            (offered->durability.v.kind >= requested->durability.v.kind) &&
            latencyCompatible(offered,requested) &&
            (offered->orderby.v.kind >= requested->orderby.v.kind) &&
            deadlineCompatible(offered,requested) &&
            livelinessCompatible(offered,requested) &&
            (offered->ownership.v.kind == requested->ownership.v.kind));

}

c_bool
v_kernelGetDurabilitySupport(
    _In_ _Const_ v_kernel kernel)
{
    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    return kernel->durabilitySupport;
}

c_bool
v_kernelHasDurabilityService(
    _In_ _Const_ v_kernel kernel)
{
    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    return kernel->hasDurabilityService;
}

c_bool
v_kernelGetAlignedState(
    v_kernel _this)
{
    c_bool result;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_kernel));

    c_lockRead(&_this->lock);
    result = _this->durabilityAligned;
    c_lockUnlock(&_this->lock);

    return result;
}

void
v_kernelSetAlignedState(
    v_kernel _this,
    c_bool aligned)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_kernel));

    c_lockWrite(&_this->lock);
    _this->durabilityAligned = aligned;
    c_lockUnlock(&_this->lock);
}

void
v_kernelTransactionsPurge(
    v_kernel _this)
{
    c_iter list;
    v_participant participant;

    /* A transaction flush purges and flushes the obsolete groups,
     * flushing is required in case the obsolete group contains samples
     * which cannot be discarded.
     */
    v_kernelGroupTransactionFlush(_this, NULL);

    list = c_iterNew(NULL);
    c_lockRead(&_this->lock);
    (void)c_walk(_this->participants, collectAllParticipants, list);
    c_lockUnlock(&_this->lock);
    while ((participant = c_iterTakeFirst(list)) != NULL) {
        v_participantTransactionsPurge(participant);
        c_free(participant);
    }
    c_iterFree(list);
}

void
v_kernelConnectGroup(
    v_kernel k,
    v_group g)
{
    c_iter list;
    v_participant p;
    assert(k);
    assert(g);
    assert(C_TYPECHECK(k,v_kernel));
    c_lockRead(&k->lock);
    list = ospl_c_select(k->participants, 0);
    c_lockUnlock(&k->lock);
    while ((p = c_iterTakeFirst(list)) != NULL) {
        v_participantConnectGroup(p, g);
        c_free(p);
    }
    c_iterFree(list);
}

/* The operation WalkPublications will visit all discovered publication messages and
 * invoke the given publication action function on each publication message.
 * The given argument arg is passed as context information to the invoked action function.
 * The walk will stop when all messages are processed or when the action function returns FALSE.
 */
v_result
v_kernelWalkPublications(
    v_kernel _this,
    v_publication_action action, /* signature: os_boolean (*action)(const v_message publication, void *arg) */
    c_voidp arg)
{
    /* The implementation of the system info storage is currently owned by
     * the spliced but should move to the kernel.
     */
    v_spliced spliced;
    v_result result;
    spliced = v_kernelGetSpliced(_this);
    result = v_splicedWalkPublications(spliced, action, arg);
    c_free(spliced);
    return result;
}

/* The operation WalkSubscriptions will visit all discovered subscription messages and
 * invoke the given subscription action function on each subscription message.
 * The given argument arg is passed as context information to the invoked action function.
 * The walk will stop when all messages are processed or when the action function returns FALSE.
 */
v_result
v_kernelWalkSubscriptions(
    v_kernel _this,
    v_subscription_action action, /* signature: os_boolean (*action)(const v_message subscription, void *arg) */
    c_voidp arg)
{
    /* The implementation of the system info storage is currently owned by
     * the spliced but should move to the kernel.
     */
    v_spliced spliced;
    v_result result;
    spliced = v_kernelGetSpliced(_this);
    result = v_splicedWalkSubscriptions(spliced, action, arg);
    c_free(spliced);
    return result;
}

struct lookupPublicationArg {
    v_gid wgid;
    v_message msg;
};

static os_boolean
lookupPublication(
    const v_message msg,
    c_voidp arg)
{
    struct lookupPublicationArg *a = (struct lookupPublicationArg *)arg;
    struct v_publicationInfo *info;

    info = v_builtinPublicationInfoData(msg);
    if (v_gidEqual(info->key, a->wgid)) {
        a->msg = c_keep(msg);
        return OS_FALSE;
    }
    return OS_TRUE;
}

/* This operation will lookup the publication message identified by the given gid.
 * If found the message will be returned and must be freed after use.
 */
v_message
v_kernelLookupPublication(
    v_kernel _this,
    v_gid gid)
{
    struct lookupPublicationArg lpa;

    lpa.wgid = gid;
    lpa.msg = NULL;
    (void)v_kernelWalkPublications(_this, lookupPublication, &lpa);
    return lpa.msg;
}

struct kernelReadActionArg {
    v_domainReadAction action;
    const void *actionArg;
    v_result result;
};

static os_boolean
kernelReadAction(
    v_groupSample sample,
    const void *arg)
{
    struct kernelReadActionArg *a = (struct kernelReadActionArg *)arg;
    v_groupInstance instance = sample->instance;

    a->result = a->action(instance->group, instance, v_groupSampleTemplate(sample)->message, a->actionArg);
    return (a->result == V_RESULT_OK);
}

v_result
v_kernelRead(
    const v_kernel _this,
    const os_char *partition,
    const os_char *topic,
    const os_char *query,
    const v_domainReadAction action,
    const void *actionArg)
{
    v_result result = V_RESULT_OK;
    v_groupStoreQuery store_query = NULL;
    v_group group;
    c_iter list;
    c_value params[2];
    struct kernelReadActionArg arg;

    arg.action = action;
    arg.actionArg = actionArg;
    arg.result = V_RESULT_OK;

    params[0]  = c_stringValue((c_string)partition);
    params[1]  = c_stringValue((c_string)topic);

    list = v_groupSetSelect(_this->groupSet, "partition.name like %0 AND topic.name like %1", params);
    while ((result == V_RESULT_OK) && (group = c_iterTakeFirst(list)) != NULL) {
        store_query = v_groupStoreQueryNew(group->store, query, NULL, 0);
        result = v_groupStoreRead(group->store, store_query, kernelReadAction, &arg);
        if (store_query) {
            v_groupStoreQueryFree(store_query);
        }
    }
    c_iterFree(list);
    return result;
}

v_result
v_kernelGroupRead(
    const v_kernel _this,
    const os_char *partition,
    const os_char *topic,
    const v_domainGroupReadAction action,
    const void *actionArg)
{
    v_result result = V_RESULT_OK;
    v_group group;
    c_iter list;
    c_value params[2];

    params[0]  = c_stringValue((c_string)partition);
    params[1]  = c_stringValue((c_string)topic);

    list = v_groupSetSelect(_this->groupSet, "partition.name like %0 AND topic.name like %1", params);
    while ((group = c_iterTakeFirst(list)) != NULL) {
        result = action(group, actionArg);
        c_free(group);
    }
    result = action(NULL, actionArg);
    c_iterFree(list);
    return result;
}

v_result
v_kernelSetIsolate(
    const v_kernel _this,
    const os_uint32 isolate)
{
    v_result result = V_RESULT_OK;
    pa_st32(&_this->isolate, (os_uint32)isolate);
    return result;
}

v_result
v_kernelGetIsolate(
    const v_kernel _this,
    os_uint32 *isolate)
{
    v_result result = V_RESULT_OK;
    *isolate = pa_ld32(&_this->isolate);
    return result;
}
