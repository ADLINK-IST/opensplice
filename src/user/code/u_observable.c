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
#include "u__user.h"
#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__dispatcher.h"
#include "u__participant.h"
#include "u_networkReader.h"
#include "u__topic.h"
#include "u_group.h"
#include "u_groupQueue.h"
#include "u_dataView.h"
#include "u_serviceManager.h"
#include "u_waitsetEntry.h"

#include "v_entity.h"
#include "v_public.h"
#include "v_participant.h"
#include "v_publisher.h"
#include "v_subscriber.h"
#include "v_writer.h"
#include "v_partition.h"
#include "v_group.h"
#include "v_topic.h"
#include "v_topicAdapter.h"
#include "v_status.h"
#include "v_service.h"
#include "v_networking.h"
#include "v_durability.h"
#include "v_nwbridge.h"
#include "v_cmsoap.h"
#include "v_rnr.h"
#include "v_dataReader.h"
#include "v_dataReaderQuery.h"
#include "v_dataViewQuery.h"
#include "v_spliced.h"
#include "v_waitset.h"
#include "v_statusCondition.h"
#include "v_networkReader.h"
#include "v_groupQueue.h"
#include "v_dataView.h"
#include "v_observer.h"

#include "os_report.h"
#include "os_atomics.h"

#define U__CLAIM_NONE (0)
#define U__CLAIM_SPLICED_RUNNING (1<<1)

u_observable
u_observableCreateProxy (
    const v_public vObject,
    const u_participant p)
{
    u_result result;
    v_participant kp;
    u_observable uObservable = NULL;

    assert(vObject != NULL);
    assert(p != NULL);

    if (v_objectKind(vObject) == K_TOPIC) {
        result = u_observableWriteClaim(u_observable(p), (v_public *)(&kp), C_MM_RESERVATION_LOW);
        if (result == U_RESULT_OK) {
            v_topicAdapter adapter = v_topicAdapterWrap(kp, v_topic(vObject));
            u_topic uTopic = u_objectAlloc(sizeof(*uTopic), U_TOPIC, u__topicDeinitW, u__topicFreeW);
            result = u_topicInit(uTopic, v_topicName(vObject), adapter, p);
            u_observableRelease(u_observable(p),C_MM_RESERVATION_LOW);
            c_free(adapter);
            if (result == U_RESULT_OK) {
                uObservable = u_observable(uTopic);
            } else {
                u_objectFree(uTopic);
            }
        } else {
            assert(FALSE);
        }
    } else {
        u_domain domain = u_observableDomain(u_observable(p));
        u_kind kind;
        size_t sz;
        switch(v_objectKind(vObject)) {
            case K_DOMAIN:          kind = U_PARTITION;       sz = sizeof(struct u_partition_s); break;
            case K_TOPIC_ADAPTER:   kind = U_TOPIC;           sz = sizeof(struct u_topic_s); break;
            case K_GROUP:           kind = U_GROUP;           sz = sizeof(struct u_group_s); break;
            case K_PUBLISHER:       kind = U_PUBLISHER;       sz = sizeof(struct u_publisher_s); break;
            case K_SUBSCRIBER:      kind = U_SUBSCRIBER;      sz = sizeof(struct u_subscriber_s); break;
            case K_WRITER:          kind = U_WRITER;          sz = sizeof(struct u_writer_s); break;
            case K_DATAREADER:      kind = U_READER;          sz = sizeof(struct u_reader_s); break;
            case K_DELIVERYSERVICE: kind = U_READER;          sz = sizeof(struct u_reader_s); break;
            case K_DATAVIEWQUERY:   kind = U_QUERY;           sz = sizeof(struct u_query_s); break;
            case K_DATAREADERQUERY: kind = U_QUERY;           sz = sizeof(struct u_query_s); break;
            case K_QUERY:           kind = U_QUERY;           sz = sizeof(struct u_query_s); break;
            case K_DATAVIEW:        kind = U_DATAVIEW;        sz = sizeof(struct u_dataView_s); break;
            case K_PARTICIPANT:     kind = U_PARTICIPANT;     sz = sizeof(struct u_participant_s); break;
            case K_SERVICEMANAGER:  kind = U_SERVICEMANAGER;  sz = sizeof(struct u_serviceManager_s); break;
            case K_NETWORKING:      kind = U_SERVICE;         sz = sizeof(struct u_service_s); break;
            case K_DURABILITY:      kind = U_SERVICE;         sz = sizeof(struct u_service_s); break;
            case K_NWBRIDGE:        kind = U_SERVICE;         sz = sizeof(struct u_service_s); break;
            case K_CMSOAP:          kind = U_SERVICE;         sz = sizeof(struct u_service_s); break;
            case K_RNR:             kind = U_SERVICE;         sz = sizeof(struct u_service_s); break;
            case K_SERVICE:         kind = U_SERVICE;         sz = sizeof(struct u_service_s); break;
            case K_SPLICED:         kind = U_SPLICED;         sz = sizeof(struct u_spliced_s); break;
            case K_WAITSET:         kind = U_WAITSETENTRY;    sz = sizeof(struct u_waitsetEntry_s); break;
            case K_LISTENER:        kind = U_LISTENER;        sz = sizeof(struct u_listener_s); break;
            case K_NETWORKREADER:   kind = U_NETWORKREADER;   sz = sizeof(struct u_networkReader_s); break;
            case K_GROUPQUEUE:      kind = U_GROUPQUEUE;      sz = sizeof(struct u_groupQueue_s); break;
            case K_KERNEL:          kind = U_DOMAIN;          sz = sizeof(struct u_domain_s); break;
            case K_STATUSCONDITION: kind = U_STATUSCONDITION; sz = sizeof(struct u_statusCondition_s); break;
            case K_TOPIC:           assert(0);
            default:                kind = U_UNDEFINED;       sz = sizeof(struct u_observable_s); break;
        }
        uObservable = u_objectAlloc(sz, kind, u__observableProxyDeinitW, u__observableProxyFreeW);
        if (uObservable != NULL) {
            result = u_observableInit(uObservable, vObject, domain);
            if (result != U_RESULT_OK) {
                u_objectFree(uObservable);
                uObservable = NULL;
            }
        }
    }
    return uObservable;
}

