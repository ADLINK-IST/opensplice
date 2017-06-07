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

#include "os_report.h"
#include "os_usrClock.h"
#include "cf_config.h"

#include "u__usrClock.h"
#include "u__user.h"

/* User clock isn't a separate service anymore, to be compatible with old service name check both names
 * see ticket OSPL-7348
 */
#define U_CONF_USRCLOCK_SERVICENAME         "UserClock"
#define U_CONF_USRCLOCK_SERVICENAME_OLD     "UserClockService"
#define U_CONF_USRCLOCK_SERVICEMODULENAME   "UserClockModule"
#define U_CONF_USRCLOCK_SERVICESTARTNAME    "UserClockStart"
#define U_CONF_USRCLOCK_SERVICESTOPNAME     "UserClockStop"
#define U_CONF_USRCLOCK_SERVICEQUERYNAME    "UserClockQuery"
#define U_CONF_USRCLOCK_Y2038READY          "y2038Ready"
#define U_CONF_DOMAIN_Y2038READY            "y2038Ready"

#define U_USRCLOCK_STARTNAME_DEFAULT "clockStart"
#define U_USRCLOCK_STOPNAME_DEFAULT  "clockStop"
#define U_USRCLOCK_QUERYNAME_DEFAULT "clockGet"


static const char *
u_usrClockConfigElementDataString (
    const cf_element element,
    const char *defaultValue)
{
    c_iter children;
    const char *data = defaultValue;
    c_value dataValue;
    c_ulong i;
    cf_node node;

    assert(element != NULL);

    children = cf_elementGetChilds (element);

    if (children) {
        for (i = 0; i < c_iterLength (children); i++) {
            node = c_iterObject (children, i);

            if (cf_nodeKind (node) == CF_DATA) {
                dataValue = cf_dataValue (cf_data(node));

                if (dataValue.kind == V_STRING) {
                    data = dataValue.is.String;
                }
            }
        }
        c_iterFree(children);
    }
    return data;
}

static os_boolean
u_usrClockConfigElementAttributeStringToBoolean (
    const cf_element element,
    const char *attribName,
    os_boolean defaultValue)
{
    os_boolean value = defaultValue;
    const char *data = NULL;
    cf_attribute attrib;
    c_value attribValue;

    assert(element != NULL);

    attrib = cf_elementAttribute (element, attribName);
    if (attrib) {
        attribValue = cf_attributeValue (attrib);
        if (attribValue.kind == V_STRING) {
            data = attribValue.is.String;

            if (strcmp(data, "true") == 0) {
                value = OS_TRUE;
            } else if (strcmp(data, "false") == 0) {
                value = OS_FALSE;
            }
        }
    }

    return value;
}

void
u_usrClockInit (
    const cf_element config)
{
    cf_element cfgUsrClockService = NULL;
    cf_element cfgUsrClockModule = NULL;
    cf_element cfgUsrClockStart = NULL;
    cf_element cfgUsrClockStop = NULL;
    cf_element cfgUsrClockQuery = NULL;
    cf_element dc = NULL;
    cf_element cfgDomainY2038 = NULL;
    const char *module = NULL;
    const char *moduleName = NULL;
    const char *clockStart = NULL;
    const char *clockStop = NULL;
    const char *clockQuery = U_USRCLOCK_QUERYNAME_DEFAULT;
    const char *domainY2038 = NULL;
    os_boolean y2038Ready = OS_FALSE;
    os_boolean domainY2038Ready = OS_FALSE;

    assert(config != NULL);
    dc = cf_element(cf_elementChild(config, CFG_DOMAIN));
    if (dc != NULL) {
        cfgUsrClockService = cf_element(cf_elementChild(dc, U_CONF_USRCLOCK_SERVICENAME));
        if (cfgUsrClockService == NULL) {
            cfgUsrClockService = cf_element(cf_elementChild(dc, U_CONF_USRCLOCK_SERVICENAME_OLD));
            if (cfgUsrClockService != NULL) {
                OS_REPORT(OS_WARNING, "u_usrClockInit", 0,
                            "Found deprecated tag 'UserClockService' in configuration. Please use 'UserClock'");
            }
        }
        if (cfgUsrClockService != NULL) {
            cfgUsrClockModule = cf_element(cf_elementChild(cfgUsrClockService, U_CONF_USRCLOCK_SERVICEMODULENAME));
            cfgUsrClockStart = cf_element(cf_elementChild(cfgUsrClockService, U_CONF_USRCLOCK_SERVICESTARTNAME));
            cfgUsrClockStop = cf_element(cf_elementChild(cfgUsrClockService, U_CONF_USRCLOCK_SERVICESTOPNAME));
            cfgUsrClockQuery = cf_element(cf_elementChild(cfgUsrClockService, U_CONF_USRCLOCK_SERVICEQUERYNAME));

            cfgDomainY2038 = cf_element(cf_elementChild(dc, U_CONF_DOMAIN_Y2038READY));
            if (cfgDomainY2038 != NULL) {
                domainY2038 = u_usrClockConfigElementDataString(cfgDomainY2038, NULL);
                if ((domainY2038 != NULL) && (strcmp(domainY2038, "true") == 0)) {
                    domainY2038Ready = OS_TRUE;
                }
            }
            y2038Ready = u_usrClockConfigElementAttributeStringToBoolean(cfgUsrClockService, U_CONF_USRCLOCK_Y2038READY, domainY2038Ready);
            if ((domainY2038Ready == OS_TRUE) &&
                (y2038Ready != domainY2038Ready)) {
                OS_REPORT(OS_WARNING, "u_usrClockInit", 0,
                          "The configuration element //OpenSplice/Domain/y2038Ready is 'true' while the attribute "
                          "//OpenSplice/Domain/UserClock/y2038Ready overrules this configuration for the userClock. "
                          "This will cause undefined behavior when using this configuration after y2038.");
            }

            if (cfgUsrClockModule) {
                module = u_usrClockConfigElementDataString(cfgUsrClockModule, NULL);
                if (module) {
                    if (strncmp (module, "file://", 7) == 0) {
                        moduleName = &module[7];
                    } else {
                        moduleName = module;
                    }
                }
            }
            if (cfgUsrClockStart) {
                if (u_usrClockConfigElementAttributeStringToBoolean(cfgUsrClockStart, "enabled", OS_TRUE) == OS_TRUE) {
                    clockStart = u_usrClockConfigElementDataString(cfgUsrClockStart, U_USRCLOCK_STARTNAME_DEFAULT);
                }
            }
            if (cfgUsrClockStop) {
                if (u_usrClockConfigElementAttributeStringToBoolean(cfgUsrClockStop, "enabled", OS_TRUE) == OS_TRUE) {
                    clockStop = u_usrClockConfigElementDataString(cfgUsrClockStop, U_USRCLOCK_STOPNAME_DEFAULT);
                }
            }
            if (cfgUsrClockQuery) {
                if (u_usrClockConfigElementAttributeStringToBoolean(cfgUsrClockQuery, "enabled", OS_TRUE) == OS_TRUE) {
                    clockQuery = u_usrClockConfigElementDataString(cfgUsrClockQuery, U_USRCLOCK_QUERYNAME_DEFAULT);
                }
            }
            os_userClockStart (moduleName, clockStart, clockStop, clockQuery, y2038Ready);
        }
    }
}
