/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "u_participant.h"
#include "u__participant.h"
#include "u__subscriber.h"
#include "u__types.h"
#include "u__dispatcher.h"
#include "u__entity.h"
#include "u__user.h"
#include "u__writer.h"
#include "u__cfElement.h"
#include "u__kernel.h"
#include "u_user.h"
#include "u_scheduler.h"

#include "v_entity.h"
#include "v_participant.h"
#include "v_leaseManager.h"
#include "v_configuration.h"
#include "v_event.h"
#include "v_statistics.h"

#include "os_report.h"

u_result
u_participantClaim(
    u_participant _this,
    v_participant *participant)
{
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (participant != NULL)) {
        *participant = v_participant(u_entityClaim(u_entity(_this)));
        if (*participant == NULL) {
            OS_REPORT_2(OS_WARNING,"u_participantClaim",0,
                        "Claim Participant failed"
                        "<_this = 0x%x, participant = 0x%x>.",
                        _this, participant);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_participantClaim",0,
                    "Illegal parameter. "
                    "<_this = 0x%x, reader = 0x%x>.",
                    _this, participant);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_participantRelease(
    u_participant _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_participantRelease",0,
                    "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

static void *
leaseManagerMain(
    void *arg)
{
    u_participant _this = u_participant(arg);
    v_participant kp;
    v_leaseManager lm;
    u_result result;

    result = u_participantClaim(_this, &kp);
    if ((result == U_RESULT_OK) && (kp != NULL)) {
        lm = v_participantGetLeaseManager(kp);
        v_leaseManagerMain(lm);
        c_free(lm);
        u_participantRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_participant::leaseManagerMain", 0,
                  "Failed to claim Participant");
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

    result = u_participantClaim(_this, &kp);
    if ((result == U_RESULT_OK) && (kp != NULL)) {
        v_participantResendManagerMain(kp);
        u_participantRelease(_this);
    } else {
        OS_REPORT(OS_WARNING, "u_participant::resendManagerMain", 0,
                  "Failed to claim Participant");
    }
    return NULL;
}

u_participant
u_participantNew(
    const c_char *uri,
    c_long timeout,
    const c_char *name,
    v_qos qos,
    c_bool enable)
{
    u_kernel kernel;
    u_participant p = NULL;
    u_result r;
    v_kernel kk = NULL;
    v_participant kp;

    kernel = u_userKernelOpen(uri, timeout);
    if (kernel != NULL) {
        r = u_kernelClaim(kernel,&kk);
        if ((r == U_RESULT_OK) && (kk != NULL)) {
            kp = v_participantNew(kk,name,qos, NULL,enable);
            if (kp != NULL) {
                p = u_entityAlloc(NULL,u_participant,kp,TRUE);
                if (p != NULL) {
                    r = u_participantInit(p, kernel);
                    if (r != U_RESULT_OK) {
                        os_free(p);
                        p = NULL;
                        OS_REPORT(OS_ERROR,"u_participantNew",0,
                                  "Initialization Participant failed.");
                    }
                } else {
                    OS_REPORT(OS_ERROR,"u_participantNew",0,
                              "Allocation user proxy failed.");
                }
                c_free(kp);
            } else {
                OS_REPORT(OS_ERROR,"u_participantNew",0,
                          "Create kernel entity failed.");
            }
            r = u_kernelRelease(kernel);
        } else {
            OS_REPORT(OS_ERROR,"u_participantNew",0,
                      "Claim Kernel failed.");
        }
    } else {
        OS_REPORT(OS_ERROR,"u_participantNew",0,
                  "Failure to open the kernel");
    }
    return p;
}

static u_cfAttribute
u_configurationResolveAttribute(
    u_cfElement e,
    const c_char* xpathExpr,
    const c_char* attrName)
{
    u_cfAttribute result;
    c_iter nodes;
    u_cfNode tmp;

    result = NULL;

    if (e != NULL) {
        nodes = u_cfElementXPath(e, xpathExpr);
        tmp = u_cfNode(c_iterTakeFirst(nodes));

        if (tmp != NULL) {
            if (u_cfNodeKind(tmp) == V_CFELEMENT) {
                result = u_cfElementAttribute(u_cfElement(tmp), attrName);
            }
        }
        while(tmp != NULL){
            u_cfNodeFree(tmp);
            tmp = u_cfNode(c_iterTakeFirst(nodes));
        }
        c_iterFree(nodes);
    }
    return result;
}

static u_cfData
u_configurationResolveParameter(
    u_cfElement e,
    const c_char* xpathExpr)
{
    u_cfData result;
    c_iter nodes;
    u_cfNode tmp;

    result = NULL;

    if(e != NULL){
        nodes = u_cfElementXPath(e, xpathExpr);
        tmp = u_cfNode(c_iterTakeFirst(nodes));

        if(tmp != NULL){
            if(u_cfNodeKind(tmp) == V_CFDATA){
                result = u_cfData(tmp);
            } else {
                u_cfNodeFree(tmp);
            }
            tmp = u_cfNode(c_iterTakeFirst(nodes));
        }

        while(tmp != NULL){
            u_cfNodeFree(tmp);
            tmp = u_cfNode(c_iterTakeFirst(nodes));
        }
        c_iterFree(nodes);
    }
    return result;
}

#define U_WATCHDOG_TEXT         "/#text"
#define U_WATCHDOG_CLASS_FMT    "%s[@name='%s']/Watchdog/Scheduling/Class"
#define U_WATCHDOG_PRIO_FMT     "%s[@name='%s']/Watchdog/Scheduling/Priority"
#define U_WATCHDOG_ATTRKIND     "priority_kind"

static void
participantGetWatchDogAttr(
    u_cfElement root,
    const c_char* element,
    const c_char* name,
    os_threadAttr * attr)
{
    c_char* path;
    c_char* schedClass;
    c_char* prioKind;
    c_ulong prio;
    c_bool success;
    u_cfData data;
    u_cfAttribute attribute;
    struct v_schedulePolicy qos;

    if (root != NULL) {
        path = os_malloc(strlen(U_WATCHDOG_CLASS_FMT U_WATCHDOG_TEXT) +
                         strlen(element) + strlen(name) + 1);
        if (path != NULL) {
            sprintf(path, U_WATCHDOG_CLASS_FMT U_WATCHDOG_TEXT, element, name);
            data = u_configurationResolveParameter(root, path);
            if (data != NULL) {
                success = u_cfDataStringValue(data, &schedClass);
                if (success) {
                    if (strcmp(schedClass, "Default")==0) {
                        qos.kind = V_SCHED_DEFAULT;
                    } else if (strcmp(schedClass, "Realtime")==0) {
                        qos.kind = V_SCHED_REALTIME;
                    } else if (strcmp(schedClass, "Timeshare")==0) {
                        qos.kind = V_SCHED_TIMESHARING;
                    } else {
                        OS_REPORT_1(OS_ERROR, "Splicedaemon initialization", 0,
                                    "Illegal 'Scheduling/Class' for %s", name);
                        success = FALSE;
                    }
                    os_free(schedClass);
                } else {
                    OS_REPORT_1(OS_ERROR, "Splicedaemon initialization", 0,
                    "Illegal 'Scheduling/Class' for %s. Applying default.",
                    name);
                }
                u_cfDataFree(data);
            } else {
                success = FALSE;
            }
            os_free(path);
        } else {
            success = FALSE;
        }

        if (success) {
            path = os_malloc(strlen(U_WATCHDOG_PRIO_FMT U_WATCHDOG_TEXT) +
                             strlen(element) + strlen(name) + 1);
            if (path != NULL) {
                sprintf(path, U_WATCHDOG_PRIO_FMT U_WATCHDOG_TEXT,
                        element, name);
                data = u_configurationResolveParameter(root, path);
                if (data != NULL) {
                    success = u_cfDataULongValue(data, &prio);
                    if (success) {
                        qos.priority = prio;
                    } else {
                        OS_REPORT_1(OS_ERROR, "Splicedaemon initialization", 0,
                        "%s Configuration: value of 'Scheduling/Priority' not "
                        "valid.  Applying default.", name);
                    }
                    u_cfDataFree(data);
                } else {
                    success = FALSE;
                }
                os_free(path);
            } else {
                success = FALSE;
            }
        }

        if (success) {
            path = os_malloc(strlen(U_WATCHDOG_PRIO_FMT) +
                             strlen(element) + strlen(name) + 1);
            if (path != NULL) {
                sprintf(path, U_WATCHDOG_PRIO_FMT, element, name);
                attribute = u_configurationResolveAttribute(root, path,
                                                            U_WATCHDOG_ATTRKIND);
                if (attribute != NULL) {
                    success = u_cfAttributeStringValue(attribute, &prioKind);
                    if (success) {
                        if (strcmp(prioKind, "Relative")==0) {
                            qos.priorityKind = V_SCHED_PRIO_RELATIVE;
                        } else if (strcmp(prioKind, "Absolute")==0) {
                            qos.priorityKind = V_SCHED_PRIO_ABSOLUTE;
                        } else {
                            OS_REPORT_1(OS_ERROR,
                                        "Splicedaemon initialization", 0,
                                        "Invalid 'priority_kind' in "
                                        "'Scheduling/Priority'. "
                                        "Applying default for %s.", name);
                            success = FALSE;
                        }
                    } else {
                        OS_REPORT_1(OS_ERROR, "Splicedaemon initialization", 0,
                        "Invalid 'priority_kind' in 'Scheduling/Priority'. "
                        "Applying default for %s.", name);
                    }
                } else {
                    success = FALSE;
                    os_free(path);
                }
            } else {
                success = FALSE;
            }
        }
        os_threadAttrInit(attr);

        if (success) {
            u_threadAttrInit (&qos, attr);
        }
    } else {
        OS_REPORT_1(OS_INFO,
                    "Splicedaemon initialization", 0,
                    "No watchdog configuration information specified\n"
                    "              for process \"%s\".\n"
                    "              The service will not be able to detect "
                    "termination of\n              this process.",
                     name);
        os_threadAttrInit(attr);
    }
}

static c_ulong
u_participantNewGroupListener(
    u_dispatcher _this,
    c_ulong event,
    c_voidp usrData)
{
    u_result r;
    v_participant kp;

    r = u_participantClaim(u_participant(_this), &kp);
    if ((r == U_RESULT_OK) && (kp != NULL)) {
        v_participantConnectNewGroup(kp,NULL);
        r = u_participantRelease(u_participant(_this));
    }
    return r;
}
    
u_result
u_participantInit (
    u_participant p,
    u_kernel kernel)
{
    u_result r;
    v_participant kp;
    os_threadAttr attr;
    os_result osr;
    u_cfElement root;
    c_ulong mask;

    if (p == NULL) {
        return U_RESULT_ILL_PARAM;
    }

    p->kernel = kernel;
    r = u_participantClaim(p, &kp);
    if ((r == U_RESULT_OK) && (kp != NULL)) {
        if (u_entity(p)->kind == U_SERVICE) {
            root = u_participantGetConfiguration(p);
            switch(u_service(p)->serviceKind) {
            case U_SERVICE_NETWORKING:
                participantGetWatchDogAttr(root, "NetworkService",
                                           v_participantName(kp), &attr);
            break;
            case U_SERVICE_DURABILITY:
                participantGetWatchDogAttr(root, "DurabilityService",
                                           v_participantName(kp), &attr);
            break;
            case U_SERVICE_CMSOAP:
                participantGetWatchDogAttr(root, "TunerService",
                                           v_participantName(kp), &attr);
            break;
            case U_SERVICE_SPLICED:
                /*
                 * There doesn't exist a proper "service" configuration for the splicedaemon
                 * the Daemon config for example doesn't have a name attribute.
                 * For now, we leave it at default
                 */
                os_threadAttrInit(&attr);
            break;
            case U_SERVICE_INCOGNITO:
                os_threadAttrInit(&attr);
            break;
            default:
                OS_REPORT(OS_ERROR, "u_participantInit", 0,
                          "Internal error: Unknown Service kind detected.");
                os_threadAttrInit(&attr);
            break;
            }
            u_cfElementFree(root);
        } else {
            os_threadAttrInit(&attr);
        }
        r = u_dispatcherInit(u_dispatcher(p));
        if (r == U_RESULT_OK) {
            r = u_kernelAdd(kernel,p);
            u_entity(p)->flags |= U_ECREATE_INITIALISED;

            osr = os_threadCreate(&p->threadId, "watchdog", &attr,
                (void *(*)(void *))leaseManagerMain,(void *)p);

            if(osr != os_resultSuccess){
                OS_REPORT(OS_ERROR, "u_participantInit", 0,
                          "Watchdog thread could not be started.\n");
            }

            osr = os_threadCreate(&p->threadIdResend, "resendManager", &attr,
                            (void *(*)(void *))resendManagerMain,(void *)p);

            if(osr != os_resultSuccess){
                OS_REPORT(OS_ERROR, "u_participantInit", 0,
                          "Watchdog thread could not be started.\n");
            }
        } else {
            OS_REPORT(OS_ERROR, "u_participantInit", 0,
                      "Dispatcher Initialization failed.");
        }
        u_dispatcherGetEventMask(u_dispatcher(p), &mask);
        u_dispatcherInsertListener(u_dispatcher(p),
                                   u_participantNewGroupListener,
                                   NULL);
        mask |= V_EVENT_NEW_GROUP;
        u_dispatcherSetEventMask(u_dispatcher(p), mask);
        r = u_participantRelease(p);
        if (r != U_RESULT_OK) {
            OS_REPORT(OS_ERROR, "u_participantInit", 0,
                      "Release Participant failed.");
        }
    } else {
        OS_REPORT(OS_WARNING, "u_participantInit", 0,
                  "failed to claim Participant.");
    }
    return r;
}

u_result
u_participantFree (
    u_participant p)
{
    u_result r = U_RESULT_OK;
    u_kernel kernel;

    if (p != NULL) {
        if (p->kernel != NULL) {
            if (u_entity(p)->flags & U_ECREATE_INITIALISED) {
                kernel = p->kernel;
                r = u_participantDeinit(p);
                if (r == U_RESULT_OK) {
                    os_free(p);
                    u_userKernelClose(kernel);
                }
            } else {
                r = u_entityFree(u_entity(p));
            }
        } else {
            OS_REPORT(OS_WARNING, "u_participantFree", 0,
                      "Participant is not longer attached to a kernel.");
            os_free(p);
        }
    } else {
        OS_REPORT(OS_WARNING, "u_participantFree", 0,
                  "The specified Participant = NIL.");
        r = U_RESULT_OK;
    }
    return r;
}

u_result
u_participantDeinit (
    u_participant p)
{
    u_result r;
    v_participant kp;
    v_leaseManager lm;

    if (p != NULL) {
        if (p->kernel != NULL) {
            r = u_participantClaim(p,&kp);
            if ((r == U_RESULT_OK) && (kp != NULL)) {
                u_dispatcherDeinit(u_dispatcher(p));
                lm = v_participantGetLeaseManager(kp);

                if (lm != NULL) {
                    v_leaseManagerNotify(lm, NULL, V_EVENT_TERMINATE);
                }

                v_participantResendManagerQuit(kp);
                if (lm != NULL) {
                    os_threadWaitExit(p->threadId, NULL);
                    c_free(lm);
                } else {
                    OS_REPORT(OS_ERROR, "u_participantDeinit", 0,
                              "Access to lease manager failed.");
                }
                os_threadWaitExit(p->threadIdResend, NULL);

                u_participantRelease(p);
            } else {
                OS_REPORT(OS_WARNING, "u_participantDeinit", 0,
                          "Failed to claim Participant.");
            }
            r = u_kernelRemove(p->kernel,p);
            if (r == U_RESULT_OK) {
                /* Disable the participant to avoid multiple Free's */
                p->kernel = NULL;
            } else {
                OS_REPORT(OS_ERROR, "u_participantDeinit", 0,
                          "Remove Participant from Kernel failed.");
            }
        } else {
            /* valid state of a disabled participant.
               No actions required here. */
            r = U_RESULT_OK;
        }
    } else {
        OS_REPORT(OS_ERROR, "u_participantDeinit", 0,
                  "Participant is not specified.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_kernel
u_participantKernel(
    u_participant p)
{
    if (p == NULL) {
        return NULL;
    }
    return p->kernel;
}

u_result
u_participantDisable(
    u_participant p)
{
    u_result r = U_RESULT_ILL_PARAM;

    if (p != NULL) {
      p->kernel = NULL;
      r = U_RESULT_OK;
    }
    return r;
}

u_cfElement
u_participantGetConfiguration(
    u_participant _this)
{
    u_result r;
    v_kernel k;
    v_configuration config;
    u_cfElement cfg = NULL;

    if (_this != NULL) {
        r = u_kernelClaim(_this->kernel,&k);
        if ((r == U_RESULT_OK) && (k != NULL)) {
            config= v_getConfiguration(k);
            if(config!= NULL){
                 cfg = u_cfElementNew(_this, v_configurationGetRoot(config));
            }
            u_kernelRelease(_this->kernel);
        } else {
            OS_REPORT(OS_ERROR,
                      "u_participantGetConfiguration", 0,
                      "Failed to claim participant.");
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "u_participantGetConfiguration", 0,
                  "Illegal parameter.");
    }
    return cfg;
}

u_result
u_participantRenewLease(
    u_participant participant,
    v_duration leasePeriod)
{
    u_result r;
    v_participant p;

    if (participant == NULL) {
        r = U_RESULT_ILL_PARAM;
    } else {
        r = u_participantClaim(participant, &p);
        if ((r == U_RESULT_OK) && (p != NULL)) {
            v_participantRenewLease(p, leasePeriod);
            r = u_participantRelease(participant);
        } else {
            OS_REPORT(OS_WARNING,
                      "u_participantRenewLease", 0,
                      "Failed to claim Participant.");
        }
    }
    return r;
}

c_iter
u_participantFindTopic(
    u_participant p,
    const c_char *name,
    v_duration timeout)
{
    u_result r;
    v_participant kp;
    u_topic t;
    v_topic kt;
    c_iter list, topics;
    os_time delay;

    topics = NULL;
    if (p== NULL) {
        OS_REPORT(OS_ERROR,
                  "User Participant",0,
                  "u_participantFindTopic: No participant specified.");
    } else {
        r = u_participantClaim(p, &kp);
        if ((r == U_RESULT_OK) && (kp != NULL)) {
            /** \todo Make real implementatio when SI912 is solved...... */
            list = v_resolveTopics(v_objectKernel(kp),name);
            if (c_iterLength(list) == 0) {
                c_iterFree(list);
                delay.tv_sec = timeout.seconds;
                delay.tv_nsec = timeout.nanoseconds;
                os_nanoSleep(delay);
                list = v_resolveTopics(v_objectKernel(kp),name);
            }
            kt = c_iterTakeFirst(list);
            while (kt != NULL) {
                t = u_entityAlloc(p,u_topic,kt,FALSE);
                c_free(kt);
                topics = c_iterInsert(topics,t);
                kt = c_iterTakeFirst(list);
            }
            c_iterFree(list);
            r = u_participantRelease(p);
        } else {
            OS_REPORT(OS_WARNING,
                      "u_participantFindTopic",0,
                      "Failed to claim Participant.");
        }
    }
    return topics;
}

u_subscriber
u_participantGetBuiltinSubscriber(
    u_participant p)
{
    u_subscriber builtinSubscriber = NULL;
    v_subscriber s;
    u_result r = U_RESULT_OK;
    v_participant kp;

    if (p == NULL) {
        OS_REPORT(OS_ERROR,
                  "u_participantGetBuiltinSubscriber",0,
                  "No participant specified.");
    } else {
        r = u_participantClaim(p, &kp);
        if ((r == U_RESULT_OK) && (kp != NULL)) {
            s = v_participantGetBuiltinSubscriber(kp);
            builtinSubscriber = u_entityAlloc(p,u_subscriber,s,FALSE);
            u_subscriberInit(builtinSubscriber);
            r = u_participantRelease(p);
            c_free(s);
        } else {
            OS_REPORT(OS_WARNING,
                      "u_participantGetBuiltinSubscriber",0,
                      "Failed to claim Participant.");
        }
    }

    return builtinSubscriber;
}

u_result
u_participantAssertLiveliness(
    u_participant p)
{
    u_result r;
    v_participant kp;

    if (p != NULL) {
        r = u_participantClaim(p, &kp);
        if ((r == U_RESULT_OK) && (kp != NULL)) {
            v_participantAssertLiveliness(kp);
            r = u_participantRelease(p);
        } else {
            OS_REPORT(OS_WARNING, "u_participantAssertLiveliness", 0,
                      "Failed to claim Participant.");
        }
    } else {
        OS_REPORT(OS_ERROR,"u_participantAssertLiveliness",0,
                  "No participant specified.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_participantDetach(
    u_participant p)
{
    u_result r;
    v_participant kp;
    v_leaseManager lm;

    if (p != NULL) {
        r = u_participantClaim(p, &kp);
        if ((r == U_RESULT_OK) && (kp != NULL)) {
            u_dispatcherDeinit(u_dispatcher(p));
            lm = v_participantGetLeaseManager(kp);
            if (lm != NULL) {
                v_leaseManagerNotify(lm, NULL, V_EVENT_TERMINATE);
            }
            v_participantResendManagerQuit(kp);
            if (lm != NULL) {
                os_threadWaitExit(p->threadId, NULL);
                c_free(lm);
            } else {
                OS_REPORT(OS_ERROR, "u_participantDetach", 0,
                          "Access to lease manager failed.");
            }
            os_threadWaitExit(p->threadIdResend, NULL);
            r = u_participantRelease(p);
            p->kernel = NULL;
        } else {
            OS_REPORT(OS_WARNING,"u_participantDetach", 0,
                      "Failed to claim Participant.");
        }
    } else {
        OS_REPORT(OS_ERROR,"u_participantDetach",0,
                  "No participant specified.");
        r = U_RESULT_ILL_PARAM;
    }

    return r;
}

u_result
u_participantDeleteHistoricalData(
    u_participant p,
    const c_char* partitionExpr,
    const c_char* topicExpr)
{
    u_result r;
    v_participant kp;

    if (p != NULL) {
        r = u_participantClaim(p, &kp);
        if ((r == U_RESULT_OK) && (kp != NULL)) {
            if(partitionExpr && topicExpr){
                v_participantDeleteHistoricalData(kp, partitionExpr, topicExpr);
            } else {
                OS_REPORT(OS_ERROR,"u_participantDeleteHistoricalData",0,
                          "Illegal parameter.");
                r = U_RESULT_ILL_PARAM;
            }
            r = u_participantRelease(p);
        } else {
            OS_REPORT(OS_WARNING, "u_participantDeleteHistoricalData", 0,
                      "Failed to claim Participant.");
        }
    } else {
        OS_REPORT(OS_ERROR,"u_participantDetach",0,
                  "No participant specified.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}


