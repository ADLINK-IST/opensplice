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

#include "cf_config.h"
#include "vortex_os.h"
#include "os_report.h"
#include "os_init.h"
#include "c_iterator.h"

#include "cfg_metaConfig.h"
#include "cfg_metaConfigParser.h"
#include "cfg_validator.h"

#define CONFIG_SYNTAX_FILE_NAME "ospl_metaconfig.xml"
#define CONFIG_SYNTAX_FILE_NAME_PREFIX  "splice_metaconfig_"
#define CONFIG_SYNTAX_FILE_NAME_POSTFIX ".1.xml"
#define CONFIG_VALIDATION_DISABLED "OSPL_CONFIG_VALIDATION_DISABLED"


#define NAME_BUFFER_MAX_SIZE 256


C_CLASS(cfg_checkContext);
C_STRUCT(cfg_checkContext) {
    cfg_element root;
    cfg_element services;
    cfg_node current;
    os_boolean valid;
    os_int32 domainId;
};


/* Check the if the configured attribute is allowed and if the provided
 * attribute value is correct.
 * A configured attribute is allowed when it can be found in the corresponding
 * meta configuration element.
 * When the attribute is allowed the value of the attribute is checked.
 * For attributes which relate to numbers, e.g. attributeInt, attributeLong, etc
 * it is checked if the provided value is between the minimum and maximum bounds.
 * For an attributeEnum attribute it is checked if the provided value is a valid label.
 * For an attributeString attribute the length of the provided value is checked against the
 * maximum allowed length of the string.
 */
static void
checkAttribute(
    cf_attribute attribute,
    cfg_checkContext context)
{
    cfg_attribute found;
    char buffer[NAME_BUFFER_MAX_SIZE];

    found = cfg_elementFindAttribute(cfg_element(context->current), cf_nodeGetName(cf_node(attribute)));
    if (found) {
        cfg_nodeIncOccurrences(cfg_node(found));
        if (!cfg_attributeCheckValue(found, cf_attributeValue(attribute).is.String)) {
            char *s;
            context->valid = OS_FALSE;
            s = c_valueImage(cf_attributeValue(attribute));
            OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                      "Element '%s': attribute '%s' incorrect value '%s'",
                      cfg_nodeGetFullName(context->current, buffer, sizeof(buffer)),
                      cf_nodeGetName(cf_node(attribute)), s);
            os_free(s);
        }
    } else {
        context->valid = OS_FALSE;
        OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                  "Element '%s': attribute '%s' not allowed",
                  cfg_nodeGetFullName(context->current, buffer, sizeof(buffer)),
                  cf_nodeGetName(cf_node(attribute)));
    }
}

/* Check if the provided value of a leaf element is correct.
 * For leaf element which relate to numbers, e.g. leafInt, leafLong, etc
 * it is checked if the provided value is between the minimum and maximum bounds.
 * For an leafEnum it is checked if the provided value is a valid label.
 * For an leafString attribute the length of the provided value is checked against the
 * maximum allowed length of the string.
 */
static void
checkData(
    cf_data data,
    cfg_checkContext context)
{
    char *s;
    char buffer[NAME_BUFFER_MAX_SIZE];

    if (!cfg_elementCheckValue(cfg_element(context->current), cf_dataValue(data).is.String)) {
        context->valid = OS_FALSE;
        s = c_valueImage(cf_dataValue(data));
        OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                  "Element '%s' incorrect value '%s'",
                  cfg_nodeGetFullName(context->current, buffer, sizeof(buffer)), s);
        os_free(s);
    }
}

static void
checkElement(
    cf_element element,
    cfg_checkContext context);


/* Check if a configured element, attribute or data value is valid
 */
static void
checkNode(
    cf_node node,
    cfg_checkContext context)
{
    switch (node->kind) {
    case CF_NODE:
        assert(OS_FALSE);
        break;
    case CF_ATTRIBUTE:
        checkAttribute(cf_attribute(node), context);
        break;
    case CF_ELEMENT:
        checkElement(cf_element(node), context);
        break;
    case CF_DATA:
        checkData(cf_data(node), context);
        break;
    case CF_COUNT:
        assert(OS_FALSE);
    }
}

/* Check if all the required attributes of an configuration element
 * are present. The occurrence count associated with each meta attribute
 * is used for this purpose. Each time an attribute is encountered in the
 * configuration file the occurrence count of that the corresponding
 * meta attribute is incremented. When the close tag of a configuration
 * element is seen the all the required attributes of that element should
 * have to occurrence count set to 1.
 */
