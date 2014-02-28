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
#include "u_participant.h"
#include "u__participant.h"
#include "u__subscriber.h"
#include "u__topic.h"
#include "u__types.h"
#include "u__dispatcher.h"
#include "u__entity.h"
#include "u__user.h"
#include "u__writer.h"
#include "u__cfElement.h"
#include "u__domain.h"
#include "u_user.h"
#include "u_scheduler.h"

#include "v_entity.h"
#include "v_participant.h"
#include "v_leaseManager.h"
#include "v_configuration.h"
#include "v_event.h"
#include "v_statistics.h"

#include "os_report.h"

static void
collect_entities(
    c_voidp object,
    c_voidp arg)
{
    c_iter *entities = (c_iter *)arg;

    u_entityKeep(u_entity(object));
    *entities = c_iterInsert(*entities, object);
}

static void *
leaseManagerMain(
    void *arg)
{
    u_participant _this = u_participant(arg);
    v_participant kp;
    v_leaseManager lm;
    u_result result;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&kp));
    if(result == U_RESULT_OK)
    {
        assert(kp);
        lm = v_participantGetLeaseManager(kp);
        v_leaseManagerMain(lm);
        c_free(lm);
        u_entityRelease(u_entity(_this));
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

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&kp));
    if(result == U_RESULT_OK)
    {
        assert(kp);
        v_participantResendManagerMain(kp);
        u_entityRelease(u_entity(_this));
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
    u_domain domain;
    u_participant p = NULL;
    u_result r;
    v_kernel kk = NULL;
    v_participant kp;
    const c_char *uri_string;

    if (uri) {
        uri_string = uri;
    } else {
        uri_string = "";
    }
    r = u_domainOpen(&domain, uri, timeout);

    if (r == U_RESULT_OK)
    {
        r = u_entityWriteClaim(u_entity(domain),(v_entity*)(&kk));
        if (r == U_RESULT_OK) {
            assert(kk);
            kp = v_participantNew(kk,name,qos, NULL,enable);
            if (kp != NULL) {
                p = u_entityAlloc(NULL,u_participant,kp,TRUE);
                if (p != NULL) {
                    r = u_participantInit(p, domain);
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
            r = u_entityRelease(u_entity(domain));
        } else {
            OS_REPORT(OS_ERROR,"u_participantNew",0,
                      "Claim Kernel failed.");
        }
    } else {
        OS_REPORT_1(OS_ERROR,"u_participantNew",0,
                    "Failure to open the domain, URI=\"%s\" "
                    "The most common cause of this error is that OpenSpliceDDS is not running (when using shared memory mode). "
                    "Please make sure to start OpenSplice before creating a DomainParticipant.",
                    uri_string);
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
#define U_GENERALWATCHDOG_CLASS_FMT    "%s/GeneralWatchdog/Scheduling/Class"
#define U_GENERALWATCHDOG_PRIO_FMT     "%s/GeneralWatchdog/Scheduling/Priority"
#define U_SPLICEDWATCHDOG_CLASS_FMT    "%s/Watchdog/Scheduling/Class"
#define U_SPLICEDWATCHDOG_PRIO_FMT     "%s/Watchdog/Scheduling/Priority"
#define U_WATCHDOG_ATTRKIND     "priority_kind"

static void
participantGetWatchDogAttr(
    u_cfElement root,
    const c_char* element,
    const c_char* name,
    os_threadAttr * attr)
{
    c_char* path;
    c_char* fallbackPath;
    c_char* schedClass;
    c_char* prioKind;
    c_ulong prio;
    c_bool success;
    u_cfData data;
    u_cfAttribute attribute;
    struct v_schedulePolicy qos;
    const char* fallbackElement = "Domain";
    c_bool deprecatedSpliced = FALSE;
    c_bool showDeprecationWarning = FALSE;

    /*
     * General search path for the watchdog parameters:
     * Service/Watchdog/Scheduling/ if a service has explicitly specified individual preferences.
     * Domain/GeneralWatchdog/Scheduling/ if a service has not set an overruling preference.
     *
     * For Spliced the search path is:
     * Domain/Daemon/Watchdog/Scheduling/ if not present,
     * Domain/Watchdog/Scheduling/ this path is now deprecated as it moved to Domain/Daemon but kept alive for backward compatibility
     * Domain/GeneralWatchdog/Scheduling/
     */

    if (root != NULL) {

        if (strcmp(element, "Domain/Daemon")==0) {
            /* old watchdog location configuration for spliced still needed for backward compatibility*/
            deprecatedSpliced = TRUE;
            path = os_malloc(strlen(U_SPLICEDWATCHDOG_CLASS_FMT U_WATCHDOG_TEXT) +
                                     strlen(element) + 1);
        } else {
            path = os_malloc(strlen(U_WATCHDOG_CLASS_FMT U_WATCHDOG_TEXT) +
                                     strlen(element) + strlen(name) + 1);
        }
        if (path != NULL) {
            if (deprecatedSpliced) {
                os_sprintf(path, U_SPLICEDWATCHDOG_CLASS_FMT U_WATCHDOG_TEXT, element);
            } else {
                os_sprintf(path, U_WATCHDOG_CLASS_FMT U_WATCHDOG_TEXT, element, name);
            }
            data = u_configurationResolveParameter(root, path);
            /* look for deprecated place Domain/Watchdog */
            if (data == NULL && deprecatedSpliced) {
                os_free(path);
                path = os_malloc(strlen(U_SPLICEDWATCHDOG_CLASS_FMT U_WATCHDOG_TEXT) +
                                         strlen(fallbackElement) + 1);
                if (path != NULL) {
                    os_sprintf(path, U_SPLICEDWATCHDOG_CLASS_FMT U_WATCHDOG_TEXT, fallbackElement);
                    data = u_configurationResolveParameter(root, path);
                    showDeprecationWarning = TRUE;
                }
            }
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
                        OS_REPORT_1(OS_ERROR, "Watchdog initialization", 0,
                                    "Illegal 'Scheduling/Class' for %s", name);
                        success = FALSE;
                    }
                    if (showDeprecationWarning) {
                        OS_REPORT(OS_WARNING, "Watchdog initialization", 0,
                                  "deprecated path for Domain Watchdog 'Scheduling/Class' please use Domain/Daemon.");
                    }
                    os_free(schedClass);
                } else {
                    OS_REPORT_1(OS_ERROR, "Watchdog initialization", 0,
                    "Illegal 'Scheduling/Class' for %s. Applying default.",
                    name);
                }
                u_cfDataFree(data);
            } else {
                fallbackPath = os_malloc(strlen(U_GENERALWATCHDOG_CLASS_FMT U_WATCHDOG_TEXT) +
                                         strlen(fallbackElement) + 1);
                if (fallbackPath != NULL) {
                    os_sprintf(fallbackPath, U_GENERALWATCHDOG_CLASS_FMT U_WATCHDOG_TEXT, fallbackElement);
                    data = u_configurationResolveParameter(root, fallbackPath);
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
                                OS_REPORT(OS_ERROR, "GeneralWatchdog initialization", 0,
                                            "Illegal 'Scheduling/Class'");
                                success = FALSE;
                            }
                            os_free(schedClass);
                        } else {
                            OS_REPORT(OS_ERROR, "GeneralWatchdog initialization", 0,
                            "Illegal 'Scheduling/Class' Applying default.");
                        }
                        u_cfDataFree(data);
                    } else {
                        success = FALSE;
                    }
                } else {
                    success = FALSE;
                }
                os_free(fallbackPath);
            }
            os_free(path);
        } else {
            success = FALSE;
        }
        showDeprecationWarning = FALSE;
        if (success) {
            if (strcmp(element, "Domain/Daemon")==0) {
                /* old watchdog location configuration for spliced still needed for backward compatibility*/
                deprecatedSpliced = TRUE;
                path = os_malloc(strlen(U_SPLICEDWATCHDOG_PRIO_FMT U_WATCHDOG_TEXT) +
                                         strlen(element) + 1);
            } else {
                path = os_malloc(strlen(U_WATCHDOG_PRIO_FMT U_WATCHDOG_TEXT) +
                                         strlen(element) + strlen(name) + 1);
            }

            if (path != NULL) {
                if (deprecatedSpliced) {
                    os_sprintf(path, U_SPLICEDWATCHDOG_PRIO_FMT U_WATCHDOG_TEXT, element);
                } else {
                    os_sprintf(path, U_WATCHDOG_PRIO_FMT U_WATCHDOG_TEXT, element, name);
                }
                data = u_configurationResolveParameter(root, path);
                /* look for deprecated place Domain/Watchdog */
                if (data == NULL && deprecatedSpliced) {
                    os_free(path);
                    path = os_malloc(strlen(U_SPLICEDWATCHDOG_PRIO_FMT U_WATCHDOG_TEXT) +
                                             strlen(fallbackElement) + 1);
                    if (path != NULL) {
                        os_sprintf(path, U_SPLICEDWATCHDOG_PRIO_FMT U_WATCHDOG_TEXT, fallbackElement);
                        data = u_configurationResolveParameter(root, path);
                        showDeprecationWarning = TRUE;
                    }
                }
                if (data != NULL) {
                    success = u_cfDataULongValue(data, &prio);
                    if (success) {
                        qos.priority = prio;
                        if(showDeprecationWarning) {
                            OS_REPORT(OS_WARNING, "Watchdog initialization", 0,
                                      "deprecated path for Domain Watchdog 'Scheduling/Priority' please use Domain/Daemon.");
                        }
                    } else {
                        OS_REPORT_1(OS_ERROR, "Watchdog initialization", 0,
                        "%s Configuration: value of 'Scheduling/Priority' not "
                        "valid.  Applying default.", name);
                    }
                    u_cfDataFree(data);
                } else {
                    fallbackPath = os_malloc(strlen(U_GENERALWATCHDOG_PRIO_FMT U_WATCHDOG_TEXT) +
                                     strlen(fallbackElement) + 1);
                    if (fallbackPath != NULL) {
                        os_sprintf(fallbackPath, U_GENERALWATCHDOG_PRIO_FMT U_WATCHDOG_TEXT,fallbackElement);
                        data = u_configurationResolveParameter(root, fallbackPath);
                        if (data != NULL) {
                            success = u_cfDataULongValue(data, &prio);
                            if (success) {
                                qos.priority = prio;
                            } else {
                                OS_REPORT(OS_ERROR, "GeneralWatchdog initialization", 0,
                                "Configuration: value of 'Scheduling/Priority' not "
                                "valid.  Applying default.");
                            }
                            u_cfDataFree(data);
                        } else {
                            success = FALSE;
                        }
                        os_free(fallbackPath);
                    } else {
                        success = FALSE;
                    }
                }
                os_free(path);
            } else {
                success = FALSE;
            }
        }
        showDeprecationWarning = FALSE;
        if (success) {
            if (strcmp(element, "Domain/Daemon")==0) {
                /* old watchdog location configuration for spliced still needed for backward compatibility*/
                deprecatedSpliced = TRUE;
                path = os_malloc(strlen(U_SPLICEDWATCHDOG_PRIO_FMT) +
                                         strlen(element) + 1);
            } else {
                path = os_malloc(strlen(U_WATCHDOG_PRIO_FMT) +
                                 strlen(element) + strlen(name) + 1);
            }
            if (path != NULL) {
                if (deprecatedSpliced) {
                     /*no need to realloc path because the alloced size is always bigger than needed here*/
                    os_sprintf(path, U_SPLICEDWATCHDOG_PRIO_FMT, element);
                } else {
                    os_sprintf(path, U_WATCHDOG_PRIO_FMT, element, name);
                }
                attribute = u_configurationResolveAttribute(root, path,
                                                            U_WATCHDOG_ATTRKIND);
                 /*look for deprecated place Domain/Watchdog*/
                if (attribute == NULL && deprecatedSpliced) {
                    os_free(path);
                    path = os_malloc(strlen(U_SPLICEDWATCHDOG_PRIO_FMT) +
                                             strlen(fallbackElement) + 1);
                    if (path != NULL) {
                        os_sprintf(path, U_SPLICEDWATCHDOG_PRIO_FMT, fallbackElement);
                        attribute = u_configurationResolveAttribute(root, path,U_WATCHDOG_ATTRKIND);
                        showDeprecationWarning = TRUE;
                    }
                }
                if (attribute != NULL) {
                    success = u_cfAttributeStringValue(attribute, &prioKind);
                    if (success) {
                        if (strcmp(prioKind, "Relative")==0) {
                            qos.priorityKind = V_SCHED_PRIO_RELATIVE;
                        } else if (strcmp(prioKind, "Absolute")==0) {
                            qos.priorityKind = V_SCHED_PRIO_ABSOLUTE;
                        } else {
                            OS_REPORT_1(OS_ERROR,
                                        "Watchdog initialization", 0,
                                        "Invalid 'priority_kind' in "
                                        "'Scheduling/Priority'. "
                                        "Applying default for %s.", name);
                            success = FALSE;
                        }
                        if(showDeprecationWarning) {
                            OS_REPORT(OS_WARNING, "Watchdog initialization", 0,
                                      "deprecated path for 'priority_kind' in Domain Watchdog 'Scheduling/Priority' please use Domain/Daemon.");
                        }
                    } else {
                        OS_REPORT_1(OS_ERROR, "Watchdog initialization", 0,
                        "Invalid 'priority_kind' in 'Scheduling/Priority'. "
                        "Applying default for %s.", name);
                    }
                } else {
                    fallbackPath = os_malloc(strlen(U_GENERALWATCHDOG_PRIO_FMT) +
                                     strlen(fallbackElement) + 1);
                    if (fallbackPath != NULL) {
                        os_sprintf(fallbackPath, U_GENERALWATCHDOG_PRIO_FMT, fallbackElement);
                        attribute = u_configurationResolveAttribute(root, fallbackPath,
                                                                    U_WATCHDOG_ATTRKIND);
                        if (attribute != NULL) {
                            success = u_cfAttributeStringValue(attribute, &prioKind);
                            if (success) {
                                if (strcmp(prioKind, "Relative")==0) {
                                    qos.priorityKind = V_SCHED_PRIO_RELATIVE;
                                } else if (strcmp(prioKind, "Absolute")==0) {
                                    qos.priorityKind = V_SCHED_PRIO_ABSOLUTE;
                                } else {
                                    OS_REPORT(OS_ERROR,
                                                "GeneralWatchdog initialization", 0,
                                                "Invalid 'priority_kind' in "
                                                "'Scheduling/Priority' "
                                                "Applying default.");
                                    success = FALSE;
                                }
                            } else {
                                OS_REPORT(OS_ERROR, "GeneralWatchdog initialization", 0,
                                "Invalid 'priority_kind' in 'Scheduling/Priority'. "
                                "Applying default");
                            }
                        } else {
                            success = FALSE;
                        }
                    } else {
                        success = FALSE;
                    }
                    os_free(fallbackPath);
                }
            } else {
                success = FALSE;
            }
            os_free(path);
        }
        os_threadAttrInit(attr);

        if (success) {
            u_threadAttrInit (&qos, attr);
        }
    } else {
        OS_REPORT_1(OS_INFO,
                    "Watchdog"
                    " initialization", 0,
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

    r = u_entityWriteClaim(u_entity(_this), (v_entity*)(&kp));
    if(r == U_RESULT_OK)
    {
        assert(kp);
        v_participantConnectNewGroup(kp,NULL);
        r = u_entityRelease(u_entity(_this));
    }
    return r;
}

u_result
u_participantInit (
    u_participant _this,
    u_domain domain)
{
    u_result r;
    v_participant kp;
    os_threadAttr attr;
    os_result osr;
    u_cfElement root;
    c_ulong mask;

    if (_this == NULL || domain == NULL) {
        OS_REPORT_2(OS_ERROR,
                    "u_participantInit", 0,
                    "Invalid argument: _this = 0x%x, domain = 0x%x",
                    _this, domain);
        return U_RESULT_ILL_PARAM;
    }

    _this->domain = domain;
    r = u_entityReadClaim(u_entity(_this), (v_entity*)(&kp));
    if(r == U_RESULT_OK)
    {
        assert(kp);
        _this->topics = NULL;
        _this->publishers = NULL;
        _this->subscribers = NULL;
        _this->builtinSubscriber = NULL;
        _this->builtinTopicCount = 0;
        if (u_entity(_this)->kind == U_SERVICE) {
            root = u_participantGetConfiguration(_this);
            switch(u_service(_this)->serviceKind) {
                case U_SERVICE_DDSI:
                    participantGetWatchDogAttr(root, "DDSI2Service",
                                               v_participantName(kp), &attr);
                    break;
                case U_SERVICE_DDSIE:
                    participantGetWatchDogAttr(root, "DDSI2EService",
                                               v_participantName(kp), &attr);
                    break;
                case U_SERVICE_SNETWORKING:
                    participantGetWatchDogAttr(root, "SNetworkService",
                                               v_participantName(kp), &attr);
                    break;
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
                    participantGetWatchDogAttr(root, "Domain/Daemon",
                                               v_participantName(kp), &attr);
                    break;
                case U_SERVICE_DBMSCONNECT:
                    participantGetWatchDogAttr(root, "DbmsConnectService",
                                               v_participantName(kp), &attr);
                    break;
                case U_SERVICE_RNR:
                    participantGetWatchDogAttr(root, "RnRService",
                                               v_participantName(kp), &attr);
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
        } else if (u_entity(_this)->kind == U_PARTICIPANT) {
            os_threadAttrInit(&attr);
            /* use the values configured for the participant */
            u_threadAttrInit (&kp->qos->watchdogScheduling, &attr);
        } else {
            os_threadAttrInit(&attr);
        }
        r = u_dispatcherInit(u_dispatcher(_this));
        if (r == U_RESULT_OK) {
            r = u_domainAddParticipant(domain,_this);

            osr = os_threadCreate(&_this->threadId, "watchdog", &attr,
                (void *(*)(void *))leaseManagerMain,(void *)_this);

            if(osr != os_resultSuccess){
                OS_REPORT(OS_ERROR, "u_participantInit", 0,
                          "Watchdog thread could not be started.\n");
            }
            /* slave ResendManager attributes from parent process */
            os_threadAttrInit(&attr);

            osr = os_threadCreate(&_this->threadIdResend, "resendManager", &attr,
                            (void *(*)(void *))resendManagerMain,(void *)_this);

            if(osr != os_resultSuccess){
                OS_REPORT(OS_ERROR, "u_participantInit", 0,
                          "Watchdog thread could not be started.\n");
            }
        } else {
            OS_REPORT(OS_ERROR, "u_participantInit", 0,
                      "Dispatcher Initialization failed.");
        }
        u_dispatcherGetEventMask(u_dispatcher(_this), &mask);
        u_dispatcherInsertListener(u_dispatcher(_this),
                                   u_participantNewGroupListener,
                                   NULL);
        mask |= V_EVENT_NEW_GROUP;
        mask |= V_EVENT_CONNECT_WRITER;
        u_dispatcherSetEventMask(u_dispatcher(_this), mask);
        r = u_entityRelease(u_entity(_this));
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
    u_participant _this)
{
    u_result result = U_RESULT_OK;
    c_bool destroy;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        destroy = u_entityDereference(u_entity(_this));
        if (destroy) {
            if (u_entityOwner(u_entity(_this))) {
                result = u_participantDeinit(_this);
            } else {
                /* This user entity is a proxy, meaning that it is not fully
                 * initialized, therefore only the entity part of the object
                 * can be deinitialized.
                 * It would be better to either introduce a separate proxy
                 * entity for clarity or fully initialize entities and make
                 * them robust against missing information.
                 */
                result = u_entityDeinit(u_entity(_this));
            }
            if (result == U_RESULT_OK) {
                u_entityDealloc(u_entity(_this));
            } else {
                OS_REPORT_2(OS_WARNING,
                            "u_participantFree",0,
                            "Operation u_participantDeinit failed: "
                            "participant = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
            "u_participantFree",0,
            "Operation u_entityLock failed: "
            "participant = 0x%x, result = %s.",
            _this, u_resultImage(result));
    }
    return result;
}

/* Precondition: Participant _this must be locked. */
u_result
u_participantDeinit (
    u_participant _this)
{
    u_result r;
    v_participant kp;
    v_leaseManager lm;

    if (_this != NULL) {
        r = u_domainRemoveParticipant(_this->domain,_this);
        if (r == U_RESULT_OK) {
            r = u_entityReadClaim(u_entity(_this), (v_entity*)(&kp));
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
                    OS_REPORT(OS_ERROR, "u_participantDeinit", 0,
                              "Access to lease manager failed.");
                }
                os_threadWaitExit(_this->threadIdResend, NULL);

                /* First Release before Deinit,
                 * otherwise Release will have no effect.
                 * results in a process that won't terminate.
                 */
                u_entityRelease(u_entity(_this));
                r = u_dispatcherDeinit(u_dispatcher(_this));
            } else {
                OS_REPORT(OS_WARNING, "u_participantDeinit", 0,
                          "Failed to claim Participant.");
            }
            u_domainFree(_this->domain);
            /* The domain may or may not be actually deinitialised, since there
             * can be more participants connected throught this domain. At least
             * remove acces through this participant to the domain. */
            _this->domain = NULL;
            assert(c_iterLength(_this->publishers) == 0);
            c_iterFree(_this->publishers);
            assert(c_iterLength(_this->subscribers) == 0);
            c_iterFree(_this->subscribers);
            assert(_this->builtinTopicCount == 0);
            assert(c_iterLength(_this->topics) == 0);
            c_iterFree(_this->topics);
         }
    } else {
        OS_REPORT(OS_ERROR, "u_participantDeinit", 0,
                  "Participant is not specified.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_participantDeleteContainedEntities(
    u_participant _this)
{
    u_result r;
    u_entity entity;
    c_iter list;

    if (_this != NULL) {

        r = u_entityLock(u_entity(_this));
        if (r == U_RESULT_OK) {
            list = _this->publishers;
            _this->publishers = NULL;
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT_2(OS_ERROR,
                        "u_participantDeleteContainedEntities",0,
                        "Lock Participant 0x%x failed: result = %s.",
                        _this, u_resultImage(r));
            list = NULL;
        }
        entity = c_iterTakeFirst(list);
        while (entity) {
            r = u_publisherDeleteContainedEntities(u_publisher(entity));
            r = u_publisherFree(u_publisher(entity));
            u_entityDereference(u_entity(_this));
            entity = c_iterTakeFirst(list);
        }
        c_iterFree(list);

        r = u_entityLock(u_entity(_this));
        if (r == U_RESULT_OK) {
            list = _this->subscribers;
            _this->subscribers = NULL;
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT_2(OS_ERROR,
                        "u_participantDeleteContainedEntities",0,
                        "Lock Participant 0x%x failed: result = %s.",
                        _this, u_resultImage(r));
        }
        entity = c_iterTakeFirst(list);
        while (entity) {
            if (u_subscriber(entity) == _this->builtinSubscriber) {
                /* The builtin subscription must remain so insert
                 * it back into the list.
                 */
                _this->subscribers = c_iterInsert(_this->subscribers,entity);
            } else {
                r = u_subscriberDeleteContainedEntities(u_subscriber(entity));
                r = u_subscriberFree(u_subscriber(entity));
                u_entityDereference(u_entity(_this));
            }
            entity = c_iterTakeFirst(list);
        }
        c_iterFree(list);
        r = u_entityLock(u_entity(_this));
        if (r == U_RESULT_OK) {
            list = _this->topics;
            _this->topics = NULL;
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT_2(OS_ERROR,
                        "u_participantDeleteContainedEntities",0,
                        "Lock Participant 0x%x failed: result = %s.",
                        _this, u_resultImage(r));
        }
        entity = c_iterTakeFirst(list);
        while (entity) {
            if (u_topicIsBuiltin(u_topic(entity))) {
                assert(_this->builtinTopicCount > 0);
                _this->builtinTopicCount--;
            } else {
                /* The ref count is only increased for user defined topics
                 * and not for implicitly created builtin topics so therefore
                 * only degrade the refcount for user defined topics.
                 */
                u_entityDereference(u_entity(_this));
            }
            r = u_topicFree(u_topic(entity));
            entity = c_iterTakeFirst(list);
        }
        c_iterFree(list);
    } else {
        OS_REPORT(OS_ERROR, "u_participantDeleteContainedEntities", 0,
                  "Participant is not specified.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_participantDetach(
    u_participant _this)
{
    u_result r;
    u_entity entity;
    c_iter list;
    v_participant kp;
    v_leaseManager lm;

    if (_this != NULL) {
        r = u_entityReadClaim(u_entity(_this), (v_entity*)(&kp));
        if(r == U_RESULT_OK)
        {
            assert(kp);
            list = _this->publishers;
            _this->publishers = NULL;

            entity = c_iterTakeFirst(list);
            while (entity) {
                r = u_publisherDeleteContainedEntities(u_publisher(entity));
                if (r == U_RESULT_OK) {
                    r = u_publisherFree(u_publisher(entity));
                    if (r == U_RESULT_OK) {
                        u_entityDereference(u_entity(_this));
                    } else {
                        OS_REPORT_2(OS_ERROR,
                                    "u_participantDetach", 0,
                                    "Delete Publisher failed: "
                                    "Participant 0x%x, Publisher 0x%x.",
                                    _this, entity);
                    }
                } else {
                    OS_REPORT_2(OS_ERROR,
                                "u_participantDetach", 0,
                                "DeleteContainedEntities on Publisher failed: "
                                "Participant 0x%x, Publisher 0x%x.",
                                _this, entity);
                }
                entity = c_iterTakeFirst(list);
            }
            c_iterFree(list);

            list = _this->subscribers;
            _this->subscribers = NULL;
            entity = c_iterTakeFirst(list);
            while (entity) {
                r = u_subscriberDeleteContainedEntities(u_subscriber(entity));
                if (r == U_RESULT_OK) {
                    r = u_subscriberFree(u_subscriber(entity));
                    if (r == U_RESULT_OK) {
                        u_entityDereference(u_entity(_this));
                    } else {
                        OS_REPORT_2(OS_ERROR,
                                    "u_participantDetach", 0,
                                    "Delete Subscriber failed: "
                                    "Participant 0x%x, Subscriber 0x%x.",
                                    _this, entity);
                    }
                } else {
                    OS_REPORT_2(OS_ERROR,
                                "u_participantDetach", 0,
                                "DeleteContainedEntities on Subscriber failed: "
                                "Participant 0x%x, Subscriber 0x%x.",
                                _this, entity);
                }
                entity = c_iterTakeFirst(list);
            }
            c_iterFree(list);

            list = _this->topics;
            _this->topics = NULL;
            entity = c_iterTakeFirst(list);
            while (entity) {
                if (u_topicIsBuiltin(u_topic(entity))) {
                    assert(_this->builtinTopicCount > 0);
                    _this->builtinTopicCount--;
                } else {
                    /* Own ref count is only increased for user defined topics
                     * and not for implicitly created builtin topics so therefore
                     * only degrade the refcount for user defined topics.
                     */
                    u_entityDereference(u_entity(_this));
                }
                r = u_topicFree(u_topic(entity));
                if (r != U_RESULT_OK) {
                    OS_REPORT_3(OS_ERROR,
                                "u_participantDetach", 0,
                                "Delete Topic failed: result = %s, "
                                "Participant 0x%x, Topic 0x%x.",
                                u_resultImage(r), _this, entity);
                }
                entity = c_iterTakeFirst(list);
            }
            c_iterFree(list);

            lm = v_participantGetLeaseManager(kp);
            if (lm != NULL) {
                v_leaseManagerNotify(lm, NULL, V_EVENT_TERMINATE);
            }
            v_participantResendManagerQuit(kp);
            if (lm != NULL) {
                os_threadWaitExit(_this->threadId, NULL);
                c_free(lm);
            } else {
                OS_REPORT(OS_ERROR, "u_participantDetach", 0,
                          "Access to lease manager failed.");
            }
            os_threadWaitExit(_this->threadIdResend, NULL);
            r = u_entityRelease(u_entity(_this));
            os_mutexLock(&u_entity(_this)->mutex); /* u_dispatcherDeinit must be called Locked on entity */
            u_dispatcherDeinit(u_dispatcher(_this));
            _this->domain = NULL;
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

u_domain
u_participantDomain(
    u_participant p)
{
    if (p == NULL) {
        return NULL;
    }
    return p->domain;
}

u_result
u_participantDisable(
    u_participant p)
{
    u_result r = U_RESULT_ILL_PARAM;

    if (p != NULL) {
      p->domain = NULL;
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
        r = u_entityReadClaim(u_entity(_this->domain),(v_entity*)(&k));
        if ((r == U_RESULT_OK) && (k != NULL)) {
            config= v_getConfiguration(k);
            if(config!= NULL){
                 cfg = u_cfElementNew(_this, v_configurationGetRoot(config));
            }
            u_entityRelease(u_entity(_this->domain));
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
    c_iter list = NULL;
    c_iter topics = NULL;
    os_time delay;
    os_time const tryPeriod = {0, 100 * 1e6}; /* 0.1s */
    os_time endTime;
    os_int retry = 1;
    c_bool error = FALSE;

    if(!c_timeIsInfinite(timeout)){
        delay.tv_sec = timeout.seconds;
        delay.tv_nsec = timeout.nanoseconds;
        endTime = os_timeAdd(os_timeGet(), delay);
    }
    if (p== NULL) {
        OS_REPORT(OS_ERROR,
                  "User Participant",0,
                  "u_participantFindTopic: No participant specified.");
    } else {
        do {
            r = u_entityLock(u_entity(p));
            if(r == U_RESULT_OK) {
                r = u_entityReadClaim(u_entity(p), (v_entity*)(&kp));
                if(r == U_RESULT_OK)
                {
                    assert(kp);
                    /** todo Make real implementatio when SI912 is solved...... */
                    list = v_resolveTopics(v_objectKernel(kp),name);
                    r = u_entityRelease(u_entity(p));
                    if(r != U_RESULT_OK) {
                        OS_REPORT(OS_WARNING,
                                  "u_participantFindTopic",0,
                                  "Failed to release the Participant.");
                        retry = 0;
                        error = TRUE;
                    }

                    r = u_entityUnlock(u_entity(p));
                    if(r != U_RESULT_OK) {
                        OS_REPORT(OS_WARNING,
                                  "u_participantFindTopic",0,
                                  "Failed to unlock the Participant.");
                        retry = 0;
                        error = TRUE;
                    }

                    if(c_iterLength(list) == 0) {
                        os_nanoSleep(tryPeriod);
                        retry = c_timeIsInfinite(timeout) || (os_timeCompare(os_timeGet(), endTime) == OS_LESS);
                    } else {
                        retry = 0;
                    }
                } else {
                    OS_REPORT(OS_WARNING,
                              "u_participantFindTopic",0,
                              "Failed to claim Participant.");
                    retry = 0;
                    error = TRUE;
                    u_entityUnlock(u_entity(p));
                }
            }  else {
                OS_REPORT(OS_WARNING,
                          "u_participantFindTopic",0,
                          "Failed to lock the Participant.");
                retry = 0;
                error = TRUE;
            }
        } while (retry);
        if (list != NULL && c_iterLength(list) != 0) {
            kt = c_iterTakeFirst(list);
            while (kt != NULL) {
                if (!error) {
                    t = u_topic(u_entityNew(v_entity(kt), p,TRUE));
                    if (t) {
                        topics = c_iterInsert(topics,t);
                    } else {
                        OS_REPORT_1(OS_WARNING,
                                    "u_participantFindTopic",0,
                                    "Found Kernel Topic '%s' without user layer entity.",
                                    name);
                    }
                }
                c_free(kt);
                kt = c_iterTakeFirst(list);
            }
        }
        c_iterFree(list);
        list = NULL;
    }
    return topics;
}

u_subscriber
u_participantGetBuiltinSubscriber(
    u_participant p)
{
    u_subscriber s = NULL;
    C_STRUCT(v_subscriberQos) sQos;

    if (p == NULL) {
        OS_REPORT(OS_ERROR,
                  "u_participantGetBuiltinSubscriber",0,
                  "No participant specified.");
    } else if (p->builtinSubscriber) {
        s = p->builtinSubscriber;
    } else {
        ((v_qos)&sQos)->kind = V_SUBSCRIBER_QOS;
        sQos.groupData.value              = NULL;
        sQos.groupData.size               = 0;
        sQos.presentation.access_scope    = V_PRESENTATION_TOPIC;
        sQos.presentation.coherent_access = FALSE;
        sQos.presentation.ordered_access  = FALSE;
        sQos.partition                    = "__BUILT-IN PARTITION__";
        sQos.share.enable                 = FALSE;
        sQos.share.name                   = NULL;
        sQos.entityFactory.autoenable_created_entities = TRUE;

        s = u_subscriberNew(p,"BuiltinSubscriber",&sQos,TRUE);
        if (s == NULL) {
            OS_REPORT(OS_WARNING,
                      "u_participantGetBuiltinSubscriber",0,
                      "Failed to create user layer builtin Subscriber.");
        }
        p->builtinSubscriber = s;
    }
    return s;
}

/* Temporary operation needed for gapi_domainParticipant delete contained entities */
c_bool
u_participantIsBuiltinSubscriber(
    u_participant _this,
    u_subscriber s)
{
    c_bool result = FALSE;

    if (_this && s) {
        if (_this->builtinSubscriber == s) {
            result = TRUE;
        }
    }
    return result;
}

u_result
u_participantAssertLiveliness(
    u_participant p)
{
    u_result r;
    v_participant kp;

    if (p != NULL) {
        r = u_entityReadClaim(u_entity(p), (v_entity*)(&kp));
        if(r == U_RESULT_OK)
        {
            assert(kp);
            v_participantAssertLiveliness(kp);
            r = u_entityRelease(u_entity(p));
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
u_participantDeleteHistoricalData(
    u_participant p,
    const c_char* partitionExpr,
    const c_char* topicExpr)
{
    u_result r;
    v_participant kp;

    if (p != NULL) {
        r = u_entityReadClaim(u_entity(p), (v_entity*)(&kp));
        if(r == U_RESULT_OK)
        {
            assert(kp);
            if(partitionExpr && topicExpr){
                v_participantDeleteHistoricalData(kp, partitionExpr, topicExpr);
            } else {
                OS_REPORT(OS_ERROR,"u_participantDeleteHistoricalData",0,
                          "Illegal parameter.");
                r = U_RESULT_ILL_PARAM;
            }
            r = u_entityRelease(u_entity(p));
        } else {
            OS_REPORT(OS_WARNING, "u_participantDeleteHistoricalData", 0,
                      "Failed to claim Participant.");
        }
    } else {
        OS_REPORT(OS_ERROR,"u_participantDeleteHistoricalData",0,
                  "No participant specified.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_participantAddPublisher(
    u_participant _this,
    u_publisher publisher)
{
    u_result result = U_RESULT_OK;

    if (publisher) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            _this->publishers = c_iterInsert(_this->publishers, publisher);
            u_entityKeep(u_entity(_this));
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_1(OS_WARNING,
                    "u_participantAddPublisher",0,
                    "Given Publisher (0x%x) is invalid.",
                    publisher);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_participantRemovePublisher(
    u_participant _this,
    u_publisher publisher)
{
    u_publisher found;
    u_result result;

    if (publisher) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            found = c_iterTake(_this->publishers,publisher);
            if (found) {
                u_entityDereference(u_entity(_this));
            }
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING,
                      "u_participantRemovePublisher",0,
                      "Failed to lock Participant.");
        }
    } else {
        OS_REPORT_1(OS_WARNING,
                    "u_participantRemovePublisher",0,
                    "Given Publisher (0x%x) is invalid.",
                    publisher);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

c_bool
u_participantContainsPublisher(
    u_participant _this,
    u_publisher publisher)
{
    c_bool found = FALSE;
    u_result result;

    if (publisher) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            found = c_iterContains(_this->publishers,publisher);
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING,
                      "u_participantContainsPublisher",0,
                      "Failed to lock Participant.");
        }
    } else {
        OS_REPORT_1(OS_WARNING,
                    "u_participantContainsPublisher",0,
                    "Given Publisher (0x%x) is invalid.",
                    publisher);
    }
    return found;
}

c_long
u_participantPublisherCount(
    u_participant _this)
{
    c_long length = -1;
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        length = c_iterLength(_this->publishers);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT(OS_WARNING,
                  "u_participantPublisherCount",0,
                  "Failed to lock Participant.");
    }
    return length;
}

u_result
u_participantWalkPublishers(
    u_participant _this,
    u_publisherAction action,
    c_voidp actionArg)
{
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        c_iterWalkUntil(_this->publishers,
                        (c_iterAction)action,
                        actionArg);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT(OS_WARNING,
                  "u_participantWalkPublishers",0,
                  "Failed to lock Participant.");
    }
    return result;
}

c_iter
u_participantLookupPublishers(
    u_participant _this)
{
    c_iter publishers = NULL;
    u_result result;

    publishers = NULL;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        c_iterWalk(_this->publishers, collect_entities, &publishers);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT_1(OS_WARNING,
                  "u_participantLookupPublishers",0,
                  "Failed to lock Participant: result = %s.",
                  u_resultImage(result));
    }
    return publishers;
}

u_result
u_participantAddSubscriber(
    u_participant _this,
    u_subscriber subscriber)
{
    u_result result = U_RESULT_OK;

    if (subscriber) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            _this->subscribers = c_iterInsert(_this->subscribers, subscriber);
            u_entityKeep(u_entity(_this));
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING,
                      "u_participantAddSubscriber",0,
                      "Failed to lock Participant.");
        }
    } else {
        OS_REPORT_1(OS_WARNING,
                    "u_participantAddSubscriber",0,
                    "Given Subscriber (0x%x) is invalid.",
                    subscriber);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_participantRemoveSubscriber(
    u_participant _this,
    u_subscriber subscriber)
{
    u_subscriber found;
    u_result result = U_RESULT_OK;

    if (subscriber) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            found = c_iterTake(_this->subscribers,subscriber);
            if (found) {
                u_entityDereference(u_entity(_this));
                if (found == _this->builtinSubscriber) {
                    _this->builtinSubscriber = NULL;
                }
            }
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING,
                      "u_participantRemoveSubscriber",0,
                      "Failed to lock Participant.");
        }
    } else {
        OS_REPORT_1(OS_WARNING,
                    "u_participantRemoveSubscriber",0,
                    "Given Subscriber (0x%x) is invalid.",
                    subscriber);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

c_bool
u_participantContainsSubscriber(
    u_participant _this,
    u_subscriber subscriber)
{
    c_bool found = FALSE;
    u_result result;

    if (subscriber) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            found = c_iterContains(_this->subscribers,subscriber);
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING,
                      "u_participantContainsSubscriber",0,
                      "Failed to lock Participant.");
        }
    } else {
        OS_REPORT_1(OS_WARNING,
                    "u_participantContainsSubscriber",0,
                    "Given Subscriber (0x%x) is invalid.",
                    subscriber);
    }
    return found;
}

c_long
u_participantSubscriberCount(
    u_participant _this)
{
    c_long length = -1;
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        length = c_iterLength(_this->subscribers);
        if (_this->builtinSubscriber) {
            assert(length > 0);
            length--;
        }
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT(OS_WARNING,
                  "u_participantSubscriberCount",0,
                  "Failed to lock Participant.");
    }
    return length;
}

u_result
u_participantWalkSubscribers(
    u_participant _this,
    u_subscriberAction action,
    c_voidp actionArg)
{
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        c_iterWalkUntil(_this->subscribers,
                        (c_iterAction)action,
                        actionArg);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT(OS_WARNING,
                  "u_participantWalkSubscribers",0,
                  "Failed to lock Participant.");
    }
    return result;
}

c_iter
u_participantLookupSubscribers(
    u_participant _this)
{
    c_iter subscribers = NULL;
    u_result result;

    subscribers = NULL;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        c_iterWalk(_this->subscribers, collect_entities, &subscribers);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT_1(OS_WARNING,
                  "u_participantLookupSubscribers",0,
                  "Failed to lock Participant: result = %s.",
                  u_resultImage(result));
    }
    return subscribers;
}

u_result
u_participantAddTopic(
    u_participant _this,
    u_topic topic)
{
    u_result result;

    if (topic) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            _this->topics = c_iterInsert(_this->topics, topic);
            if (u_topicIsBuiltin(topic)) {
                _this->builtinTopicCount++;
            } else {
                /* Only increase reference count for user created topics
                 * and not for implicitly created builtin topics otherwise
                 * the delete participant will fail because of the ref count
                 * which never will decrease to 0.
                 */
                u_entityKeep(u_entity(_this));
            }
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING,
                      "u_participantAddTopic",0,
                      "Failed to lock Participant.");
        }
    } else {
        OS_REPORT_1(OS_WARNING,
                    "u_participantAddTopic",0,
                    "Given Topic (0x%x) is invalid.",
                    topic);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_participantRemoveTopic(
    u_participant _this,
    u_topic topic)
{
    u_topic found;
    u_result result;

    if (topic) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            found = c_iterTake(_this->topics,topic);
            if (found) {
                if (u_topicIsBuiltin(topic)) {
                    assert(_this->builtinTopicCount > 0);
                    _this->builtinTopicCount--;
                } else {
                    /* The ref count is only increased for user defined topics
                     * and not for implicitly created builtin topics so therefore
                     * only degrade the refcount for user defined topics.
                     */
                    u_entityDereference(u_entity(_this));
                }
            }
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING,
                      "u_participantRemoveTopic",0,
                      "Failed to lock Participant.");
            result = U_RESULT_ILL_PARAM;
        }
    } else {
        OS_REPORT_1(OS_WARNING,
                    "u_participantRemoveTopic",0,
                    "Given Topic (0x%x) is invalid.",
                    topic);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

c_bool
u_participantContainsTopic(
    u_participant _this,
    u_topic topic)
{
    c_bool found = FALSE;
    u_result result;

    if (topic) {
        result = u_entityLock(u_entity(_this));
        if (result == U_RESULT_OK) {
            found = c_iterContains(_this->topics,topic);
            u_entityUnlock(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING,
                      "u_participantContainsTopic",0,
                      "Failed to lock Participant.");
        }
    } else {
        OS_REPORT_1(OS_WARNING,
                    "u_participantContainsTopic",0,
                    "Given Topic (0x%x) is invalid.",
                    topic);
    }
    return found;
}

struct collect_topics_arg {
    const c_char *topic_name;
    c_iter topics;
};

static void
collect_topics(
    c_voidp object,
    c_voidp arg)
{
    struct collect_topics_arg *a = (struct collect_topics_arg *)arg;
    u_topic t = (u_topic)object;
    c_char *name;

    if (a->topic_name == NULL) {
        a->topics = c_iterInsert(a->topics, t);
        u_entityKeep(u_entity(t));
    } else {
        name = u__topicName(t);
        if (strcmp(name, a->topic_name) == 0)
        {
            a->topics = c_iterInsert(a->topics, t);
            u_entityKeep(u_entity(t));
        }
    }
}

c_iter
u_participantLookupTopics(
    u_participant _this,
    const c_char *topic_name)
{
    struct collect_topics_arg arg;
    u_result result;

    /* topic_name == NULL is treated as wildcard '*' */
    arg.topic_name = topic_name;
    arg.topics = NULL;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        c_iterWalk(_this->topics, collect_topics, &arg);
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT(OS_WARNING,
                  "u_participantLookupTopics",0,
                  "Failed to lock Participant.");
    }
    return arg.topics;
}

