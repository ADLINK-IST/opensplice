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

#include "os_report.h"
#include "cf_config.h"

#include "u__user.h"
#include "u_types.h"
#include "u__usrReportPlugin.h"

#define U_CONF_REPORTPLUGIN             "ReportPlugin"
#define U_CONF_REPORTPLUGIN_LIBRARY     "Library"
#define U_CONF_REPORTPLUGIN_INITIALIZE  "Initialize"
#define U_CONF_REPORTPLUGIN_REPORT      "Report"
#define U_CONF_REPORTPLUGIN_TYPEDREPORT      "TypedReport"
#define U_CONF_REPORTPLUGIN_FINALIZE    "Finalize"
#define U_CONF_SUPPRESS_DEFAULT_LOGS    "SuppressDefaultLogs"
#define U_CONF_SERVICES_ONLY    "ServicesOnly"
#define U_REPORTPLUGINS_MAX     (10)


void
u_reportPluginGetSuppressDefaultLogs (
    const cf_element reportPluginElement,
    const os_char *attributeName,
    u_bool *b)
{
    cf_data elementData;
    c_value value;

    assert(reportPluginElement != NULL);

    elementData = cf_data(cf_elementChild(reportPluginElement, attributeName));

    if (elementData != NULL) {
        value = cf_dataValue(elementData);

        if (os_strncasecmp(value.is.String, "TRUE", 4) == 0) {
            *b = TRUE;
        }
    }
}


u_bool
u_usrReportPluginConfigElementAttributeString (
    const cf_element element,
    const char *attribName,
    os_char **data)
{
    cf_attribute attrib;
    c_value attribValue;
    u_bool result = FALSE;

    assert(element != NULL);

    attrib = cf_elementAttribute (element, attribName);

    if (attrib) {
        attribValue = cf_attributeValue (attrib);

        if (attribValue.kind == V_STRING) {
            *data = attribValue.is.String;
            result = TRUE;
        }
    }
    return result;
 }




