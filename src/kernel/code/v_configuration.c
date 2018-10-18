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
#include "v_configuration.h"
#include "v_public.h"
#include "v_cfAttribute.h"
#include "vortex_os.h"
#include "os_report.h"

static v_cfNode
v_configurationNodeResolveNode (
    v_cfNode node,
    c_ulong id);

v_configuration
v_configurationNew(
    v_kernel kernel)
{
    v_configuration config;

    config = v_configuration(v_objectNew(kernel,K_CONFIGURATION));
    v_publicInit(v_public(config));
    config->root = NULL;
    config->uri = NULL;
    config->idCounter = 0;

    return config;
}

void
v_configurationFree(
    v_configuration config)
{
    assert(config != NULL);
    assert(C_TYPECHECK(config, v_configuration));
    v_publicFree(v_public(config));
    c_free(config->uri);
    c_free(config->root);
    c_free(config);
}

void
v_configurationSetRoot(
    v_configuration config,
    v_cfElement root)
{
    assert(config != NULL);
    assert(C_TYPECHECK(config, v_configuration));
    assert(root != NULL);
    assert(C_TYPECHECK(root, v_cfElement));

    config->root = root;
}

v_cfElement
v_configurationGetRoot(
    v_configuration config)
{
    assert(config != NULL);
    assert(C_TYPECHECK(config, v_configuration));

    return c_keep(config->root);
}

void
v_configurationSetUri(
    v_configuration config,
    const c_char *uri)
{
    assert(config != NULL);
    assert(C_TYPECHECK(config, v_configuration));
    if (config->uri != NULL) {
        c_free(config->uri);

    }
    if (uri != NULL) {
        config->uri = c_stringNew(c_getBase(config), uri);
    } else {
        config->uri = NULL;
    }
}

const c_char *
v_configurationGetUri(
    v_configuration config)
{
    const c_char *result = NULL;
    if(config != NULL) {
        assert(C_TYPECHECK(config, v_configuration));
        result = config->uri;
    }
    return result;
}

c_ulong
v_configurationIdNew(
    v_configuration config)
{
    c_ulong result;

    assert(config != NULL);
    assert(C_TYPECHECK(config, v_configuration));

    result = config->idCounter;
    config->idCounter++;

    return result;
}

v_cfNode
v_configurationGetNode(
    v_configuration config,
    c_ulong id)
{
    v_cfNode node;

    assert(config != NULL);
    assert(C_TYPECHECK(config, v_configuration));

    node = v_cfNode(config->root);

    if(node->id != id){
        node = v_configurationNodeResolveNode(node, id);
    }
    return node;
}

static v_cfNode
v_configurationNodeResolveNode(
    v_cfNode node,
    c_ulong id)
{
    v_cfNode result, child;
    c_iter iter;

    result = NULL;

    switch(node->kind){
        case V_CFELEMENT:
        iter = v_cfElementGetChildren(v_cfElement(node));
        child = v_cfNode(c_iterTakeFirst(iter));

        while((child != NULL) && (result == NULL)){
            if(child->id == id){
                result = child;
            } else {
                result = v_configurationNodeResolveNode(child, id);
            }
            child = v_cfNode(c_iterTakeFirst(iter));
        }
        c_iterFree(iter);

        if(result == NULL){
            iter = v_cfElementGetAttributes(v_cfElement(node));
            child = v_cfNode(c_iterTakeFirst(iter));

            while((child != NULL) && (result == NULL)){
                if(child->id == id){
                    result = child;
                } else {
                    result = v_configurationNodeResolveNode(child, id);
                }
                child = v_cfNode(c_iterTakeFirst(iter));
            }
            c_iterFree(iter);
        }
        break;
        case V_CFDATA:
        case V_CFATTRIBUTE:
        default:
        break;
    }
    return result;
}

static v_cfData
ResolveParameter (
    v_cfElement e,
    const c_char* xpathExpr)
{
    v_cfData result;
    c_iter nodes;
    v_cfNode tmp;

    result = NULL;

    if(e != NULL){
        nodes = v_cfElementXPath(e, xpathExpr);
        tmp = v_cfNode(c_iterTakeFirst(nodes));
        if(tmp != NULL){
            if(v_cfNodeKind(tmp) == V_CFDATA){
                result = v_cfData(tmp);
            }
        }
        c_iterFree(nodes);
    }
    return result;
}

