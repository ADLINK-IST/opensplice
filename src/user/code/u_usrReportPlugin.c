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

#include "os_report.h"
#include "cf_config.h"

#include "u__user.h"

#define U_CONF_REPORTPLUGIN             "ReportPlugin"
#define U_CONF_REPORTPLUGIN_LIBRARY 	"Library"
#define U_CONF_REPORTPLUGIN_INITIALIZE	"Initialize"
#define U_CONF_REPORTPLUGIN_REPORT	"Report"
#define U_CONF_REPORTPLUGIN_FINALIZE	"Finalize"
#define U_REPORTPLUGINS_MAX	(10)

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

char *
u_usrReportPluginConfigElementAttributeString (
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
u_usrReportPluginReadAndRegister (
    cf_element config)
{
    cf_node child = NULL;
    cf_element cfgUsrReportPluginLibrary = NULL;
    cf_element cfgUsrReportPluginInitialize = NULL;
    cf_element cfgUsrReportPluginReport = NULL;
    cf_element cfgUsrReportPluginFinalize = NULL;
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

    assert(config != NULL);

    OS_REPORT(OS_INFO,OSRPT_CNTXT_USER,0,
              "u_usrReportPluginReadAndRegister\n");

    dc = cf_element(cf_elementChild(config, CFG_DOMAIN));

    if (dc != NULL) {
        elementList = cf_elementGetChilds(dc);
                 
        if (c_iterLength(elementList) > 0) {

	    child = c_iterTakeFirst (elementList);
                        
	    while (child != NULL){
                if ((strcmp(cf_nodeGetName(child), U_CONF_REPORTPLUGIN)) == 0) { 
                    cfgUsrReportPluginLibrary = cf_element(cf_elementChild(cf_element(child), U_CONF_REPORTPLUGIN_LIBRARY));
       	            cfgUsrReportPluginInitialize = cf_element(cf_elementChild(cf_element(child), U_CONF_REPORTPLUGIN_INITIALIZE));
                    cfgUsrReportPluginReport = cf_element(cf_elementChild(cf_element(child), U_CONF_REPORTPLUGIN_REPORT));
                    cfgUsrReportPluginFinalize = cf_element(cf_elementChild(cf_element(child), U_CONF_REPORTPLUGIN_FINALIZE));
        
                    if (cfgUsrReportPluginLibrary) {
                        library = u_usrReportPluginConfigElementAttributeString(cfgUsrReportPluginLibrary, "file_name");
                        if (library) {
                            if (strncmp (library, "file://", 7) == 0) {
                                libraryName = &library[7];
                            } else {
                                libraryName = library;
                            }
                        }
                    }
		
                    if (cfgUsrReportPluginInitialize) 
                    {
                        reportInitialize = u_usrReportPluginConfigElementAttributeString(cfgUsrReportPluginInitialize, "symbol_name");
                        initializeArgument = u_usrReportPluginConfigElementAttributeString(cfgUsrReportPluginInitialize, "argument");
                    }
		
                    if (cfgUsrReportPluginReport) {
	                reportReport = u_usrReportPluginConfigElementAttributeString(cfgUsrReportPluginReport, "symbol_name");         
                    }
                
                    if (cfgUsrReportPluginFinalize) {
                        reportFinalize = u_usrReportPluginConfigElementAttributeString(cfgUsrReportPluginFinalize, "symbol_name");
                    } 

                    if (reportPluginAdmin == NULL){
                        reportPluginAdmin = u_reportPluginAdminNew (U_REPORTPLUGINS_MAX);                        
                    }

		    osr = os_reportRegisterPlugin (libraryName, reportInitialize, initializeArgument, reportReport, reportFinalize, &reportPlugin);
                    osr = u_reportPluginAdminRegister (
                        reportPluginAdmin,
                        reportPlugin
			);
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
        for (i = 0; i < reportPluginAdmin->size; i--){ 
	    if (reportPluginAdmin->reportArray[i] != NULL) {
	        osr = os_reportUnregisterPlugin (reportPluginAdmin->reportArray[i]);
                reportPluginAdmin->length--;
            }
        }
    }
}
