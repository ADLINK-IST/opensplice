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


#include "cms_configuration.h"
#include "u_participant.h"
#include "u_participantQos.h"
#include "u_cfElement.h"
#include "u_cfNode.h"
#include "u_cfData.h"
#include "os_heap.h"
#include "os_report.h"
#include <string.h>
#include <stdio.h>
#include "u_scheduler.h"

#define NS_IN_S (1e9F)

static void cms_configurationInit(cms_configuration config, cms_service cms, const c_char* name);

static u_cfData cms_configurationResolveParameter(u_cfElement e,const c_char* xpathExpr);

static void cms_configurationInitMaxClients(cms_configuration config, u_cfElement e);
static void cms_configurationInitMaxThreadsPerClient(cms_configuration config, u_cfElement e);
static void cms_configurationInitBacklog(cms_configuration config, u_cfElement e);
static void cms_configurationInitVerbosity(cms_configuration config, u_cfElement e);
static void cms_configurationInitPort(cms_configuration config, u_cfElement e);
static void cms_configurationInitLeasePeriod(cms_configuration config, u_cfElement e);
static void cms_configurationInitLeaseRenewalPeriod(cms_configuration config, u_cfElement e);
static void cms_configurationInitClientLeasePeriod(cms_configuration config, u_cfElement e);

static void cms_configurationInitReportingLevel(cms_configuration config, u_cfElement e);
static void cms_configurationInitReportingEvents(cms_configuration config, u_cfElement e);
static void cms_configurationInitReportingPeriodic(cms_configuration config, u_cfElement e);
static void cms_configurationInitReportingOneShot(cms_configuration config, u_cfElement e);

static void cms_configurationInitClientScheduling(cms_configuration config, u_cfElement e);
static void cms_configurationInitGarbageCollectorScheduling(cms_configuration config, u_cfElement e);
static void cms_configurationInitLeaseThreadScheduling(cms_configuration config, u_cfElement e);

static c_char* ReportLevelMap[] = { "None",
                                    "Basic",
                                    "Low_frequent",
                                    "Medium_frequent",
                                    "High_frequent",
                                    "Full"
                                  };

c_bool
cms_configurationNew(
    const c_char* name,
    cms_service cms)
{
    cms_configuration config;
    c_bool success;

    success = FALSE;

    if((cms != NULL) && (cms->uservice != NULL)){
        config = cms_configuration(os_malloc(C_SIZEOF(cms_configuration)));
        cms_object(config)->kind = CMS_CONFIGURATION;
        cms_configurationInit(config, cms, name);
        cms->configuration = config;

        success = TRUE;
    }
    return success;
}

void
cms_configurationFree(
    cms_configuration config)
{
    if(config != NULL){
        if (config->name != NULL) {
            os_free(config->name);
            config->name = NULL;
        }
        if(config->watchdogQos){
            u_participantQosFree(config->watchdogQos);
            config->watchdogQos = NULL;
        }
        os_free(config);
    }
}

c_char*
cms_configurationFormat(
    cms_configuration config)
{
    c_char buf[1024];

    os_sprintf(buf,
        "\n%s Configuration:\n"
        "-------------------------------\n"
        "- Maximum #clients        : %d\n"
        "- Maximum #threads/client : %d\n"
        "- Backlog                 : %d\n"
        "- Verbosity               : %d\n"
        "- Port                    : %d\n"
        "- LeasePeriod             : %d.%d\n"
        "- LeaseRenewalPeriod      : %d.%d\n"
        "- ClientLeasePeriod       : %d.%d\n"
        "-------------------------------\n",
        config->name,
        config->maxClients,
        config->maxThreadsPerClient,
        config->backlog,
        config->verbosity,
        config->port,
        config->leasePeriod.seconds,
        config->leasePeriod.nanoseconds,
        config->leaseRenewalPeriod.seconds,
        config->leaseRenewalPeriod.nanoseconds,
        config->clientLeasePeriod.seconds,
        config->clientLeasePeriod.nanoseconds);

    return (c_char*)(os_strdup(buf));
}