c_long
u_participantTopicCount(
    u_participant _this)
{
    c_long length = -1;
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        length = c_iterLength(_this->topics);
        length = length - _this->builtinTopicCount;
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT(OS_WARNING,
                  "u_participantRemoveTopic",0,
                  "Failed to lock Participant.");
    }
    return length;
}

u_result
u_participantWalkTopics(
    u_participant _this,
    u_topicAction action,
    c_voidp actionArg)
{
    u_result result;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        c_iterWalkUntil(_this->topics, (c_iterAction)action, actionArg);
        u_entityUnlock(u_entity(_this));
        result = TRUE;
    } else {
        OS_REPORT(OS_WARNING,
                  "u_participantWalkTopics",0,
                  "Failed to lock Participant.");
    }
    return result;
}

/* What is needed is atomic lookup and create:
 * topic =  u_participantCreateTopic(p,name, typeName, keyList, qos);
 * The Operation u_participantCreateTopic will use u_topicNew to
 * construct the Topic but will perform lookup and insert of the
 * Topic so this can be removed from u_topicNew.
 */
u_topic
u_participantCreateTopic(
    u_participant _this,
    const c_char *name,
    const c_char *typeName,
    const c_char *keyList,
    v_topicQos qos)
{
    u_result result;
    u_topic topic = NULL;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        if (topic == NULL) {
            topic = u_topicNew(_this, name, typeName, keyList, qos);
            /* if topic == NULL then the c_iterInsert will be a noop. */
            _this->topics = c_iterInsert(_this->topics, topic);
        }
        u_entityUnlock(u_entity(_this));
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_participantCreateTopic",0,
                    "Failed to lock Participant 0x%x. "
                    "Aborting creation of Topic '%s'.",
                    _this, name);
    }
    return topic;
}

u_result
u_participantFederationSpecificPartitionName (
    u_participant _this,
    c_char *buf,
    os_size_t bufsize)
{
    u_result result;
    if ((result = u_entityLock (u_entity(_this))) == U_RESULT_OK) {
        result = u_domainFederationSpecificPartitionName (_this->domain, buf, bufsize);
        u_entityUnlock (u_entity (_this));
    }
    return result;
}
