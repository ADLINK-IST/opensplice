/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/** \file os/win32/code/os_process.c
 *  \brief WIN32 process management
 *
 * Implements process management for WIN32
 */

#include "os_process.h"
#include "os__process.h"
#include "os__signalHandler.h"
#include "os_thread.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_time.h"
#include "os_report.h"
#include "os_signal.h"
#include "os_init.h"
#include "os__debug.h"
#include "os_atomics.h"
#include "os_errno.h"

//#include <stdio.h>
#include <cstdio>
//#include <assert.h>
#include <cassert>

/* List of exithandlers */
struct _ospl_handlerList_t {
    void (*handler)(void);
    struct _ospl_handlerList_t *next;
};

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

#define _OS_PROC_PROCES_NAME_LEN_ (512)

/* Protected functions */
void
os_processModuleInit(void)
{
    char *process_name;
    char *leaf_name;
    DWORD nSize;
    DWORD allocated = 0;
    wchar_t *tmp;
    wchar_t *lpFilename = NULL;

    /* Register an atexit to signal the abstraction layer that all threads may
     * be forcefully terminated.
     *
     * The restrictions imposed by atexit(...) and process termination under
     * Windows make that it doesn't make sense to run the routines registered
     * with os_procAtExit(...), since code in the atexit may not access shared
     * resources (like SHM and u_userDetach(...)) that can be left in an
     * undefined state. */
    (void) atexit(os__procExit);

    (void) os__signalHandlerNew();

    process_name = os_getenv("SPLICE_PROCNAME");
    if (process_name != NULL)
    {
        processName = process_name;
    }
    else
    {
        do {
            allocated++;
            tmp = (wchar_t*) os_realloc(lpFilename, allocated * _OS_PROC_PROCES_NAME_LEN_ * sizeof (wchar_t));
            if(tmp)
            {
                lpFilename = tmp;

                /* First parameter NULL retrieves module-name of executable */
                nSize = GetModuleFileName (NULL, lpFilename, allocated * _OS_PROC_PROCES_NAME_LEN_);
                if (nSize == 0)
                {
                    /* ensure ramdom memory can not get used for the process identifier */
                   lpFilename = NULL; /* Will break loop */
                }
            }
            else
            {
                /* Memory-claim denied, revert to default */
                if(lpFilename)
                {
                    os_free(lpFilename);
                    lpFilename = NULL; /* Will break loop */
                }
            }

            /* lpFilename will only be guaranteed to be NULL-terminated if nSize <
             * (allocated * _OS_PROC_PROCES_NAME_LEN_), so continue until that's true */
        } while (lpFilename && nSize >= (allocated * _OS_PROC_PROCES_NAME_LEN_));

        if(lpFilename)
        {
            process_name = wce_wctomb (lpFilename);
            leaf_name = strrchr (process_name, '\\');
            processName = leaf_name ? os_strdup (leaf_name + 1) : os_strdup (process_name);
            os_free(lpFilename);
            os_free(process_name);
        }
        else
        {
            processName = os_strdup("");
        }
    }

    return;
}