u_result
u_observableInit(
    const u_observable _this,
    const v_public vObject,
    const u_domain domain)
{
    u_result result = U_RESULT_OK;

    assert(_this != NULL);
    assert(vObject != NULL);


    /* a domain is an observable, but it has a weak reference to itself */
    if (_this->_parent.kind == U_DOMAIN) {
        _this->domain = domain;
    } else {
        _this->domain = u_domainKeep(domain);
    }
    _this->dispatcher = NULL;
    _this->userData = NULL;
    _this->magic = v_objectKernel(vObject);
    _this->gid = v_publicGid(vObject);
    _this->handle = u_handleNew(vObject);

    if (u_handleIsNil(_this->handle)) {
        OS_REPORT(OS_ERROR, "user::u_observableInit", U_RESULT_OUT_OF_RESOURCES,
                  "Out of memory: unable to create handle for Entity 0x%"PA_PRIxADDR".", (os_address)_this);
        result = U_RESULT_OUT_OF_MEMORY;
    }
    return result;
}

static void
publicFree(
    const v_public o)
{
    assert(o != NULL);

#define _FREE_(c) c##Free(c(o)); break

    switch(v_objectKind(o)) {
    case K_PARTICIPANT:      _FREE_(v_participant);
    case K_PUBLISHER:        _FREE_(v_publisher);
    case K_SUBSCRIBER:       _FREE_(v_subscriber);
    case K_WRITER:           _FREE_(v_writer);
    case K_DATAREADER:       _FREE_(v_dataReader);
    case K_NETWORKREADER:    _FREE_(v_networkReader);
    case K_GROUPQUEUE:       _FREE_(v_groupQueue);
    case K_DATAVIEW:         _FREE_(v_dataView);
    case K_TOPIC_ADAPTER:    _FREE_(v_topicAdapter);
    case K_TOPIC: break;     /* _FREE_(v_topic); due to refcount issue */
    case K_DOMAIN:           _FREE_(v_partition);
    case K_GROUP: break;     /* _FREE_(v_group); due to refount issue */
    case K_SPLICED:          _FREE_(v_spliced);
    case K_SERVICE:          _FREE_(v_service);
    case K_NETWORKING:       _FREE_(v_networking);
    case K_DURABILITY:       _FREE_(v_durability);
    case K_NWBRIDGE:         _FREE_(v_nwbridge);
    case K_CMSOAP:           _FREE_(v_cmsoap);
    case K_RNR:              _FREE_(v_rnr);
    case K_WRITERSTATUS:     _FREE_(v_status);
    case K_DATAVIEWQUERY:    _FREE_(v_dataViewQuery);
    case K_DATAREADERQUERY:  _FREE_(v_dataReaderQuery);
    case K_WAITSET:          _FREE_(v_waitset);
    case K_STATUSCONDITION:  _FREE_(v_statusCondition);
    case K_SERVICEMANAGER:   /* Is never freed! and depricated. */
    case K_LISTENER:
    case K_KERNEL:
    break;
    default:
        OS_REPORT(OS_ERROR, "user::publicFree", U_RESULT_ILL_PARAM,
                  "invalid entity kind (%d) specified", v_objectKind(o));
        assert(0);
    break;
    }
#undef _FREE_
}