static v_cfAttribute
ResolveAttribute(
    v_cfElement e,
    const c_char* xpathExpr,
    const c_char* attrName)
{
    v_cfAttribute result;
    c_iter nodes;
    v_cfNode tmp;

    result = NULL;

    if (e != NULL) {
        nodes = v_cfElementXPath(e, xpathExpr);
        tmp = v_cfNode(c_iterTakeFirst(nodes));
        if (tmp != NULL) {
            if (v_cfNodeKind(tmp) == V_CFELEMENT) {
                result = v_cfElementAttribute(v_cfElement(tmp), attrName);
            }
        }
        c_iterFree(nodes);
    }
    return result;
}

#define V_WATCHDOG_TEXT         "/#text"
#define V_WATCHDOG_CLASS_FMT    "%s[@name='%s']/Watchdog/Scheduling/Class"
#define V_WATCHDOG_PRIO_FMT     "%s[@name='%s']/Watchdog/Scheduling/Priority"
#define V_WATCHDOG_ATTRKIND     "priority_kind"
#define V_GENERALWATCHDOG_CLASS_FMT    "%s/GeneralWatchdog/Scheduling/Class"
#define V_GENERALWATCHDOG_PRIO_FMT     "%s/GeneralWatchdog/Scheduling/Priority"
#define V_SPLICEDWATCHDOG_CLASS_FMT    "%s/Watchdog/Scheduling/Class"
#define V_SPLICEDWATCHDOG_PRIO_FMT     "%s/Watchdog/Scheduling/Priority"


static void
GetSchedulingPolicy (
    v_configuration config,
    const c_char* element,
    const c_char* name,
    v_schedulePolicyI *policy)
{
    c_char* path;
    c_value value;
    v_cfData data;
    v_cfAttribute attribute;
    const c_char* defaultElement = "Domain";

    path = os_malloc(strlen(V_WATCHDOG_CLASS_FMT V_WATCHDOG_TEXT) +
                     strlen(element) + strlen(name) + 1);
    if (path != NULL) {
        os_sprintf(path, V_WATCHDOG_CLASS_FMT V_WATCHDOG_TEXT, element, name);
        data = ResolveParameter(config->root, path);
        if (data == NULL) {
            os_free(path);
            path = os_malloc(strlen(V_GENERALWATCHDOG_CLASS_FMT V_WATCHDOG_TEXT) +
                             strlen(defaultElement) + 1);
            if (path != NULL) {
                os_sprintf(path, V_GENERALWATCHDOG_CLASS_FMT V_WATCHDOG_TEXT, defaultElement);
                data = ResolveParameter(config->root, path);
            }
        }
        if (data != NULL) {
            value = v_cfDataValue(data);
            if (value.kind == V_STRING) {
                if (strcmp(value.is.String, "Default")==0) {
                    policy->v.kind = V_SCHED_DEFAULT;
                } else if (strcmp(value.is.String, "Realtime")==0) {
                    policy->v.kind = V_SCHED_REALTIME;
                } else if (strcmp(value.is.String, "Timeshare")==0) {
                    policy->v.kind = V_SCHED_TIMESHARING;
                } else {
                    OS_REPORT(OS_WARNING, "v_configurationGetScedulingPolicy", V_RESULT_INTERNAL_ERROR,
                                "Invalid value for %s. Applying default.", path);
                }
            } else {
                OS_REPORT(OS_WARNING, "v_configurationGetScedulingPolicy", V_RESULT_INTERNAL_ERROR,
                            "Invalid value for %s. Applying default.", path);
            }
        }
        os_free(path);
    } else {
        OS_REPORT(OS_ERROR, "v_configurationGetScedulingPolicy", V_RESULT_INTERNAL_ERROR,
                    "Invalid resources to resolve value for %s. Applying default.", name);
    }
    path = os_malloc(strlen(V_WATCHDOG_PRIO_FMT V_WATCHDOG_TEXT) +
                     strlen(element) + strlen(name) + 1);
    if (path != NULL) {
        os_sprintf(path, V_WATCHDOG_PRIO_FMT V_WATCHDOG_TEXT, element, name);
        data = ResolveParameter(config->root, path);
        if (data == NULL) {
            os_free(path);
            path = os_malloc(strlen(V_GENERALWATCHDOG_PRIO_FMT V_WATCHDOG_TEXT) +
                             strlen(defaultElement) + 1);
            if (path != NULL) {
                os_sprintf(path, V_GENERALWATCHDOG_PRIO_FMT V_WATCHDOG_TEXT, defaultElement);
                data = ResolveParameter(config->root, path);
            }
        }
        if (data != NULL) {
            value = v_cfDataValue(data);
            if (value.kind == V_STRING) {
                (void)sscanf(value.is.String, "%d", &policy->v.priority);
            } else {
                OS_REPORT(OS_WARNING, "v_configurationGetScedulingPolicy", V_RESULT_INTERNAL_ERROR,
                            "Invalid value for %s", path);
            }
        }
        os_free(path);
    } else {
        OS_REPORT(OS_ERROR, "v_configurationGetScedulingPolicy", V_RESULT_INTERNAL_ERROR,
                    "Invalid resources to resolve value for %s. Applying default.", name);
    }
    path = os_malloc(strlen(V_WATCHDOG_PRIO_FMT) +
                     strlen(element) + strlen(name) + 1);
    if (path != NULL) {
        os_sprintf(path, V_WATCHDOG_PRIO_FMT, element, name);
        attribute = ResolveAttribute(config->root, path, V_WATCHDOG_ATTRKIND);
        if (attribute == NULL) {
            os_free(path);
            path = os_malloc(strlen(V_GENERALWATCHDOG_PRIO_FMT) +
                             strlen(defaultElement) + 1);
            if (path != NULL) {
                os_sprintf(path, V_GENERALWATCHDOG_PRIO_FMT, defaultElement);
                attribute = ResolveAttribute(config->root, path, V_WATCHDOG_ATTRKIND);
            }
        }
        if (attribute != NULL) {
            value = v_cfAttributeValue(attribute);
            if (value.kind == V_STRING) {
                if (strcmp(value.is.String, "Relative")==0) {
                    policy->v.priorityKind = V_SCHED_PRIO_RELATIVE;
                } else if (strcmp(value.is.String, "Absolute")==0) {
                    policy->v.priorityKind = V_SCHED_PRIO_ABSOLUTE;
                } else {
                    OS_REPORT(OS_WARNING, "v_configurationGetScedulingPolicy", V_RESULT_INTERNAL_ERROR,
                                "Invalid priority kind for %s", path);
                }
            } else {
                OS_REPORT(OS_WARNING, "v_configurationGetScedulingPolicy", V_RESULT_INTERNAL_ERROR,
                            "Invalid priority kind for %s", path);
            }
        }
        os_free(path);
    }
}

