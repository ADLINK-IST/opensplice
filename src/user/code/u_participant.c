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
#include "u_participant.h"
#include "u_subscriber.h"
#include "u_writer.h"
#include "u__participant.h"
#include "u__topic.h"
#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__user.h"
#include "u__cfElement.h"
#include "u__domain.h"
#include "u__user.h"

#include "v_entity.h"
#include "v_participant.h"
#include "v_participantQos.h"
#include "v_leaseManager.h"
#include "v_configuration.h"
#include "v_event.h"
#include "v_topicAdapter.h"
#include "v_typeRepresentation.h"

#include "os_process.h"
#include "os_report.h"
#include "os_atomics.h"


#define u_participantReadClaim(_this, participant, claim) \
        u_observableReadClaim(u_observable(_this), (v_public *)(participant), claim)

#define u_participantWriteClaim(_this, participant, claim) \
        u_observableWriteClaim(u_observable(_this), (v_public *)(participant), claim)

#define u_participantRelease(_this, claim) \
        u_observableRelease(u_observable(_this), claim)

static void
decrThreadWaitCount(u_participant p)
{
    os_mutexLock(&p->mutex);
    if (--(p->threadWaitCount) == 0) {
        os_condBroadcast(&p->cv);
    }
    os_mutexUnlock(&p->mutex);
}

static void *
leaseManagerMain(
    void *arg)
{
    u_participant _this = u_participant(arg);
    v_participant kp;
    v_leaseManager lm;
    u_result result;

    assert(_this != NULL);

    result = u_participantReadClaim(_this, &kp, C_MM_RESERVATION_ZERO);
    decrThreadWaitCount(_this);
    if(result == U_RESULT_OK)
    {
        v_kernelThreadFlags(V_KERNEL_THREAD_FLAG_SERVICETHREAD, V_KERNEL_THREAD_FLAG_SERVICETHREAD);
        assert(kp);
        lm = v_participantGetLeaseManager(kp);
        v_leaseManagerMain(lm);
        c_free(lm);
        u_participantRelease(_this, C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_participant::leaseManagerMain", result,
                  "Failed to claim Participant, lease manager is not started");
    }
    return NULL;
}

static void *
resendManagerMain(
    void *arg)
{
    u_participant _this = u_participant(arg);
    v_participant kp;
    u_result result;

    assert(_this != NULL);

    result = u_participantReadClaim(_this, &kp, C_MM_RESERVATION_ZERO);
    decrThreadWaitCount(_this);
    if(result == U_RESULT_OK)
    {
        v_kernelThreadFlags(V_KERNEL_THREAD_FLAG_SERVICETHREAD, V_KERNEL_THREAD_FLAG_SERVICETHREAD);
        assert(kp);
        v_participantResendManagerMain(kp);
        u_participantRelease(_this, C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_participant::resendManagerMain", result,
                  "Failed to claim Participant, resend manager is not started");
    }
    return NULL;
}

void
u__participantFreeW (
    void *_this)
{
    u_participant p;
    p = u_participant(_this);

    while (pa_ld32(&p->useCount) > 0) {
        os_duration t = OS_DURATION_INIT(0, 100000000);
        os_sleep(t);
    }

    u__entityFreeW(_this);
}

u_result
u__participantDeinitW (
    void *_this)
{
    u_result r = U_RESULT_OK;
    u_participant p;
    u_domain domain;
    p = u_participant(_this);
    assert(p != NULL);

    os_condDestroy(&p->cv);
    os_mutexDestroy(&p->mutex);
    (void)pa_dec32_nv(&p->useCount);
    domain = u_observableDomain(u_observable(p));
    if (domain) {
        r = u_participantDetach(p);
        if (r == U_RESULT_OK) {
            u__entityDeinitW(_this);
            (void) u_domainRemoveParticipant(domain, p);
            r = u_domainClose(domain);
        } else if (r == U_RESULT_ALREADY_DELETED) {
            r = U_RESULT_OK;
        }
    } else {
        (void)u__entityDeinitW(_this);
    }

    return r;
}