u_result
u__observableProxyDeinitW(
    void *_vthis)
{
    u_observable _this = _vthis;
    v_public o;
    u_result r;
    if (_this->dispatcher != NULL) {
        if ((r = u_observableReadClaim(_this, &o, C_MM_RESERVATION_NO_CHECK)) == U_RESULT_OK) {
            u_dispatcherFree(_this->dispatcher);
            _this->dispatcher = NULL;
            u_observableRelease(_this, C_MM_RESERVATION_NO_CHECK);
        }
    }
    _this->gid = v_publicGid(NULL);
    u__objectDeinitW(_this);
    return U_RESULT_OK;
}

u_result
u__observableDeinitW(
    void *_vthis)
{
    u_observable _this = _vthis;
    v_public o;
    u_result r;
    r = u_observableReadClaim(_this, &o, C_MM_RESERVATION_NO_CHECK);
    if (r == U_RESULT_OK) {
        if (_this->dispatcher != NULL) {
            u_dispatcherFree(_this->dispatcher);
            _this->dispatcher = NULL;
        }
        publicFree(o);
        u_observableRelease(_this, C_MM_RESERVATION_NO_CHECK);
    }/* else
      * This happens when an entity is freed and its u_participant
      * already has been freed. It means this case is a valid situation.
      */
    _this->gid = v_publicGid(NULL);
    u__objectDeinitW(_this);
    return U_RESULT_OK;
}

void
u__observableProxyFreeW(
    void *_vthis)
{
    u_observable _this = _vthis;
    assert(_this->domain);

    /* a domain is an observable, but it has a weak reference to itself */
    if (_this->_parent.kind != U_DOMAIN) {
        u_domainFree(_this->domain);
    }
    _this->domain = NULL;
    _this->magic = NULL;
    u__objectFreeW(_this);
}

void
u__observableFreeW(
    void *_vthis)
{
    u_observable _this = _vthis;
    assert(_this->domain);

    /* a domain is an observable, but it has a weak reference to itself */
    if (_this->_parent.kind != U_DOMAIN) {
        u_domainFree(_this->domain);
    }
    _this->domain = NULL;
    _this->magic = NULL;
    u__objectFreeW(_this);
}

u_domain
u_observableDomain (
    const u_observable _this)
{
    return _this->domain;
}