static void
GetSplicedSchedulingPolicy (
    v_configuration config,
    const c_char* element,
    const c_char* name,
    v_schedulePolicyI *policy)
{
    c_char* path;
    c_value value;
    v_cfData data;
    v_cfAttribute attribute;
    const c_char* defaultElement = "Domain";
    c_bool showDeprecationWarning = FALSE;

    path = os_malloc(strlen(V_SPLICEDWATCHDOG_CLASS_FMT V_WATCHDOG_TEXT) +
                     strlen(element) + 1);
    if (path != NULL) {
        os_sprintf(path, V_SPLICEDWATCHDOG_CLASS_FMT V_WATCHDOG_TEXT, element);
        data = ResolveParameter(config->root, path);
        if (data == NULL) {
            os_free(path);
            path = os_malloc(strlen(V_SPLICEDWATCHDOG_CLASS_FMT V_WATCHDOG_TEXT) +
                             strlen(defaultElement) + 1);
            if (path != NULL) {
                os_sprintf(path, V_SPLICEDWATCHDOG_CLASS_FMT V_WATCHDOG_TEXT, defaultElement);
                data = ResolveParameter(config->root, path);
                showDeprecationWarning = TRUE;
            }
        }
        if (data == NULL) {
            os_free(path);
            path = os_malloc(strlen(V_GENERALWATCHDOG_CLASS_FMT V_WATCHDOG_TEXT) +
                             strlen(defaultElement) + 1);
            if (path != NULL) {
                os_sprintf(path, V_GENERALWATCHDOG_CLASS_FMT V_WATCHDOG_TEXT, defaultElement);
                data = ResolveParameter(config->root, path);
            }
        }
        if (data != NULL) {
            value = v_cfDataValue(data);
            if (value.kind == V_STRING) {
                if (showDeprecationWarning) {
                    OS_REPORT(OS_WARNING, "Watchdog initialization", V_RESULT_OK,
                                "deprecated path '%s' for Domain Watchdog "
                                "please use: 'Domain/Daemon'.", path);
                }
                if (strcmp(value.is.String, "Default")==0) {
                    policy->v.kind = V_SCHED_DEFAULT;
                } else if (strcmp(value.is.String, "Realtime")==0) {
                    policy->v.kind = V_SCHED_REALTIME;
                } else if (strcmp(value.is.String, "Timeshare")==0) {
                    policy->v.kind = V_SCHED_TIMESHARING;
                } else {
                    OS_REPORT(OS_WARNING, "v_configuration::GetScedulingPolicy", V_RESULT_INTERNAL_ERROR,
                                "Invalid value for %s. Applying default.", path);
                }
            } else {
                OS_REPORT(OS_WARNING, "v_configuration::GetScedulingPolicy", V_RESULT_INTERNAL_ERROR,
                            "Invalid value for %s. Applying default.", path);
            }
        }
        os_free(path);
    } else {
        OS_REPORT(OS_ERROR, "v_configurationGetScedulingPolicy", V_RESULT_INTERNAL_ERROR,
                    "Invalid resources to resolve value for %s. Applying default.", name);
    }
    showDeprecationWarning = FALSE;
    path = os_malloc(strlen(V_WATCHDOG_PRIO_FMT V_WATCHDOG_TEXT) +
                     strlen(element) + strlen(name) + 1);
    if (path != NULL) {
        os_sprintf(path, V_WATCHDOG_PRIO_FMT V_WATCHDOG_TEXT, element, name);
        data = ResolveParameter(config->root, path);
        if (data == NULL) {
            os_free(path);
            path = os_malloc(strlen(V_SPLICEDWATCHDOG_PRIO_FMT V_WATCHDOG_TEXT) +
                             strlen(defaultElement) + 1);
            if (path != NULL) {
                os_sprintf(path, V_SPLICEDWATCHDOG_PRIO_FMT V_WATCHDOG_TEXT, defaultElement);
                data = ResolveParameter(config->root, path);
                showDeprecationWarning = TRUE;
            }
        }
        if (data == NULL) {
            os_free(path);
            path = os_malloc(strlen(V_GENERALWATCHDOG_PRIO_FMT V_WATCHDOG_TEXT) +
                                     strlen(defaultElement) + 1);
            if (path != NULL) {
                os_sprintf(path, V_GENERALWATCHDOG_PRIO_FMT V_WATCHDOG_TEXT, defaultElement);
                data = ResolveParameter(config->root, path);
            }
        }
        if (data != NULL) {
            value = v_cfDataValue(data);
            if (value.kind == V_STRING) {
                if (showDeprecationWarning) {
                    OS_REPORT(OS_WARNING, "Watchdog initialization", V_RESULT_OK,
                                "deprecated path '%s' for Domain Watchdog "
                                "please use: 'Domain/Daemon'.", path);
                }
                (void)sscanf(value.is.String, "%d", &policy->v.priority);
            } else {
                OS_REPORT(OS_WARNING, "v_configurationGetScedulingPolicy", V_RESULT_OK,
                            "Invalid value for %s", path);
            }
        }
        os_free(path);
    } else {
        OS_REPORT(OS_ERROR, "v_configurationGetScedulingPolicy", V_RESULT_INTERNAL_ERROR,
                    "Invalid resources to resolve value for %s. Applying default.", name);
    }
    showDeprecationWarning = FALSE;
    path = os_malloc(strlen(V_SPLICEDWATCHDOG_PRIO_FMT) + strlen(element) + 1);
    os_sprintf(path, V_SPLICEDWATCHDOG_PRIO_FMT, element);
    attribute = ResolveAttribute(config->root, path, V_WATCHDOG_ATTRKIND);
    if (attribute == NULL) {
        os_free(path);
        path = os_malloc(strlen(V_SPLICEDWATCHDOG_PRIO_FMT) + strlen(defaultElement) + 1);
        os_sprintf(path, V_SPLICEDWATCHDOG_PRIO_FMT, defaultElement);
        attribute = ResolveAttribute(config->root, path, V_WATCHDOG_ATTRKIND);
        showDeprecationWarning = TRUE;
    }
    if (attribute == NULL) {
        os_free(path);
        path = os_malloc(strlen(V_GENERALWATCHDOG_PRIO_FMT) + strlen(defaultElement) + 1);
        os_sprintf(path, V_GENERALWATCHDOG_PRIO_FMT, defaultElement);
        attribute = ResolveAttribute(config->root, path, V_WATCHDOG_ATTRKIND);
        showDeprecationWarning = TRUE;
    }
    if (attribute != NULL) {
        value = v_cfAttributeValue(attribute);
        if (value.kind == V_STRING) {
            if (showDeprecationWarning) {
                OS_REPORT(OS_WARNING, "Watchdog initialization", V_RESULT_OK,
                          "deprecated path '%s' for 'priority_kind' in Domain Watchdog "
                          "please use: 'Domain/Daemon'.", path);
            }
            if (strcmp(value.is.String, "Relative")==0) {
                policy->v.priorityKind = V_SCHED_PRIO_RELATIVE;
            } else if (strcmp(value.is.String, "Absolute")==0) {
                policy->v.priorityKind = V_SCHED_PRIO_ABSOLUTE;
            } else {
                OS_REPORT(OS_WARNING, "v_configurationGetScedulingPolicy", V_RESULT_OK,
                          "Invalid priority kind for %s", path);
            }
        } else {
            OS_REPORT(OS_WARNING, "v_configurationGetScedulingPolicy", V_RESULT_OK,
                      "Invalid priority kind for %s", path);
        }
    }
    os_free(path);
}