static void
checkRequiredAttributes(
    cfg_checkContext context)
{
    c_iter attributes;
    c_iterIter it;
    cfg_attribute attribute;
    char buffer[NAME_BUFFER_MAX_SIZE];

    attributes = cfg_elementGetAttributes(cfg_element(context->current));
    if (!attributes) {
        return;
    }

    it = c_iterIterGet(attributes);
    while ((attribute = c_iterNext(&it)) != NULL ) {
        if (cfg_nodeGetOccurrences(cfg_node(attribute)) > 1) {
            context->valid = OS_FALSE;
            OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                      "Element '%s' attribute '%s' specified multiple times",
                      cfg_nodeGetFullName(context->current, buffer, sizeof(buffer)),
                      cfg_nodeGetName(cfg_node(attribute)));
        } else if (cfg_attributeIsRequired(attribute) && (cfg_nodeGetOccurrences(cfg_node(attribute)) == 0)) {
            context->valid = OS_FALSE;
            OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                      "Element '%s' required attribute '%s' is missing",
                      cfg_nodeGetFullName(context->current, buffer, sizeof(buffer)),
                      cfg_nodeGetName(cfg_node(attribute)));
        }
        cfg_nodeResetOccurrences(cfg_node(attribute));
    }
}


/* Check if the occurrences of all the elements that are children of an elements
 * correspond to the minimum and maximum number of occurences allowed for these
 * elements.
 * The occurrence count associated with each meta element
 * is used for this purpose. Each time an element is encountered in the
 * configuration file the occurrence count of that the corresponding
 * meta element is incremented. When the close tag of a configuration
 * element is seen the all the child elements of that element should
 * have to occurrence count within the bounds provided by the
 * minOccurrences and maxOccurrences value specified in the meta configuration
 * file.
 */
static void
checkOccurrences(
    cfg_element element,
    cfg_checkContext context)
{
    c_iter children;
    c_iterIter it;
    cfg_element child;
    char buffer[NAME_BUFFER_MAX_SIZE];

    children = cfg_elementGetChildren(element);
    if (!children) {
        return;
    }

    it = c_iterIterGet(children);
    while ((child = c_iterNext(&it)) != NULL ) {
        os_uint32 count = cfg_nodeGetOccurrences(cfg_node(child));
        if (count < cfg_elementGetMinOccurrences(child)) {
            context->valid = OS_FALSE;
            OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                      "Required element '%s' is missing",
                      cfg_nodeGetFullName(cfg_node(child), buffer, sizeof(buffer)));
        } else if (count > cfg_elementGetMaxOccurrences(child)) {
            context->valid = OS_FALSE;
            OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                      "Element '%s' is specified more than '%u' times",
                      cfg_nodeGetFullName(cfg_node(child), buffer, sizeof(buffer)),
                      cfg_elementGetMaxOccurrences(child));
        }
        cfg_nodeResetOccurrences(cfg_node(child));
    }
}

/* Check the validity of a configuration element.
 * First check if the element is allowed by checking if the element can be
 * found in the context of the current meta configuration element.
 * Next check the validity of all the attributes specified.
 * Next check if all the required attributes are present.
 * Next check the validity of all the child element specified.
 * Next check it the occurrences of all the child element corresponds to
 * the minimum and maximum occurrences that are allowed for that element.
 */
static void
checkElement(
    cf_element element,
    cfg_checkContext context)
{
    c_iter attributes;
    c_iter children;
    cf_attribute attribute;
    cf_node child;
    cfg_element found = NULL;
    char buffer[NAME_BUFFER_MAX_SIZE];

    if (context->current) {
       found = cfg_elementFindChild(cfg_element(context->current), cf_nodeGetName(cf_node(element)));
       if (!found) {
           context->valid = OS_FALSE;
           OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                     "For element '%s' the child element '%s' is not allowed",
                     cfg_nodeGetFullName(context->current, buffer, sizeof(buffer)),
                     cf_nodeGetName(cf_node(element)));
       }
    } else {
        found = context->root;
        if (strcmp(cfg_nodeGetName(cfg_node(found)), cf_nodeGetName(cf_node(element))) != 0) {
            context->valid = OS_FALSE;
            OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                      "Root element missing expected '%s' but found '%s'",
                      cfg_nodeGetName(cfg_node(found)),
                      cf_nodeGetName(cf_node(element)));
        }
    }

    if (found) {
        context->current = cfg_node(found);
        cfg_nodeIncOccurrences(context->current);

        attributes = cf_elementGetAttributes(element);

        attribute = c_iterTakeFirst(attributes);
        while (attribute) {
            checkAttribute(attribute, context);
            attribute = c_iterTakeFirst(attributes);
        }
        c_iterFree(attributes);

        checkRequiredAttributes(context);

        children = cf_elementGetChilds(element);

        child = c_iterTakeFirst(children);
        while (child) {
            checkNode(child, context);
            child = c_iterTakeFirst(children);
        }
        c_iterFree(children);

        checkOccurrences(found, context);

        context->current = cfg_nodeGetParent(context->current);
    }
}