static u_result
u_observableClaimCommon(
    const u_observable _this,
    v_public *vObject,
    const os_uint32 checks,
    os_address memoryClaim)
{
#if 0
    static os_boolean serviceWarningGiven = OS_FALSE;
#endif
    static os_boolean appWarningGiven = OS_FALSE;
    u_result r;
    u_kind kind;
    os_boolean isService = OS_FALSE;

    /* Precondition: entity must be locked. */
    assert(_this != NULL);
    assert(vObject != NULL);

    *vObject = NULL;
    kind = u_objectKind(u_object(_this));

    r = u_domainProtect(_this->domain);
    if (r == U_RESULT_OK) {
        if(kind == U_DOMAIN) {
            *vObject = v_public(u_domain(_this)->kernel);
            if(*vObject == NULL) {
                r = U_RESULT_INTERNAL_ERROR;
                OS_REPORT(OS_ERROR, "user::u_observableClaimCommon", r,
                          "Unable to obtain kernel entity for domain 0x%"PA_PRIxADDR"", (os_address)_this);
                u_domainUnprotect();
            }
        } else {
            r = u_handleClaim(_this->handle,vObject);
            if (r != U_RESULT_OK) {
                OS_REPORT(OS_WARNING, "user::u_observableClaimCommon", r,
                          "Invalid handle detected: result = %s, "
                          "Entity = 0x%"PA_PRIxADDR" (kind = %s)", u_resultImage(r),
                          (os_address)_this, u_kindImage(u_objectKind(u_object(_this))));
                u_domainUnprotect();
            }
        }
    } else if (r == U_RESULT_ALREADY_DELETED) {
        os_reportType verbosity = OS_WARNING;
        /*
         * This return value is quite valid when considering the next situation:
         *    - The last participant of a domain is deinitted
         *    - Causing the domain to be freed
         *    - Causing a call to u_objectFree with that domain
         *    - Causing a claim
         *    - Causing the stack to reach this, because the domain has
         *      already indicated, within itself, that is detaching.
         * This can happen f.i. with services like durability.
         *
         * So, because this case is valid for a domain, change the report from an
         * warning into an info to reduce the warning clutter in the log files.
         */
        if (kind == U_DOMAIN) {
            verbosity = OS_INFO;
        }
        /* OSPL-6199 : For the Java ListenerDispatcher it is perfectly
         * valid that the claim of the listener can return ALREADY_DELETED,
         * returning a WARNING is in that case incorrect.
         * For a listener the report is suppressed when the domain has been
         * deleted.
         * The following statements sets the report to OS_INFO to avoid warnings
         * appearing in the error log causing test cases to fail.
         * Correctness of this solution in general still needs to be verified.
         */
        if (kind != U_LISTENER || u_domainProtectAllowed(_this->domain)) {
            OS_REPORT(u_domainProtectAllowed(_this->domain) ? verbosity : OS_INFO,
                    "user::u_observable::u_observableClaimCommon", r,
                    "Claim Entity failed because the process "
                    "is detaching from the domain \"%s\" (%u). Entity = 0x%"PA_PRIxADDR" (kind = %s).",
                    u_domainName(_this->domain), u_domainId(_this->domain), (os_address)_this, u_kindImage(u_objectKind(u_object(_this))));
        }
    } else {
        OS_REPORT(OS_ERROR, "u_observableClaimCommon", r,
                  "u_domainProtect() failed for domain \"%s\" (%u): result = %s, "
                  "Domain = 0x%"PA_PRIxADDR", Entity = 0x%"PA_PRIxADDR" (kind = %s)",
                  u_domainName(_this->domain), u_domainId(_this->domain),
                  u_resultImage(r), (os_address)_this->domain, (os_address)_this,
                  u_kindImage(u_objectKind(u_object(_this))));
    }

    if (r == U_RESULT_OK) {
        /* Except for services a participant must verify if the spliced is still running.
         * This claim operation shall fail if it is not running anymore.
         */
        isService = u_domainIsService(_this->domain);
        if((!isService) && (kind != U_PARTICIPANT) && (checks & U__CLAIM_SPLICED_RUNNING)) {
            /* Ignore running check during cleanup in order to get trigger
             * events, etc through when threads are blocked on conditions in
             * shared memory.
             */
            v_kernel kernel = v_objectKernel(v_object(*vObject));
            if ((!kernel->splicedRunning) && (*vObject != v_public(kernel)))
                /* Ignore running check for kernel object.
                 * The claim must also succeed during shutdown when
                 * splicedRunning is already set to FALSE.
                 */
            {
                r = U_RESULT_ALREADY_DELETED;
                if (pa_ld32(&_this->domain->state) != U_DOMAIN_STATE_ALIVE) {
                    OS_REPORT(OS_WARNING, "u_observableClaimCommon", r,
                          "The splice deamon for domain \"%s\" (%u) is no longer running for entity 0x%"PA_PRIxADDR". "
                          "The claim will return ALREADY_DELETED.",
                          u_domainName(_this->domain), u_domainId(_this->domain),
                          (os_address)_this);
                }
            }/* else let splicedRunning remain false */
        }

        if (r == U_RESULT_OK) {
            c_base base = c_getBase(c_object(*vObject));
            if (!c_baseMakeMemReservation(base, memoryClaim)) {
                r = U_RESULT_OUT_OF_MEMORY;
                if (!appWarningGiven) {
                   appWarningGiven = OS_TRUE;
                   OS_REPORT(OS_WARNING, "u_observableClaimCommon", r,
                           "Unable to complete claim for service. Shared "
                           "memory has run out. You can try to free up some "
                           "memory by terminating (a) DDS application(s).");
                }
            }
        }

        /* If the result has become not ok, release the handle claim */
        if(r != U_RESULT_OK) {
            *vObject = NULL;
            u_observableRelease(_this, C_MM_RESERVATION_ZERO);
        }
    }
    return r;
}