u_participant
u_participantNew(
    const os_char *uri,
    const u_domainId_t id,
    os_uint32 timeout,
    const os_char *name,
    const u_participantQos qos,
    u_bool enable)
{
    u_domain domain;
    u_participant p = NULL;
    u_result r;
    v_kernel kk = NULL;
    v_participant kp;

    if (timeout > OS_MAX_INTEGER(os_int32)) {
        return NULL;
    }

    r = u_domainOpen(&domain, uri, id, (os_int32) timeout);
    if (r == U_RESULT_OK)
    {
        r = u_observableWriteClaim(u_observable(domain),(v_public*)(&kk), C_MM_RESERVATION_HIGH);
        if (r == U_RESULT_OK) {
            assert(kk);
            kp = v_participantNew(kk, name, (v_participantQos)qos, enable);
            if (kp != NULL) {
                p = u_objectAlloc(sizeof(*p), U_PARTICIPANT, u__participantDeinitW, u__participantFreeW);
                if (p != NULL) {
                    r = u_participantInit(p, kp, domain);
                    if (r != U_RESULT_OK) {
                        OS_REPORT(OS_ERROR,"u_participantNew",r,
                                  "Initialization Participant failed.");
                        u_objectFree(u_object(p));
                        p = NULL;
                    }
                } else {
                    OS_REPORT(OS_ERROR,"u_participantNew",U_RESULT_INTERNAL_ERROR,
                              "Allocation user proxy failed.");
                }
                c_free(kp);
            } else {
                OS_REPORT(OS_ERROR,"u_participantNew",U_RESULT_INTERNAL_ERROR,
                          "Create kernel entity failed.");
            }
            u_observableRelease(u_observable(domain), C_MM_RESERVATION_HIGH);
        } else {
            OS_REPORT(OS_ERROR,"u_participantNew",r,
                      "Claim Kernel failed.");
        }

        if (p == NULL) {
           (void)u_domainClose(domain);
        }
    } else {
        const c_char *uri_string;
        if (uri) {
            uri_string = uri;
        } else {
            uri_string = "<NULL>";
        }
        OS_REPORT(OS_ERROR,"u_participantNew",r,
                    "Failure to open the domain, domain id = %d and URI=\"%s\" "
                    "The most common cause of this error is that OpenSpliceDDS "
                    "is not running (when using shared memory mode). "
                    "Please make sure to start OpenSplice before creating a "
                    "DomainParticipant.",
                    id, uri_string);
    }
    return p;
}

static c_ulong
u_participantNewGroupListener(
    u_observable _this,
    c_ulong event,
    c_voidp usrData)
{
    u_result r;
    v_participant kp;

    OS_UNUSED_ARG(event);
    OS_UNUSED_ARG(usrData);

    r = u_observableWriteClaim(_this, (v_public*)(&kp),C_MM_RESERVATION_HIGH);
    if(r == U_RESULT_OK)
    {
        assert(kp);
        v_participantConnectNewGroup(kp,NULL);
        u_observableRelease(_this, C_MM_RESERVATION_HIGH);
    }
    return r;
}

