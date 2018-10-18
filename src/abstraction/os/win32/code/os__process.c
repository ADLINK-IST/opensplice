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
/** \file os/win32/code/os_process.c
 *  \brief WIN32 process management
 *
 * Implements process management for WIN32
 */
#include "os_errno.h"
#include "os_process.h"
#include "os__process.h"
#include "os__signalHandler.h"
#include "os__thread.h"
#include "os_thread.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_time.h"
#include "os_report.h"
#include "os_signal.h"
#include "os_init.h"
#include "os__debug.h"
#include "os_atomics.h"

#include <process.h>
#include "os_win32incs.h"

/* List of exithandlers */
struct _ospl_handlerList_t {
    void (*handler)(void);
    struct _ospl_handlerList_t *next;

};

/* #642 fix : define mapping between scheduling abstraction and windows
 * Windows provides 6 scheduling classes for the process
 * IDLE_PRIORITY_CLASS
 * BELOW_NORMAL_PRIORITY_CLASS
 * NORMAL_PRIORITY_CLASS
 * ABOVE_NORMAL_PRIORITY_CLASS
 * HIGH_PRIORITY_CLASS
 * REALTIME_PRIORITY_CLASS */

/* These defaults should be modifiable through configuration */
static const os_schedClass TIMESHARE_DEFAULT_SCHED_CLASS = NORMAL_PRIORITY_CLASS;
static const os_schedClass REALTIME_DEFAULT_SCHED_CLASS  = REALTIME_PRIORITY_CLASS;

static pa_voidp_t _ospl_handlerList                  = PA_VOIDP_INIT(0);
static char* processName                             = NULL;
static os_boolean _ospl_threadsTerminatedByAtExit    = FALSE;

static void
os__procExit(
    void)
{
    /* When this function is called all threads may be ungracefully terminated. */
    _ospl_threadsTerminatedByAtExit = TRUE;
}

/* Protected functions */
void
os_processModuleInit(void)
{
    /* Register an atexit to signal the abstraction layer that all threads may
     * be forcefully terminated.
     *
     * The restrictions imposed by atexit(...) and process termination under
     * Windows make that it doesn't make sense to run the routines registered
     * with os_procAtExit(...), since code in the atexit may not access shared
     * resources (like SHM and u_userDetach(...)) that can be left in an
     * undefined state. */
    (void) atexit(os__procExit);
#ifndef LITE
    (void) os__signalHandlerNew();
#endif
    return;
}


void
os_processModuleExit(void)
{
    struct _ospl_handlerList_t *oldHandler, *nextHandler;
#ifndef LITE
    os__signalHandlerFree();
#endif
    os_free(processName);

    /* Free the handlers registered through os_procAtExit(...) */
    do {
        oldHandler = pa_ldvoidp(&_ospl_handlerList);
    } while (!pa_casvoidp(&_ospl_handlerList, oldHandler, NULL));

    while(oldHandler){
        nextHandler = oldHandler->next;
        os_free(oldHandler);
        oldHandler = nextHandler;
    }
}

void
os_procCallExitHandlers(void)
{
    struct _ospl_handlerList_t *oldHandler, *nextHandler;

    do {
        oldHandler = pa_ldvoidp(&_ospl_handlerList);
    } while (!pa_casvoidp(&_ospl_handlerList, oldHandler, NULL));

    while(oldHandler){
        if(!_ospl_threadsTerminatedByAtExit) {
            oldHandler->handler();
        }
        nextHandler = oldHandler->next;
        os_free(oldHandler);
        oldHandler = nextHandler;
    }
}

/* Public functions */

/** \brief Register an process exit handler
 *
 * \b os_procAtExit registers an process exit
 * handler the \b function
 * to be called when the process exists.
 *
 */