u_result
u_observableWriteClaim(
    const u_observable _this,
          v_public* vObject,
          os_address memoryClaim)
{
    return u_observableClaimCommon(
        _this, vObject, U__CLAIM_SPLICED_RUNNING, memoryClaim);
}

u_result
u_observableReadClaim(
    const u_observable _this,
          v_public* vObject,
          os_address memoryClaim)
{
    return u_observableClaimCommon(
        _this, vObject, U__CLAIM_SPLICED_RUNNING, memoryClaim);
}

u_result
u_observableTriggerClaim(
    const u_observable _this,
    v_public *vObject,
    os_address memoryClaim)
{
    return u_observableClaimCommon(
        _this, vObject, U__CLAIM_NONE, memoryClaim);
}

void
u_observableRelease(
    const u_observable _this,
    os_address memoryClaimed)
{
    u_kind kind;

    assert(_this != NULL);

    if (memoryClaimed) {
        c_base base = c_getBase(_this->magic);
        c_baseReleaseMemReservation(base, memoryClaimed);
    }

    kind = u_objectKind(u_object(_this));
    if(kind != U_DOMAIN) {
        u_result result = u_handleRelease(_this->handle);
        if (result != U_RESULT_OK) {
            OS_REPORT(OS_INFO, "user::u_observableRelease", result,
                      "Failed to release the handle of entity 0x%"PA_PRIxADDR" (kind = %s),"
                      OS_REPORT_NL "result = %s (This is normal on process exit)",
                      (os_address)_this, u_kindImage(kind), u_resultImage(result));
        }
    }
    u_domainUnprotect();
}

void *
u_observableGetUserData(
    const u_observable _this)
{
    assert(_this != NULL);
    return _this->userData;
}

void *
u_observableSetUserData(
    const u_observable _this,
    void *userData)
{
    void *old = NULL;

    assert(_this != NULL);

    old = _this->userData;
    _this->userData = userData;
    if (old && userData) {
        OS_REPORT(OS_WARNING, "u_observableSetUserData", 0,
                  "Old value existed! and is now overwritten. Entity = 0x%"PA_PRIxADDR"",
                  (os_address)_this);
    }
    return old;
}

