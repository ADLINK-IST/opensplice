/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/* TODO: Check consistency between namespaces. Overlap between them is not a
 * valid situation.
 */
#include "d__configuration.h"
#include "d__durability.h"
#include "d__nameSpace.h"
#include "d__policy.h"
#include "d__filter.h"
#include "d__table.h"
#include "d__misc.h"
#include "d__element.h"
#include "d_object.h"
#include "ut_xmlparser.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_version.h"
#include "os_gitrev.h"
#include "os_stdlib.h"
#include "v_builtin.h"
#include "u_domain.h"
#include "q_expr.h"


typedef struct mustAlignHelper {
    u_cfElement cfg;
    char *serviceName;
    c_bool mustAlign;
    c_bool hasNetwork;
} mustAlignHelper;


static c_bool
isServiceEnabled(
    u_cfElement serviceElement)
{
    c_bool enabled, success;

    assert(serviceElement);

    success = u_cfElementAttributeBoolValue(serviceElement, "enabled", &enabled);

    if (!success || enabled) {
        return TRUE;
    }
    return FALSE;
}

static void
isNetworkService (
    void* o,
    c_iterActionArg userData)
{
    u_cfElement element = (u_cfElement)o;
    mustAlignHelper *helper = (mustAlignHelper *)userData;
    c_char *networkServiceName;
    c_bool success;

    assert(helper->serviceName);

    if ((!helper->mustAlign) && element) {
        if(isServiceEnabled(element)){
            /* Get the "name"-attribute of the network service */
            success = u_cfElementAttributeStringValue(element, "name", &networkServiceName);
            if (success) {
                if (strcmp(helper->serviceName, networkServiceName) == 0) {
                    /* The enabled service is a <NetworkService>.
                     * In this case durability must generate
                     * the builtin topics.
                     */
                    helper->mustAlign = TRUE;
                    helper->hasNetwork = TRUE;
                }
                os_free(networkServiceName);
            }
        }
    }
}

static void
cfElementIterFree (
    c_iter iter)
{
    u_cfElement elem;
    while ((elem = c_iterTakeFirst(iter)) != NULL) {
        u_cfElementFree(elem);
    }
    c_iterFree(iter);
}

static void
isDDSIServiceWithGenerateBuiltinTopics (
    void* o,
    c_iterActionArg userData)
{
    u_cfElement element = (u_cfElement)o;
    mustAlignHelper *helper = (mustAlignHelper *)userData;
    c_char *ddsiServiceName;
    c_iter iter;
    u_cfData data;
    c_bool success, b;

    assert(helper->serviceName);

    if ((!helper->mustAlign) && element) {
        if(isServiceEnabled(element)){
            /* Get the "name"-attribute of the ddsi service */
            success = u_cfElementAttributeStringValue(element, "name", &ddsiServiceName);
            if (success) {
                if (strcmp(helper->serviceName, ddsiServiceName) == 0) {
                    iter = u_cfElementXPath(element, "Discovery/GenerateBuiltinTopics/#text");
                    if (iter) {
                        if (c_iterLength(iter) == 1) {
                            data = u_cfData(c_iterTakeFirst(iter));
                            if (data) {
                                success = u_cfDataBoolValue(data, &b);
                                if (success == TRUE) {
                                    /* If <GenerateBuiltinTopics> is set to
                                     * "false" for this DDSI service then
                                     * durability must align the built-in
                                     * topics.
                                     */
                                    helper->mustAlign = !b;
                                }
                                u_cfDataFree(data);
                            }
                        } else if (c_iterLength(iter) > 1) {
                            /* Multiple <GenerateBuiltinTopics> elements are
                             * present for this DDSI service. This should not
                             * occur and should have resulted in an error
                             * when parsing the configuration. To be on the
                             * safe side we treat this case as if durability
                             * must align the built-in topics.
                             */
                            OS_REPORT(OS_ERROR, D_CONTEXT, 0,
                            "%d <GenerateBuiltinTopics> configurations found for serviceName '%s', ignoring these settings.",
                            c_iterLength(iter), helper->serviceName);
                            helper->mustAlign = TRUE;
                        } else {
                            /* No <GenerateBuiltinTopics>-element was
                             * present.  Assume the default; DDSI
                             * generates built-in topics. This means
                             * durability must not align built-in
                             * topics.
                             */
                            helper->mustAlign = FALSE;
                        }
                        cfElementIterFree(iter);
                    }
                    helper->hasNetwork = TRUE;
                }
            }
            os_free(ddsiServiceName);
        }
    }
}


static void
durabilityMustAlign(
    void* o,
    c_iterActionArg userData)
{
    u_cfElement element = (u_cfElement)o;
    mustAlignHelper *helper = (mustAlignHelper *)userData;
    char *serviceName;
    c_iter iter;
    c_bool success;
    c_bool enabled = TRUE;

    assert(helper->cfg);

    if ((!helper->mustAlign) && element) {
        /* Get the "name"-attribute of the service */
        success = u_cfElementAttributeStringValue(element, "name", &serviceName);
        if (success) {
            helper->serviceName = serviceName;
            /* Get the (optional) "enabled"-attribute of the service.
             * If not present then "enabled=true" is assumed.
             * Only if enabled="false" the service must not be checked.
             */
            success = u_cfElementAttributeBoolValue(element, "enabled", &enabled);
            if (!success || enabled) {
                iter = u_cfElementXPath(helper->cfg, "NetworkService");
                if (iter) {
                    c_iterWalk(iter, isNetworkService, helper);
                    cfElementIterFree(iter);
                }
                if (!helper->mustAlign) {
                    iter = u_cfElementXPath(helper->cfg, "SNetworkService");
                    if (iter) {
                        c_iterWalk(iter, isNetworkService, helper);
                        cfElementIterFree(iter);
                    }
                }
                if (!helper->mustAlign) {
                    iter = u_cfElementXPath(helper->cfg, "DDSI2Service");
                    if (iter) {
                        c_iterWalk(iter, isDDSIServiceWithGenerateBuiltinTopics, helper);
                        cfElementIterFree(iter);
                    }
                }
                if (!helper->mustAlign) {
                    iter = u_cfElementXPath(helper->cfg, "DDSI2EService");
                    if (iter) {
                        c_iterWalk(iter, isDDSIServiceWithGenerateBuiltinTopics, helper);
                        cfElementIterFree(iter);
                    }
                }
            }
            os_free(serviceName);
        }
    }
}


/** \brief Determine if the durability service must align the builtin topics.
 */
static c_bool
d_configurationDurabilityMustAlignBuiltin(
    u_cfElement cfg)
{
    c_iter iter;
    mustAlignHelper helper;

    helper.cfg = cfg;
    helper.serviceName = NULL;
    helper.mustAlign = FALSE;
    helper.hasNetwork = FALSE;

    if (cfg) {
        iter = u_cfElementXPath(cfg, "Domain/Service");
        if (iter) {
            c_iterWalk(iter, durabilityMustAlign, &helper);
            cfElementIterFree(iter);

            if((helper.mustAlign == FALSE) && (helper.hasNetwork == FALSE)){
                helper.mustAlign = TRUE;
            }
        }
    }
    return helper.mustAlign;
}


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

    OS_UNUSED_ARG(domainId);
    /* Allocate configuration object */
    config = d_configuration(os_malloc(C_SIZEOF(d_configuration)));
    memset(config, 0, C_SIZEOF(d_configuration));

    /* Call super-init */
    d_objectInit(d_object(config), D_CONFIGURATION,
                 (d_objectDeinitFunc)d_configurationDeinit);
    /* Initialize the configuration */
    cfg = u_participantGetConfiguration(u_participant(d_durabilityGetService(service)));
    if (cfg != NULL) {
        iter = u_cfElementXPath(cfg, "Domain");
        if (c_iterLength(iter) > 1) {
            OS_REPORT(OS_WARNING, D_CONTEXT, 0,
            "%d Domain configurations found.",
            c_iterLength(iter));
        } else if (c_iterLength(iter) == 0) {
            OS_REPORT(OS_WARNING, D_CONTEXT, 0,
            "No Domain configuration found. Applying defaults...");
        }

        domainElement = NULL;
        element = u_cfElement(c_iterTakeFirst(iter));
        while (element) {
            if (domainElement) {
               u_cfElementFree(domainElement);
            }
            domainElement = element;
            element = u_cfElement(c_iterTakeFirst(iter));
        }
        c_iterFree(iter);

        iter  = u_cfElementXPath(cfg, "DurabilityService");
        if (c_iterLength(iter) > 1) {
            OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                "%d DurabilityService configurations found for serviceName '%s'.",
                c_iterLength(iter), serviceName);
        } else if(c_iterLength(iter) == 0) {
            OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                "No DurabilityService configurations found for serviceName '%s'.",
                serviceName);
        }

        found = NULL;
        element = u_cfElement(c_iterTakeFirst(iter));
        while (element) {
            success = u_cfElementAttributeStringValue(element, "name", &attrValue);
            if (success == TRUE) {
                if (strcmp(serviceName, attrValue) == 0) {
                    if (found) {
                        u_cfElementFree(found);
                    }
                    found = element;
                    element = NULL;
                }
                os_free(attrValue);
            }
            if (element) {
                u_cfElementFree(element);
            }
            element = u_cfElement(c_iterTakeFirst(iter));
        }

        if (found) {
            if (d_configurationInit(config, cfg, service, domainElement, found) != 0) {
                d_configurationFree(config);
                config = NULL;
            }
            u_cfElementFree(found);
        } else {
            OS_REPORT(OS_FATAL, D_CONTEXT, 0,
                "No DurabilityService configurations found for serviceName '%s'",
                serviceName);
            d_configurationFree(config);
            config = NULL;
        }
        if (domainElement) {
            u_cfElementFree(domainElement);
        }
        c_iterFree(iter);
        u_cfElementFree(cfg);
    }

    if (config) {
        d_configurationReport(config, service);
    }
    return config;
}