os_result
os_procAtExit(
    void (*function)(void))
{
    struct _ospl_handlerList_t *handler, *old;
    os_result result;

    assert(function != NULL);

    /* We don't implement 'atexit()' for Windows, since Windows calls the
     * handlers too late (after all threads have been terminated).
     *
     * The restrictions imposed by atexit(...) and process termination under
     * Windows make that it doesn't make sense to run the routines registered
     * with os_procAtExit(...), since code in the atexit may not access shared
     * resources (like SHM and u_userDetach(...)) that can be left in an
     * undefined state.
     *
     * The routines are stored for the case when os_procExit(...) is used to
     * terminate the process (e.g., as done in our service-wrappers), but they
     * aren't invoked on atexit(...). */

    handler = os_malloc(sizeof *handler);
    if (handler) {
        handler->handler = function;
        do {
            old = pa_ldvoidp(&_ospl_handlerList);
            handler->next = old;
        } while (!pa_casvoidp(&_ospl_handlerList, old, handler));
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }
    return result;
}

/** \brief Returns if threads are terminated by atexit
 *
 * Return values:
 * TRUE - if threads are ungracefully terminated by atexit
 * FALSE - all other situations
 */
os_boolean
os_procAreThreadsTerminatedByAtExit(
    void)
{
    return _ospl_threadsTerminatedByAtExit;
}

/** \brief Terminate the process and return the status
 *         the the parent process
 *
 * \b os_procExit terminates the process by calling \b exit.
 */
void
os_procExit(
    os_exitStatus status)
{
    os_procCallExitHandlers();
    exit((int)status);
}

#define OS_STRLEN_SPLICE_PROCNAME (16) /* strlen("SPLICE_PROCNAME="); */

/** \brief Create a process that is an instantiation of a program
 *
 * First an argument list is build from \b arguments.
 * Then \b os_procCreate creates a process by forking the current
 * process.
 *
 * The child process processes the lock policy attribute from
 * \b procAttr and sets the lock policy accordingly by calling
 * \b mlockall if required. If the process has root privileges
 * it processes the user credentials from \b procAttr and sets
 * the user credentials of the child process accordingly.
 * The child precess then replaces the running program with the
 * program provided by the \b executable_file by calling \b execve.
 *
 * The parent process processes the scheduling class and
 * scheduling priority attributes from \b procAttr and
 * sets the scheduling properties of the child process
 * accordingly by calling \b sched_setscheduler.
 */
