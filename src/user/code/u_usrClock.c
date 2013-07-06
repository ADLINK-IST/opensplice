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

#include "os_report.h"
#include "os_usrClock.h"
#include "cf_config.h"

#include "u__usrClock.h"
#include "u__user.h"

#define U_CONF_USRCLOCK_SERVICENAME         "UserClockService"
#define U_CONF_USRCLOCK_SERVICEMODULENAME   "UserClockModule"
#define U_CONF_USRCLOCK_SERVICESTARTNAME    "UserClockStart"
#define U_CONF_USRCLOCK_SERVICESTOPNAME     "UserClockStop"
#define U_CONF_USRCLOCK_SERVICEQUERYNAME    "UserClockQuery"

#define U_USRCLOCK_STARTNAME_DEFAULT "clockStart"
#define U_USRCLOCK_STOPNAME_DEFAULT  "clockStop"
#define U_USRCLOCK_QUERYNAME_DEFAULT "clockGet"


char *
u_usrClockConfigElementDataString (
    cf_element element)
{
    c_iter children;
    char *data = NULL;
    c_value dataValue;
    int i;
    cf_node node;

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

char *
u_usrClockConfigElementAttributeString (
    cf_element element,
    const char *attribName)
{
    char *data = NULL;
    cf_attribute attrib;
    c_value attribValue;

    attrib = cf_elementAttribute (element, attribName);
    if (attrib) {
        attribValue = cf_attributeValue (attrib);
        if (attribValue.kind == V_STRING) {
            data = attribValue.is.String;
        }
    } else {
        data = "true";
    }

    return data;
}


void
u_usrClockInit (
    cf_element config)
{
    cf_element cfgUsrClockService = NULL;
    cf_element cfgUsrClockModule = NULL;
    cf_element cfgUsrClockStart = NULL;
    cf_element cfgUsrClockStop = NULL;
    cf_element cfgUsrClockQuery = NULL;
    cf_element dc=NULL;
    char *module = NULL;
    char *moduleName = NULL;
    char *clockStart = NULL;
    char *clockStop = NULL;
    char *clockQuery = U_USRCLOCK_QUERYNAME_DEFAULT;
    char *enabled=NULL;

    assert(config != NULL);

    dc = cf_element(cf_elementChild(config, CFG_DOMAIN));
    if (dc != NULL) {
        cfgUsrClockService = cf_element(cf_elementChild(dc, U_CONF_USRCLOCK_SERVICENAME));
    
        if (cfgUsrClockService != NULL) {
            cfgUsrClockModule = cf_element(cf_elementChild(cfgUsrClockService, U_CONF_USRCLOCK_SERVICEMODULENAME));
    	    cfgUsrClockStart = cf_element(cf_elementChild(cfgUsrClockService, U_CONF_USRCLOCK_SERVICESTARTNAME));
            cfgUsrClockStop = cf_element(cf_elementChild(cfgUsrClockService, U_CONF_USRCLOCK_SERVICESTOPNAME));
            cfgUsrClockQuery = cf_element(cf_elementChild(cfgUsrClockService, U_CONF_USRCLOCK_SERVICEQUERYNAME));
        
            if (cfgUsrClockModule) {
                module = u_usrClockConfigElementDataString(cfgUsrClockModule);
                if (module) {
                    if (strncmp (module, "file://", 7) == 0) {
                        moduleName = &module[7];
                    } else {
                        moduleName = module;
                    }
                }
            }
            if (cfgUsrClockStart) {
                enabled = u_usrClockConfigElementAttributeString(cfgUsrClockStart, "enabled");
                if ( strcmp(enabled, "true") == 0) {
                    clockStart = u_usrClockConfigElementDataString(cfgUsrClockStart);
                    if (!clockStart) {
                        clockStart = U_USRCLOCK_STARTNAME_DEFAULT;
                    }
                }
                
            }
            if (cfgUsrClockStop) {
                enabled = u_usrClockConfigElementAttributeString(cfgUsrClockStop, "enabled");
                if (strcmp(enabled, "true") == 0) {
                    clockStop = u_usrClockConfigElementDataString(cfgUsrClockStop);
                    if (!clockStop) {
                        clockStop = U_USRCLOCK_STOPNAME_DEFAULT;
                    }
                }
            }
            if (cfgUsrClockQuery) {
                enabled = u_usrClockConfigElementAttributeString(cfgUsrClockQuery, "enabled");
                if (strcmp(enabled, "true") == 0) {
                    clockQuery = u_usrClockConfigElementDataString(cfgUsrClockQuery);
                    if (!clockQuery) {
                        clockQuery = U_USRCLOCK_QUERYNAME_DEFAULT;
                    }
                }
            } 
            os_userClockStart (moduleName, clockStart, clockStop, clockQuery);
        }
    }
}