void
os_processModuleExit(void)
{
    struct _ospl_handlerList_t *oldHandler, *nextHandler;

    os__signalHandlerFree();
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

struct readPipeHelper{
    os_procId pid;
    HANDLE hChildStdoutWr;
    HANDLE hChildStdoutRd;
};

static void*
readFromPipe(
    void* args)
{
    DWORD dwRead, dwWritten;
    CHAR chBuf[4096];
    int proceed = 1;
    struct readPipeHelper* helper;

    helper = (struct readPipeHelper*)args;

    WaitForSingleObject ((HANDLE)helper->pid, 0);
    CloseHandle(helper->hChildStdoutWr);
    while (proceed) {
        if((!ReadFile(helper->hChildStdoutRd, chBuf, 4096, &dwRead, NULL)) ||
           (dwRead == 0)) {
            proceed = 0;
        }
        if (!WriteFile(stdout, chBuf, dwRead, &dwWritten, NULL)) {
            proceed = 0;
        }
    }
    CloseHandle(helper->hChildStdoutRd);
    os_free(helper);

    return NULL;
}

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
   //    STARTUPINFO si;                                        // mha - no StarupInfo in CE
   char *inargs;
   MSGQUEUEOPTIONS msgqOptionsWrite, msgqOptionsRead;

   SECURITY_ATTRIBUTES saAttr;
   struct readPipeHelper* helper;
   os_threadId tid;
   os_threadAttr tattr;

   LPWSTR w_executable_file;
   LPWSTR w_inargs;
   size_t size;

   //    assert(executable_file != NULL);
   //    assert(name != NULL);
   //    assert(arguments != NULL);
   assert(procAttr != NULL);
   assert(procId != NULL);

   //    inargs = (char*)os_malloc(strlen (name) + strlen (arguments) + 4);
   inargs = (char*)os_malloc(strlen (arguments) + 4);

   //    strcpy(inargs, "\"");
   //    strcat(inargs, name);
   //    strcat(inargs, " ");
   //    strcat(inargs, "\" ");
   //    strcat(inargs, arguments);
   strcpy(inargs, arguments);

   //     memset(&si, 0, sizeof(STARTUPINFO));
   //    si.cb = sizeof(STARTUPINFO);

   if (procAttr->activeRedirect)
   {
       saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
       saAttr.bInheritHandle = TRUE;
       saAttr.lpSecurityDescriptor = NULL;
       helper = (struct readPipeHelper*)os_malloc(sizeof(struct readPipeHelper));

       // write message queue options
       msgqOptionsWrite.dwFlags = MSGQUEUE_NOPRECOMMIT;
       msgqOptionsWrite.dwMaxMessages = 0;
       msgqOptionsWrite.cbMaxMessage = 4096;
       msgqOptionsWrite.bReadAccess = FALSE;
       msgqOptionsWrite.dwSize = sizeof(msgqOptionsWrite);
       // Read message queue options
       msgqOptionsRead.dwFlags = MSGQUEUE_NOPRECOMMIT;
       msgqOptionsRead.dwMaxMessages = 0;
       msgqOptionsRead.cbMaxMessage = 4096;
       msgqOptionsRead.bReadAccess = TRUE;
       msgqOptionsRead.dwSize = sizeof(msgqOptionsRead);

       // Create Read message queue
       helper->hChildStdoutRd = CreateMsgQueue(NULL, &msgqOptionsRead);
       // Create Write message queue
       helper->hChildStdoutWr = CreateMsgQueue(NULL, &msgqOptionsWrite);

       if (helper->hChildStdoutRd == INVALID_HANDLE_VALUE || helper->hChildStdoutWr == INVALID_HANDLE_VALUE)
       {
          OS_DEBUG_2("osServiceThread", "Failed to create pipe %s %d",
                     "", os_getErrno());
       }

       //        if (!CreatePipe(&(helper->hChildStdoutRd), &(helper->hChildStdoutWr), &saAttr, 0)) {
       //            OS_DEBUG_1("os_procCreate", "CreatePipe failed with %d", (int)GetLastError());
       //        }
       //        SetHandleInformation(helper->hChildStdoutRd, HANDLE_FLAG_INHERIT, 0);                                                          // mha - this flag not supported

       //        si.hStdError = helper->hChildStdoutWr;
       //        si.hStdOutput = helper->hChildStdoutWr;
       //        si.hStdInput = stdin;
       //        si.dwFlags |= STARTF_USESTDHANDLES;                                                                                                                            // mha - not supported on CE
   }

   size = mbstowcs(NULL, executable_file, 0);
   w_executable_file = (wchar_t *)os_malloc( (size + 1) * sizeof( wchar_t ));
   mbstowcs(w_executable_file, executable_file, size+1);

   size = mbstowcs(NULL, inargs, 0);
   w_inargs = (wchar_t *)os_malloc( (size + 1) * sizeof( wchar_t ));
   mbstowcs(w_inargs, inargs, size+1);

   if (CreateProcess(w_executable_file,
                     w_inargs,
                     NULL,                     // ProcessAttributes
                     NULL,                     // ThreadAttributes
                     FALSE,
                     0,
                     NULL,                     // Environment
                     NULL,                     // CurrentDirectory
                     NULL,
                     &process_info) == 0)
   {
      const os_int errorCode= os_getErrno();
      OS_DEBUG_1("os_procCreate", "Failed with %d", errorCode);
      os_free(inargs);
      os_free(w_executable_file);
      os_free(w_inargs);
      return (ERROR_FILE_NOT_FOUND == errorCode ||
              ERROR_PATH_NOT_FOUND == errorCode ||
              ERROR_ACCESS_DENIED  == errorCode)
      ? os_resultInvalid : os_resultFail;
   }

   *procId = (os_procId)process_info.hProcess;

   if (procAttr->activeRedirect) {
      helper->pid = (os_procId)process_info.hProcess;
      os_threadAttrInit(&tattr);
      os_threadCreate(&tid, "readFromPipeThread", &tattr, readFromPipe, helper);
      os_threadWaitExit(tid, NULL);
   }

   os_free(inargs);
   os_free(w_executable_file);
   os_free(w_inargs);

   /*
    * Windows CE has a different thread priority model to Windows.
    * It should use CeSetThreadPriority which takes a priority value
    * 0 - 255 (0 being highest priority and 255 the lowest).  Most
    * applications should use the 248 - 255 range with our default
    * CE_THREAD_PRIO_256_NORMAL mapping to value 251.
    *
    * However we should not restrict the user to the 248 - 255 range
    */

   if (CeSetThreadPriority(process_info.hThread, procAttr->schedPriority) == 0)
   {
      OS_DEBUG_1("os_procCreate", "CeSetThreadPriority failed with %d", os_getErrno());
   }
   CloseHandle(process_info.hThread);

   return os_resultSuccess;
}

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
    DWORD err;
    register int callstatus;

    assert(status != NULL);

    if (OS_INVALID_PID == procId) {
        return os_resultInvalid;
    }

    callstatus = GetExitCodeProcess((HANDLE)procId, &tr);

    if (callstatus == 0) {
        err = os_getErrno();
        if (err == ERROR_INVALID_HANDLE) {
            return os_resultUnavailable;
        }

        OS_DEBUG_1("os_procCheckStatus", "os_procCheckStatus GetExitCodeProcess Failed %d", err);
        return os_resultFail;
    }

    if (tr == STILL_ACTIVE) {
        return os_resultBusy;
    }

    CloseHandle ((HANDLE)procId);
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
    os_result result = os_resultSuccess;
    LPWSTR w_path;
    size_t size;
    MSGQUEUEOPTIONS destroyMessageOptions;
    HANDLE handle;
    char data[] = "destroy message";
    BOOL ret;

    if ((OS_INVALID_PID == procId) ||
        (signal != OS_SIGHUP  &&
         signal != OS_SIGINT  &&
         signal != OS_SIGKILL &&
         signal != OS_SIGALRM &&
         signal != OS_SIGTERM)) {
       return os_resultInvalid;
    }

    /* simulate pipe behaviour with CreateMsgQueue */
    snprintf(pname, 256, "osplpipe_%d", procId);

    size = mbstowcs(NULL, pname, 0);
    w_path = (wchar_t *)os_malloc( (size + 1) * sizeof( wchar_t ));
    mbstowcs(w_path, pname, size+1);

    destroyMessageOptions.dwFlags = MSGQUEUE_NOPRECOMMIT;
    destroyMessageOptions.dwSize = sizeof(destroyMessageOptions);
    destroyMessageOptions.dwMaxMessages = 0; // unlimited
    destroyMessageOptions.cbMaxMessage = 4096;
    destroyMessageOptions.bReadAccess = FALSE;

    handle = CreateMsgQueue(w_path,&destroyMessageOptions);
    if (handle == INVALID_HANDLE_VALUE)
    {
        OS_REPORT(OS_INFO, "os_procDestroy CreateMsgQueue FAIL :", 0, "os_getErrno() = %d\n", os_getErrno());
    }

    ret = WriteMsgQueue(handle,data,sizeof(data),INFINITE,MSGQUEUE_MSGALERT);
    if (ret == FALSE)
    {
        OS_REPORT(OS_INFO, "os_procDestroy WriteMsgQueue FAIL :", 0, "os_getErrno() = %d\n", os_getErrno());
    }

    os_free(w_path);
    return result;
}