void
d_configurationDeinit(
    d_configuration config)
{
    d_nameSpace ns;
    d_policy policy;
    d_filter filter;
    c_char* name;
    struct durablePolicy *dp;

    assert(d_configurationIsValid(config));

    if (config->persistentStoreDirectory) {
        os_free(config->persistentStoreDirectory);
        config->persistentStoreDirectory = NULL;
    }
    if (config->persistentKVStoreStorageType) {
        os_free(config->persistentKVStoreStorageType);
        config->persistentKVStoreStorageType = NULL;
    }
    if (config->persistentKVStoreStorageParameters) {
        os_free(config->persistentKVStoreStorageParameters);
        config->persistentKVStoreStorageParameters = NULL;
    }
    if (config->filters) {
        filter = d_filter(c_iterTakeFirst(config->filters));
        while (filter) {
            d_filterFree(filter);
            filter = d_filter(c_iterTakeFirst(config->filters));
        }
        c_iterFree(config->filters);
        config->filters = NULL;
    }
    if (config->policies) {
        policy = d_policy(c_iterTakeFirst(config->policies));
        while (policy) {
            d_policyFree(policy);
            policy = d_policy(c_iterTakeFirst(config->policies));
        }
        c_iterFree(config->policies);
        config->policies = NULL;
    }
    if (config->nameSpaces) {
        ns = d_nameSpace(c_iterTakeFirst(config->nameSpaces));
        while (ns) {
            d_nameSpaceFree(ns);
            ns = d_nameSpace(c_iterTakeFirst(config->nameSpaces));
        }
        c_iterFree(config->nameSpaces);
        config->nameSpaces = NULL;
    }
    if (config->networkServiceNames) {
        d_tableFree(config->networkServiceNames);
        config->networkServiceNames = NULL;
    }
    if (config->services) {
        name = (c_char*)(c_iterTakeFirst(config->services));
        while(name){
            os_free(name);
            name = (c_char*)(c_iterTakeFirst(config->services));
        }
        c_iterFree(config->services);
        config->services = NULL;
    }
    if (config->durablePolicies) {
        dp = (struct durablePolicy *)(c_iterTakeFirst(config->durablePolicies));
        while(dp){
            os_free(dp->obtain);
            os_free(dp);
            dp = (struct durablePolicy *)(c_iterTakeFirst(config->durablePolicies));
        }
        c_iterFree(config->durablePolicies);
        config->durablePolicies = NULL;
    }
    if (config->publisherName) {
        os_free(config->publisherName);
        config->publisherName = NULL;
    }
    if (config->subscriberName) {
        os_free(config->subscriberName);
        config->subscriberName = NULL;
    }
    if (config->partitionName) {
        os_free(config->partitionName);
        config->partitionName = NULL;
    }
    if (config->clientDurabilityPartitionName) {
        os_free(config->clientDurabilityPartitionName);
        config->clientDurabilityPartitionName = NULL;
    }
    if (config->clientDurabilityPublisherName) {
        os_free(config->clientDurabilityPublisherName);
        config->clientDurabilityPublisherName = NULL;
    }
    if (config->clientDurabilitySubscriberName) {
        os_free(config->clientDurabilitySubscriberName);
        config->clientDurabilitySubscriberName = NULL;
    }
    if (config->tracingOutputFileName) {
        if( (strcmp(config->tracingOutputFileName, "stdout") != 0) &&
            (strcmp(config->tracingOutputFileName, "stderr") != 0)) {
            if (config->tracingOutputFile){
                fclose(config->tracingOutputFile);
                config->tracingOutputFile = NULL;
            }
        }
        os_free(config->tracingOutputFileName);
        config->tracingOutputFileName = NULL;
    }
    if (config->role) {
        os_free (config->role);
    }
    /* Call super-deinit */
    d_objectDeinit(d_object(config));
}

void
d_configurationFree(
    d_configuration config)
{
    assert(d_configurationIsValid(config));

    d_objectFree(d_object(config));
}

void
d_configurationSetBuiltinTopicsEnabled(
    d_configuration config,
    c_bool enabled);


static void
d_configurationParseEnvVars(
    d_configuration config)
{
    char *p;

    /* Capabilities are enabled by default, unless they have
     * been disabled using an environment variable. The only
     * way to disable capabilities is to set the environment
     * variable OSPL_DURABILITY_CAPABILITY_SUPPORT to 0
     */
    if ((p = os_getenv ("OSPL_DURABILITY_CAPABILITY_SUPPORT")) != NULL && atoi (p) == 0) {
        config->capabilitySupport = FALSE;
    }

    /* Ability to hash data sets is enabled by default, unless
     * it is disabled using an environment variable. The only
     * way to disable the hash ability is to set the environment
     * variable OSPL_DURABILITY_CAPABILITY_HASH_DATA_SETS to 0
     */
    if ((p = os_getenv ("OSPL_DURABILITY_CAPABILITY_GROUP_HASH")) != NULL && atoi (p) == 0) {
        config->capabilityGroupHash = FALSE;
    }

    /* Ability to align End Of Transactions (EOT) */
    if ((p = os_getenv ("OSPL_DURABILITY_CAPABILITY_EOT_SUPPORT")) != NULL && atoi (p) == 0) {
        config->capabilityEOTSupport = FALSE;
    }

    /* Ability to specify master selection algorithm */
    if ((p = os_getenv ("OSPL_DURABILITY_CAPABILITY_MASTER_SELECTION")) != NULL) {
        config->capabilityMasterSelection = (c_ulong)atoi(p);
    }

    /* Ability to control majority voting threshold */
    if ((p = os_getenv ("OSPL_DURABILITY_MAJORITY_VOTING_THRESHOLD")) != NULL) {
        d_configurationSetMajorityVotingThreshold(config,(c_ulong)atoi(p));
    }

    /* Ability to test for rollover of merge states */
    if ((p = os_getenv ("OSPL_DURABILITY_INITIAL_MERGE_STATE_VALUE")) != NULL) {
        config->initialMergeStateValue = (c_ulong)atoi(p);
    }

    /* Set thread liveliness report period */
    if ((p = os_getenv ("OSPL_DURABILITY_THREAD_LIVELINESS_REPORT_PERIOD")) != NULL) {
        d_configurationSetDuration(&(config->threadLivelinessReportPeriod), (c_float)atof(p));
    }

    /* Trace mask settings */
    if ((p = os_getenv ("OSPL_DURABILITY_TRACE_ALL")) != NULL && atoi (p) != 0) {
        config->traceMask |= D_TRACE_ALL;
    } else if ((p = os_getenv ("OSPL_DURABILITY_TRACE_NONE")) != NULL && atoi (p) != 0) {
        config->traceMask |= D_TRACE_NONE;
    } else {
        /* Trace mask setting for priority queue */
        if ((p = os_getenv ("OSPL_DURABILITY_TRACE_PRIO_QUEUE")) != NULL && atoi (p) != 0) {
            config->traceMask |= D_TRACE_PRIO_QUEUE;
        }
        /* Trace mask setting for group instances and messages */
        if ((p = os_getenv ("OSPL_DURABILITY_TRACE_GROUP")) != NULL && atoi (p) != 0) {
            config->traceMask |= D_TRACE_GROUP;
        }
        /* Trace mask setting for client durability */
        if ((p = os_getenv ("OSPL_DURABILITY_TRACE_CLIENT_DURABILITY")) != NULL && atoi (p) != 0) {
            config->traceMask |= D_TRACE_CLIENT_DURABILITY;
        }
        /* Trace mask setting for client durability */
        if ((p = os_getenv ("OSPL_DURABILITY_TRACE_GROUP_HASH")) != NULL && atoi (p) != 0) {
            config->traceMask |= D_TRACE_GROUP_HASH;
        }
        /* Trace mask setting for master selection */
        if ((p = os_getenv ("OSPL_DURABILITY_TRACE_MASTER_SELECTION")) != NULL && atoi (p) != 0) {
            config->traceMask |= D_TRACE_MASTER_SELECTION;
        }
        /* Trace mask setting for master selection */
        if ((p = os_getenv ("OSPL_DURABILITY_TRACE_COMBINE_REQUESTS")) != NULL && atoi (p) != 0) {
            config->traceMask |= D_TRACE_COMBINE_REQUESTS;
        }
        if ((p = os_getenv ("OSPL_DURABILITY_TRACE_CHAINS")) != NULL && atoi (p) != 0) {
            config->traceMask |= D_TRACE_CHAINS;
        }

    }
    /* Start the durability service with the specified sequence numbers. */
    if ((p = os_getenv ("OSPL_DURABILITY_SEQNUM_INITIAL_VALUE")) != NULL) {
        config->seqnum_initval = (os_uint32)strtoul(p, NULL, 10);
    }
    return;
}

c_bool
d_configurationCheckFilterInNameSpace(d_configuration config, d_nameSpace nameSpace) {
    c_iterIter iter1, iter2;
    c_bool found = FALSE;
    d_filter filter = NULL;
    d_element element;

    iter1 = c_iterIterGet(config->filters);
    while((!found) && ((filter = d_filter(c_iterNext(&iter1))) != NULL)) {
        iter2 = c_iterIterGet(filter->elements);
        while ((!found) && ((element = d_element(c_iterNext(&iter2))) != NULL)) {
            found = (d_nameSpaceIsAligner(nameSpace) && (d_nameSpaceGetMasterPriority(nameSpace) > D_MINIMUM_MASTER_PRIORITY) && (d_configurationInNameSpace(nameSpace, element->partition, element->topic, TRUE))) ;
            if (found) {
                OS_REPORT(OS_ERROR, D_CONTEXT, 0,
                                   "Unable to be a master of namespace %s as the filter \"%s\" for partition \"%s\" topic \"%s\" is defined which will cause to filter out some data."
                                   "This is not allowed for a master", nameSpace->name, filter->sqlExpression, element->partition, element->topic);
            }
        }
    }
    return found;
}

static int
d_configurationSanityCheck(
    d_configuration config)
{
    c_iterIter iter, iter2;
    d_tableIter tableIter;
    int result = 0;
    struct durablePolicy *dp;
    d_nameSpace nameSpace;
    d_element element;

    assert(d_configurationIsValid(config));

    /* Perform all kinds of sanity checks.
     * Failure to meet these sanity checks will result in durability not starting up.
     */

    /* Check that there is no overlap between the NameSpaces and DurabilityPolicies */
    if (config->durablePolicies) {
        iter = c_iterIterGet(config->durablePolicies);
        while ((!result) && ((dp = (struct durablePolicy *)c_iterNext(&iter)) != NULL)) {
            iter2 = c_iterIterGet(config->nameSpaces);
            while ((!result) && ((nameSpace = d_nameSpace(c_iterNext(&iter2))) != NULL)) {
                element = d_element(d_tableIterFirst(nameSpace->elements, &tableIter));
                while ((!result) && (element != NULL)) {
                    char *str;
                    os_size_t len = strlen(element->partition) + strlen(element->topic) + 2;

                    str = os_malloc(len);
                    snprintf(str, len, "%s.%s", element->partition, element->topic);
                    result = (d_patternMatch(str, dp->obtain) || d_patternMatch(dp->obtain, str));
                     if (result) {
                        OS_REPORT(OS_ERROR, D_CONTEXT, 0,
                                   "Ambiguous namespace and durable policy definitions, the durability service is not started. "
                                   "The expressions '%s' in //OpenSplice/DurabilityService/NameSpaces/ and "
                                   "'%s' in //OpenSplice/Domain/DurablePolicies/ are not disjunct.", str, dp->obtain);
                         result = -1;
                    }
                    os_free(str);

                    element = d_element(d_tableIterNext(&tableIter));
                }
            }
        }
    }

    /* Verify that the merge policy for builtin is IGNORE or MERGE when the durability
     * service is responsible for the alignment of builtin topics.
     */
    if ((config->mustAlignBuiltinTopics) && (result == 0) && (config->nameSpaces)) {
        c_bool found = FALSE;
        iter = c_iterIterGet(config->nameSpaces);
        while ((!found) && ((nameSpace = d_nameSpace(c_iterNext(&iter))) != NULL)) {
            /* Look for the nameSpace containing the DCPSTopic */
            found = d_configurationInNameSpace(nameSpace, V_BUILTIN_PARTITION, V_TOPICINFO_NAME, TRUE);
            if (found) {
                d_mergePolicy mergePolicy = d_nameSpaceGetMergePolicy(nameSpace, config->role);
                if ((mergePolicy != D_MERGE_IGNORE) && (mergePolicy != D_MERGE_MERGE)) {
                    OS_REPORT(OS_ERROR, D_CONTEXT, 0,
                               "The merge policy for namespace '%s' that is responsible for the builtin topics must be IGNORE or MERGE. "
                               "Check //OpenSplice/DurabilityService/NameSpaces/Policy in the deployment manual.", nameSpace->name);
                    result = -1;
                }
            }
        }
    }
    /* Verify that there are no filters for namespaces we're master for */
    if ((config->filters) && (result == 0 ) && (config->nameSpaces)) {
        c_bool found = FALSE;

        iter = c_iterIterGet(config->nameSpaces);
        while ((!found) && ((nameSpace = d_nameSpace(c_iterNext(&iter))) != NULL)) {
            found = d_configurationCheckFilterInNameSpace(config, nameSpace);
            if (found) {
                OS_REPORT(OS_ERROR, D_CONTEXT, 0, "Termination durability");
                result = -1;
            }
        }
    }

    return result;
}