/* Retrieve the data value associated with a XML element
 *
 */
static char *
getElementData(
    cf_element el)
{
    cf_node node;
    char *data = NULL;

    node = cf_elementChild(el, "#text");
    if (node && (cf_nodeKind(node) == CF_DATA)) {
        c_value v = cf_dataValue(cf_data(node));
        assert(v.kind == V_STRING);
        if (v.kind == V_STRING) {
            data = os_strdup(v.is.String);
        }
    }

    return data;
}

/* Retrieve the attribute value associated with a XML attribute
 *
 */
static char *
getAttributeValue(
    cf_element element,
    const char *name)
{
    cf_attribute attr;
    char *value = NULL;

    assert(element);
    assert(name);

    attr = cf_elementAttribute(element, name);
    if (attr) {
        c_value v = cf_attributeValue(attr);
        assert(v.kind == V_STRING);
        if (v.kind == V_STRING) {
            value = os_strdup(v.is.String);
        }
    }
    return value;
}

/* The ServiceEntry is used to relate a particular ServiceMapping
 * with the name of the service.
 *
 * The Services listed under the Domain tag have an name attribute and a
 * command. Each specific command relates to a particular service.
 * For example:
 * - the command "durability" refers to the DurabilityService.
 * - the command "ddsi2" refers to the DDSI2Service
 * - the command "ddsi2e" refers to the DDSI2EService.
 *
 * To lists are build.
 * - the listed services which are the service entries found under the Domain tag.
 *   For example when the Domain contains an entry for a service where the command
 *   is "cmsoap" then the corresponding serviceMapping (TunerService) is added to
 *   the listed services list.
 * - the configured services which contains entries for the corresponding service
 *   configurations. For example when the configuration contains an entry for
 *   the NetworkService the corresponding serviceMapping is looked up and together
 *   with it's name added to the configured services list.
 *
 * When valid both lists should contain the same services.
 */
C_CLASS(ServiceEntry);
C_STRUCT(ServiceEntry) {
    cfg_serviceMapping mapping;
    char *name;
    os_boolean enabled;
};

static ServiceEntry
serviceEntryNew(
    const cfg_serviceMapping mapping,
    const char *name,
    os_boolean enabled)
{
    ServiceEntry entry = os_malloc(C_SIZEOF(ServiceEntry));

    entry->mapping = mapping;
    entry->name = os_strdup(name);
    entry->enabled = enabled;

    return entry;
}

static void
serviceEntryFree(
    ServiceEntry _this)
{
    if (_this) {
        os_free(_this->name);
        os_free(_this);
    }
}

static c_equality
serviceEntryCompare(
    c_voidp o,
    c_iterResolveCompareArg arg)
{
    c_equality r = C_NE;
    ServiceEntry e1 = o;
    ServiceEntry e2 = arg;

    if ((e1->mapping == e2->mapping) &&
        (strcmp(e1->name, e2->name) == 0)) {
        r = C_EQ;
    }

    return r;
}

/* Find a serviceMapping in the meta configuration by the command string.
 *
 */
