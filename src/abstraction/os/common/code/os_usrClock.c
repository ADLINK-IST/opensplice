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

#include "os_usrClock.h"
#include "os_library.h"
#include "os_report.h"
#include "os_time.h"
#include "os_process.h"

#ifdef VXWORKS_RTP
#include <string.h>
#endif

typedef int (*startClock)(void);
typedef int (*stopClock)(void);
typedef os_time (*getClock)(void);

static stopClock _stopFunction = NULL;

os_result
os_userClockStart (
    char *userClockModule,
    char *startName,
    char *stopName,
    char *getName
    )
{
    os_result result;
    os_library moduleHandle;
    os_libraryAttr attr;
    startClock startFunction;
    getClock getFunction;
    stopClock stopFunction;
    int startFuncResult;

    startFunction = NULL;
    getFunction = NULL;
    stopFunction = NULL;

    result = os_resultFail;

    if (startName && (strlen(startName) == 0)) {
        startName = "clockStart";
    }
    if (stopName && (strlen(stopName) == 0)) {
        stopName = "clockStop";
    }
    if (getName && (strlen(getName)) == 0) {
        getName = "clockGet";
    }

    os_libraryAttrInit(&attr);
    moduleHandle = os_libraryOpen(userClockModule, &attr);
    /* If auto-translate fails, retry with exact name */
    if (moduleHandle == NULL) {
        attr.autoTranslate = OS_FALSE;
        moduleHandle = os_libraryOpen(userClockModule, &attr);
    }

    if (moduleHandle != NULL) {
        if (startName) {
            startFunction = (startClock)os_fptr(os_libraryGetSymbol(moduleHandle, startName));
        }
        if (stopName) {
            stopFunction = (stopClock)os_fptr(os_libraryGetSymbol(moduleHandle, stopName));
        }
        getFunction = (getClock)os_fptr(os_libraryGetSymbol(moduleHandle, getName));

        if (getFunction) {
            if (startName && (startFunction == NULL)) {
                OS_REPORT_2(OS_INFO, "os_userClockStart", 0,
                    "User clock module start function %s is not defined in module %s",
                    startName, userClockModule);
            } else if (stopName && (stopFunction == NULL)) {
                OS_REPORT_2(OS_INFO, "os_userClockStart", 0,
                    "User clock module stop function %s is not defined in module %s",
                    stopName, userClockModule);
            } else {
                if (stopFunction != NULL) {
                    _stopFunction = stopFunction;
                }
                if (startFunction != NULL) {
                    startFuncResult = startFunction();
                    if (startFuncResult != 0) {
                        OS_REPORT_1(OS_ERROR, "os_userClockStart", 0,
                            "User clock start failed with code %d",
                            startFuncResult);
                    } else {
                        os_timeSetUserClock(getFunction);
                        result = os_resultSuccess;

                    }
                } else {
                    os_timeSetUserClock(getFunction);
                    result = os_resultSuccess;
                }
                os_procAtExit((void (*)(void))os_userClockStop);
            }
        } else {
            OS_REPORT_2(OS_ERROR, "os_userClockStart", 0,
                "User clock module get function %s is not defined in module %s",
                (getName == NULL) ? "NULL" : getName,
                userClockModule);
        }
    }  else {
        OS_REPORT_1(OS_ERROR, "os_userClockStart", 0,
            "User clock module %s could not be opened",
            (userClockModule == NULL) ? "NULL" : userClockModule);
    }
    return result;
}

os_result
os_userClockStop (
    void
    )
{
    int stopResult;
    os_result result;

    result = os_resultFail;

    os_timeSetUserClock (NULL);
    if (_stopFunction) {
        stopResult = _stopFunction();
        if (stopResult != 0) {
            OS_REPORT_1 (OS_ERROR, "os_userClockStart", 0,
                "User clock stop failed with code %d", stopResult);
        } else {
            result = os_resultSuccess;
        }
        _stopFunction = NULL;
    } else {
        result = os_resultSuccess;
    }
    return result;
}