static c_bool
d_configurationHasTag(
    d_configuration config,
    u_cfElement element,
    const c_char* tag)
{
    c_iter iter;
    c_bool result = FALSE;
    u_cfElement e;

    OS_UNUSED_ARG(config);

    iter = u_cfElementXPath(element, tag);
    if (iter) {
        result = (c_iterLength(iter) > 0);
        /* Clean up the elements */
        e = u_cfElement(c_iterTakeFirst(iter));
        while(e) {
            u_cfElementFree(e);
            e = u_cfElement(c_iterTakeFirst(iter));
        }
        c_iterFree(iter);
    }
    return result;
}


int
d_configurationInit(
    d_configuration config,
    u_cfElement cfg,
    d_durability durability,
    u_cfElement domainElement,
    u_cfElement element)
{
    d_policy policy;
    c_ulong i;
    int result = 0;
    d_nameSpace ns = NULL;
    c_iterIter iter;

    assert(config);

    /* First apply all defaults. */
    d_printTimedEvent(durability, D_LEVEL_FINER, "Initializing configuration...\n");

    config->persistentStoreDirectory           = NULL;
    config->persistentStoreMode                = D_STORE_TYPE_XML;
    config->persistentKVStoreStorageType       = NULL;
    config->persistentKVStoreStorageParameters = NULL;
    config->persistentKVStoreCompressionEnabled = FALSE;
    config->persistentKVStoreCompressionAlgorithm = D_COMPRESSION_NONE;
    config->partitionName               = NULL;
    config->publisherName               = NULL;
    config->subscriberName              = NULL;
    config->clientDurabilityPartitionName  = NULL;
    config->clientDurabilityPublisherName  = NULL;
    config->clientDurabilitySubscriberName = NULL;
    config->tracingOutputFile           = NULL;
    config->tracingOutputFileName       = NULL;
    config->tracingSynchronous          = FALSE;
    config->networkServiceNames         = d_tableNew(strcmp, os_free);
    config->services                    = c_iterNew(NULL);
    config->tracingVerbosityLevel       = D_LEVEL_INFO;
    config->nameSpaces                  = NULL;
    config->policies                    = NULL;
    config->durablePolicies             = NULL;
    config->filters                     = NULL;
    config->tracingTimestamps           = TRUE;
    config->tracingRelativeTimestamps   = FALSE;
    config->timeAlignment               = TRUE;
    config->startWTime                  = os_timeWGet();           /* RTC start time, used in log messages when tracingRelativeTimestamps = FALSE */
    config->startMTime                  = os_timeMGet();           /* Monotonic start time, used in log messages when tracingRelativeTimestamps = TRUE */
    config->startETime                  = os_timeEGet();           /* Elapsed start time */
    config->networkMaxWaitCount         = D_DEFAULT_NETWORK_MAX_WAITCOUNT;
    config->builtinTopicsEnabled        = TRUE;
    config->waitForRemoteReaders        = FALSE;
    config->role                        = NULL;
    config->clientDurabilityEnabled     = FALSE;                   /* Initially no client durability */
    config->capabilitySupport           = TRUE;                    /* Capability interface enabled by default, can be overruled by OSPL_DURABILITY_CAPABILITY_SUPPORT environment variable */
    config->capabilityGroupHash         = TRUE;                    /* Capability to hash data sets per group */
    config->capabilityEOTSupport        = TRUE;                    /* Capability to align EOT messages */
    config->capabilityY2038Ready        = FALSE;                   /* Capability to support times beyond 2038 */
    config->traceMask                   = 0;                       /* Initially do not log any trace information */
    config->seqnum_initval              = 0ul;                     /* Initial start sequence number */
    config->capabilityMasterSelection   = D_USE_MASTER_PRIORITY_ALGORITHM;  /* Capability to use master priorities in the selection algorithm */
    config->masterSelectionLegacy       = TRUE;                    /* Initially use the legacy master selection algorithm */
    config->initialMergeStateValue      = 0;                       /* Initial merge state value */

    /* Apply defaults */
    d_configurationSetDuration(&(config->heartbeatExpiryTime), D_DEFAULT_HEARTBEAT_EXPIRY_TIME);
    config->heartbeatExpiry = D_DEFAULT_HEARTBEAT_EXPIRY_TIME;
    d_configurationSetDuration(&(config->heartbeatUpdateInterval), D_DEFAULT_HEARTBEAT_UPDATE_FACTOR*D_DEFAULT_HEARTBEAT_EXPIRY_TIME);
    d_configurationSetDuration(&(config->livelinessUpdateInterval), D_DEFAULT_LIVELINESS_UPDATE_FACTOR*D_DEFAULT_LIVELINESS_EXPIRY_TIME);
    config->livelinessExpiry = D_DEFAULT_LIVELINESS_EXPIRY_TIME;
    d_configurationSetDuration(&(config->livelinessExpiryTime), D_DEFAULT_LIVELINESS_EXPIRY_TIME);
    d_configurationSetDuration(&(config->timeToWaitForAligner), D_DEFAULT_TIME_TO_WAIT_FOR_ALIGNER);
    d_configurationSetDuration(&(config->serviceTerminatePeriod), D_DEFAULT_SERVICE_TERMINATE_PERIOD);
    d_configurationSetDuration(&(config->terminateRemovalPeriod), D_DEFAULT_TERMINATE_REMOVAL_PERIOD);
    d_configurationSetDuration(&(config->masterElectionWaitTime), D_DEFAULT_MASTER_ELECTION_WAIT_TIME);
    d_configurationSetDuration(&(config->threadLivelinessReportPeriod), D_DEFAULT_THREAD_LIVELINESS_REPORT_PERIOD);

    /* Parse environment variables */
    d_configurationParseEnvVars(config);

    /* Parse the configuration and apply the settings as specified */
    os_threadAttrInit(&config->livelinessScheduling);
    os_threadAttrInit(&config->heartbeatScheduling);
    os_threadAttrInit(&config->persistentScheduling);
    os_threadAttrInit(&config->alignerScheduling);
    os_threadAttrInit(&config->aligneeScheduling);
    config->persistentStoreStackSize = config->persistentScheduling.stackSize; /* Set the default stackSize to the OS default */

    d_configurationSetTimingInitialWaitPeriod               (config, D_DEFAULT_TIMING_INITIAL_WAIT_PERIOD);
    d_configurationSetNetworkWaitForAttachmentMaxWaitCount  (config, D_DEFAULT_NETWORK_MAX_WAITCOUNT);
    d_configurationSetPublisherName                         (config, D_PUBLISHER_NAME);
    d_configurationSetSubscriberName                        (config, D_SUBSCRIBER_NAME);
    d_configurationSetPartitionName                         (config, D_PARTITION_NAME);
    d_configurationSetTracingOutputFile                     (config, NULL);
    d_configurationSetPersistentStoreSleepTime              (config, D_DEFAULT_PERSISTENT_STORE_SLEEP_TIME);
    d_configurationSetPersistentStoreSessionTime            (config, D_DEFAULT_PERSISTENT_STORE_SESSION_TIME);
    d_configurationSetPersistentQueueSize                   (config, D_DEFAULT_PERSISTENT_QUEUE_SIZE);
    d_configurationSetPersistentKVCompressionEnabled        (config, D_DEFAULT_PERSISTENT_KV_COMPRESSION_ENABLED);
    d_configurationSetPersistentKVCompressionAlgorithm      (config, D_DEFAULT_PERSISTENT_KV_COMPRESSION_ALGORITHM);
    d_configurationSetOptimizeUpdateInterval                (config, D_DEFAULT_OPTIMIZE_INTERVAL);

    d_configurationSetInitialRequestCombinePeriod           (config, D_DEFAULT_INITIAL_REQUEST_COMBINE_PERIOD);
    d_configurationSetOperationalRequestCombinePeriod       (config, D_DEFAULT_OPERATIONAL_REQUEST_COMBINE_PERIOD);
    d_configurationSetLatencyBudget                         (config, D_DEFAULT_LATENCY_BUDGET);
    d_configurationSetHeartbeatLatencyBudget                (config, D_DEFAULT_LATENCY_BUDGET);
    d_configurationSetAlignmentLatencyBudget                (config, D_DEFAULT_LATENCY_BUDGET);
    d_configurationSetTransportPriority                     (config, D_DEFAULT_TRANSPORT_PRIORITY);
    d_configurationSetHeartbeatTransportPriority            (config, D_DEFAULT_TRANSPORT_PRIORITY);
    d_configurationSetAlignmentTransportPriority            (config, D_DEFAULT_TRANSPORT_PRIORITY);
    d_configurationSetRole                                  (config, D_DEFAULT_ROLE);
    d_configurationSetMajorityVotingThreshold               (config, D_DEFAULT_MAJORITY_VOTING_THRESHOLD);

    /* Determine if durability must align builtin topics.
     * The current implementation will not align builtin topics
     * if all DDSI services in the configuration have their
     * <GenerateBuiltinTopics> element set to TRUE, and no native
     * networking service is configured. If no networking at all
     * is configured then mustAlignBuiltinTopics = TRUE.
     *
     * NOTE: Whether or not to align is decided based on analysis
     * of the configuration during startup. This is a rather static
     * approach and does not work in use cases where services
     * can be created or die dynamically, wich may affect this
     * decision. To support these kind of use cases a better
     * approach would be for the durability service be notified
     * if a networking services or DDSI services dies or joins,
     * so durability can review the decision whether or not to align
     * builtin topics. This will probably require new functionality
     * in the process framework.
     */

    config->mustAlignBuiltinTopics = d_configurationDurabilityMustAlignBuiltin(cfg);
    /* Only wait for remote readers if durability does NOT have to align
     * builtin topics.
     */
    config->waitForRemoteReaders = (!config->mustAlignBuiltinTopics);
    config->deadlockDetection = FALSE;
    durability->configuration = config;

    if(element){
        d_printTimedEvent(durability, D_LEVEL_INFO,
            "Configuration defaults applied. Now searching for actual one...\n");

        if(domainElement){
            d_configurationValueFloat(config, domainElement, "Lease/ExpiryTime/#text", d_configurationSetLivelinessExpiryTime);
            d_configurationSetLivelinessUpdateFactor(config, domainElement, "Lease/ExpiryTime", "update_factor");
            d_configurationValueString(config, domainElement, "Role/#text", d_configurationSetRole);
            d_configurationAttrValueBoolean(config, domainElement, "BuiltinTopics", "enabled", d_configurationSetBuiltinTopicsEnabled);
            d_configurationValueFloat(config, domainElement, "ServiceTerminatePeriod/#text", d_configurationSetServiceTerminatePeriod);
            d_configurationAttrValueBoolean(config, domainElement, "Watchdog", "deadlockDetection", d_configurationSetDeadlockDetection);
            d_configurationValueString(config, domainElement, "y2038Ready/#text", d_configurationSetY2038Ready);
            d_configurationValueString (config, domainElement, "GeneralWatchdog/Scheduling/Class/#text", d_configurationSetLivelinessSchedulingClass);
            d_configurationValueLong   (config, domainElement, "GeneralWatchdog/Scheduling/Priority/#text", d_configurationSetLivelinessSchedulingPriority);
            config->durablePolicies = d_configurationResolveDurablePolicies(domainElement, "DurablePolicies/Policy");
            config->filters = d_configurationResolveFilters(config, domainElement, "Filters/Filter", durability);
        } else {
            OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                "No Domain configuration found. Applying default Lease and Role...");
        }
        d_configurationAttrValueBoolean(config, element, "Watchdog", "deadlockDetection", d_configurationSetDeadlockDetection);
        d_configurationValueString (config, element, "Watchdog/Scheduling/Class/#text", d_configurationSetLivelinessSchedulingClass);
        d_configurationValueLong   (config, element, "Watchdog/Scheduling/Priority/#text", d_configurationSetLivelinessSchedulingPriority);

        d_configurationValueString (config, element, "Persistent/StoreDirectory/#text", d_configurationSetPersistentStoreDirectory);

        d_configurationValueString (config, element, "Persistent/Scheduling/Class/#text", d_configurationSetPersistentSchedulingClass);
        d_configurationValueLong   (config, element, "Persistent/Scheduling/Priority/#text", d_configurationSetPersistentSchedulingPriority);

        d_configurationValueFloat  (config, element, "Persistent/StoreSleepTime/#text", d_configurationSetPersistentStoreSleepTime);
        d_configurationValueULong  (config, element, "Persistent/QueueSize/#text", d_configurationSetPersistentQueueSize);
        d_configurationValueString (config, element, "Persistent/StoreMode/#text", d_configurationSetPersistentStoreMode);

        d_configurationResolvePersistentKVConfig (config, element, "Persistent/KeyValueStore");

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
        d_configurationValueFloat  (config, element, "Network/Alignment/TimeToWaitForAligner/#text", d_configurationSetTimeToWaitForAligner);
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

        d_configurationSetNetworkWaitForAttachment(config, element, "Network/WaitForAttachment", "ServiceName/#text");

        /* MasterElectionWaitTime */
        d_configurationValueFloat  (config, element, "MasterElection/WaitTime/#text", d_configurationSetMasterElectionWaitTime);

        /* Client durability, enable by default if the ClientDurability tag is available */
        if (d_configurationHasTag(config, element, "ClientDurability")) {
            config->clientDurabilityEnabled = TRUE;
        }
        d_configurationAttrValueBoolean(config, element, "ClientDurability", "enabled", d_configurationSetClientDurabilityEnabled);
        if (config->clientDurabilityEnabled) {
            /* Set default entitynames for client durability.
             * Note that we use the durability partition as the default partition
             */
            d_configurationSetClientDurabilityPartitionName         (config, config->partitionName);
            d_configurationSetClientDurabilityPublisherName         (config, D_CLIENT_DURABILITY_PUBLISHER_NAME);
            d_configurationSetClientDurabilitySubscriberName        (config, D_CLIENT_DURABILITY_SUBSCRIBER_NAME);
            /* Overwrite the defaults if specified in the config */
            d_configurationValueString (config, element, "ClientDurability/EntityNames/Partition/#text", d_configurationSetClientDurabilityPartitionName);
            d_configurationValueString (config, element, "ClientDurability/EntityNames/Publisher/#text", d_configurationSetClientDurabilityPublisherName);
            d_configurationValueString (config, element, "ClientDurability/EntityNames/Subscriber/#text", d_configurationSetClientDurabilitySubscriberName);
        }
        /* NameSpaces */
        config->policies = d_configurationResolvePolicies (element, "NameSpaces/Policy");
        config->nameSpaces = d_configurationResolveNameSpaces(config, element, "NameSpaces/NameSpace", &result);
    } else {
        d_printTimedEvent(durability, D_LEVEL_INFO, "Configuration defaults applied. No actual one found...\n");
    }

    if (c_iterLength(config->policies) == 0) {
        /* If no policies are found, create a default policy for all namespaces */
        policy = d_policyNew("*", TRUE, D_ALIGNEE_INITIAL, FALSE, D_DURABILITY_ALL,
                             D_DEFAULT_EQUALITY_CHECK, D_DEFAULT_MASTER_PRIORITY);
        config->policies = c_iterInsert(config->policies, policy);
    }
    /* Determine whether or not to use the legacy master selection algorithm
     * We only use the new algorithm if there is at least one policy with a priority that differs from D_MAXIMUM_MASTER_PRIORITY
     */
    iter = c_iterIterGet(config->policies);
    while (((policy = d_policy(c_iterNext(&iter))) != NULL) && (config->masterSelectionLegacy)) {
        config->masterSelectionLegacy = (policy->masterPriority == D_MAXIMUM_MASTER_PRIORITY);
    }

    if (c_iterLength(config->nameSpaces) == 0) {
        d_printTimedEvent(durability, D_LEVEL_SEVERE, "No namespaces configured, unable to start.\n");
        OS_REPORT(OS_FATAL, D_CONTEXT, 0, "Unable to start with empty namespaces configuration");
        result = -1;
    } else {
        c_bool found = FALSE;
        /* Make sure the V_BUILTIN_PARTITION is part of the namespace */
        for(i=0; i<c_iterLength(config->nameSpaces) && !found; i++){
            ns = d_nameSpace(c_iterObject(config->nameSpaces, i));
            found = d_configurationInNameSpace(
                    ns,
                    V_BUILTIN_PARTITION, V_TOPICINFO_NAME, TRUE);
            if (!found) {
                ns = NULL;
            }
        }
        /* If DCPSTopic is not found, don't bother looking for the others. If it is found, look for the
         * other topics in the same namespace.
         * If builtin-topics are not enabled, just create namespace for DCPSTopic.
         */
        if (found && config->builtinTopicsEnabled) {
            for (i = 0; d_builtinTopics[i] != NULL && found; i++) {
                found = d_configurationInNameSpace(ns, V_BUILTIN_PARTITION, (d_topic) d_builtinTopics[i], TRUE);
            }
        }

        if (!found) {
            if (ns) {
                char bs[256], *bsp = bs;
                for (i = 0; d_builtinTopics[i] != NULL && bsp < bs + sizeof (bs); i++) {
                    bsp += snprintf (bsp, (size_t) (bs + sizeof (bs) - bsp), "%s%s", (i == 0) ? "" : ", ", d_builtinTopics[i]);
                }
                d_printTimedEvent(durability, D_LEVEL_SEVERE, "%s should all be in the same namespace.\n", bs);
                OS_REPORT(OS_ERROR, D_CONTEXT, 0, "%s should all be in the same namespace.", bs);
                result = -1;
            } else {
                d_policy policy;
                d_mergePolicy mergeType;

                policy = d_policyNew("AutoBuiltinTopics", TRUE, D_ALIGNEE_INITIAL, FALSE, D_DURABILITY_TRANSIENT,
                    D_DEFAULT_EQUALITY_CHECK, config->masterSelectionLegacy);

                ns = d_nameSpaceNew("AutoBuiltinTopics", policy);

                d_nameSpaceAddElement(ns, "NoName", V_BUILTIN_PARTITION, V_TOPICINFO_NAME);
                /* Only add these groups to the namespace when builtin-topics are enabled */
                if(config->builtinTopicsEnabled) {
                    for (i = 0; d_builtinTopics[i] != NULL; i++) {
                        d_nameSpaceAddElement(ns, "NoName", V_BUILTIN_PARTITION, d_builtinTopics[i]);
                    }
                }
                config->nameSpaces = c_iterInsert(config->nameSpaces, ns);
                /* Decide the policy of this nameSpace based on the mustAlignBuiltinTopics setting */
                if (config->mustAlignBuiltinTopics) {
                    /* builtin topics must be merged */
                    mergeType = D_MERGE_MERGE;
                } else {
                    /* builtin topics can be ignored */
                    mergeType = D_MERGE_IGNORE;
                }
                /* Use the merge policy */
                d_policyAddMergeRule(policy, mergeType, "AutoBuiltinTopics");
                d_policyFree(policy);
            }
        }
    }

    if (result == 0) {
        result = d_configurationSanityCheck(config);
    }

    return result;
}