static void
cms_configurationSetDefaults(cms_configuration config)
{
    config->maxClients = 2;
    config->maxThreadsPerClient = 2;
    config->backlog = 5;
    config->verbosity = 1;
    config->port = 8000;
    config->leasePeriod.seconds=10;
    config->leasePeriod.nanoseconds=0;
    config->leaseRenewalPeriod.seconds=2; /* = defaultExpiryTime * defaultUpdateFactor == 10.0 * 0.2 */
    config->leaseRenewalPeriod.nanoseconds=0;
    config->clientLeasePeriod.seconds=15;
    config->clientLeasePeriod.nanoseconds=0;

    os_threadAttrInit(&config->clientScheduling);
    os_threadAttrInit(&config->leaseScheduling);
    os_threadAttrInit(&config->garbageScheduling);
    config->reportLevel    = 1;
    config->reportEvents   = FALSE;
    config->reportPeriodic = FALSE;
    config->reportOneShot  = FALSE;

    config->watchdogQos = u_participantQosNew(NULL);
}

static void
cms_configurationInit(
    cms_configuration config,
    cms_service cms,
    const c_char* name )
{
    u_cfElement root;
    u_cfElement e;
    c_iter elements = NULL;
    c_char* path;

    config->name = os_strdup(name);
    cms_configurationSetDefaults(config);

    root = u_participantGetConfiguration(u_participant(cms->uservice));

    if(root != NULL){
        /* first retrieve lease configuration from domain settings */
        cms_configurationInitLeasePeriod(config, root);
        cms_configurationInitLeaseRenewalPeriod(config, root);

        /* then retrieve configuration from TunerService with supplied name */
        path = os_malloc( strlen("TunerService[@name='']") + strlen(name) + 1);
        if(path != NULL){
            path[0] = '\0';
            os_sprintf(path, "TunerService[@name='%s']", name);
            elements = u_cfElementXPath(root, path);
            os_free(path);
        }

        e = u_cfElement(c_iterTakeFirst(elements));
        if(e != NULL){
            cms_configurationInitMaxClients(config, e);
            cms_configurationInitMaxThreadsPerClient(config, e);
            cms_configurationInitBacklog(config, e);
            cms_configurationInitVerbosity(config, e);
            cms_configurationInitPort(config, e);
            cms_configurationInitClientLeasePeriod(config, e);

            cms_configurationInitReportingLevel(config, e);
            cms_configurationInitReportingEvents(config, e);
            cms_configurationInitReportingPeriodic(config, e);
            cms_configurationInitReportingOneShot(config, e);

            cms_configurationInitClientScheduling(config, e);
            cms_configurationInitGarbageCollectorScheduling(config, e);
            cms_configurationInitLeaseThreadScheduling(config, e);

            u_cfElementFree(e);
            e = u_cfElement(c_iterTakeFirst(elements));

            while(e != NULL){
                u_cfElementFree(e);
                e = u_cfElement(c_iterTakeFirst(elements));
            }
        }
        c_iterFree(elements);
        u_cfElementFree(root);
    }
}

static u_cfAttribute
cms_configurationResolveAttribute(
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
cms_configurationResolveParameter(
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

static void
cms_configurationInitMaxClients(
    cms_configuration config,
    u_cfElement e)
{
    u_cfData data;
    c_ulong max;
    c_bool success;


    if(e != NULL){
        data = cms_configurationResolveParameter(e, "Client/MaxClients/#text");

        if(data != NULL){
            success = u_cfDataULongValue(data, &max);

            if(success){
                if(max > 0 ){
                    config->maxClients = max;
                } else {
                    OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                     "%s Configuration: 'MaxClients' <= 0 not valid.  Applying default.", config->name);
                }
            } else {
                OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                 "%s Configuration: 'MaxClients' not valid. Applying default.", config->name);
            }
            u_cfDataFree(data);
        }
    }
}

static void
cms_configurationInitMaxThreadsPerClient(
    cms_configuration config,
    u_cfElement e)
{
    u_cfData data;
    c_ulong max;
    c_bool success;


    if(e != NULL){
        data = cms_configurationResolveParameter(e, "Client/MaxThreadsPerClient/#text");

        if(data != NULL){
            success = u_cfDataULongValue(data, &max);

            if(success){
                if(max > 0 ){
                    config->maxThreadsPerClient = max;
                } else {
                    OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                     "%s Configuration: 'MaxThreadsPerClient' <= 0 not valid.  Applying default.", config->name);
                }
            } else {
                OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                 "%s Configuration: 'MaxThreadsPerClient' not valid. Applying default.", config->name);
            }
            u_cfDataFree(data);
        }
    }
}

