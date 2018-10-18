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
static os_library moduleHandle = NULL;

os_result
os_userClockStart (
    const char *userClockModule,
    const char *startName,
    const char *stopName,
    const char *getName,
    os_boolean y2038_ready)
{
    os_result result = os_resultFail;
    os_libraryAttr attr;
    startClock startFunction = NULL;
    getClock getFunction = NULL;
    os_fptr getFunction64 = NULL;
    stopClock stopFunction = NULL;
    int startFuncResult = 0;

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

        if (y2038_ready == OS_TRUE) {
            getFunction64 = os_fptr(os_libraryGetSymbol(moduleHandle, getName));
        } else {
            getFunction = (getClock)os_fptr(os_libraryGetSymbol(moduleHandle, getName));
        }

        if (getFunction || getFunction64) {
            if (startName && (startFunction == NULL)) {
                OS_REPORT(OS_INFO, "os_userClockStart", 0,
                    "User clock module start function %s is not defined in module %s",
                    startName, userClockModule);
            } else if (stopName && (stopFunction == NULL)) {
                OS_REPORT(OS_INFO, "os_userClockStart", 0,
                    "User clock module stop function %s is not defined in module %s",
                    stopName, userClockModule);
            } else {
                if (stopFunction != NULL) {
                    _stopFunction = stopFunction;
                }
                if (startFunction != NULL) {
                    startFuncResult = startFunction();
                    if (startFuncResult != 0) {
                        OS_REPORT(OS_ERROR, "os_userClockStart", 0,
                            "User clock start failed with code %d",
                            startFuncResult);
                    }
                }
                if (startFuncResult == 0) {
                    if (getFunction64) {
                        os_timeSetUserClock64(getFunction64);
                    } else {
                        os_timeSetUserClock(getFunction);
                    }
                    result = os_resultSuccess;
                }
                os_procAtExit((void (*)(void))os_userClockStop);
            }
        } else {
            OS_REPORT(OS_ERROR, "os_userClockStart", 0,
                "User clock module get function %s is not defined in module %s",
                (getName == NULL) ? "NULL" : getName,
                userClockModule);
        }
    }  else {
        OS_REPORT(OS_ERROR, "os_userClockStart", 0,
            "User clock module %s could not be opened",
            (userClockModule == NULL) ? "NULL" : userClockModule);
    }

    return result;
}

os_result
os_userClockStop (
    void)
{
    int stopResult;
    os_result result;

    result = os_resultFail;

    os_timeSetUserClock64(NULL);
    os_timeSetUserClock(NULL);
    if (_stopFunction) {
        assert(moduleHandle != NULL);
        stopResult = _stopFunction();
        if (stopResult != 0) {
            OS_REPORT (OS_ERROR, "os_userClockStart", 0,
                "User clock stop failed with code %d", stopResult);
        } else {
            result = os_resultSuccess;
        }
        _stopFunction = NULL;
    } else {
        result = os_resultSuccess;
    }
    (void)os_libraryClose(moduleHandle);
    moduleHandle = NULL;
    return result;
}