u_result
u_participantInit (
    const u_participant _this,
    const v_participant kp,
    const u_domain domain)
{
    u_result r;
    os_threadAttr attr;
    os_mutexAttr mutexAttr;
    os_condAttr condAttr;
    os_result osr;
    c_ulong mask;

    assert(_this != NULL);
    assert(domain != NULL);

    r = u_entityInit(u_entity(_this), v_entity(kp), domain);
    if (r == U_RESULT_OK) {
        assert(kp);

        pa_st32(&_this->useCount, 1);

        os_mutexAttrInit(&mutexAttr);
        os_condAttrInit(&condAttr);
        osr = os_mutexInit(&_this->mutex, &mutexAttr);
        if (osr == os_resultSuccess) {
            osr = os_condInit(&_this->cv, &_this->mutex, &condAttr);
            if (osr == os_resultSuccess) {
                os_mutexLock(&_this->mutex);
                _this->threadWaitCount = 0;

                r = u_domainAddParticipant(domain,_this);

                os_threadAttrInit(&attr);
                switch (kp->qos->watchdogScheduling.v.kind) {
                case V_SCHED_REALTIME:
                    attr.schedClass = OS_SCHED_REALTIME;
                break;
                case V_SCHED_TIMESHARING:
                    attr.schedClass = OS_SCHED_TIMESHARE;
                break;
                default:
                    attr.schedClass = os_procAttrGetClass ();
                break;
                }
                attr.schedPriority = kp->qos->watchdogScheduling.v.priority;
                if (kp->qos->watchdogScheduling.v.priorityKind == V_SCHED_PRIO_RELATIVE) {
                    attr.schedPriority += os_procAttrGetPriority();
                }

                osr = os_threadCreate(&_this->threadId, "watchdog", &attr, leaseManagerMain, _this);
                if(osr == os_resultSuccess){
                    (_this->threadWaitCount)++;
                } else {
                    OS_REPORT(OS_CRITICAL, "u_participantInit", osr,
                              "Watchdog thread could not be started.\n");
                }

                osr = os_threadCreate(&_this->threadIdResend, "resendManager", &attr,
                                (void *(*)(void *))resendManagerMain,(void *)_this);

                if(osr == os_resultSuccess){
                    (_this->threadWaitCount)++;
                } else {
                    OS_REPORT(OS_CRITICAL, "u_participantInit", osr,
                              "Watchdog thread could not be started.\n");
                }

                (void)u_observableGetListenerMask(u_observable(_this), &mask);
                (void)u_observableAddListener(u_observable(_this), u_participantNewGroupListener, NULL);
                mask |= V_EVENT_NEW_GROUP;
                mask |= V_EVENT_CONNECT_WRITER;
                (void)u_observableSetListenerMask(u_observable(_this), mask);

                while (_this->threadWaitCount > 0) {
                    os_condWait(&_this->cv, &_this->mutex);
                }
                os_mutexUnlock(&_this->mutex);
            } else {
                OS_REPORT(OS_CRITICAL, "u_participantInit", osr,
                          "Unable to initialize condition variable.\n");
                r = U_RESULT_INTERNAL_ERROR;
            }
        } else {
            OS_REPORT(OS_CRITICAL, "u_participantInit", osr,
                      "Unable to initialize mutex.\n");
            r = U_RESULT_INTERNAL_ERROR;
        }

    }
    return r;
}