static void
d_configurationReportPolicy(
    c_voidp object,
    c_voidp userData)
{
    d_policy policy;
    d_durability durability;
    const c_char* aligner;
    const c_char* alignee;
    const c_char* kind;
    const c_char* name;
    const c_char* delayed;
    const c_char* equality;

    policy = d_policy(object);
    durability = d_durability(userData);

    switch(d_policyGetDurabilityKind(policy)){
        case D_DURABILITY_ALL:
            kind = "ALL";
            break;
        case D_DURABILITY_PERSISTENT:
            kind = "PERSISTENT";
            break;
        case D_DURABILITY_TRANSIENT:
            kind = "TRANSIENT";
            break;
        case D_DURABILITY_TRANSIENT_LOCAL:
            kind = "TRANSIENT_LOCAL";
            break;
        default:
            kind = "NOT_VALID";
            (void)kind;
            assert(FALSE);
            break;
    }
    switch(d_policyGetAlignmentKind(policy)){
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
            (void)alignee;
            assert(FALSE);
            break;
    }

    if(d_policyGetAligner(policy)){
        aligner = "TRUE";
    } else {
        aligner = "FALSE";
    }
    if(d_policyGetDelayedAlignment(policy)){
        delayed = "TRUE";
    } else {
        delayed = "FALSE";
    }
    if (d_policyGetEqualityCheck(policy)) {
        equality = "TRUE";
    } else {
        equality = "FALSE";
    }
    name = d_policyGetNameSpace(policy);

    d_printEvent(durability, D_LEVEL_CONFIG,
                "    - Policy:\n" \
                "        - NameSpace        : %s\n" \
                "        - Aligner          : %s\n" \
                "        - Alignee          : %s\n" \
                "        - DurabilityKind   : %s\n" \
                "        - DelayedAlignment : %s\n" \
                "        - EqualityCheck    : %s\n",
                name, aligner, alignee, kind, delayed, equality);
    if (d_policyGetAligner(policy) == TRUE) {
        d_printEvent(durability, D_LEVEL_CONFIG,
                "        - MasterPriority   : %u\n",
                d_policyGetMasterPriority(policy));
    }

    /* Print the merge rules, if any */
    if (policy->mergePolicyRules == NULL) {
         d_printEvent(durability, D_LEVEL_CONFIG,
                "        - Rules            : NULL\n");
    } else {
         d_policyMergeRule* rule;
         char *mergeTypeStr = "IGNORE";
         c_iterIter iter = c_iterIterGet(policy->mergePolicyRules);
         d_printEvent(durability, D_LEVEL_CONFIG,
                "        - Rules            :");
         while ((rule = (d_policyMergeRule *)c_iterNext(&iter)) != NULL) {
             switch (rule->mergeType) {
                 case D_MERGE_IGNORE :
                     mergeTypeStr = "IGNORE";
                     break;
                 case D_MERGE_MERGE :
                     mergeTypeStr = "MERGE";
                     break;
                 case D_MERGE_DELETE :
                     mergeTypeStr = "DELETE";
                     break;
                 case D_MERGE_REPLACE :
                     mergeTypeStr = "REPLACE";
                     break;
                 case D_MERGE_CATCHUP :
                     mergeTypeStr = "CATCHUP";
                     break;
             }
             d_printEvent(durability, D_LEVEL_CONFIG, " (%s,%s)", rule->scope, mergeTypeStr);
         }
         d_printEvent(durability, D_LEVEL_CONFIG, "\n");
    }

}


static void
d_configurationReportDurablePolicies(
    d_durability durability)
{
    d_configuration config = d_durabilityGetConfiguration(durability);
    c_iterIter iter = c_iterIterGet(config->durablePolicies);
    struct durablePolicy *rule;

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- DurablePolicies                             :");
    while ((rule = (struct durablePolicy *)c_iterNext(&iter)) != NULL) {
        d_printEvent(durability, D_LEVEL_CONFIG, " (%s,%s)", rule->obtain, rule->cache ? "TRUE" : "FALSE");
    }
    d_printEvent(durability, D_LEVEL_CONFIG, "\n");
}