static void
cms_configurationInitBacklog(
    cms_configuration config,
    u_cfElement e)
{
    u_cfData data;
    c_ulong max;
    c_bool success;


    if(e != NULL){
        data = cms_configurationResolveParameter(e, "Server/Backlog/#text");

        if(data != NULL){
            success = u_cfDataULongValue(data, &max);

            if(success){
                if(max > 0 ){
                    config->backlog = max;
                } else {
                    OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                     "%s Configuration: 'Backlog' < 0 not valid. Applying default.", config->name);
                }
            } else {
                OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                 "%s Configuration: 'Backlog' not valid. Applying Default", config->name);
            }
            u_cfDataFree(data);
        }
    }
}

static void
cms_configurationInitVerbosity(
    cms_configuration config,
    u_cfElement e)
{
    u_cfData data;
    c_ulong max;
    c_bool success;


    if(e != NULL){
        data = cms_configurationResolveParameter(e, "Server/Verbosity/#text");

        if(data != NULL){
            success = u_cfDataULongValue(data, &max);

            if(success){
                config->verbosity = max;
            } else {
                OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                 "%s Configuration: 'Verbosity' not valid. Applying default.", config->name);
            }
            u_cfDataFree(data);
        }
    }
}

static void
cms_configurationInitPort(
    cms_configuration config,
    u_cfElement e)
{
    u_cfData data;
    c_ulong max;
    c_bool success;
    c_char* value;


    if(e != NULL){
        data = cms_configurationResolveParameter(e, "Server/PortNr/#text");

        if(data != NULL){
            success = u_cfDataStringValue(data, &value);
            if (success) {
                if (strcmp(value, "Auto")==0) {
                    config->port = 0; /* port not configured let the SOAP service choose its own port */
                } else {
                    success = u_cfDataULongValue(data, &max);

                    if(success){
                        if(max > 0 ){
                            config->port = max;
                        } else {
                            OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                             "%s Configuration: 'Port' <= 0 not valid.  Applying default.", config->name);
                        }
                    } else {
                        OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                         "%s Configuration: 'Port' <= 0 not valid. Applying default", config->name);
                    }
                }
                os_free(value);
            }
            u_cfDataFree(data);

        }
    }
}

static void
cms_configurationInitLeasePeriod(
    cms_configuration config,
    u_cfElement e)
{
    u_cfData data;
    c_float value;
    c_bool success;


    if(e != NULL){
        data = cms_configurationResolveParameter(e, "Domain/Lease/ExpiryTime/#text");

        if(data != NULL){
            success = u_cfDataFloatValue(data, &value);

            if(success){
                if(value >= 0.2F){
                    config->leasePeriod.seconds     = (unsigned int) value;
                    config->leasePeriod.nanoseconds = (unsigned int) ((value - (float)((unsigned int) value)) * NS_IN_S);
                } else {
                    OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                     "%s Configuration: 'LeasePeriod' < 0.2 seconds not allowed.  Applying default.",
                     config->name);
                }
            } else {
                OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                 "%s Configuration: 'LeasePeriod' not valid. Applying default.", config->name);
            }
            u_cfDataFree(data);
        }
    }
}

static void
cms_configurationInitLeaseRenewalPeriod(
    cms_configuration config,
    u_cfElement e)
{
    u_cfAttribute data;
    c_float value;
    c_bool success;
    c_float leasePeriod;


    if(e != NULL){
        data = cms_configurationResolveAttribute(e, "Domain/Lease/ExpiryTime", "update_factor");

        if(data != NULL){
            success = u_cfAttributeFloatValue(data, &value);

            if(success){
                if((value >= 0.01F) && (value <= 1.0F)){
                    leasePeriod = config->leasePeriod.seconds + (
                                  config->leasePeriod.nanoseconds / NS_IN_S);
                    value = leasePeriod * value;
                    config->leaseRenewalPeriod.seconds     = (unsigned int) value;
                    config->leaseRenewalPeriod.nanoseconds = (unsigned int) ((value - (float)((unsigned int) value)) * NS_IN_S);
                } else if(value < 1.0F){
                    OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                        "%s Configuration: 'update_factor' < 0.01 not allowed. Applying default.",
                         config->name);
                } else {
                    OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                                            "%s Configuration: 'update_factor' > 1 not allowed. Applying default.",
                                             config->name);
                }
            } else {
                OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                 "%s Configuration: 'update_factor' not valid. Applying default.", config->name);
            }
            u_cfAttributeFree(data);
        }
    }
}