os_result
os_procCreate(
    const char *executable_file,
    const char *name,
    const char *arguments,
    os_procAttr *procAttr,
    os_procId *procId)
{
    PROCESS_INFORMATION process_info;
    STARTUPINFO si;
    char *inargs;
    LPTCH environment;
    LPTCH environmentCopy;

    os_schedClass effective_process_class;
    os_int32 effective_priority;
    //    assert(executable_file != NULL);
    //    assert(name != NULL);
    //    assert(arguments != NULL);
    assert(procAttr != NULL);
    assert(procId != NULL);

    inargs = (char*)os_malloc(strlen (name) + strlen (arguments) + 4);

    os_strcpy(inargs, "\"");
    os_strcat(inargs, name);
    os_strcat(inargs, "\" ");
    os_strcat(inargs, arguments);

    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);

    if (procAttr->activeRedirect) {
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.dwFlags |= STARTF_USESTDHANDLES;
    }

    /* Duplicate the environment of the current process for the child-process
     * and set the SPLICE_PROCNAME environment variable to name.
     * CreateEnvironmentBlock cannot be used conveniently here, since the
     * environment has to be modified and SPLICE_PROCNAME can be set in the
     * current process too. */
    environment = GetEnvironmentStrings();
    if(environment){
        size_t len = 0;
        size_t newLen = 0;
        size_t spliceVarLen = 0;
        LPTSTR currentVar = (LPTSTR)environment;
        LPTSTR spliceVar = currentVar; /* If all vars are larger, then insert has to happen at beginning */
        int equals;

        /* The environment block needs to be ordered */
        while(*currentVar){
            if(spliceVarLen == 0){ /* Only do strncmp if EXACT location not found */
                equals = strncmp("SPLICE_PROCNAME=", currentVar, OS_STRLEN_SPLICE_PROCNAME);
                if(equals == 0){
                    spliceVar = currentVar; /* Mark address where replacement has to happen */
                    spliceVarLen = lstrlen(spliceVar) + 1;
                } else if (equals > 0){
                    /* Mark address where insertion has to happen */
                    spliceVar = currentVar + lstrlen(currentVar) + 1;
                }
            }
            len += lstrlen(currentVar) + 1;
            currentVar = environment + len;
        }

        newLen = len - spliceVarLen
            + OS_STRLEN_SPLICE_PROCNAME + strlen(name) + 1;

        environmentCopy = (LPTCH)os_malloc(newLen + 1 /* End of array */);
        if(environmentCopy){
            size_t until = spliceVar - environment;
            size_t newSpliceVarLen;
            /* First copy up until location of spliceVar */
            memcpy(environmentCopy, environment, until);
            /* Now write the new SPLICE_PROCNAME */
            newSpliceVarLen = sprintf(environmentCopy + until, "SPLICE_PROCNAME=%s", name) + 1;
            /* Now copy tail */
            memcpy(environmentCopy + until + newSpliceVarLen, spliceVar + spliceVarLen, len - (until + spliceVarLen));
            environmentCopy[newLen] = (TCHAR)0;
        } else {
            OS_REPORT(OS_ERROR,
                        "os_procCreate", 1,
                        "Out of (heap) memory.");
        }
        FreeEnvironmentStrings(environment);
    } else {
        OS_REPORT(OS_ERROR,
                "os_procCreate", 1,
                "GetEnvironmentStrings failed, environment will be inherited from parent-process without modifications.", os_getErrno());
    }

    if (CreateProcess(executable_file,
                      inargs,
                      NULL,                     // ProcessAttributes
                      NULL,                     // ThreadAttributes
                      procAttr->activeRedirect, // InheritHandles
                      CREATE_NO_WINDOW,         // dwCreationFlags
                      (LPVOID)environmentCopy,  // Environment
                      NULL,                     // CurrentDirectory
                      &si,
                      &process_info) == 0) {
        const os_int errorCode = os_getErrno ();
        OS_DEBUG_2("os_procCreate", "Process creation for exe file %s failed with %d", executable_file, errorCode);
        os_free(inargs);
        return (ERROR_FILE_NOT_FOUND == errorCode ||
                ERROR_PATH_NOT_FOUND == errorCode ||
                ERROR_ACCESS_DENIED  == errorCode)
               ? os_resultInvalid : os_resultFail;
    }

    if(environmentCopy){
        os_free(environmentCopy);
    }
    *procId = process_info.dwProcessId;

    os_free(inargs);

/* Check to see if the client has requested "realtime" behaviour
   via the OS_SCHED_REALTIME abstraction
 */
/* #642 fix */
    if (procAttr->schedClass == OS_SCHED_REALTIME) {
        effective_process_class = REALTIME_DEFAULT_SCHED_CLASS;
    } else {
        effective_process_class = TIMESHARE_DEFAULT_SCHED_CLASS;
    }
    if (SetPriorityClass(process_info.hProcess, effective_process_class) == 0) {
        OS_DEBUG_1("os_procCreate", "SetPriorityClass failed with %d", os_getErrno());
    }

    effective_priority = procAttr->schedPriority;
    if (effective_process_class == REALTIME_PRIORITY_CLASS) {
        if (procAttr->schedPriority < -7) {
            effective_priority = THREAD_PRIORITY_IDLE;
        }
        if (procAttr->schedPriority > 6) {
            effective_priority = THREAD_PRIORITY_TIME_CRITICAL;
        }
    } else {
        if (procAttr->schedPriority < THREAD_PRIORITY_LOWEST) {
            effective_priority = THREAD_PRIORITY_IDLE;
        }
        if (procAttr->schedPriority > THREAD_PRIORITY_HIGHEST) {
            effective_priority = THREAD_PRIORITY_TIME_CRITICAL;
        }
    }
    if (effective_priority != THREAD_PRIORITY_NORMAL) {
        if (SetThreadPriority(process_info.hThread, procAttr->schedPriority) == 0) {
            OS_DEBUG_1("os_procCreate", "SetThreadPriority failed with %d", os_getErrno());
        }
    }

    return os_resultSuccess;
}