u_result
u_observableAction(
    const u_observable _this,
    void (*action)(v_public p, void *arg),
    void *arg)
{
    u_result result;
    v_public kp;

    assert(_this != NULL);
    assert(action != NULL);

    result = u_observableReadClaim(_this, &kp, C_MM_RESERVATION_ZERO);

    if(result == U_RESULT_OK){
        action(kp,arg);
        u_observableRelease(_this, C_MM_RESERVATION_ZERO);
        result = U_RESULT_OK;
    }
    return result;
}

u_result
u_observableWriteAction(
    const u_observable _this,
    void (*action)(v_public p, void *arg),
    void *arg)
{
    u_result result;
    v_public kp;

    assert(_this != NULL);
    assert(action != NULL);

    result = u_observableWriteClaim(_this, &kp, C_MM_RESERVATION_HIGH);

    if(result == U_RESULT_OK){
        action(kp,arg);
        u_observableRelease(_this, C_MM_RESERVATION_HIGH);
        result = U_RESULT_OK;
    }
    return result;
}

c_type
u_observableResolveType(
    const u_observable _this)
{
    v_public vObject;
    c_type type = NULL;
    u_result result;

    assert(_this != NULL);

    result = u_observableReadClaim(_this, &vObject, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK){
        switch(v_objectKind(vObject)){
        case K_TOPIC:
        case K_TOPIC_ADAPTER:
            type = v_topicDataType(v_topic(vObject));
        break;
        default:
            type = c_getType((c_object)vObject);
        break;
        }
        u_observableRelease(_this,C_MM_RESERVATION_ZERO);
    }
    return type;
}

v_gid
u_observableGid (
    const u_observable _this)
{
    return _this->gid;
}

u_handle
u_observableHandle (
    const u_observable _this)
{
    return _this->handle;
}

u_result
u_observableAddListener(
    const u_observable _this,
    const u_observableListener listener,
    void *userData)
{
    assert(_this != NULL);
    assert(listener != NULL);

    if (_this->dispatcher == NULL) {
        _this->dispatcher = u_dispatcherNew(_this);
    }
    return u_dispatcherInsertListener(_this->dispatcher, listener, userData);
}

u_result
u_observableRemoveListener(
    const u_observable _this,
    const u_observableListener listener)
{
    u_result result;

    assert(_this != NULL);

    if (_this->dispatcher != NULL) {
        result = u_dispatcherRemoveListener(_this->dispatcher, listener);
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

u_result
u_observableNotifyListener(
    const u_observable _this)
{
    u_result result;
    v_observer ko;

    assert(_this != NULL);

    result = u_observableReadClaim(_this, (v_public *)(&ko), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(ko);
        /* Wakeup the dispatch thread */
        v_observerLock(ko);
        v_observerNotify(ko, NULL, NULL);
        v_observerUnlock(ko);
        u_observableRelease(_this, C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_observableSetListenerMask(
    const u_observable _this,
    const u_eventMask eventMask)
{
    u_result result;
    v_observer ko;

    assert(_this != NULL);

    result = u_observableReadClaim(_this, (v_public *)(&ko), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(ko);
        v_observerSetEventMask(ko,eventMask);
        u_observableRelease(_this, C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_observableGetListenerMask(
    const u_observable _this,
    u_eventMask *eventMask)
{
    u_result result;
    v_observer ko;

    assert(_this != NULL);
    assert(eventMask != NULL);

    result = u_observableReadClaim(_this, (v_public *)(&ko), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(ko);
        *eventMask = v_observerGetEventMask(ko);
        u_observableRelease(_this, C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_bool
u_observableGetY2038Ready (
    u_observable _this)
{
    if (_this && _this->domain) {
        return _this->domain->y2038Ready;
    }
    return FALSE;
}

u_domainId_t
u_observableGetDomainId(
    u_observable _this)
{
    if (_this && _this->domain) {
        return _this->domain->id;
    }
    return -1;
}

#undef U__CLAIM_CHECK_MEMORY
#undef U__CLAIM_SPLICED_RUNNING
#undef U__CLAIM_NONE
