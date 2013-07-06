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
/** \file os/win32/code/os_debug.c
 *  \brief win32 manages the destination of Windows debug reports.
 *
 * Implements the options of redirecting
 * the output for windows debug messages.
 * Normally, _ASSERT results in a dialog
 * This can be changed by means of
 * os_debugModeInit().
 */
/* interface */
#include "os__debug.h"

/* implementation */
#include <crtdbg.h>
#include "os_heap.h"
#include "os_stdlib.h"


typedef struct os_crtReportInfo_s {
    int mode;
    _HFILE file;
} *os_crtReportInfo;

typedef struct os_crtReportInfoBlock_s {
    os_crtReportInfo warnInfo;
    os_crtReportInfo errorInfo;
    os_crtReportInfo assertInfo;
} *os_crtReportInfoBlock;

/* Global variable for this module */
static os_crtReportInfoBlock infoBlock = NULL;

#define REPORTFILE_STDOUT          "stdout"
#define REPORTFILE_STDERR          "stderr"
#define _CRTDBG_MODE_ERROR         (-1)

/* Local convenience function for setting the CrtReport mode */
/* The original settings are returned in a crtReportInfo, which */
/* has to be freed by the caller */
static os_crtReportInfo
setCrtMode(
    int modeKind,
    const char *envName,
    const char *defaultName,
    const char *prefix)
{
    const char *fileName;
    char *fullFileName;
    os_size_t size;
    _HFILE reportFile;
    os_crtReportInfo result = NULL;

    fileName = os_getenv(envName);
    if (!fileName) {
        fileName = defaultName;
    }
    if (strncmp(fileName, REPORTFILE_STDOUT, sizeof(REPORTFILE_STDOUT)) == 0) {
        reportFile = _CRTDBG_FILE_STDOUT;
    } else if (strncmp(fileName, REPORTFILE_STDERR, sizeof(REPORTFILE_STDERR)) == 0) {
        reportFile = _CRTDBG_FILE_STDERR;
    } else {
        size = strlen(prefix) + 1 + strlen(fileName) + 1;
        fullFileName = os_malloc(size);
        snprintf(fullFileName, size, "%s%c%s", prefix, OS_FILESEPCHAR, fileName);\

        reportFile = CreateFile(fullFileName, GENERIC_WRITE, FILE_SHARE_WRITE,
                         NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (reportFile == INVALID_HANDLE_VALUE) {
            OS_DEBUG_1("setCrtMode", "Unable to open file \"%s\" for debug reports.", fullFileName);
        }
        os_free(fullFileName);
    }

    if (reportFile != INVALID_HANDLE_VALUE) {
        result = os_malloc(sizeof(*result));
        result->mode = _CrtSetReportMode(modeKind, _CRTDBG_MODE_FILE);
        if (result->mode == _CRTDBG_MODE_ERROR) {
            OS_DEBUG("setCrtMode", "_CrtSetReportMode failed, unable to set report mode to FILE");
        } else {
            result->file = _CrtSetReportFile(_CRT_WARN, reportFile);
            if (result->file == _CRTDBG_HFILE_ERROR) {
                OS_DEBUG_1("setCrtMode",
                           "_CrtSetReportFile failed, unable to set report file to \"%s\"", fileName);
            }
        }
    }
    return result;
}

/* Local function for restoring the original CrtReport settings */
/* This method also frees the info parameter */
static void
restoreCrtMode(
    int modeKind,
    os_crtReportInfo info)
{
    int currentMode;
    _HFILE currentFile;

    if (info) {
        currentMode = _CrtSetReportMode(modeKind, info->mode);
        if (currentMode == _CRTDBG_MODE_FILE) {
            /* Get currently used handle */
            currentFile = _CrtSetReportFile(modeKind, _CRTDBG_REPORT_FILE);
            /* and close the corresponding file */
            CloseHandle(currentFile);
        }
        if (info->mode == _CRTDBG_MODE_FILE) {
            _CrtSetReportFile(modeKind, info->file);
        }
        os_free(info);
    }
}


#define REPORTFILE_PATHNAME        "OSPL_DEBUG_LOGPATH"
#define REPORTFILE_WARNFILENAME    "OSPL_DEBUG_LOGFILE_WARN"
#define REPORTFILE_ERRORFILENAME   "OSPL_DEBUG_LOGFILE_ERROR"
#define REPORTFILE_ASSERTFILENAME  "OSPL_DEBUG_LOGFILE_ASSERT"
#define REPORTFILE_WARNDEFAULT     "ospl-crtdbg-warning.log"
#define REPORTFILE_ERRORDEFAULT    "ospl-crtdbg-error.log"
#define REPORTFILE_ASSERTDEFAULT   "ospl-crtdbg-assert.log"

/**
 * os_debugModeInit() reads the environment
 * variable OSPL_DEBUG_FILE. If it is set,
 * a file with the corresponding name will
 * be opened. Otherwise, the default mode is
 * kept the way it is.
 * os_debugModeInit is idempotent. */

void
os_debugModeInit()
{
    char *pathName;

    /* Only do this if no-one has called function before
     * unless os_debugModeExit has been called */
    if (!infoBlock) {
        pathName = os_getenv(REPORTFILE_PATHNAME);
        if (pathName) {
            infoBlock = (os_crtReportInfoBlock)os_malloc(sizeof(*infoBlock));
            infoBlock->warnInfo = setCrtMode(_CRT_WARN, REPORTFILE_WARNFILENAME,
                                             REPORTFILE_WARNDEFAULT, pathName);
            infoBlock->errorInfo = setCrtMode(_CRT_ERROR, REPORTFILE_ERRORFILENAME,
                                             REPORTFILE_ERRORDEFAULT, pathName);
            infoBlock->assertInfo = setCrtMode(_CRT_ASSERT, REPORTFILE_ASSERTFILENAME,
                                             REPORTFILE_ASSERTDEFAULT, pathName);
        }
    }
}

void
os_debugModeExit()
{
    if (infoBlock) {
        if (infoBlock->warnInfo) {
            restoreCrtMode(_CRT_WARN, infoBlock->warnInfo);
            infoBlock->warnInfo = NULL;
        }
        if (infoBlock->errorInfo) {
            restoreCrtMode(_CRT_ERROR, infoBlock->errorInfo);
            infoBlock->errorInfo = NULL;
        }
        if (infoBlock->assertInfo) {
            restoreCrtMode(_CRT_ASSERT, infoBlock->assertInfo);
            infoBlock->assertInfo = NULL;
        }
        os_free(infoBlock);
        infoBlock = NULL;
    }
}