#undef OS_STRLEN_SPLICE_PROCNAME

/** \brief Check the child exit status of the identified process
 *
 * \b os_procCheckStatus calls \b waitpid with flag \b WNOHANG
 * to check the status of the child process.
 * - On return of \b waitpid with result \b procId, the process
 *   has terminated and its result status is in \b status.
 * - On return of \b waitpid with result 0, the child process is
 *   not yet terminated and \b os_resultBusy is returned.
 * - On return of \b waitpid with result -1 and \b errno is \b ECHILD
 *   the identified child process is not found and \b
 *   os_resultUnavailable is returned.
 * - On any other return from \b waitpid, \b os_resultFail is returned.
 */
os_result
os_procCheckStatus(
    os_procId procId,
    os_int32 *status)
{
    DWORD tr;
    int err;
    HANDLE procHandle;
    register int callstatus;

    if (procId == OS_INVALID_PID) {
        return os_resultInvalid;
    }
    assert(status != NULL);
    procHandle = os_procIdToHandle(procId);

    callstatus = GetExitCodeProcess(procHandle, &tr);
    if (callstatus == 0) {
        err = os_getErrno();
        CloseHandle (procHandle);
        if (err == ERROR_INVALID_HANDLE) {
            return os_resultUnavailable;
        }

        OS_DEBUG_1("os_procCheckStatus", "os_procCheckStatus GetExitCodeProcess Failed %d", err);
        return os_resultFail;
    }
    CloseHandle (procHandle);

    if (tr == STILL_ACTIVE) {
        return os_resultBusy;
    }

    *status = (os_int32)tr;
    return os_resultSuccess;
}

/** \brief Send a signal to the identified process
 *
 * \b os_procDestroy sends the dignal \b signal
 * to process \b procId by calling \b kill with
 * the same parameters.
 *
 * - If \b kill returns a value != -1, \b os_resultSuccess is
 *   returned.
 * - If \b kill returns with -1 and \b errno is set to \b EINVAL,
 *   \b os_resultInvalid is returned to indicate an invalid
 *   parameter.
 * - If \b kill returns with -1 and \b errno is set to \b ESRCH,
 *   \b os_resultUnavailable is returned to indicate that
 *   the identified process is not found.
 * - On any other return from \b kill, \b os_resultFail is returned.
 */
os_result
os_procDestroy(
    os_procId procId,
    os_int32 signal)
{
    char pname[256];
    HANDLE ph;
    os_result result;
    DWORD written;
    const os_char* errMsg;
    HANDLE procHandle;

    if (procId == OS_INVALID_PID) {
        return os_resultInvalid;
    }
    procHandle = os_procIdToHandle(procId);
    if (!procHandle)
    {
        errMsg = os_strError(os_getErrno());
    }
    if (signal == OS_SIGKILL)
    {
        result = (TerminateProcess(procHandle, 1) == 0 ? os_resultFail : os_resultSuccess);
        if (result == os_resultFail)
        {
            if (os_getErrno() == ERROR_ACCESS_DENIED)
            {
                OS_REPORT (OS_ERROR, "os_procDestroy", 0,
                        "OS_SIGKILL / TerminateProcess of PID %d failed - HANDLE procId lacks TERMINATE_PROCESS "
                        OS_REPORT_NL "access right or process has already terminated.", procId);
            }
            else
            {
                OS_REPORT (OS_ERROR, "os_procDestroy", 0,
                        "OS_SIGKILL / TerminateProcess of PID %d failed: %s", procId, os_strError (os_getErrno ()));
            }
        }
        CloseHandle(procHandle);
        return result;
    }

    if (signal != OS_SIGHUP  &&
        signal != OS_SIGINT  &&
        signal != OS_SIGALRM &&
        signal != OS_SIGTERM) {
       return os_resultInvalid;
    }

    result = os_resultSuccess;
    snprintf(pname, 256, "\\\\.\\pipe\\osplpipe_%d", procId);
    ph = CreateFile(pname,
                    GENERIC_WRITE, 0,
                    NULL, OPEN_EXISTING,
                    0, NULL);
    if (ph != INVALID_HANDLE_VALUE) {
       if (os_getErrno() != ERROR_PIPE_BUSY) {
          written = 0;
          WriteFile(ph, &signal, sizeof(os_int32), &written, NULL);
          if (written == 0) {
             result = os_resultFail;
          }
       }
       CloseHandle(ph);
    } else {
        if (os_getErrno() == ERROR_FILE_NOT_FOUND ) {
            result = os_resultUnavailable;
        } else {
            result = os_resultFail;
        }
    }
    CloseHandle(procHandle);
    return result;
}