static cfg_serviceMapping
findServiceMappingByCommand(
    cfg_element services,
    const char *command)
{
    cfg_serviceMapping mapping = NULL;
    cfg_serviceMapping item;
    c_iter list;
    c_iterIter iter;
    char *ptr, *normalizedCmd;

    assert(services);

    /* For matching, the basename of command without suffix is required */
    normalizedCmd = os_fileNormalize(command);
    ptr = os_rindex(normalizedCmd, '.');
    if (ptr) {
        *ptr = '\0';
    }

    ptr = os_rindex(normalizedCmd, OS_FILESEPCHAR);
    if (ptr) {
        ptr++;
    } else {
        ptr = normalizedCmd;
    }

    list = cfg_elementGetChildren(services);
    if (list) {
        iter = c_iterIterGet(list);
        item = c_iterNext(&iter);
        while (item && !mapping) {
            const char *cmd = cfg_serviceMappingGetCommand(item);
            if (cmd && ptr) {
                if (strcmp(cmd, ptr) == 0) {
                    mapping = item;
                } else {
                    item = c_iterNext(&iter);
                }
            }
        }
    }

    os_free(normalizedCmd);

    return mapping;
}

/* Return the list of all children which have the specified name
 *
 */
static c_iter
findChildrenByName(
    cf_element element,
    const char *name)
{
    c_iter list = NULL;
    c_iter children;
    cf_node child;

    children = cf_elementGetChilds(element);
    child = c_iterTakeFirst(children);
    while (child) {
        if (strcmp(cf_nodeGetName(child), name) == 0) {
            list = c_iterInsert(list, child);
        }
        child = c_iterTakeFirst(children);
    }
    c_iterFree(children);

    return list;
}

/* Find the a boolean attribute and return it's value.
 * When the attribute exists and the attribute value is
 * 'true' return OS_TRUE, otherwise return OS_FALSE.
 * When the attribute does not exists then return the
 * specified default value.
 */
static os_boolean
getBooleanAttributeValue(
    cf_element element,
    const char *name,
    os_boolean defval)
{
    os_boolean result = defval;
    cf_attribute attribute;

    attribute = cf_elementAttribute(element, name);
    if (attribute) {
        c_value v = cf_attributeValue(attribute);
        char *value = cfg_stringValue(v);
        if (value && (os_strcasecmp(value, "true") == 0)) {
            result = OS_TRUE;
        } else {
            result = OS_FALSE;
        }
    }

    return result;
}

static cfg_serviceMapping
validateServiceCommand(
    cf_element configRoot,
    const char *name,
    const char *command,
    cfg_checkContext context)
{
    cfg_serviceMapping mapping = NULL;
    c_iter list;
    c_iterIter iter;
    c_bool cont = TRUE;
    char *fullname;

    cfg_serviceMapping result = NULL;

    assert(configRoot);
    assert(name);
    
    fullname = os_locate(command, OS_ROK | OS_XOK);
    if (!fullname) {
        cont = FALSE;
    }
    os_free(fullname);    

    list = cfg_elementGetChildren(context->services);
    if (list) {
        iter = c_iterIterGet(list);
        while (cont && (mapping = cfg_serviceMapping(c_iterNext(&iter))) != NULL) {
            c_iter nodeList = findChildrenByName(configRoot, cfg_nodeGetName(cfg_node(mapping)));
            cf_node node = c_iterTakeFirst(nodeList);
            while (node && cont) {
                if (cf_nodeKind(node) == CF_ELEMENT) {
                    char *foundName = getAttributeValue(cf_element(node), "name");
                    if (foundName) {
                        if (!strcmp(name, foundName)) {
                            cont = FALSE;
                            result = mapping;
                            OS_REPORT_WID(OS_INFO, "configuration validator", 0, context->domainId,
                                              "Element <Service name=\"%s\"><Command>%s<Command></Service> command '%s' is non-default for service %s but executable, accepting it as valid command",
                                              name, command, command, foundName);
                            
                        }
                        os_free(foundName);
                    }
                }
                node = c_iterTakeFirst(nodeList);
            }
            c_iterFree(nodeList);
        }
    }
    

    return result;

}


/* Get the list of configured services.
 * The meta configuration contains the list of serviceMappings
 * Walk this list of serviceMappings to find a corresponding
 * service configuration. When found add the related serviceMapping
 * and the name of the configured service to the result list.
 * For example when the configuration file contains an entry for the
 * NetworkService, e,g. <NetworkService name="networking"> then the
 * corresponding serviceMapping from the meta information is associated
 * with the name and added to the list of configured services.
 */