os_result
os_procServiceDestroy(
    os_int32 pid,
    os_boolean isblocking,
    os_int32 checkCount)
{
    os_result osr;
    os_int32 procBusyChecks = 0;
    HANDLE hProcess;
    os_int32 procResult;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

    if (hProcess == INVALID_HANDLE_VALUE)
    {
        int err = os_getErrno();
        return os_resultFail;
    }
    else
    {
        /* remove process*/

        osr = os_procDestroy((os_procId)hProcess, OS_SIGTERM);
        if(osr != os_resultSuccess)
        {
            OS_REPORT(OS_ERROR,"os_procServiceDestroy",0, "Failed to send the term signal to the splice daemon process %d", (os_procId)hProcess);
        }
        osr = os_procCheckStatus((os_procId)hProcess, &procResult);

        procBusyChecks = checkCount;
        while ((osr == os_resultBusy) && (procBusyChecks > 0)) {
            procBusyChecks--;
            if ((procBusyChecks % 10) == 0) printf (".");
            fflush(stdout);
            Sleep(100);
            osr = os_procCheckStatus((os_procId)hProcess, &procResult);

        }
        printf ("\n");
        fflush(stdout);

        CloseHandle(hProcess);

        if (osr == os_resultBusy)
        {
            /* If we're going to try a pseudo sigkill we need the PROCESS_TERMINATE access control right */
            hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | (OS_SIGKILL ? PROCESS_TERMINATE : 0),
                                    FALSE, pid);
            if(hProcess != INVALID_HANDLE_VALUE)
            {
                if (os_procCheckStatus((os_procId)hProcess, &procResult) == os_resultBusy)
                {
                    /* A process is still running with specified PID.
                    Check the handles process creation times against those in the iterator
                    process info to make sure they are the same process. */

                    OS_REPORT(OS_ERROR,"os_procServiceDestroy", 0,
                                            "OpenSplice service process PID %u still running after spliced terminated." OS_REPORT_NL
                                            "Sending termination signal.",pid);
                    os_procDestroy ((os_procId)hProcess, OS_SIGKILL);

                    CloseHandle(hProcess);
                }

                if (os_procCheckStatus((os_procId)hProcess, &procResult) == os_resultBusy)
                {
                    /* A process is still running with specified PID.

                    /* Process matches. */
                    /* (100 x 0.1) Ten seconds for an orphaned process to expire subject
                    to a *kill* should be way more than enough (?). */
                    procBusyChecks = checkCount;
                    osr = os_procCheckStatus((os_procId)hProcess, &procResult);
                    while ((osr == os_resultBusy) && (procBusyChecks > 0))
                    {
                        procBusyChecks--;
                        if ((procBusyChecks % 10) == 0) printf (".");
                        fflush(stdout);
                        Sleep(100);
                        osr = os_procCheckStatus((os_procId)hProcess, &procResult);
                    }
                    printf ("\n");
                    fflush(stdout);

                    if (osr == os_resultBusy)
                    {
                        OS_REPORT(OS_CRITICAL,"os_procServiceDestroy", 0,
                                                "OpenSplice service process PID %u could not be terminated.",pid);
                    }
                }
                CloseHandle(hProcess);
            }
            else
            {
                osr = os_resultFail;
            }
        }
        else
        {
            osr = os_resultSuccess;
        }
    }

    return osr;
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
    procAttr->schedPriority = CE_THREAD_PRIO_256_NORMAL;
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
HANDLE
os_procIdToHandle(os_procId procId)
{
    return OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, procId);
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

