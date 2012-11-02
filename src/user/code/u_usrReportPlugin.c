/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include "os_report.h"
#include "cf_config.h"

#include "u__user.h"

#define U_CONF_REPORTPLUGIN             "ReportPlugin"
#define U_CONF_REPORTPLUGIN_LIBRARY     "Library"
#define U_CONF_REPORTPLUGIN_INITIALIZE  "Initialize"
#define U_CONF_REPORTPLUGIN_REPORT      "Report"
#define U_CONF_REPORTPLUGIN_FINALIZE    "Finalize"
#define U_CONF_SUPPRESS_DEFAULT_LOGS    "SuppressDefaultLogs"
#define U_REPORTPLUGINS_MAX     (10)

typedef struct u_reportPluginAdmin_s {
    unsigned int size;
    unsigned int length;
    os_reportPlugin * reportArray;
} *u_reportPluginAdmin;

static u_reportPluginAdmin reportPluginAdmin = NULL;

u_reportPluginAdmin
u_reportPluginAdminNew(
    unsigned int size)
{
    u_reportPluginAdmin admin = NULL;
    unsigned int i;

    admin = os_malloc(sizeof(struct u_reportPluginAdmin_s));
    admin->reportArray = os_malloc(sizeof(os_reportPlugin) * size);

    for (i = 0; i < size; i++) {
      admin->reportArray[i] = NULL;
    }

    admin->size = size;
    admin->length =0;

    return admin;
}

os_result
u_reportPluginAdminRegister (
    u_reportPluginAdmin rPluginAdmin,
    os_reportPlugin plugin)
{
    if (rPluginAdmin){
        if (rPluginAdmin->length < rPluginAdmin->size){
            rPluginAdmin->reportArray[rPluginAdmin->length++] = plugin;
            return 0;
        }
    }

    return -1;
}

void
u_reportPluginGetSuppressDefaultLogs (
    cf_element reportPluginElement,
    const c_char *attributeName,
    c_bool *b)    
{
    cf_data elementData;
    c_value value;

    elementData = cf_data(cf_elementChild(reportPluginElement, attributeName));

    if (elementData != NULL) 
    {
        value = cf_dataValue(elementData);

        if (os_strncasecmp(value.is.String, "TRUE", 4) == 0) 
        {
	    *b = TRUE;
        }
    }
}


c_bool
u_usrReportPluginConfigElementAttributeString (
    cf_element element,
    const char *attribName,
    c_char **data)
{
    cf_attribute attrib;
    c_value attribValue;
    c_bool result = FALSE;
    
    attrib = cf_elementAttribute (element, attribName);

    if (attrib) 
    {
        attribValue = cf_attributeValue (attrib);

        if (attribValue.kind == V_STRING) 
        {
            *data = attribValue.is.String;
            result = TRUE;
        }
    } 

    return result;
 }

 