static c_iter
getConfiguredServices(
    cf_element configRoot,
    cfg_checkContext context)
{
    c_iter list;
    c_iterIter iter;
    cfg_serviceMapping mapping;
    c_iter result = NULL;

    list = cfg_elementGetChildren(context->services);
    if (list) {
        iter = c_iterIterGet(list);
        while ((mapping = cfg_serviceMapping(c_iterNext(&iter))) != NULL) {
            c_iter nodeList = findChildrenByName(configRoot, cfg_nodeGetName(cfg_node(mapping)));
            cf_node node = c_iterTakeFirst(nodeList);
            while (node) {
                if (cf_nodeKind(node) == CF_ELEMENT) {
                    char *name = getAttributeValue(cf_element(node), "name");
                    if (name) {
                        ServiceEntry entry = serviceEntryNew(mapping, name, OS_TRUE);
                        result = c_iterAppend(result, entry);
                    }
                    os_free(name);
                }
                node = c_iterTakeFirst(nodeList);
            }
            c_iterFree(nodeList);
        }
    }

    return result;
}

/* Get the list of listed services.
 * For each service element found under Domain the corresponding
 * serviceMapping is looked up in the meta information by relating
 * the specified command.
 * For example when the configuration file contains an entry for
 * the durability service. e.g.
 *  <Service name="durability"><Command>durability</Command></Service>
 * then the corresponding serviceMapping from the meta information is
 * looked up by the name of the command (durability) and the relation
 * between the found serviceMapping and the specified name is added
 * to the list of listed services.
 */
static c_iter
getListedServices(
    cf_element configRoot,
    cfg_checkContext context)
{
    c_iter list = NULL;
    cf_element domain;
    c_iter children;
    cf_node child;
    cf_node node;
    char *name;
    char *command;
    os_boolean enabled;
    cfg_serviceMapping mapping;
    ServiceEntry entry;

    domain = cf_element(cf_elementChild(configRoot, CFG_DOMAIN));
    if (domain) {
        children = cf_elementGetChilds(domain);
        child = c_iterTakeFirst(children);
        while (child) {
            if (cf_nodeKind(child) == CF_ELEMENT) {
                if (strcmp(cf_nodeGetName(child), CFG_SERVICE) == 0) {
                    name = getAttributeValue(cf_element(child), "name");
                    if (name) {
                        enabled = getBooleanAttributeValue(cf_element(child), "enabled", OS_TRUE);
                        node = cf_elementChild(cf_element(child), CFG_COMMAND);
                        if (node && (cf_nodeKind(node) == CF_ELEMENT)) {
                            command = getElementData(cf_element(node));
                            if (command) {
                                mapping = findServiceMappingByCommand(context->services, command);
                                if (!mapping) {
                                    mapping = validateServiceCommand(configRoot, name, command, context);
                                }
                                if (mapping) {
                                    entry = serviceEntryNew(mapping, name, enabled);
                                    list = c_iterAppend(list, entry);
                                } else {
                                    context->valid = OS_FALSE;
                                    OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                                              "Element <Service name=\"%s\"><Command>%s<Command></Service> command '%s' is incorrect",
                                              name, command, command);
                                }
                                os_free(command);
                            } else {
                                context->valid = OS_FALSE;
                                OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                                          "Element <Service name=\"%s\"><Command><Command></Service> missing command value",
                                          name);
                            }
                        } else {
                            /* Error already logged */
                            context->valid = OS_FALSE;
                        }
                        os_free(name);
                    } else {
                        /* Error already logged */
                        context->valid = OS_FALSE;
                    }
                }
            }
            child = c_iterTakeFirst(children);
        }
        c_iterFree(children);
    }

    return list;
}