os_int32
os_procFigureIdentity(
    char *procIdentity,
    os_uint procIdentitySize)
{
    os_int32 size = 0;
    char process_name[_OS_PROC_PROCES_NAME_LEN_];

    size = os_procGetProcessName(process_name,_OS_PROC_PROCES_NAME_LEN_);

    if (size > 0) {
        size = snprintf(procIdentity, procIdentitySize, "%s <%d>",
                process_name, os_procIdSelf());
    }
    else {
        /* No processname could be determined, so default to PID */
        size = snprintf(procIdentity, procIdentitySize, "<%d>",
                os_procIdSelf());
    }

    return size;
}

os_int32
os_procGetProcessName(
    char *procName,
    os_uint procNameSize)
{
    return ((os_int32)snprintf(procName, procNameSize, "%s", processName));
}
#undef _OS_PROC_PROCES_NAME_LEN_

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
    os_schedClass cl = OS_SCHED_DEFAULT;

//    DWORD prioClass;

//    prioClass = GetPriorityClass(GetCurrentProcess());                                // mha - Not available in CE
//    if (prioClass == OS_SCHED_DEFAULT/*REALTIME_PRIORITY_CLASS*/) {
//        cl = OS_SCHED_REALTIME;
//    }

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
    return CeGetThreadPriority(GetCurrentThread());
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