static void
d_configurationNameSpacesCombine(
    c_voidp object,
    c_voidp userData)
{
    d_nameSpace ns;
    d_durability durability;
    c_char* partitionTopic;
    c_char* name;

    const c_char* aligner;
    const c_char* alignee;
    const c_char* kind;

    ns = d_nameSpace(object);
    durability = d_durability(userData);
    partitionTopic = d_nameSpaceGetPartitionTopics(ns);

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
        case D_DURABILITY_TRANSIENT_LOCAL:
            kind = "TRANSIENT_LOCAL";
            break;
        default:
            kind = "NOT_VALID";
            (void)kind;
            assert(FALSE);
            break;
    }
    switch(d_nameSpaceGetAlignmentKind(ns)){
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
            (void)alignee;
            assert(FALSE);
            break;
    }

    if(d_nameSpaceIsAligner(ns)){
        aligner = "TRUE";
    } else {
        aligner = "FALSE";
    }

    name = d_nameSpaceGetName(ns);

    d_printEvent(durability, D_LEVEL_CONFIG,
                "    - NameSpace:\n" \
                "        - Name             : %s\n" \
                "        - Aligner          : %s\n" \
                "        - Alignee          : %s\n" \
                "        - DurabilityKind   : %s\n" \
                "        - PartitionTopic   : %s\n" \
                "        - DelayedAlignment : %s\n",
                name, aligner, alignee, kind, partitionTopic,
                d_nameSpaceGetDelayedAlignment(ns) ? "TRUE" : "FALSE");

    os_free(partitionTopic);

    return;
}

static c_bool
d_configurationServiceNamesCombine(
    c_char* serviceName,
    c_voidp userData)
{
    d_durability durability;

    durability = d_durability(userData);

    d_printEvent(durability, D_LEVEL_CONFIG, "    - %s\n", serviceName);

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
    const c_char* tracingSynchronous;
    const c_char* algorithm;

    d_printTimedEvent(durability, D_LEVEL_CONFIG, "Configuration:\n");

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- OSPL Version                                : %s\n",
            OSPL_VERSION_STR);
    if (strcmp(OSPL_INNER_REV_STR, "") != 0) {
        d_printEvent(durability, D_LEVEL_CONFIG,
                "  - ospli revision                            : %s\n",
                OSPL_INNER_REV_STR);
    }
    if (strcmp(OSPL_OUTER_REV_STR, "") != 0) {
        d_printEvent(durability, D_LEVEL_CONFIG,
                "  - osplo revision                            : %s\n",
                OSPL_OUTER_REV_STR);
    }

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Version                                     : %hu.%hu (vendorId %x%x)\n",
            durability->myVersion.major, durability->myVersion.minor,
            durability->myVersion.vendorId.vendorId[0], durability->myVersion.vendorId.vendorId[1]);

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Role                                        : %s\n",
            config->role);

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
            "- ServiceTerminatePeriod                      : %"PA_PRIduration"\n" \
            "- Liveliness.ExpiryTime                       : %"PA_PRIduration"\n" \
            "- Liveliness.UpdateInterval                   : %"PA_PRIduration"\n" \
            "- TerminateRemovalPeriod                      : %"PA_PRIduration"\n" \
            "- MasterElection.WaitTime                     : %"PA_PRIduration"\n" \
            "- Liveliness.Scheduling.Class                 : %s\n" \
            "- Liveliness.Scheduling.Priority              : %d\n" \
            "- Liveliness.DeadlockDetection                : %s\n"
            , OS_DURATION_PRINT(config->serviceTerminatePeriod)
            , OS_DURATION_PRINT(config->livelinessExpiryTime)
            , OS_DURATION_PRINT(config->livelinessUpdateInterval)
            , OS_DURATION_PRINT(config->terminateRemovalPeriod)
            , OS_DURATION_PRINT(config->masterElectionWaitTime)
            , class
            , config->livelinessScheduling.schedPriority
            , (config->deadlockDetection?"TRUE":"FALSE"));

    d_configurationReportDurablePolicies(durability);

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
        case D_STORE_TYPE_KV:
            pstoreMode = "KV";
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
            "- Persistent.StoreMode                        : %s%s%s\n"
            , pstoreDir
            , pstoreMode
            , config->persistentKVStoreStorageType ? ":" : ""
            , config->persistentKVStoreStorageType ? config->persistentKVStoreStorageType : "");

    if (config->persistentStoreMode == D_STORE_TYPE_KV) {
        d_printEvent(durability, D_LEVEL_CONFIG,
                    "- Persistent.KeyValueStore.KeyValueStore      : %s\n" \
                    "- Persistent.KeyValueStore.Compression.enabled: %s\n",
                    config->persistentKVStoreStorageParameters ? config->persistentKVStoreStorageParameters : "",
                    config->persistentKVStoreCompressionEnabled ? "TRUE" : "FALSE");
        if (config->persistentKVStoreCompressionEnabled == TRUE) {
            algorithm = d_compressionKVImage(config->persistentKVStoreCompressionAlgorithm);
            d_printEvent(durability, D_LEVEL_CONFIG,
                        "- Persistent.KeyValueStore.Compression.algorithm: %s\n" \
                        , algorithm);
        }
    }

    d_printEvent(durability, D_LEVEL_CONFIG,
                "- Persistent.QueueSize                        : %u\n"
                , config->persistentQueueSize);

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Persistent.StoreSleepTime                   : %"PA_PRIduration"\n" \
            "- Persistent.StoreSessionTime                 : %"PA_PRIduration"\n" \
            "- Persistent.StoreOptimizeInterval            : %d\n" \
            "- Persistent.Scheduling.Class                 : %s\n" \
            "- Persistent.Scheduling.Priority              : %d\n"
            , OS_DURATION_PRINT(config->persistentStoreSleepTime)
            , OS_DURATION_PRINT(config->persistentStoreSessionTime)
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
    if(config->tracingSynchronous == TRUE){
        tracingSynchronous = "TRUE";
    } else {
        tracingSynchronous = "FALSE";
    }
    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Tracing.Verbosity                           : %s\n" \
            "- Tracing.OutputFile                          : %s\n" \
            "- Tracing.Synchronous                         : %s\n" \
            "- Tracing.Timestamps                          : %s\n" \
            "- Tracing.RelativeTimestamps                  : %s\n"
            , verbosity
            , config->tracingOutputFileName
            , tracingSynchronous
            , timestamps
            , relativeTimestamps);

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Network.InitialDiscoveryPeriod              : %"PA_PRIduration"\n" \
            "- Network.LatencyBudget                       : %"PA_PRIduration"\n" \
            "- Network.TransportPriority                   : %d\n"
            , OS_DURATION_PRINT(config->timingInitialWaitPeriod)
            , OS_DURATION_PRINT(config->latencyBudget)
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
            "- Network.Heartbeat.ExpiryTime                : %"PA_PRIduration"\n" \
            "- Network.Heartbeat.UpdateInterval            : %"PA_PRIduration"\n" \
            "- Network.Heartbeat.LatencyBudget             : %"PA_PRIduration"\n" \
            "- Network.Heartbeat.TransportPriority         : %d\n" \
            "- Network.Heartbeat.Scheduling.Class          : %s\n" \
            "- Network.Heartbeat.Scheduling.Priority       : %d\n" \
            , OS_DURATION_PRINT(config->heartbeatExpiryTime)
            , OS_DURATION_PRINT(config->heartbeatUpdateInterval)
            , OS_DURATION_PRINT(config->heartbeatLatencyBudget)
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
            "- Network.Alignment.RequestCombine.Initial    : %"PA_PRIduration"\n" \
            "- Network.Alignment.RequestCombine.Operational: %"PA_PRIduration"\n" \
            "- Network.Alignment.LatencyBudget             : %"PA_PRIduration"\n" \
            "- Network.Alignment.TransportPriority         : %d\n"  \
            , timeAlignment
            , OS_DURATION_PRINT(config->initialRequestCombinePeriod)
            , OS_DURATION_PRINT(config->operationalRequestCombinePeriod)
            , OS_DURATION_PRINT(config->alignerLatencyBudget)
            , config->alignerTransportPriority);

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Network.Alignment.AlignerScheduling.Class   : %s\n"  \
            "- Network.Alignment.AlignerScheduling.Priority: %d\n"  \
            "- Network.Alignment.AligneeScheduling.Class   : %s\n"  \
            "- Network.Alignment.AligneeScheduling.Priority: %d\n"  \
            "- Network.Alignment.TimeToWaitForAligner      : %"PA_PRIduration"\n"
            , class
            , config->alignerScheduling.schedPriority
            , class2
            , config->aligneeScheduling.schedPriority
            , OS_DURATION_PRINT(config->timeToWaitForAligner));


    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Network.WaitForAttachment.MaxWaitCount      : %u\n" \
            "- Network.WaitForAttachment.ServiceNames      : \n"
            , config->networkMaxWaitCount
            );

    d_printEvent(durability, D_LEVEL_CONFIG,
        "- BuiltinTopics.enabled                       : %s\n",
        (config->builtinTopicsEnabled) ? "TRUE" : "FALSE");

    d_printEvent(durability, D_LEVEL_CONFIG,
        "- BuiltinTopicAlignment                       : %s\n",
        (config->mustAlignBuiltinTopics) ? "TRUE" : "FALSE");

    d_printEvent(durability, D_LEVEL_CONFIG,
        "- MajorityVotingThreshold                     : %u\n", config->majorityVotingThreshold);

    d_printEvent(durability, D_LEVEL_CONFIG,
        "- WaitForRemoteReaders                        : %s\n",
        (config->waitForRemoteReaders) ? "TRUE" : "FALSE");

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- capabilitySupport enabled                   : %s\n",
            (config->capabilitySupport) ? "TRUE" : "FALSE");

    if (config->capabilitySupport) {
        d_printEvent(durability, D_LEVEL_CONFIG,
            "  - groupHash                                 : %s\n" \
            "  - EOTSupport                                : %s\n" \
            "  - y2038Ready                                : %s\n",
            (config->capabilityGroupHash) ? "TRUE" : "FALSE",
            (config->capabilityEOTSupport) ? "TRUE" : "FALSE",
            (config->capabilityY2038Ready) ? "TRUE" : "FALSE");
    }

    if (config->clientDurabilityEnabled) {
        d_printEvent(durability, D_LEVEL_CONFIG,
                "- ClientDurabilityEnabled                     : TRUE\n" \
                "- ClientDurability.EntityNames.Publisher      : %s\n" \
                "- ClientDurability.EntityNames.Subscriber     : %s\n" \
                "- ClientDurability.EntityNames.Partition      : %s\n"
                , config->clientDurabilityPublisherName
                , config->clientDurabilitySubscriberName
                , config->clientDurabilityPartitionName);
    } else {
        d_printEvent(durability, D_LEVEL_CONFIG,
                "- ClientDurabilityEnabled                     : FALSE\n");
    }

    d_printEvent(durability, D_LEVEL_CONFIG,
        "- masterSelectionLegacy                       : %s\n",
        (config->masterSelectionLegacy) ? "TRUE" : "FALSE");

    d_printEvent(durability, D_LEVEL_CONFIG,
        "- ThreadLivelinessReportPeriod                : %"PA_PRIduration"\n",
        OS_DURATION_PRINT(config->threadLivelinessReportPeriod));

    d_tableWalk(config->networkServiceNames, d_configurationServiceNamesCombine, durability);

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- NameSpaces                                  :\n");
    c_iterWalk(config->nameSpaces, d_configurationNameSpacesCombine, durability);

    d_printEvent(durability, D_LEVEL_CONFIG,
            "- Policies                                    :\n");
    c_iterWalk(config->policies, d_configurationReportPolicy, durability);
}