static void
checkServices(
    cf_element configRoot,
    cfg_checkContext context)
{
    c_iter configuredServices;
    c_iter listedServices;
    ServiceEntry ce, le;

    configuredServices = getConfiguredServices(configRoot, context);
    listedServices = getListedServices(configRoot, context);

    if (listedServices && configuredServices) {
        ce = c_iterTakeFirst(configuredServices);
        while (ce) {
            le = c_iterResolve(listedServices, serviceEntryCompare, ce);
            if (le) {
                c_iterTake(listedServices, le);
                serviceEntryFree(le);
            } else {
                context->valid = OS_FALSE;
                OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                          "No matching service with name=\"%s\" found under Domain",
                          ce->name);
            }
            serviceEntryFree(ce);
            ce = c_iterTakeFirst(configuredServices);
        }
        le = c_iterTakeFirst(listedServices);
        while (le) {
            if (le->enabled) {
                context->valid = OS_FALSE;
                OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                        "No configuration found for the <Service name=\"%s\">",
                        le->name);
            }
            serviceEntryFree(le);
            le = c_iterTakeFirst(listedServices);
        }
    } else if (listedServices) {
        le = c_iterTakeFirst(listedServices);
        while (le) {
            if (le->enabled) {
                context->valid = OS_FALSE;
                OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                        "No configuration found for the <Service name=\"%s\">",
                        le->name);
            }
            serviceEntryFree(le);
            le = c_iterTakeFirst(listedServices);
        }
    } else if (configuredServices) {
        context->valid = OS_FALSE;
        ce = c_iterTakeFirst(configuredServices);
        while (ce) {
            OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context->domainId,
                      "No matching service with name=\"%s\" found under Domain",
                      ce->name);
            serviceEntryFree(ce);
            ce = c_iterTakeFirst(configuredServices);
        }
    }

    if (configuredServices) {
        ce = c_iterTakeFirst(configuredServices);
        while (ce) {
            serviceEntryFree(ce);
            ce = c_iterTakeFirst(configuredServices);
        }
        c_iterFree(configuredServices);
    }

    if (listedServices) {
        le = c_iterTakeFirst(listedServices);
        while (le) {
            serviceEntryFree(le);
            le = c_iterTakeFirst(listedServices);
        }
        c_iterFree(listedServices);
    }
}

static os_boolean
validateConfigurationDisabled(void)
{
    const char *str;
    os_boolean disabled = OS_FALSE;

    str = os_getenv(CONFIG_VALIDATION_DISABLED);
    if (str) {
        if (os_strcasecmp(str, "true") == 0) {
            disabled = OS_TRUE;
        }
    }

    return disabled;
}

/* This function returns the absolute path of the meta config file in the development tree.
 * This alternative location is only applicable for internal use in the development environment.
 * In normal installed environments the meta config file is located at $OSPL_HOME/etc or
 * in the current directory in case a user wants to override the meta configuration file.
 * The development location is $OSPL_HOME/src/tools.cm/config/code
 */
static char *
alternativeMetaConfigFileName(void)
{
    const char *osplhome;
    const char *dirs[] = {"src", "tools", "cm", "config", "code"};
    const char *fs = os_fileSep();
    const char *version = os_versionString();
    char *filename;
    os_size_t i, fl, len, n;

    osplhome = os_getenv("OSPL_HOME_NORMALIZED");
    if (!osplhome) {
        osplhome = os_getenv("OSPL_HOME");
    }

    if (!osplhome) {
        return NULL;
    }

    fl = strlen(fs);
    len = strlen(osplhome) + fl;

    for (i = 0; i < sizeof(dirs)/sizeof(char *); i++) {
        len += strlen(dirs[i]) + fl;
    }

    len += strlen(CONFIG_SYNTAX_FILE_NAME_PREFIX) + strlen(CONFIG_SYNTAX_FILE_NAME_POSTFIX) + 2;

    filename = os_malloc(len);
    n = (os_size_t)snprintf(filename, len, "%s%s", osplhome, fs);
    for (i = 0; i < sizeof(dirs)/sizeof(char *); i++) {
        n += (os_size_t)snprintf(&filename[n], len-n, "%s%s", dirs[i], fs);
    }

    n += (os_size_t)snprintf(&filename[n], len-n, "%s%c%s", CONFIG_SYNTAX_FILE_NAME_PREFIX, version[0], CONFIG_SYNTAX_FILE_NAME_POSTFIX);

    return filename;
}

static char *
defaultMetaConfigFileName(void)
{
    const char *osplhome;
    const char *dir = "etc";
    const char *fs = os_fileSep();
    char *filename;
    os_size_t fl, len;

    osplhome = os_getenv("OSPL_HOME");
    if (!osplhome) {
        return NULL;
    }

    fl = strlen(fs);
    len = strlen(osplhome) + fl + strlen(dir) + fl + strlen(CONFIG_SYNTAX_FILE_NAME) + 1;

    filename = os_malloc(len);

    (void)snprintf(filename, len, "%s%s%s%s%s", osplhome, fs, dir, fs, CONFIG_SYNTAX_FILE_NAME);

    return filename;
}