/** \brief Terminate process \b pid by progressive use of stronger kill
 * signals followed by \b checkcount cycles of status tests to confirm
 * action of kill signal. Checkcount cycles are 100ms long.
 * Blocking state is passed via \b isblocking
 *
 * \b returns appropriate \b os_resultSuccess or Fail following final
 * status check
 */

os_result
os_procServiceDestroy(
    os_int32 pid,
    os_boolean isblocking,
    os_int32 checkcount)
{
    os_result osr;
    os_int32 procBusyChecks = 0;
    os_procId procID;
    HANDLE procHandle;
    os_int32 procResult;

    procID = (os_procId)pid;
    if (procID == OS_INVALID_PID)
    {
        return os_resultFail;
    }
    else if (os_procIdToHandle(procID) == 0)
    {
        if (os_getErrno() == ERROR_ACCESS_DENIED)
        {
            OS_REPORT(OS_INFO,"ospl",0, "Failed to open process %d no access", pid);
            return os_resultFail;
        }
        /* the process already died so no need to remove it */
    }
    else
    {
        /* remove process*/
        osr = os_procDestroy(procID, OS_SIGTERM);
        if(osr != os_resultSuccess)
        {
            OS_REPORT(OS_INFO,"ospl",0, "Failed to send the SIGTERM signal to the splice daemon process %d", procID);
        }
        osr = os_procCheckStatus(procID, &procResult);

        procBusyChecks = checkcount;
        while ((osr == os_resultBusy) && (procBusyChecks > 0)) {
            procBusyChecks--;
            if ((procBusyChecks % 10) == 0) printf (".");
            fflush(stdout);
            Sleep(100);
            osr = os_procCheckStatus(procID, &procResult);

        }
        printf ("\n");
        fflush(stdout);

        if (osr == os_resultBusy)
        {
            /* If we're going to try a pseudo sigkill we need the PROCESS_TERMINATE access control right */
            procHandle = OpenProcess(PROCESS_QUERY_INFORMATION | (OS_SIGKILL ? PROCESS_TERMINATE : 0),
                                    FALSE, pid);
            procID = os_handleToProcId(procHandle);
            if (procHandle != INVALID_HANDLE_VALUE)
            {
                if (os_procCheckStatus(procID, &procResult) == os_resultBusy)
                {
                    /* A process is still running with specified PID.
                    Check the handles process creation times against those in the iterator
                    process info to make sure they are the same process. */

                    OS_REPORT(OS_INFO,"ospl", 0,
                                            "OpenSplice service process PID %u still running after spliced terminated." OS_REPORT_NL
                                            "Sending SIGKILL signal.",pid);
                    os_procDestroy (procID, OS_SIGKILL);

                    CloseHandle(procHandle);
                }

                if (os_procCheckStatus(procID, &procResult) == os_resultBusy)
                {
                    /* A process is still running with specified PID.

                    /* Process matches. */
                    /* (100 x 0.1) Ten seconds for an orphaned process to expire subject
                    to a *kill* should be way more than enough (?). */
                    procBusyChecks = checkcount;
                    osr = os_procCheckStatus(procID, &procResult);
                    while ((osr == os_resultBusy) && (procBusyChecks > 0))
                    {
                        procBusyChecks--;
                        if ((procBusyChecks % 10) == 0) printf (".");
                        fflush(stdout);
                        Sleep(100);
                        osr = os_procCheckStatus(procID, &procResult);
                    }
                    printf ("\n");
                    fflush(stdout);

                    if (osr == os_resultBusy)
                    {
                        OS_REPORT(OS_CRITICAL,"ospl", 0,
                                                "OpenSplice service process PID %u could not be terminated.",pid);
                    }
                }
                CloseHandle(procHandle);
            }
            else
            {
                return os_resultFail;
            }
        }
    }

    return os_resultSuccess;
}