void
d_configurationSetRole(
    d_configuration config,
    const c_char* role)
{
    if(config->role){
        os_free(config->role);
        config->role = NULL;
    }
    config->role = os_strdup(role);
}

void
d_configurationSetServiceTerminatePeriod(
    d_configuration config,
    const c_float seconds)
{
    c_float sec;

    sec = seconds;

    if (sec < D_MINIMUM_SERVICE_TERMINATE_PERIOD) {
        sec = D_MINIMUM_SERVICE_TERMINATE_PERIOD;
    }
    if (sec > D_MAXIMUM_SERVICE_TERMINATE_PERIOD) {
        sec = D_MAXIMUM_SERVICE_TERMINATE_PERIOD;
    }
    config->serviceTerminate = sec;
    d_configurationSetDuration(&(config->serviceTerminatePeriod), sec);
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
                if (sec < D_MINIMUM_LIVELINESS_UPDATE_FACTOR) {
                    sec = D_MINIMUM_LIVELINESS_UPDATE_FACTOR;
                }
                if (sec > D_MAXIMUM_LIVELINESS_UPDATE_FACTOR) {
                    sec = D_MAXIMUM_LIVELINESS_UPDATE_FACTOR;
                }
                sec = config->livelinessExpiry * sec;
                d_configurationSetDuration(&(config->livelinessUpdateInterval), sec);
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
d_configurationSetPersistentSchedulingStackSize(
    d_configuration config,
    c_ulong stackSize)
{
    config->persistentScheduling.stackSize = stackSize;
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
                if (sec < D_MINIMUM_HEARTBEAT_UPDATE_FACTOR) {
                    sec = D_MINIMUM_HEARTBEAT_UPDATE_FACTOR;
                }
                if (sec > D_MAXIMUM_HEARTBEAT_UPDATE_FACTOR) {
                    sec = D_MAXIMUM_HEARTBEAT_UPDATE_FACTOR;
                }
                sec = config->heartbeatExpiry * sec;
                d_configurationSetDuration(&(config->heartbeatUpdateInterval), sec);
            }
            u_cfElementFree(expiryElement);
            expiryElement = u_cfElement(c_iterTakeFirst(elements));
        }
        c_iterFree(elements);
    }
}

void
d_configurationSetBuiltinTopicsEnabled(
    d_configuration config,
    c_bool enabled)
{
    config->builtinTopicsEnabled = enabled;
}

void
d_configurationSetDeadlockDetection(
    d_configuration config,
    c_bool deadlockDetection)
{
    config->deadlockDetection = deadlockDetection;
}

void
d_configurationSetY2038Ready(
    d_configuration config,
    const c_char *y2038Ready)
{
    config->capabilityY2038Ready = (os_strcasecmp(y2038Ready, "true") == 0);
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
    d_configurationSetDuration(&(config->heartbeatExpiryTime), sec);
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
    d_configurationSetDuration(&(config->timingInitialWaitPeriod), sec);
}


void
d_configurationSetMasterElectionWaitTime(
    d_configuration config,
    c_float seconds)
{
    c_float sec;

    sec = seconds;

    if (sec < D_MINIMUM_MASTER_ELECTION_WAIT_TIME) {
        sec = D_MINIMUM_MASTER_ELECTION_WAIT_TIME;
    }
    if (sec > D_MAXIMUM_MASTER_ELECTION_WAIT_TIME) {
        sec = D_MAXIMUM_MASTER_ELECTION_WAIT_TIME;
    }
    d_configurationSetDuration(&(config->masterElectionWaitTime), sec);
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
d_configurationSetMajorityVotingThreshold(
    d_configuration config,
    c_ulong threshold)
{
    c_ulong p;

    p = threshold;

    if (p < D_MINIMUM_MAJORITY_VOTING_THRESHOLD) {
        p = D_MINIMUM_MAJORITY_VOTING_THRESHOLD;
    }
    if (p > D_MAXIMUM_MAJORITY_VOTING_THRESHOLD) {
        p = D_MAXIMUM_MAJORITY_VOTING_THRESHOLD;
    }
    config->majorityVotingThreshold = p;
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
    d_configurationSetDuration(&(config->initialRequestCombinePeriod), sec);
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
    d_configurationSetDuration(&(config->operationalRequestCombinePeriod), sec);
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
    /* Set the maxWaitTime */
    config->networkMaxWaitTime = os_realToDuration((c_float)config->networkMaxWaitCount * 0.1f /* 100 ms */);
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
    c_char*     serviceName;

    attachIter = u_cfElementXPath(elementParent, attachName);
    element = u_cfElement(c_iterTakeFirst(attachIter));

    if(element){
        success = u_cfElementAttributeULongValue(element, "maxWaitCount", &maxWaitCount);

        if(success){
            d_configurationSetNetworkWaitForAttachmentMaxWaitCount(config, maxWaitCount);
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

/**
 * \brief Set client durability according to the ClientDurability[@enabled]
 *        attribute in the configuration.
 */
void
d_configurationSetClientDurabilityEnabled(
    d_configuration config,
    const c_bool enabled)
{
    config->clientDurabilityEnabled = enabled;
}

void
d_configurationSetPublisherName(
    d_configuration  config,
    const c_char * publisherName)
{
    if (config) {
        if (publisherName != NULL) {
            if (config->publisherName) {
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
            if (config->partitionName) {
                d_free(config->partitionName);
                config->partitionName = NULL;
            }
            config->partitionName = os_strdup(partitionName);
        }
    }
}


void
d_configurationSetClientDurabilityPartitionName(
    d_configuration  config,
    const c_char * partitionName)
{
    if (config) {
        if (partitionName != NULL) {
            if (config->clientDurabilityPartitionName) {
                d_free(config->clientDurabilityPartitionName);
             }
             config->clientDurabilityPartitionName = os_strdup(partitionName);
        }
    }
}

void
d_configurationSetClientDurabilityPublisherName(
    d_configuration  config,
    const c_char * publisherName)
{
    if (config) {
        if (publisherName != NULL) {
            if (config->clientDurabilityPublisherName) {
                d_free(config->clientDurabilityPublisherName);
             }
             config->clientDurabilityPublisherName = os_strdup(publisherName);
        }
    }
}


void
d_configurationSetClientDurabilitySubscriberName(
    d_configuration  config,
    const c_char * subscriberName)
{
    if (config) {
        if (subscriberName != NULL) {
            if (config->clientDurabilitySubscriberName) {
                d_free(config->clientDurabilitySubscriberName);
             }
             config->clientDurabilitySubscriberName = os_strdup(subscriberName);
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
            char *filename;
            if (os_strcasecmp(value, "") == 0) {
                OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                    "OutputFile configuration-parameter is empty, using default value \"" D_DEFAULT_TRACING_OUTFILE "\"");
                filename = os_fileNormalize(D_DEFAULT_TRACING_OUTFILE);
            } else {
                filename = os_fileNormalize(value);
            }

            if (filename) {
                config->tracingOutputFile = fopen(filename, "a");
                if (!config->tracingOutputFile) {
                    const os_char *msg = os_getErrno() ? os_strError(os_getErrno()) : NULL;
                    OS_REPORT(OS_ERROR, D_CONTEXT, 0,
                        "Failed to open tracing OutputFile \"%s\": %s",
                        filename, msg ? msg : "Unknown reason");
                }
                config->tracingOutputFileName = os_strdup(filename);
                os_free(filename);
            }
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
d_configurationSetTimeToWaitForAligner(
    d_configuration  config,
    c_float seconds)
{
    c_float sec;

    sec = seconds;

    if (sec < D_MINIMUM_TIME_TO_WAIT_FOR_ALIGNER) {
        sec = D_MINIMUM_TIME_TO_WAIT_FOR_ALIGNER;
    }
    if (sec > D_MAXIMUM_TIME_TO_WAIT_FOR_ALIGNER) {
        sec = D_MAXIMUM_TIME_TO_WAIT_FOR_ALIGNER;
    }
    /* The only allowed supported values at the moment are
     * D_MINIMUM_TIME_TO_WAIT_FOR_ALIGNER (0), or the default
     * D_MAXIMUM_TIME_TO_WAIT_FOR_ALIGNER (maxfloat).
     * Any specified value > D_MINIMUM_TIME_TO_WAIT_FOR_ALIGNER
     * will for the moment be mapped to D_MAXIMUM_TIME_TO_WAIT_FOR_ALIGNER
     * and a warning is generated.
     *
     * The support for more values is identified as a future extension.
     */
    if (sec > D_MINIMUM_TIME_TO_WAIT_FOR_ALIGNER) {
        sec = D_MAXIMUM_TIME_TO_WAIT_FOR_ALIGNER;
    }
    /* generate a warning if necessary */
    if ( (seconds != D_MINIMUM_TIME_TO_WAIT_FOR_ALIGNER) &&
         (seconds != D_MAXIMUM_TIME_TO_WAIT_FOR_ALIGNER) ) {
        OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                  "TimeToWaitForAligner currently only supports 0.0 and 1.0 (infinite), using 1.0 for now");
    }
    d_configurationSetDuration(&(config->timeToWaitForAligner), sec);
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
            } else if(os_strcasecmp(storeModeName, "KV") == 0 || os_strncasecmp(storeModeName, "KV:", 3) == 0){
                config->persistentStoreMode = D_STORE_TYPE_KV;
            } else {
                config->persistentStoreMode = D_STORE_TYPE_XML;
            }
        }
    }
}

void
d_configurationSetPersistentKVStorageParameters (
    d_configuration  config,
    const c_char * parameters)
{
    if (config) {
        if (parameters != NULL) {
            if(config->persistentKVStoreStorageParameters) {
                d_free(config->persistentKVStoreStorageParameters);
                config->persistentKVStoreStorageParameters = NULL;
            }
            config->persistentKVStoreStorageParameters = os_strdup(parameters);
        }
    }
}

void
d_configurationResolvePersistentKVConfig (
    d_configuration config,
    u_cfElement elementParent,
    const c_char *elementName)
{
    c_iter       iter;
    u_cfElement  element;
    c_char      *value;
    c_bool       found;

    iter = u_cfElementXPath(elementParent, elementName);
    element = (u_cfElement)c_iterTakeFirst(iter);

    while (element) {
        found = u_cfElementAttributeStringValue(element, "type", &value);
        if (found) {
            if (config->persistentKVStoreStorageType) {
                os_free(config->persistentKVStoreStorageType);
            }
            config->persistentKVStoreStorageType = value;
        }
        d_configurationValueString (config, element, "StorageParameters/#text", d_configurationSetPersistentKVStorageParameters);

        d_configurationResolvePersistentKVCompression (config, element, "Compression");
        u_cfElementFree(element);
        element = (u_cfElement)c_iterTakeFirst(iter);
    }
    c_iterFree(iter);
}

void
d_configurationResolvePersistentKVCompression (
    d_configuration config,
    u_cfElement elementParent,
    const c_char *elementName)
{
    c_iter       iter;
    u_cfElement  element;
    c_char      *algorithm;
    c_bool       enabled;

    iter = u_cfElementXPath(elementParent, elementName);
    element = (u_cfElement)c_iterTakeFirst(iter);

    if (element) {
        /* If the compression element is defined compression should be enabled unless
         * the optional enabled field is set to false.
         */
        d_configurationSetPersistentKVCompressionEnabled(config, TRUE);
    }

    while (element) {
        if (u_cfElementAttributeStringValue(element, "algorithm", &algorithm)) {
            d_configurationSetPersistentKVCompressionAlgorithm(config, algorithm);
        }
        if (u_cfElementAttributeBoolValue(element, "enabled", &enabled)) {
            d_configurationSetPersistentKVCompressionEnabled(config, enabled);
        }
        u_cfElementFree(element);
        element = (u_cfElement)c_iterTakeFirst(iter);
    }

    if ((config->persistentKVStoreCompressionEnabled == TRUE) &&
        (config->persistentKVStoreCompressionAlgorithm == D_COMPRESSION_LZF)) {
        /* LZF compression requires a big stackSize to operate, make sure it
         * has at least 1MB
         */
        if (config->persistentStoreStackSize < 1048576) {
            config->persistentStoreStackSize = 1048576;
        }
        d_configurationSetPersistentSchedulingStackSize(config, config->persistentStoreStackSize);
    }

    c_iterFree(iter);
}

void
d_configurationSetPersistentKVCompressionEnabled (
    d_configuration config,
    c_bool enabled)
{
    config->persistentKVStoreCompressionEnabled = enabled;
}

void
d_configurationSetPersistentKVCompressionAlgorithm (
    d_configuration config,
    const c_char *algorithm)
{
    assert(algorithm);

    if (os_strcasecmp(algorithm, "LZF") == 0) {
        config->persistentKVStoreCompressionAlgorithm = D_COMPRESSION_LZF;
    } else if (os_strcasecmp(algorithm, "SNAPPY") == 0) {
        config->persistentKVStoreCompressionAlgorithm = D_COMPRESSION_SNAPPY;
    } else if (os_strcasecmp(algorithm, "ZLIB") == 0) {
        config->persistentKVStoreCompressionAlgorithm = D_COMPRESSION_ZLIB;
    } else if (os_strcasecmp(algorithm, "CUSTOM") == 0) {
        config->persistentKVStoreCompressionAlgorithm = D_COMPRESSION_CUSTOM;
    } else {
        config->persistentKVStoreCompressionAlgorithm = D_COMPRESSION_NONE;
        OS_REPORT(OS_WARNING, OS_FUNCTION, 0,
                  "Invalid Persistent/KeyValueStore/Compression[@algorithm] selected, compression disabled");
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
    d_configurationSetDuration(&(config->persistentStoreSleepTime), sec);
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
    d_configurationSetDuration(&(config->persistentStoreSessionTime), sec);
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
    os_duration *duration,
    c_float seconds)
{
    *duration = os_realToDuration((os_timeReal)seconds);
}

int
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
    int result;
    result = 0;

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
                    result = d_nameSpaceAddElement(nameSpace, name, partition, topic);
                    os_free(partition);
                }
            }
            u_cfNodeFree(data);
        } else {
            result = d_nameSpaceAddElement(nameSpace, name, "", topic);
        }
        c_iterFree(iter2);
        u_cfElementFree(partitionElement);
        partitionElement = u_cfElement(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);

    return result;
}