cfgprs_status
cfg_validateConfiguration(
    cf_element configRoot)
{
    cfgprs_status status = CFGPRS_OK;
    C_STRUCT(cfg_checkContext) context = {NULL, NULL, NULL, OS_FALSE, 0};
    char *syntaxFileName;
    cfg_element syntaxRoot = NULL;
    cfg_element serviceMapping = NULL;
    FILE *fp = NULL;

    if (validateConfigurationDisabled()) {
        return CFGPRS_OK;
    }

    if (cfg_determineDomainId(configRoot, &context.domainId) != CFGPRS_OK) {
        context.domainId = -1;
    }

    /* First check for a local config file in current directory */
    syntaxFileName = os_strdup(CONFIG_SYNTAX_FILE_NAME);
    fp = fopen(syntaxFileName, "r");

    /* If no local config file then use the default OSPL_HOME/etc/ospl_metaconfig.xml config file. */
    if (!fp) {
        os_free(syntaxFileName);
        syntaxFileName = defaultMetaConfigFileName();
        if (syntaxFileName) {
            fp = fopen(syntaxFileName, "r");
        }
        /* If also no default config file then look for alternative.
         * This alternative location is only applicable for internal use in the development environment.
         */
        if (!fp) {
            char *alternativeFileName;
            alternativeFileName = alternativeMetaConfigFileName();
            if (alternativeFileName) {
                fp = fopen(alternativeFileName, "r");
                os_free(alternativeFileName);
            }
        }
        if (!fp) {
            const char *osplhome = os_getenv("OSPL_HOME");
            if (osplhome) {
                OS_REPORT_NOW(OS_ERROR, "configuration validator", 0, context.domainId,
                          "Failed to open meta configuration file \"%s\".\n"
                          "              The file was not found in the current directory nor\n"
                          "              at the default location: %s%setc",
                          CONFIG_SYNTAX_FILE_NAME, osplhome, os_fileSep());
            } else {
                OS_REPORT_NOW(OS_ERROR, "configuration validator", 0, context.domainId,
                          "Failed to open meta configuration file \"%s\".\n"
                          "              The file was not found in the current directory nor\n"
                          "              could the file be found at the default location because\n"
                          "              the environment variable OSPL_HOME was not set",
                          CONFIG_SYNTAX_FILE_NAME);
            }
        }
    }

    if (fp) {
        status = cfg_parseMetaConfig(fp, &syntaxRoot, &serviceMapping);
        if (status == CFGPRS_OK) {
            context.root = syntaxRoot;
            context.services = serviceMapping;
            context.valid = OS_TRUE;
            context.current = NULL;
            checkElement(configRoot, &context);
            checkServices(configRoot, &context);

            status = context.valid ? CFGPRS_OK : CFGPRS_ERROR;

        } else {
            OS_REPORT_WID(OS_ERROR, "configuration validator", 0, context.domainId,
                          "Meta configuration parse error(s) in file: %s", syntaxFileName);
        }
        cfg_nodeFree(cfg_node(syntaxRoot));
        cfg_nodeFree(cfg_node(serviceMapping));
        (void)fclose(fp);
    } else {
        status = CFGPRS_OK;
    }

    os_free(syntaxFileName);

    return status;
}


cfgprs_status
cfg_validateConfigurationByUri(
    const char *uri)
{
    cfgprs_status status = CFGPRS_OK;
    cf_element configRoot = NULL;

    if (validateConfigurationDisabled()) {
        return CFGPRS_OK;
    }

    status = cfg_parse_ospl(uri, &configRoot);
    if (status == CFGPRS_OK) {
        status = cfg_validateConfiguration(configRoot);
    } else {
        OS_REPORT(OS_ERROR, "configuration validator", 0,
                  "Failed to parse configuration file: %s", uri);
    }

    if (configRoot) {
        cf_elementFree(configRoot);
    }

    return status;
}

cfgprs_status
cfg_determineDomainId(
    cf_element configRoot,
    os_int32 *domainId)
{
    cfgprs_status status = CFGPRS_ERROR;
    cf_node domain;

    *domainId = -1;

    domain = cf_elementChild(configRoot, CFG_DOMAIN);
    if (domain && (cf_nodeKind(domain) == CF_ELEMENT)) {
        cf_node id = cf_elementChild(cf_element(domain), CFG_ID);
        if (id && (cf_nodeKind(id) == CF_ELEMENT)) {
            char *value = getElementData(cf_element(id));
            if (value) {
                char *endptr;
                *domainId = (os_int32)strtol (value, &endptr, 0);
                if ((*endptr == '\0') && (*domainId >= 0) && (*domainId <= 230)) {
                    status = CFGPRS_OK;
                }
                os_free(value);
            }
        }
    }

    return status;
}