/** \brief Initialize process attributes
 *
 * Set \b procAttr->schedClass to \b OS_SCHED_DEFAULT
 * (take the platforms default scheduling class, Time-sharing for
 * non realtime platforms, Real-time for realtime platforms)
 * Set \b procAttr->schedPriority to \b 0
 * Set \b procAttr->lockPolicy to \b OS_LOCK_DEFAULT
 * (no locking on non realtime platforms, locking on
 * realtime platforms)
 * Set \b procAttr->userCred.uid to 0
 * (don't change the uid of the process)
 * Set \b procAttr->userCred.gid to 0
 * (don't change the gid of the process)
 */
void
os_procAttrInit(
    os_procAttr *procAttr)
{
    assert(procAttr != NULL);
    procAttr->schedClass = OS_SCHED_DEFAULT;
    procAttr->schedPriority = THREAD_PRIORITY_NORMAL;
    procAttr->lockPolicy = OS_LOCK_DEFAULT;
    procAttr->userCred.uid = 0;
    procAttr->userCred.gid = 0;
    procAttr->activeRedirect = 0;
}

/** \brief Return the process ID of the calling process
 *
 * Possible Results:
 * - returns the process ID of the calling process
 */
os_procId
os_procIdSelf(void)
{
   /* returns a pseudo HANDLE to process, no need to close it */
   return GetProcessId (GetCurrentProcess());
}

#if (_WIN32_WINNT <= 0x0502)
static
DWORD
os__processQueryFlag(void)
{
    DWORD flag;

    /* For Windows Server 2003 and Windows XP:  PROCESS_QUERY_LIMITED_INFORMATION is not supported.
     * PROCESS_QUERY_INFORMATION need higher access permission which are not always granted when using OpenSplice
     * as a windows service due to UAC so use PROCESS_QUERY_LIMITED_INFORMATION instead */
    if(LOBYTE(LOWORD(GetVersion())) < 6) {
        flag = PROCESS_QUERY_INFORMATION;
    } else {
        flag = 0x1000; /* PROCESS_QUERY_LIMITED_INFORMATION */
    }
    return flag;
}
#else
# define os__processQueryFlag() PROCESS_QUERY_LIMITED_INFORMATION
#endif

HANDLE
os_procIdToHandle(os_procId procId)
{
    return OpenProcess(os__processQueryFlag() | SYNCHRONIZE | PROCESS_TERMINATE, FALSE, procId);
}
os_procId
os_handleToProcId(HANDLE procHandle)
{
    return GetProcessId(procHandle);
}

/** \brief Figure out the identity of the current process
 *
 * Possible Results:
 * - returns the actual length of procIdentity
 *
 * Postcondition:
 * - \b procIdentity is ""
 *     the process identity could not be determined
 * - \b procIdentity is "<decimal number>"
 *     only the process numeric identity could be determined
 * - \b procIdentity is "name <pid>"
 *     the process name and numeric identity could be determined
 *
 * \b procIdentity will not be filled beyond the specified \b procIdentitySize
 */
#define _OS_PROC_PROCES_NAME_LEN (512)
os_int32
os_procFigureIdentity(
    char *procIdentity,
    os_uint procIdentitySize)
{
    int size = 0;
    char process_name[_OS_PROC_PROCES_NAME_LEN];

    size = os_procGetProcessName(process_name,_OS_PROC_PROCES_NAME_LEN);

    if (size > 0) {
        size = snprintf(procIdentity, procIdentitySize, "%s <%d>",
                process_name, os_procIdSelf());
    }
    else {
        /* No processname could be determined, so default to PID */
        size = snprintf(procIdentity, procIdentitySize, "<%d>",
                os_procIdSelf());
    }

    return (os_int32)size;
}