void
u_usrReportPluginReadAndRegister (
    cf_element config)
{
    cf_node child = NULL;
    cf_element cfgUsrReportPluginLibrary = NULL;
    cf_element cfgUsrReportPluginInitialize = NULL;
    cf_element cfgUsrReportPluginReport = NULL;
    cf_element cfgUsrReportPluginFinalize = NULL;
    cf_element cfgUsrSuppressDefaultLogs = NULL;
    cf_element dc = NULL;
    char *library = NULL;
    char *libraryName = NULL;
    char *reportInitialize = NULL;
    char *reportFinalize = NULL;
    char *reportReport = NULL;
    char *initializeArgument = NULL;
    os_reportPlugin reportPlugin;
    c_iter elementList;
    os_int32 osr;
    c_bool result;
    c_bool suppressDefaultLogs = FALSE;

    assert(config != NULL);

    dc = cf_element(cf_elementChild(config, CFG_DOMAIN));

    if (dc != NULL) {
        elementList = cf_elementGetChilds(dc);
                 
        if (c_iterLength(elementList) > 0) {

            child = c_iterTakeFirst (elementList);
                        
            while (child != NULL)
            {
                if ((strcmp(cf_nodeGetName(child), U_CONF_REPORTPLUGIN)) == 0) { 
                    cfgUsrReportPluginLibrary = cf_element(cf_elementChild(cf_element(child), U_CONF_REPORTPLUGIN_LIBRARY));
                    cfgUsrReportPluginInitialize = cf_element(cf_elementChild(cf_element(child), U_CONF_REPORTPLUGIN_INITIALIZE));
                    cfgUsrReportPluginReport = cf_element(cf_elementChild(cf_element(child), U_CONF_REPORTPLUGIN_REPORT));
                    cfgUsrReportPluginFinalize = cf_element(cf_elementChild(cf_element(child), U_CONF_REPORTPLUGIN_FINALIZE));
                    cfgUsrSuppressDefaultLogs = cf_element(cf_elementChild(cf_element(child), U_CONF_SUPPRESS_DEFAULT_LOGS )); 

                    if (cfgUsrReportPluginLibrary) 
                    {
                        result = u_usrReportPluginConfigElementAttributeString (cfgUsrReportPluginLibrary, "file_name", &library);

                        if (!result) 
                        {
                            OS_REPORT_1 (OS_ERROR, "u_usrReportPluginReadAndRegister", 0,
                                "ReportPlugin library name invalid: %s", library);                              
                        }
                        else
                        {
                            if (strncmp (library, "file://", 7) == 0) 
                            {
                                libraryName = &library[7];
                            } 
                            else 
                            {
                                libraryName = library;
                            }
                        }
                    }
                
                    if (cfgUsrReportPluginInitialize && result) 
                    {
                        result = u_usrReportPluginConfigElementAttributeString(cfgUsrReportPluginInitialize, 
                                                                               "symbol_name", 
                                                                               &reportInitialize);

                        if (!result)
                        {
                            OS_REPORT_1 (OS_ERROR, "u_usrReportPluginReadAndRegister", 0,
                            "ReportPlugin initialize method invalid: %s", reportInitialize);                                                    
                        }
                        else
                        {
                            u_usrReportPluginConfigElementAttributeString(cfgUsrReportPluginInitialize, 
                                                                          "argument", &initializeArgument );
                        }
                    }
                
                    if (cfgUsrReportPluginReport && result) {
                        result = u_usrReportPluginConfigElementAttributeString (cfgUsrReportPluginReport, 
                                                                                "symbol_name", &reportReport);         
                    }
                
                    if (cfgUsrReportPluginFinalize && result) {
                        result = u_usrReportPluginConfigElementAttributeString (cfgUsrReportPluginFinalize, 
                                                                                "symbol_name", &reportFinalize);
                    }
  
                    if (cfgUsrSuppressDefaultLogs && result)
		    {
                        u_reportPluginGetSuppressDefaultLogs (cfgUsrSuppressDefaultLogs, "#text", &suppressDefaultLogs);
		    }

                    if (result)
                    {
                        osr = os_reportRegisterPlugin (libraryName, 
                                                       reportInitialize, 
                                                       initializeArgument, 
                                                       reportReport, 
                                                       reportFinalize,                                                        
                                                       suppressDefaultLogs,
                                                       &reportPlugin);

                        if (osr == 0)
                        {
                            if (reportPluginAdmin == NULL)
                            {
                                reportPluginAdmin = u_reportPluginAdminNew (U_REPORTPLUGINS_MAX);                        
                            }

                            osr = u_reportPluginAdminRegister (
                                reportPluginAdmin,
                                reportPlugin
                                );
                        }
                    }
                }
                        
                child = c_iterTakeFirst (elementList);
            }

            c_iterFree (elementList);
        }
    }
}

void
u_usrReportPluginUnregister ()
{
    unsigned int i;
    os_result osr;

    if (reportPluginAdmin != NULL){  
        for (i = 0; i < reportPluginAdmin->size; i++){ 
            if (reportPluginAdmin->reportArray[i] != NULL) {
                osr = os_reportUnregisterPlugin (reportPluginAdmin->reportArray[i]);
            }
        }
    }
}
