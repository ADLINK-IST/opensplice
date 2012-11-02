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

/* Implements user clock functions, generic part for all
 * operating systems
 */

#include "code/os_usrClockSupport.c"

#include "os_time.h"
#include "os_process.h"
#include "os_defs.h"
#include "os__time.h"

#ifdef VXWORKS_RTP
#include <string.h>
#endif

typedef	int (*startClock)(void);
typedef int (*stopClock)(void);
typedef os_time (*getClock)(void);

static stopClock stopFunction;

os_result
os_userClockStart (
    char *userClockModule,
    char *startName,
    char *stopName,
    char *getName
    )
{
    os_result result = os_resultFail;
    int startResult;
    void *moduleHandle;
    startClock startFunction;
    getClock getFunction;
    char* module;

    if(userClockModule){
        module = userClockModule;
    } else {
        module = "NULL";
    }

    if (startName && (strlen(startName) == 0)) {
    	startName = "clockStart";
    }
    if (stopName && (strlen(stopName) == 0)) {
    	stopName = "clockStop";
    }
    if (getName && (strlen(getName) == 0)) {
    	getName = "clockGet";
    }            
    moduleHandle = open_userClockModule (userClockModule);
    
    if (moduleHandle) {
    	startFunction = (startClock)find_userClockFunction (moduleHandle, startName);
    	stopFunction = (stopClock)find_userClockFunction (moduleHandle, stopName);
    	getFunction = (getClock)find_userClockFunction (moduleHandle, getName);
        
    	if (((startName == NULL) || (startName && (startFunction != NULL))) &&
	    ((stopName == NULL) || (stopName && (stopFunction != NULL)))) {
    	    if (getFunction) {
    	        if (startName && !startFunction) {
    	    	    OS_REPORT_2 (OS_INFO, "os_userClockStart", 0,
    	    	        "User clock module start function %s is not defined in module %s",
    	    	        startName,
    	    	        module);
    	        }
    	        if (stopName && !stopFunction) {
    	    	    OS_REPORT_2 (OS_INFO, "os_userClockStart", 0,
    	    	        "User clock module stop function %s is not defined in module %s",
    	    	        stopName,
    	    	        module);
       	        }
       	        if (startFunction) {
       	    	    startResult = startFunction ();
       	            if (startResult) {
       	    	        OS_REPORT_1 (OS_ERROR, "os_userClockStart", 0,
       	    	            "User clock start failed with code %d", startResult);
       	            } else {
       	    	        os_timeSetUserClock (getFunction);
       	    	        result = os_resultSuccess;
       	            }
       	        } else {
       	    	    os_timeSetUserClock (getFunction);
       	    	    result = os_resultSuccess;
       	        }
		      os_procAtExit ((void (*)(void))os_userClockStop);
    	    } else {
    	        OS_REPORT_2 (OS_ERROR, "os_userClockStart", 0,
    	            "User clock module get function %s is not defined in module %s",
    	            getName,
    	            module);
    	    }
        }
        /* Not clear why the userClockModule is closed here...
         * This leads to unloading of the library for win32,
         * resulting in segfaults */
        /* close_userClockModule (moduleHandle); */
    } else {
    	OS_REPORT_1 (OS_ERROR, "os_userClockStart", 0,
    	    "User clock module %s could not be opened",
    	    module);
    }
    return result;
}
    
os_result
os_userClockStop (
    void
    )
{
    int stopResult;
    os_result result = os_resultFail;
    
    os_timeSetUserClock (NULL);
    if (stopFunction) {
    	stopResult = stopFunction();
    	if (stopResult) {
       	    OS_REPORT_1 (OS_ERROR, "os_userClockStart", 0,
       	        "User clock stop failed with code %d", stopResult);
    	} else {
	    result = os_resultSuccess;
    	}
    } else {
	result = os_resultSuccess;
    }
    return result;
}