void
v_configurationGetSchedulingPolicy (
    v_configuration config,
    const c_char* element,
    const c_char* name,
    v_schedulePolicyI *policy)
{
    if ((config != NULL) && (element != NULL) && (name != NULL) && (policy != NULL)) {
        if (strcmp(element, "Domain/Daemon") == 0) {
            GetSplicedSchedulingPolicy (config, element, name, policy);
        } else {
            GetSchedulingPolicy (config, element, name, policy);
        }
    } else {
        OS_REPORT(OS_INFO, "v_configurationGetScedulingPolicy", V_RESULT_OK,
                    "No watchdog configuration information specified for process \"%s\".\n",
                     name);
    }
}


static c_bool
v_configurationScanBoolean(
    c_char *value)
{
    c_bool result = FALSE;
    size_t l;

    l = strspn(value, " \t\n");
    if (l <= strlen(value)) {
        if (os_strncasecmp(&value[l], "TRUE", 4) == 0) {
            result = TRUE;
        }
    }

    return result;
}

#define V_SERVICE_ENTRY_FMT         "Domain/Service[@name='%s']"
#define V_SERVICE_ATTR_NAME         "name"
#define V_SERVICE_ATTR_ENABLED      "enabled"

static c_bool
v_configurationServiceIsEnabled(
    v_configuration config,
    const c_char *serviceName)
{
    c_bool enabled = FALSE;
    v_cfAttribute attr;
    c_char *path;
    c_value value;

    path = os_malloc(strlen(V_SERVICE_ENTRY_FMT) + strlen(serviceName) + 1);
    os_sprintf(path, V_SERVICE_ENTRY_FMT, serviceName);
    attr = ResolveAttribute(config->root, path, V_SERVICE_ATTR_ENABLED);
    if (attr) {
        value = v_cfAttributeValue(attr);
        if (value.kind == V_STRING) {
            enabled = v_configurationScanBoolean(value.is.String);
        }
    } else {
        enabled = TRUE;
    }
    os_free(path);

    return enabled;
}

c_bool
v_configurationContainsService(
    v_configuration config,
    const char *serviceName)
{
    c_bool found = FALSE;
    c_iter nodes;
    v_cfNode node;
    c_value value;

    nodes = v_cfElementXPath(config->root, serviceName);
    node = v_cfNode(c_iterTakeFirst(nodes));
    while (!found && node) {
        if (v_cfNodeKind(node) == V_CFELEMENT) {
            value = v_cfElementAttributeValue(v_cfElement(node), V_SERVICE_ATTR_NAME);
            if (value.kind == V_STRING) {
                found = v_configurationServiceIsEnabled(config, value.is.String);
            }
        }
        node = v_cfNode(c_iterTakeFirst(nodes));
    }

    c_iterFree(nodes);

    return found;
}