os_int32
os_procGetProcessName(
    char *procName,
    os_uint procNameSize)
{
    int size = 0;
    char *process_name = NULL;
    char *process_env_name;
    char *exec = NULL;

    if (processName == NULL) {
        /* free is done in os_processModuleExit() */
        processName = (char*) os_malloc(_OS_PROC_PROCES_NAME_LEN);
        *processName = '\0';
        process_env_name = os_getenv("SPLICE_PROCNAME");
        if (process_env_name != NULL) {
            size = snprintf(processName, _OS_PROC_PROCES_NAME_LEN, "%s",process_env_name);
        } else {
            char *tmp;
            DWORD nSize;
            DWORD allocated = 0;
            do {
                   /* While procNameSize could be used (since the caller cannot
                    * store more data anyway, it is not used. This way the amount that
                    * needs to be allocated to get the full-name can be determined. */
                   allocated++;
                   tmp = (char*) os_realloc(process_name, allocated * _OS_PROC_PROCES_NAME_LEN);
                   if(tmp){
                       process_name = tmp;

                       /* First parameter NULL retrieves module-name of executable */
                       nSize = GetModuleFileNameA (NULL, process_name, allocated * _OS_PROC_PROCES_NAME_LEN);
                   } else {
                       /* Memory-claim denied, revert to default */
                       size = 0;
                       if(process_name){
                           os_free(process_name);
                           process_name = NULL; /* Will break loop */
                       }
                   }

               /* process_name will only be guaranteed to be NULL-terminated if nSize <
                * (allocated * _OS_PROC_PROCES_NAME_LEN), so continue until that's true */
               } while (process_name && nSize >= (allocated * _OS_PROC_PROCES_NAME_LEN));

            if(process_name){
                exec = strrchr(process_name,'\\');
                if (exec) {
                    /* skip all before the last '\' */
                    exec++;
                    snprintf(processName, _OS_PROC_PROCES_NAME_LEN, "%s", exec);
                } else {
                    snprintf(processName, _OS_PROC_PROCES_NAME_LEN, "%s", process_name);
                }
                os_free(process_name);
            }
        }
    }
    size = snprintf(procName, procNameSize, "%s", processName);
    return (os_int32)size;
}
#undef _OS_PROC_PROCES_NAME_LEN

/** \brief Get the process effective scheduling class
 *
 * Possible Results:
 * - process scheduling class is OS_SCHED_REALTIME
 * - process scheduling class is OS_SCHED_TIMESHARE
 * - process scheduling class is OS_SCHED_DEFAULT in
 *   case class could not be determined
 */
os_schedClass
os_procAttrGetClass(void)
{
    os_schedClass cl = OS_SCHED_TIMESHARE;
    DWORD prioClass;

    prioClass = GetPriorityClass(GetCurrentProcess());
    if (prioClass == REALTIME_PRIORITY_CLASS) {
        cl = OS_SCHED_REALTIME;
    }

    return cl;
}

/** \brief Get the process effective scheduling priority
 *
 * Possible Results:
 * - any platform dependent valid priority
 */
os_int32
os_procAttrGetPriority(void)
{
    return GetThreadPriority(GetCurrentThread());
}

os_result
os_procMLockAll(
    os_uint flags)
{
    OS_REPORT(OS_ERROR, "os_procMLockAll", 0, "Page locking not support on WIN32");
    return os_resultFail;
}

os_result
os_procMLock(
    const void *addr,
    os_address length)
{
    OS_REPORT(OS_ERROR, "os_procMLock", 0, "Page locking not support on WIN32");
    return os_resultFail;
}

os_result
os_procMUnlock(
    const void *addr,
    os_address length)
{
    OS_REPORT(OS_ERROR, "os_procMUnlock", 0, "Page locking not support on WIN32");
    return os_resultFail;
}

os_result
os_procMUnlockAll(void)
{
    OS_REPORT(OS_ERROR, "os_procMUnlockAll", 0, "Page locking not support on WIN32");
    return os_resultFail;
}