static void
cms_configurationInitClientLeasePeriod(
    cms_configuration config,
    u_cfElement e)
{
    u_cfData data;
    c_float value;
    c_bool success;

    if(e != NULL){
        data = cms_configurationResolveParameter(e, "Client/LeasePeriod/#text");

        if(data != NULL){
            success = u_cfDataFloatValue(data, &value);

            if(success){
               if((value) < 10.0F){
                     OS_REPORT_1(OS_INFO, CMS_CONTEXT, 0,
                     "%s Configuration: 'ClientLeasePeriod' < 10 seconds is not valid. Applying default.",
                     config->name);
                } else {
                    config->clientLeasePeriod.seconds     = (unsigned int) value;
                    config->clientLeasePeriod.nanoseconds = (unsigned int) ((value - (float)((unsigned int) value)) * NS_IN_S);
                }
            } else {
                OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                   "%s Configuration: 'ClientLeasePeriod' not valid. Applying default",
                   config->name);
            }
            u_cfDataFree(data);
        }
    }
}

static void cms_configurationInitReportingLevel(
    cms_configuration config,
    u_cfElement e)
{
    u_cfData data;
    c_char* value;
    c_bool success;
    unsigned int i;

    if(e != NULL){
        data = cms_configurationResolveParameter(e, "Reporting/Level/#text");

        if(data != NULL){
            success = u_cfDataStringValue(data, &value);
            if(success){
                for (i=0; i<(sizeof(ReportLevelMap)/sizeof(c_char*)); i++) {
                    if (strcmp(value, ReportLevelMap[i]) == 0) {
                        config->reportLevel = i;
                        break;
                    }
                }
                if ( i>=(sizeof(ReportLevelMap)/sizeof(c_char*)) ) {
                    OS_REPORT_2(OS_ERROR, CMS_CONTEXT, 0,
                       "%s Configuration: 'Reporting/Level' %s not valid. Applying default",
                       config->name, value);
                }
            } else {
                OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                   "%s Configuration: 'Reporting/Level' not valid. Applying default",
                   config->name);
            }
            u_cfDataFree(data);
        }
    }
}

static void cms_configurationInitReportingEvents(
    cms_configuration config,
    u_cfElement e)
{
    u_cfData data;
    c_bool value;
    c_bool success;

    if(e != NULL){
        data = cms_configurationResolveParameter(e, "Reporting/Events/#text");

        if(data != NULL){
            success = u_cfDataBoolValue(data, &value);
            if(success){
                config->reportEvents = value;
            } else {
                OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                   "%s Configuration: 'Reporting/Events' not valid. Applying default",
                   config->name);
            }
            u_cfDataFree(data);
        }
    }
}

static void cms_configurationInitReportingPeriodic(
    cms_configuration config,
    u_cfElement e)
{
    u_cfData data;
    c_bool value;
    c_bool success;

    if(e != NULL){
        data = cms_configurationResolveParameter(e, "Reporting/Periodic/#text");

        if(data != NULL){
            success = u_cfDataBoolValue(data, &value);
            if(success){
                config->reportPeriodic = value;
            } else {
                OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                   "%s Configuration: 'Reporting/Periodic' not valid. Applying default",
                   config->name);
            }
            u_cfDataFree(data);
        }
    }
}

static void cms_configurationInitReportingOneShot(
    cms_configuration config,
    u_cfElement e)
{
    u_cfData data;
    c_bool value;
    c_bool success;

    if(e != NULL){
        data = cms_configurationResolveParameter(e, "Reporting/OneShot/#text");

        if(data != NULL){
            success = u_cfDataBoolValue(data, &value);
            if(success){
                config->reportPeriodic = value;
            } else {
                OS_REPORT_1(OS_ERROR, CMS_CONTEXT, 0,
                   "%s Configuration: 'Reporting/Periodic' not valid. Applying default",
                   config->name);
            }
            u_cfDataFree(data);
        }
    }
}