u_result
u_participantGetQos (
    const u_participant _this,
          u_participantQos *qos)
{
    u_result result;
    v_participant vParticipant;
    v_participantQos vQos;

    assert(_this != NULL);
    assert(qos != NULL);

    result = u_participantReadClaim(_this, &vParticipant, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        vQos = v_participantGetQos(vParticipant);
        *qos = u_participantQosNew(vQos);
        c_free(vQos);
        u_participantRelease(_this, C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_participantSetQos (
    const u_participant _this,
    const u_participantQos qos)
{
    u_result result;
    v_participant vParticipant;

    assert(_this != NULL);
    assert(qos != NULL);

    result = u_participantReadClaim(_this, &vParticipant, C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernel(v_participantSetQos(vParticipant, qos));
        u_participantRelease(_this, C_MM_RESERVATION_LOW);
    }
    return result;
}

void
u_participantIncUseCount(
    const u_participant _this)
{
    pa_inc32(&_this->useCount);
}

void
u_participantDecUseCount(
    const u_participant _this)
{
    (void)pa_dec32_nv(&_this->useCount);
}

u_result
u_participantDetach(
    const u_participant _this)
{
    u_result r;
    v_participant kp;
    v_leaseManager lm;

    assert(_this != NULL);

    r = u_participantReadClaim(_this, &kp, C_MM_RESERVATION_NO_CHECK);
    if(r == U_RESULT_OK)
    {
        assert(kp);

        lm = v_participantGetLeaseManager(kp);
        if (lm != NULL) {
            v_leaseManagerNotify(lm, NULL, V_EVENT_TERMINATE);
        }
        v_participantResendManagerQuit(kp);
        if (lm != NULL) {
            os_threadWaitExit(_this->threadId, NULL);
            c_free(lm);
        } else {
            OS_REPORT(OS_CRITICAL, "u_participantDetach", U_RESULT_INTERNAL_ERROR,
                      "Access to lease manager failed.");
        }
        os_threadWaitExit(_this->threadIdResend, NULL);
        u_participantRelease(_this, C_MM_RESERVATION_NO_CHECK);
    } else {
        OS_REPORT(OS_WARNING,"u_participantDetach", r,
                  "Failed to claim Participant.");
    }

    return r;
}

u_domain
u_participantDomain(
    const u_participant _this)
{
    assert(_this != NULL);
    return u_observableDomain(u_observable(_this));
}

u_domainId_t
u_participantGetDomainId(
    const u_participant _this)
{
    u_domainId_t domainId;
    u_domain domain;

    assert(_this != NULL);

    domain = u_observableDomain(u_observable(_this));
    if (domain == NULL) {
        domainId = U_DOMAIN_ID_INVALID ;
    } else {
        domainId = u_domainId(domain);
    }

    return domainId;
}

u_cfElement
u_participantGetConfiguration(
    const u_participant _this)
{
    u_domain domain;
    u_result r;
    v_kernel k;
    v_configuration config;
    u_cfElement cfg = NULL;

    assert(_this != NULL);

    domain = u_observableDomain(u_observable(_this));
    r = u_observableReadClaim(u_observable(domain),(v_public*)(&k), C_MM_RESERVATION_ZERO);
    if ((r == U_RESULT_OK) && (k != NULL)) {
        config= v_getConfiguration(k);
        if(config!= NULL){
             cfg = u_cfElementNew(_this, v_configurationGetRoot(config));
        }
        u_observableRelease(u_observable(domain), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_ERROR,
                  "u_participantGetConfiguration", r,
                  "Failed to claim domain.");
    }

    return cfg;
}

c_iter
u_participantFindTopic(
    const u_participant _this,
    const os_char *name,
    const os_duration timeout)
{
    u_result r;
    v_participant kp;
    u_topic t;
    v_topic kt;
    v_topicAdapter kta;
    c_iter list = NULL;
    c_iter topics = NULL;
    os_int retry = 1;
    os_timeM now, endTime = OS_TIMEM_INVALID;
    os_duration delta, tryPeriod = 100*OS_DURATION_MILLISECOND;

    if (!OS_DURATION_ISINFINITE(timeout)) {
        endTime = os_timeMAdd(os_timeMGet(), timeout);
    }
    topics = NULL;

    do {
        r = u_participantReadClaim(_this, &kp, C_MM_RESERVATION_ZERO);

        if(r == U_RESULT_OK){
            assert(kp);
            /** \todo Make real implementation when SI912 is solved...... */
            list = v_resolveTopics(v_objectKernel(kp),name);
            if (c_iterLength(list) != 0) {
                kt = c_iterTakeFirst(list);
                while (kt != NULL) {
                    kta = v_topicAdapterWrap(kp, kt);
                    if (kta) {
                        t = u_objectAlloc(sizeof(*t), U_TOPIC, u__topicDeinitW, u__topicFreeW);
                        r = u_topicInit(t, v_topicAdapterName(kta), kta, _this);
                        if (r == U_RESULT_OK) {
                            topics = c_iterInsert(topics, t);
                        }
                        c_free(kta);
                    }
                    c_free(kt);
                    kt = c_iterTakeFirst(list);
                }
            }
            c_iterFree(list);
            list = NULL;
            u_participantRelease(_this, C_MM_RESERVATION_ZERO);

            if(topics == NULL){
                if (!OS_DURATION_ISINFINITE(timeout)) {
                    now = os_timeMGet();

                    if (os_timeMCompare(now, endTime) == OS_LESS) {
                        delta = os_timeMDiff(endTime, now);

                        if (os_durationCompare(delta, tryPeriod) == OS_LESS) {
                            tryPeriod = delta;
                        }
                    } else {
                        retry = 0; /* timeout expired */
                    }
                }
            } else {
                retry = 0; /* topic found or error */
            }
            if (retry) {
                os_sleep(tryPeriod);
            }
        } else {
            retry = 0;
            OS_REPORT(OS_WARNING,
                      "u_participantFindTopic", r,
                      "Failed to claim Participant.");
        }
    } while (retry);

    return topics;
}

u_result
u_participantAssertLiveliness(
    const u_participant _this)
{
    u_result r;
    v_participant kp;

    assert(_this != NULL);

    r = u_participantReadClaim(_this, &kp, C_MM_RESERVATION_ZERO);
    if(r == U_RESULT_OK)
    {
        assert(kp);
        v_participantAssertLiveliness(kp);
        u_participantRelease(_this, C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_participantAssertLiveliness", r,
                  "Failed to claim Participant.");
    }
    return r;
}

u_result
u_participantDeleteHistoricalData(
    const u_participant _this,
    const os_char *partitionExpr,
    const os_char *topicExpr)
{
    u_result r;
    v_participant kp;

    assert(_this != NULL);
    assert(topicExpr != NULL);
    assert(partitionExpr != NULL);

    r = u_participantReadClaim(_this, &kp, C_MM_RESERVATION_LOW);
    if(r == U_RESULT_OK)
    {
        assert(kp);
        if(partitionExpr && topicExpr){
            v_participantDeleteHistoricalData(kp, partitionExpr, topicExpr);
        } else {
            r = U_RESULT_ILL_PARAM;
            OS_REPORT(OS_ERROR,"u_participantDeleteHistoricalData", r,
                      "Illegal parameter.");
        }
        u_participantRelease(_this, C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_participantDeleteHistoricalData", r,
                  "Failed to claim Participant.");
    }
    return r;
}

u_result
u_participantRegisterTypeRepresentation (
    u_participant _this,
    const u_typeRepresentation tr)
{
    u_result result;
    v_participant kp;
    v_typeRepresentation ktr;

    assert(_this != NULL);
    assert(tr != NULL);
    assert(tr->metaData != NULL);
    assert(tr->metaDataLen != 0);

    result = u_participantReadClaim(_this, &kp, C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        ktr = v_typeRepresentationNew(kp,
                tr->typeName,
                tr->dataRepresentationId,
                tr->typeHash,
                tr->metaData, tr->metaDataLen,
                tr->extentions, tr->extentionsLen);
        c_free(ktr);
        u_participantRelease(_this, C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_participantRegisterTypeRepresentation", result,
                  "Failed to claim Participant.");
    }

    return result;
}

void*
u_domainParticipant_create_copy_cache(
    u_participant _this,
    void* (action)(void* arg),
    void* arg)
{
    v_participant kp;
    u_result result;
    void* pvReturn = NULL;

    result = u_participantReadClaim(_this, &kp, C_MM_RESERVATION_ZERO);
    if(result == U_RESULT_OK)
    {
        assert(kp);
        pvReturn = action(arg);
        u_participantRelease(_this, C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_participant::u_domainParticipant_create_copy_cache", 0,
                  "Failed to claim Participant");
    }
    return pvReturn;
}

u_result
u_participantFederationSpecificPartitionName (
    u_participant _this,
    c_char *buf,
    os_size_t bufsize)
{
    u_result result;
    u_domain domain;
#if 0
    if ((result = u_entityLock (u_entity(_this))) == U_RESULT_OK) {
#endif
        domain = u_observableDomain(u_observable(_this));
        result = u_domainFederationSpecificPartitionName(domain, buf, bufsize);
#if 0
        u_entityUnlock (u_entity (_this));
    }
#endif
    return result;
}