u_result
u_usrReportPluginReadAndRegister (
    const cf_element config,
    os_int32 domainId,
	c_iter* reportPlugins)
{
    cf_node child = NULL;
    cf_element cfgUsrReportPluginLibrary = NULL;
    cf_element cfgUsrReportPluginInitialize = NULL;
    cf_element cfgUsrReportPluginReport = NULL;
    cf_element cfgUsrReportPluginTypedReport = NULL;
    cf_element cfgUsrReportPluginFinalize = NULL;
    cf_element cfgUsrSuppressDefaultLogs = NULL;
    cf_element cfgUsrServicesOnly = NULL;
    cf_element dc = NULL;
    char *library = NULL;
    char *libraryName = NULL;
    char *reportInitialize = NULL;
    char *reportFinalize = NULL;
    char *reportReport = NULL;
    char *reportTypedReport = NULL;
    char *initializeArgument = NULL;
    os_reportPlugin reportPlugin;
    c_iter elementList;
    u_result uResult = U_RESULT_OK;
    c_bool result = FALSE;
    c_bool suppressDefaultLogs = FALSE;
    c_bool pluginIsForservicesOnly = FALSE;

    assert(config != NULL);

    dc = cf_element(cf_elementChild(config, CFG_DOMAIN));

    if (dc != NULL) {
        elementList = cf_elementGetChilds(dc);

        if (c_iterLength(elementList) > 0) {

            child = c_iterTakeFirst (elementList);

            while (child != NULL)
            {
                result = FALSE;
                if ((strcmp(cf_nodeGetName(child), U_CONF_REPORTPLUGIN)) == 0) {
                    cfgUsrReportPluginLibrary = cf_element(cf_elementChild(cf_element(child), U_CONF_REPORTPLUGIN_LIBRARY));
                    cfgUsrReportPluginInitialize = cf_element(cf_elementChild(cf_element(child), U_CONF_REPORTPLUGIN_INITIALIZE));
                    cfgUsrReportPluginReport = cf_element(cf_elementChild(cf_element(child), U_CONF_REPORTPLUGIN_REPORT));
                    cfgUsrReportPluginTypedReport = cf_element(cf_elementChild(cf_element(child), U_CONF_REPORTPLUGIN_TYPEDREPORT));
                    cfgUsrReportPluginFinalize = cf_element(cf_elementChild(cf_element(child), U_CONF_REPORTPLUGIN_FINALIZE));
                    cfgUsrSuppressDefaultLogs = cf_element(cf_elementChild(cf_element(child), U_CONF_SUPPRESS_DEFAULT_LOGS ));
                    cfgUsrServicesOnly = cf_element(cf_elementChild(cf_element(child), U_CONF_SERVICES_ONLY ));

                    if (cfgUsrReportPluginLibrary)
                    {
                        result = u_usrReportPluginConfigElementAttributeString (cfgUsrReportPluginLibrary, "file_name", &library);

                        if (!result)
                        {
                            OS_REPORT (OS_ERROR, "u_usrReportPluginReadAndRegister", U_RESULT_ILL_PARAM,
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
                            OS_REPORT (OS_ERROR, "u_usrReportPluginReadAndRegister", U_RESULT_INTERNAL_ERROR,
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

                    if (cfgUsrReportPluginTypedReport && result) {
                        result = u_usrReportPluginConfigElementAttributeString (cfgUsrReportPluginTypedReport,
                                                                                "symbol_name", &reportTypedReport);
                    }

                    if (cfgUsrReportPluginFinalize && result) {
                        result = u_usrReportPluginConfigElementAttributeString (cfgUsrReportPluginFinalize,
                                                                                "symbol_name", &reportFinalize);
                    }

                    if (cfgUsrSuppressDefaultLogs && result)
                    {
                        u_reportPluginGetSuppressDefaultLogs (cfgUsrSuppressDefaultLogs, "#text", &suppressDefaultLogs);
                    }

                    if (cfgUsrServicesOnly && result)
                    {
                        /* This method has a very silly name */
                        u_reportPluginGetSuppressDefaultLogs (cfgUsrServicesOnly, "#text", &pluginIsForservicesOnly);
                    }

                    /* Register plug-in ... */
                    if (result)
                    {
                        /* but only if process is a service when plug-in is 'services only' */
                        if (pluginIsForservicesOnly && ! os_procIsOpenSpliceService())
                        {
                            /* OK - plug-in was service only; this is not a service: drop through */
                        }
                        else
                        {
                            os_int32 iResult;
                            iResult = os_reportRegisterPlugin (libraryName,
                                                            reportInitialize,
                                                            initializeArgument,
                                                            reportReport,
                                                            reportTypedReport,
                                                            reportFinalize,
                                                            suppressDefaultLogs,
                                                            domainId,
                                                            &reportPlugin);

                            if (iResult == 0)
                            {
                            	*reportPlugins = c_iterInsert(*reportPlugins, reportPlugin);
                            }
                            else
                            {
                                uResult = U_RESULT_PRECONDITION_NOT_MET;
                                OS_REPORT(OS_ERROR, "u_usrReportPluginReadAndRegister", result,
                                          "ReportPlugin registration failed");

                                return uResult;
                            }
                        }
                    }
                    else
                    {
                        uResult = U_RESULT_PRECONDITION_NOT_MET;
                        OS_REPORT(OS_ERROR, "u_usrReportPluginReadAndRegister", result,
                                  "Load ReportPlugin failed");

                        return uResult;
                    }
                }

                child = c_iterTakeFirst (elementList);
            }

            c_iterFree (elementList);
        }
    }

    return uResult;
}

void
u_usrReportPluginUnregister (c_iter reportPlugins)
{

	os_reportPlugin plugin = c_iterTakeFirst(reportPlugins);

    while (plugin != NULL){
    	os_reportUnregisterPlugin (plugin);
    	plugin = c_iterTakeFirst(reportPlugins);
    }
}