#define CLASS_PATH "/Class/#text"
#define PRIO_PATH "/Priority/#text"
#define REL_PATH "/Priority"
#define REL_ATTR "priority_kind"

static void cms_configurationInitScheduling(
    cms_configuration config,
    os_threadAttr * attr,
    u_cfElement e,
    c_char * path)
{
    c_long schedPrio;
    c_char* schedClass;
    c_char* prioKind;
    c_char* tmp;
    u_cfData data;
    u_cfAttribute attribute;
    c_bool success;

    struct v_schedulePolicy schedule;

    /* TODO: Verify correctness of defaults */
    schedule.kind = V_SCHED_DEFAULT;
    schedule.priorityKind = V_SCHED_PRIO_RELATIVE;
    schedule.priority = 0;

    if (e != NULL) {
        tmp = os_malloc(strlen(path) + strlen(CLASS_PATH) +1 );
        os_sprintf(tmp, "%s%s", path, CLASS_PATH);

        data = cms_configurationResolveParameter(e, tmp);
        os_free(tmp);

        if (data != NULL) {
            success = u_cfDataStringValue(data, &schedClass);
            if (success) {
                if (strcmp(schedClass, "Realtime")==0) {
                    schedule.kind = V_SCHED_REALTIME;
                } else if (strcmp(schedClass, "Timeshare")==0) {
                    schedule.kind = V_SCHED_TIMESHARING;
                } else if (strcmp(schedClass, "Default")==0) {
                    schedule.kind = V_SCHED_DEFAULT;
                } else {
                    OS_REPORT_2(OS_ERROR, CMS_CONTEXT, 0,
                       "%s Configuration: '%s/Class' not valid. Applying default",
                       config->name, path);
                }
            } else {
                OS_REPORT_2(OS_ERROR, CMS_CONTEXT, 0,
                   "%s Configuration: '%s/Class' not valid. Applying default",
                   config->name, path);
            }
            u_cfDataFree(data);
        }

        tmp = os_malloc(strlen(path) + strlen(PRIO_PATH) +1 );
        os_sprintf(tmp, "%s%s", path, PRIO_PATH);

        data = cms_configurationResolveParameter(e, tmp);
        os_free(tmp);

        if (data != NULL) {
            success = u_cfDataLongValue(data, &schedPrio);
            if (success) {
                schedule.priority = schedPrio;
            } else {
                OS_REPORT_2(OS_ERROR, CMS_CONTEXT, 0,
                   "%s Configuration: '%s/Priority' not valid. Applying default",
                   config->name, path);
            }
        }

        tmp = os_malloc(strlen(path) + strlen(REL_PATH) + 1);
        os_sprintf(tmp, "%s%s", path, REL_PATH);
        attribute = cms_configurationResolveAttribute(e, tmp, REL_ATTR);
        os_free(tmp);

        if (data != NULL) {
            success = u_cfAttributeStringValue(attribute, &prioKind);
            if (success) {
                if (strcmp(prioKind, "Relative")==0) {
                    schedule.priorityKind = V_SCHED_PRIO_RELATIVE;
                } else if (strcmp(prioKind, "Absolute")==0) {
                    schedule.priorityKind = V_SCHED_PRIO_ABSOLUTE;
                } else {
                    OS_REPORT_2(OS_ERROR, CMS_CONTEXT, 0,
                       "%s Configuration: '%s/Class' not valid. Applying default",
                       config->name, path);
                }
            } else {
                OS_REPORT_2(OS_ERROR, CMS_CONTEXT, 0,
                   "%s Configuration: '%s/Class' not valid. Applying default",
                   config->name, path);
            }
            u_cfDataFree(data);
        }
    }
    u_threadAttrInit(&schedule, attr);
}

static void cms_configurationInitClientScheduling(
    cms_configuration config,
    u_cfElement e)
{
   cms_configurationInitScheduling(config, &(config->clientScheduling), e, "Client/Scheduling");
}

static void cms_configurationInitGarbageCollectorScheduling(cms_configuration config, u_cfElement e)
{
   cms_configurationInitScheduling(config, &(config->garbageScheduling), e, "GarbageCollector/Scheduling");
}

static void cms_configurationInitLeaseThreadScheduling(cms_configuration config, u_cfElement e)
{
   cms_configurationInitScheduling(config, &(config->leaseScheduling), e, "LeaseManagement/Scheduling");
}