int
d_configurationResolvePartitionTopic(
    d_nameSpace nameSpace,
    u_cfElement element,
    c_char* name,
    const c_char* tag)
{
    c_iter iter, iter2;
    u_cfElement partitionElement;
    u_cfNode data;
    c_ulong size;
    c_bool found;
    c_char* partitionTopic;
    int result;
    result = 0;

    iter = u_cfElementXPath(element, tag);
    partitionElement = u_cfElement(c_iterTakeFirst(iter));

    while(partitionElement){
        iter2 = u_cfElementGetChildren(partitionElement);
        size = c_iterLength(iter2);

        if(size != 0){
            data = u_cfNode(c_iterTakeFirst(iter2));

            if(u_cfNodeKind(data) == V_CFDATA){
                found = u_cfDataStringValue(u_cfData(data), &partitionTopic);
                if (found == TRUE) {
                    result = d_nameSpaceAddElement(nameSpace, name, partitionTopic, NULL);
                    os_free(partitionTopic);
                }
            }
            u_cfNodeFree(data);
        } else {
            result = d_nameSpaceAddElement(nameSpace, name, "*.*", NULL);
        }
        c_iterFree(iter2);
        u_cfElementFree(partitionElement);
        partitionElement = u_cfElement(c_iterTakeFirst(iter));
    }
    c_iterFree(iter);

    return result;
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
d_configurationAttrValueULong(
    d_configuration configuration,
    u_cfElement  element,
    const char * tag,
    const char * attr,
    void         (* const setAction)(d_configuration config, c_ulong longValue) )
{
    c_iter   iter;
    u_cfElement e;
    u_cfAttribute a;
    c_ulong  ulongValue;
    c_bool   found;

    iter = u_cfElementXPath(element, tag);
    e = u_cfElement(c_iterTakeFirst(iter));

    while (e != NULL) {
        a = u_cfElementAttribute(e, attr);

        if(a){
            found = u_cfAttributeULongValue(a, &ulongValue);
            /* QAC EXPECT 2100; */
            if (found == TRUE) {
                setAction(configuration, ulongValue);
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
d_configurationValueSize(
    d_configuration configuration,
    u_cfElement     element,
    const char      *tag,
    void            (* const setAction)(d_configuration config, c_size size))
{
    c_iter   iter;
    u_cfData data;
    u_size   size;
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

void
d_configurationValueMemAddr(
            d_configuration configuration,
            u_cfElement  element,
            const char * tag,
            void         (* const setAction)(d_configuration config, c_address addr) )
{
            c_iter   iter;
            u_cfData data;
            c_bool   found;
            c_char *   str;
            c_address addr;

            iter = u_cfElementXPath(element, tag);
            data = u_cfData(c_iterTakeFirst(iter));

            while (data) {
                found = u_cfDataStringValue(data, &str);
                if (found == TRUE) {
                                if ( (strlen(str) > 2) &&
                                         (strncmp("0x", str, 2) == 0) ) {
                                        sscanf(str, "0x" PA_ADDRFMT, &addr);
                                } else {
                                        sscanf(str, PA_ADDRFMT, &addr);
                                }
                    setAction(configuration, addr);
                    os_free(str);
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
d_configurationInNameSpace(
    d_nameSpace ns,
    d_partition partition,
    d_topic topic,
    c_bool aligner)
{
    c_bool result;
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

    return result;
}

void
d_configurationResolveMergePolicies(
    d_policy policy,
    u_cfElement  elementParent,
    const c_char * mergePolicyName)
{
    c_iter              iter;
    u_cfElement         element;
    c_bool              found;
    c_string            mergeType_str, scope;
    d_mergePolicy       mergeType;

    mergeType = D_MERGE_IGNORE;
    iter = u_cfElementXPath(elementParent, mergePolicyName);

    element = (u_cfElement)c_iterTakeFirst(iter);

    while (element)
    {
        found = u_cfElementAttributeStringValue (element, "type", &mergeType_str);
        if (found){
            if (os_strcasecmp (mergeType_str, "IGNORE") == 0){
                mergeType = D_MERGE_IGNORE;
            }else if (os_strcasecmp (mergeType_str, "MERGE") == 0){
                mergeType = D_MERGE_MERGE;
            }else if (os_strcasecmp (mergeType_str, "DELETE") == 0){
                mergeType = D_MERGE_DELETE;
            }else if (os_strcasecmp (mergeType_str, "REPLACE") == 0){
                mergeType = D_MERGE_REPLACE;
            }else if (os_strcasecmp (mergeType_str, "CATCHUP") == 0){
                mergeType = D_MERGE_CATCHUP;
            }
            os_free (mergeType_str);
        }

        scope = NULL;
        found = u_cfElementAttributeStringValue (element, "scope", &scope);

        /* Role should always be available */
        assert (found && scope != NULL);

        /* Add merge policy to role */
        d_policyAddMergeRule (policy, mergeType, scope);

        os_free (scope);

        u_cfElementFree(element);
        element = (u_cfElement)c_iterTakeFirst(iter);
    }

    c_iterFree (iter);
}

c_iter
d_configurationResolvePolicies(
    u_cfElement  elementParent,
    const c_char * policyName )
{
    c_iter              iter, result;
    u_cfElement         element;
    c_char *            durabilityKind;
    c_char *            aligner;
    c_char *            delayedAlignment;
    c_char *            alignmentKind;
    c_char *            namespace;
    c_bool              found;
    d_policy            policy;
    c_bool              isAligner;
    c_bool              delayAlignmentEnabled;
    d_alignmentKind     akind;
    d_durabilityKind    dkind;
    c_ulong             length;
    c_bool              equalityCheck;
    c_long              longMasterPriority;
    c_ulong             masterPriority;

    result = c_iterNew(NULL);
    iter = u_cfElementXPath(elementParent, policyName);
    element = c_iterTakeFirst(iter);

    while (element) {

        /* Parse durability kind element */
        found = u_cfElementAttributeStringValue(element, "durability", &durabilityKind);
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

        /* Parse aligner element */
        found = u_cfElementAttributeStringValue(element, "aligner", &aligner);
        if (found){
            if (os_strcasecmp(aligner, "TRUE") == 0){
                isAligner = TRUE;
            }else {
                isAligner = FALSE;
            }
            os_free(aligner);
        }else
        {
            isAligner = TRUE;
        }

        /* Parse aligner element */
        found = u_cfElementAttributeStringValue(element, "delayedAlignment", &delayedAlignment);
        if (found){
            if (os_strcasecmp(delayedAlignment, "TRUE") == 0){
                delayAlignmentEnabled = TRUE;
            }else {
                delayAlignmentEnabled = FALSE;
            }
            os_free(delayedAlignment);
        }else
        {
            delayAlignmentEnabled = FALSE;
        }

        /* Parse alignment kind element */
        found = u_cfElementAttributeStringValue(element, "alignee", &alignmentKind);
        if(found){
            if(os_strcasecmp(alignmentKind, "ON_REQUEST") == 0){
                akind = D_ALIGNEE_ON_REQUEST;
            } else if(os_strcasecmp(alignmentKind, "INITIAL") == 0){
                akind = D_ALIGNEE_INITIAL;
            } else if(os_strcasecmp(alignmentKind, "LAZY") == 0){
                akind = D_ALIGNEE_LAZY;
            } else {
                akind = D_ALIGNEE_INITIAL;
            }
            os_free(alignmentKind);
        } else {
            akind = D_ALIGNEE_INITIAL;
        }

        /* Parse namespace element */
        found = u_cfElementAttributeStringValue(element, "nameSpace", &namespace);
        if(!found){

            /* TODO: Shouldn't this be moved to the code which checks the
             * empty namespace (backwards compatibility)
             */
            length = c_iterLength(result);
            namespace = os_malloc(17);
            os_sprintf(namespace, "NoName%u", length);
        }

        /* Parse equalityCheck atribute */
        found = u_cfElementAttributeBoolValue(element, "equalityCheck", &equalityCheck);
        if (!found) {
            equalityCheck = D_DEFAULT_EQUALITY_CHECK;
        }

        found = u_cfElementAttributeLongValue(element, "masterPriority", &longMasterPriority);
        if (!found) {
            masterPriority = D_DEFAULT_MASTER_PRIORITY;
        } else {
            if (longMasterPriority < D_MINIMUM_MASTER_PRIORITY) {
                OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                        "Configuration //OpenSplice/DurabilityService/NameSpaces/Policy[@masterPriority] "
                        "value(%d) less then minimum supported value(%d) using minimum.",
                        longMasterPriority, D_MINIMUM_MASTER_PRIORITY);
                masterPriority = D_MINIMUM_MASTER_PRIORITY;
            } else if (longMasterPriority > D_MAXIMUM_MASTER_PRIORITY) {
                OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                        "Configuration //OpenSplice/DurabilityService/NameSpaces/Policy[@masterPriority] "
                        "value(%d) greater then maximum supported value(%d) using maximum.",
                        longMasterPriority, D_MAXIMUM_MASTER_PRIORITY);
                masterPriority = D_MAXIMUM_MASTER_PRIORITY;
            } else {
                masterPriority = (c_ulong)longMasterPriority;
            }

            if (isAligner == FALSE) {
            /* When policy has aligner set to false the masterPriority is not relevant */
            OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                    "Configuration set with no effect on //OpenSplice/DurabilityService/NameSpaces/Policy. "
                    "When [@aligner] is set to \'false\' [@masterPriority] has no effect.");
            }
        }

        /* Create new policy */
        policy = d_policyNew (namespace, isAligner, akind, delayAlignmentEnabled,
                              dkind, equalityCheck, masterPriority);
        os_free(namespace);

        /* Resolve merge policies */
        d_configurationResolveMergePolicies(policy, element, "Merge");

        /* Insert policy in result */
        result = c_iterInsert(result, policy);

        u_cfElementFree(element);
        element = (u_cfElement)c_iterTakeFirst(iter);
    }
    c_iterFree(iter);

    return result;
}

static c_bool
resolveNameSpaceDeprecated (
    u_cfElement element,
    d_durabilityKind* durabilityKind_out,
    d_alignmentKind* alignmentKind_out,
    c_bool* isAligner_out)
{
    c_bool useDeprecated;
    c_bool              isAligner;
    d_alignmentKind     akind;
    d_durabilityKind    dkind;
    c_char *            durabilityKind;
    c_char *            alignmentKind;
    c_bool              found;

    useDeprecated = FALSE;
    isAligner = FALSE;

    /* Parse durability kind element */
    found = u_cfElementAttributeStringValue(element, "durabilityKind", &durabilityKind);
    if(found){
        useDeprecated = TRUE;

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

    /* Parse alignment kind element */
    found = u_cfElementAttributeStringValue(element, "alignmentKind", &alignmentKind);
    if(found){
        useDeprecated = TRUE;

        if(os_strcasecmp(alignmentKind, "ON_REQUEST") == 0){
            akind = D_ALIGNEE_ON_REQUEST;
        } else if(os_strcasecmp(alignmentKind, "INITIAL") == 0){
            akind = D_ALIGNEE_INITIAL;
        } else if (os_strcasecmp(alignmentKind, "INITIAL_AND_ALIGNER") == 0){
            akind = D_ALIGNEE_INITIAL;
            isAligner = TRUE;
        } else if(os_strcasecmp(alignmentKind, "LAZY") == 0){
            akind = D_ALIGNEE_LAZY;
        } else {
            akind = D_ALIGNEE_INITIAL;
        }
        os_free(alignmentKind);
    } else {
        akind = D_ALIGNEE_INITIAL;
    }

    *durabilityKind_out = dkind;
    *alignmentKind_out = akind;
    *isAligner_out = isAligner;

    return useDeprecated;
}

c_iter
d_configurationResolveNameSpaces(
    d_configuration config,
    u_cfElement  elementParent,
    const c_char * nameSpaceName,
    int * resolveResult)
{
    c_iter      iter, result;
    u_cfElement element;
    c_char *    name;
    c_bool      found;
    d_nameSpace ns;
    c_ulong length;
    c_bool noError;
    d_policy policy;
    /* For deprecated configuration */
    c_bool              isAligner;
    d_alignmentKind     akind;
    d_durabilityKind    dkind;
    c_bool              useDeprecated;

    *resolveResult = 0;

    result = c_iterNew(NULL);
    iter = u_cfElementXPath(elementParent, nameSpaceName);
    element = c_iterTakeFirst(iter);
    useDeprecated = FALSE;
    akind = D_ALIGNEE_INITIAL;
    dkind = D_DURABILITY_ALL;
    isAligner = TRUE;
    noError = TRUE;

    while (element && noError) {
        useDeprecated =
                useDeprecated || resolveNameSpaceDeprecated (
                                            element,
                                            &dkind,
                                            &akind,
                                            &isAligner);

        /* Parse name element */
        found = u_cfElementAttributeStringValue(element, "name", &name);
        if(!found){
            length = c_iterLength(result);
            name = os_malloc(17);
            os_sprintf(name, "NoName%u", length);
            useDeprecated = TRUE;
        }

        /* If deprecated, create namespace with private policy */
        if (useDeprecated) {
            policy = d_policyNew(name, isAligner, akind, FALSE, dkind, D_DEFAULT_EQUALITY_CHECK, D_DEFAULT_MASTER_PRIORITY);
        } else {
            policy = d_configurationFindPolicyForNameSpace(config, name);
        }
        ns = d_nameSpaceNew(name, policy);

        d_policyFree(policy);
        os_free(name);

        if (ns) {
            *resolveResult = d_configurationResolvePartition(ns, element, "NoName", "Partition", "*");
            if (*resolveResult == 0) {
              *resolveResult = d_configurationResolvePartitionTopic(ns, element, "NoName", "PartitionTopic");
            }
            if (*resolveResult == 0) {
              result = c_iterInsert(result, ns);
            } else {
              d_nameSpaceFree(ns);
              OS_REPORT(OS_ERROR, D_CONTEXT, 0, "Topic in //OpenSplice/DurabilityService/NameSpaces/NameSpace/PartitionTopic of config file cannot be blank. The notation is 'partition.topic'");
              c_iterFree(result);
              result = NULL;
              noError = FALSE;
            }
        } else {
            ns = d_nameSpace(c_iterTakeFirst(result));

            while(ns){
                d_nameSpaceFree(ns);
                ns = d_nameSpace(c_iterTakeFirst(result));
            }
            c_iterFree(result);
            result = NULL;
            noError = FALSE;
        }
        u_cfElementFree(element);

        if(noError){
            element = (u_cfElement)c_iterTakeFirst(iter);
        }
    }
    /* Not all elements may have been taken from iter and freed in previous
     * loop. Ensuring clean-up here.
     */
    element = (u_cfElement)c_iterTakeFirst(iter);

    while(element){
        u_cfElementFree(element);
        element = (u_cfElement)c_iterTakeFirst(iter);
    }
    c_iterFree(iter);

    return result;
}


c_iter
d_configurationResolveFilters(
    d_configuration config,
    u_cfElement elementParent,
    const c_char *filterElement,
    d_durability durability)
{
    c_iter iter, result;
    u_cfElement element;
    c_bool noError = TRUE;
    c_char *xmlExpression = NULL;
    c_bool found;
    d_filter filter;

    OS_UNUSED_ARG(config);

    result = c_iterNew(NULL);
    iter = u_cfElementXPath(elementParent, filterElement);
    element = (u_cfElement)c_iterTakeFirst(iter);
    while (element && noError) {
        /* Parse filter expression attribute */
        xmlExpression = NULL;
        found = u_cfElementAttributeStringValue(element, "content", &xmlExpression);
        if (!found) {
            /* A filter without any content attribute is detected.
             * We use a strict parsing regime and consider a filter
             * without any content expression as an error.
             */
            OS_REPORT(OS_WARNING, D_CONTEXT, 0,
            "Missing 'content' attribute of <Filter>-element in the configuration, ignoring this filter");
        } else if (strlen(xmlExpression )== 0) {
            /* This is the empty string, print a warning and ignore */
            OS_REPORT(OS_WARNING, D_CONTEXT, 0,
            "The 'content' attribute of <Filter>-element in the configuration resolves to the empty string, ignoring this filter");
        } else {
            /* The filter expression has a valid syntax.
             * Create a filter and add the <Partition> and
             * <PartitionTopic> child elements
             */
            filter = d_filterNew(xmlExpression);
            if (filter) {
                if (d_configurationResolveFilterPartitionTopic(filter, element, "NoName", "PartitionTopic") == 0) {
                    OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                    "The <Filter>-element with content attribute '%s' has no matching <PartitionTopic>, ignoring this filter", xmlExpression);
                    d_filterFree(filter);
                } else {
                    result = c_iterAppend(result, filter);
                }
            } else {
                OS_REPORT(OS_ERROR, D_CONTEXT, 0,
                "Failed to create a filter for <Filter>-expression with content '%s', remove all filters and terminate", xmlExpression);
                noError = FALSE;
            }
        }
        if (noError) {
            element = (u_cfElement)c_iterTakeFirst(iter);
        }
    } /* while */
    if (noError == FALSE) {
        if (iter) {
            /* Clean up cfElements */
            element = (u_cfElement)c_iterTakeFirst(iter);
            while (element) {
                u_cfElementFree(element);
                element = (u_cfElement)c_iterTakeFirst(iter);
            }
            c_iterFree(iter);
            iter = NULL;
        }
        if (result) {
            /* Clean up filters */
            filter = d_filter(c_iterTakeFirst(result));
            while (filter) {
                d_filterFree(filter);
                filter = d_filter(c_iterTakeFirst(result));
            }
            c_iterFree(result);
            result = NULL;
        }
        /* Terminate durability */
        d_durabilityTerminate(durability, TRUE);
    }
    c_iterFree(iter);

    return result;
}



/**
 * \brief Parse the <PartitionTopic>-fields in a <Filter>-expression
 *
 * @return the number of successfully parsed expressions, or -1 in case of error
 */
int
d_configurationResolveFilterPartitionTopic(
    d_filter filter,
    u_cfElement elementParent,
    const c_char *name,
    const c_char *tag)
{
    c_iter iter, iter2;
    u_cfElement partitionTopicElement;
    char *partitionTopic;
    c_bool found;
    u_cfNode data;
    int i = 0;

    assert(d_filterIsValid(filter));

    /* Currently only support <Partitiontopic> */
    iter = u_cfElementXPath(elementParent, tag);
    partitionTopicElement = u_cfElement(c_iterTakeFirst(iter));
    while (partitionTopicElement) {
        iter2 = u_cfElementGetChildren(partitionTopicElement);
        if (c_iterLength(iter2) != 0) {
            data = u_cfNode(c_iterTakeFirst(iter2));
            if (u_cfNodeKind(data) == V_CFDATA) {
                found = u_cfDataStringValue(u_cfData(data), &partitionTopic);
                if (found == TRUE) {
                    if (d_filterAddElement(filter, name, partitionTopic, NULL)) {
                        i++;
                    }
                    os_free(partitionTopic);
                }
            }
            u_cfNodeFree(data);
        }
        c_iterFree(iter2);
        u_cfElementFree(partitionTopicElement);
        partitionTopicElement = u_cfElement(c_iterTakeFirst(iter));
    } /* while */
    c_iterFree(iter);
    return i;
}


c_iter
d_configurationResolveDurablePolicies(
    u_cfElement  elementParent,
    const c_char * policy)
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
        if (!found) {
            OS_REPORT(OS_WARNING, D_CONTEXT, 0,
                      "Missing 'obtain' attribute of //Opensplice/Domain/durablePolicies/Policy-element in the configuration, ignoring this setting");
        } else {
            /* Parse cache attribute */
            found = u_cfElementAttributeStringValue(element, "cache", &cache);
            if (found){
                isCache = (os_strcasecmp(cache, "TRUE") == 0) ? TRUE : FALSE;
                os_free(cache);
            } else {
                isCache = D_DEFAULT_DURABLE_POLICY_CACHE;
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

/**
 * \brief Find the first matching policy for the given namespace
 *
 * @return first matching policy, or NULL if not found
 */
d_policy
d_configurationFindPolicyForNameSpace(
    d_configuration config,
    const char * nameSpaceName)
{
    d_policy policy = NULL;
    c_iterIter iter;

    iter = c_iterIterGet(config->policies);
    while ((policy = (d_policy)c_iterNext(&iter)) != NULL) {
        if (d_patternMatch(nameSpaceName, policy->nameSpace)) {
            policy = d_policy(d_objectKeep(d_object(policy)));
            break;
        }
    }
    return policy;
}
