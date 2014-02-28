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
/** \file os/win32/code/os_process.c
 *  \brief WIN32 process management
 *
 * Implements process management for WIN32
 */
#include "os_process.h"
#include "os__process.h"
#include "os__thread.h"
#include "os_thread.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_time.h"
#include "os_report.h"
#include "os_signal.h"
#include "os_init.h"
#include "os__debug.h"

#include <stdio.h>
#include <assert.h>
#include <process.h>

/* List of exithandlers */
typedef void *(*_ospl_exitHandler)(void);

struct _ospl_handlerList_t {
    _ospl_exitHandler          handler;
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


static struct _ospl_handlerList_t *_ospl_handlerList = (struct _ospl_handlerList_t *)0;
static HANDLE _ospl_exceptionHandlerThreadId         = 0;
static int    _ospl_exceptionHandlerThreadTerminate  = 0;
static HANDLE _ospl_exceptionEvent                   = 0;
static os_procTerminationHandler _ospl_termHandler   = (os_procTerminationHandler)0;
/* The signal emulator has to be removed in the future. */
static HANDLE _ospl_signalEmulatorHandle             = INVALID_HANDLE_VALUE;
static HANDLE _ospl_signalEmulatorThreadId           = 0;
static char* processName                             = NULL;

static BOOL
CtrlHandler(
    DWORD fdwCtrlType)
{
    os_int32 terminate;

    switch (fdwCtrlType) {
    case CTRL_LOGOFF_EVENT:
        /* It is not correct to ever exit when this event is received. see dds#2732
        From MSDN re CTRL_LOGOFF_EVENT:
        "A signal that the system sends to all console processes when a user is
        logging off. This signal does not indicate which user is logging off, so
        no assumptions can be made. Note that this signal is received only by
        services. Interactive applications are terminated at logoff, so they are
        not present when the system sends this signal." */
        return TRUE;
    }
/*
    BOOL result;

    switch (fdwCtrlType) {
    case CTRL_C_EVENT:
        result = TRUE;
        break;
    case CTRL_CLOSE_EVENT:
        result = TRUE;
        break;
    case CTRL_BREAK_EVENT:
        result = TRUE;
        break;
    case CTRL_LOGOFF_EVENT:
        result = TRUE;
        break;
    case CTRL_SHUTDOWN_EVENT:
        result = TRUE;
        break;
    default:
        all other types just refer to next handler
        result = FALSE;
        break;
    }
*/
    if(os_serviceGetSingleProcess())
    {
        /* ES 08-aug-2011 in case of ctrl-c being used in single process mode,
         * then bypass everything and terminate without calling any exit handlers.
         * This is a temporary measure.
         */
        _exit(0);
    }
    if (_ospl_termHandler) {
        terminate = _ospl_termHandler(OS_TERMINATION_NORMAL);
    } else {
        terminate = 1;
    }

    if (terminate) {
        os_procCallExitHandlers();
        return FALSE; /* always call the next control handler */
    } else {
        return TRUE; /* do not call the next control handler */
    }
}

static void *
exceptionHandlerThread(
    void *arg)
{
    DWORD result;

    do {
        result = WaitForSingleObject(_ospl_exceptionEvent, INFINITE);
    } while (result != WAIT_OBJECT_0);

    if (!_ospl_exceptionHandlerThreadTerminate) {
        os_procCallExitHandlers();
    }

    return NULL;
}

static LONG CALLBACK exceptionHandler(
  __in PEXCEPTION_POINTERS ExceptionInfo)
{
    DWORD nRead;
    os_result osr;

    /* We need to clear the Direction Flag first as we might have received this
     * exception during copy constructions (like memcpy, strcpy etc, and then
     * the direction flag is set).
     * The instruction movs is used to copy source string into the destination.
     * Since we'd like to move several bytes at a time, movs instructions are
     * done in batches using rep prefix. The number of movements is specified
     * by CX register. See the example below:
     *     :
     *    lds   si, [src]
     *    les   di, [dest]
     *    cld
     *    mov   cx, 100
     *    rep   movsb
     *     :
     *
     * This example will copy 100 bytes from src to dest. If you remove the rep
     * prefix, the CX register will have no effect. You will move one byte. Assembly
     * gurus use this instruction a lot, because arrays can be copied in the very
     * same way. You can use this to emulate C/C++'s strcpy
     *
     */
    //__asm__ __volatile__ ("pushfq ; popq %0" : "=g" (rflags));
    //__asm__ __volatile__ ("cld")
    printf("Exception code: %d\n", ExceptionInfo->ExceptionRecord->ExceptionCode);
    SetEvent(_ospl_exceptionEvent);
    if (GetExitCodeThread(_ospl_exceptionHandlerThreadId, &nRead) == 0) {
        printf("GetExitCodeThread Failed %d", (int)GetLastError());
        osr = os_resultFail;
    } else {
        while (nRead == STILL_ACTIVE) {
            Sleep(500);
            if (GetExitCodeThread(_ospl_exceptionHandlerThreadId, &nRead) == 0) {
                printf("GetExitCodeThread Failed %d", (int)GetLastError());
                osr = os_resultFail;
                nRead = 0; /* break loop */
            }
            printf("Wait for exception handler\n");
        }
        printf("exception handler done\n");
    }
    //if (rflags & (1 << 10)) { /* direction flag was set, so reset */
    //__asm__ __volatile__ ("std");
    //}
    return EXCEPTION_CONTINUE_SEARCH;
}

static void *
signalEmulatorThread(
    void *arg)
{
    os_int32 sig;
    DWORD nread;
    BOOL result;
    os_time delay = { 0, 100000000 }; /* 100millesec */
    int terminate = 0;

    os_threadSetThreadName(-1, "ospSigEmulator OSPL Signal Emulator Thread");

    while (!terminate) {
        result = (ConnectNamedPipe(_ospl_signalEmulatorHandle, NULL)?TRUE:(GetLastError() == ERROR_PIPE_CONNECTED));
        if (result) {
            /* read signal */
            sig = 0;
            result = ReadFile(_ospl_signalEmulatorHandle, &sig, sizeof(sig), &nread, NULL);
            if (result && (nread > 0)) {
                OS_DEBUG_1("signalEmulatorThread", "Received signal %d", sig);
                terminate = 1;
            }
        } else {
            os_nanoSleep(delay);
        }
    }
    if (sig == OS_SIGKILL) {
        exit(0);
    }
    if (sig > 0) {
        if (_ospl_termHandler) {
            _ospl_termHandler(OS_TERMINATION_NORMAL);
        }
    }
    CloseHandle(_ospl_signalEmulatorHandle);
    _ospl_signalEmulatorHandle = INVALID_HANDLE_VALUE;

    return NULL;
}

static void
installSignalEmulator(void)
{
    DWORD threadIdent;
    char pname[256];

/* First create the pipe */
if (_ospl_signalEmulatorHandle == INVALID_HANDLE_VALUE) {
    snprintf(pname, 256, "\\\\.\\pipe\\osplpipe_%d", _getpid());
    _ospl_signalEmulatorHandle = CreateNamedPipe(pname,
           PIPE_ACCESS_INBOUND,
           PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
           1 /* max instances */,
           sizeof(os_int32),
           sizeof(os_int32),
           200 /* timeout in millisec */,
           NULL);
    if (_ospl_signalEmulatorHandle == INVALID_HANDLE_VALUE) {
        OS_DEBUG_1("installSignalEmulator", "failure named pipe: %d", GetLastError());
        assert(_ospl_signalEmulatorHandle != INVALID_HANDLE_VALUE);
        return;
    }

    _ospl_signalEmulatorThreadId = CreateThread(NULL,
                    (SIZE_T)1024*1024,
                    (LPTHREAD_START_ROUTINE)signalEmulatorThread,
                    (LPVOID)0,
                    (DWORD)0, &threadIdent);
}
}
#define _OS_PROC_PROCES_NAME_LEN (512)
/* Protected functions */
void
os_processModuleInit(void)
{
/*    DWORD threadIdent;*/

    installSignalEmulator();

    /* first setup termination handling */
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

    /* Setup exception handling to ensure proper disconnect when
     * exception (like SEGV) occurs
     */
#if 0
    _ospl_exceptionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    AddVectoredExceptionHandler(1, exceptionHandler);

    _ospl_exceptionHandlerThreadId = CreateThread(NULL,
                (SIZE_T)128*1024,
                (LPTHREAD_START_ROUTINE)exceptionHandlerThread,
                (LPVOID)0,
                (DWORD)0, &threadIdent);
#endif
}
#undef _OS_PROC_PROCES_NAME_LEN

void
os_processModuleExit(void)
{
    os_int32 signal = -15; /* use negative number to stop the thread */
    DWORD written;

    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, FALSE);
    if (processName) {
        os_free(processName);
    }

    if (_ospl_signalEmulatorHandle != INVALID_HANDLE_VALUE) {
        WriteFile(_ospl_signalEmulatorHandle, &signal, sizeof(os_int32), &written, NULL);
    }

/*    RemoveVectoredExceptionHandler(exceptionHandler);
    _ospl_exceptionHandlerThreadTerminate = 1;
    SetEvent(_ospl_exceptionEvent); /* terminate exceptionHandlerThread */
}

void
os_procCallExitHandlers(void)
{
    struct _ospl_handlerList_t *handler;

    handler = _ospl_handlerList;
    while (handler) {
        handler->handler();
        handler = handler->next;
        os_free(_ospl_handlerList);
        _ospl_handlerList = handler;
    }
}

/* Public functions */

os_procTerminationHandler
os_procSetTerminationHandler(
    os_procTerminationHandler handler)
{
    os_procTerminationHandler oldHandler;

    oldHandler = _ospl_termHandler;
    _ospl_termHandler = handler;
    return oldHandler;
}

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
    struct _ospl_handlerList_t *handler;
    os_result result;

    assert(function != NULL);

    /**
     * Do not use 'atexit()' for windows, since windows
     * calls the handlers too late (after all threads
     * have been terminated.
     * Instead we keep our own administration.
     */
    handler = os_malloc(sizeof(struct _ospl_handlerList_t));
    if (handler) {
        handler->handler = (_ospl_exitHandler)function;
        handler->next = _ospl_handlerList;
        _ospl_handlerList = handler;
        result = os_resultSuccess;
    } else
    {
        result = os_resultFail;
    }
    return result;
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
    struct _ospl_handlerList_t *handler;

    assert((status != OS_EXIT_SUCCESS) || (status != OS_EXIT_FAILURE));
    /**
     * First call all exit handlers then call exit()
     */
    handler = _ospl_handlerList;
    while (handler) {
        handler->handler();
        handler = handler->next;
        os_free(_ospl_handlerList);
        _ospl_handlerList = handler;
    }
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

    SECURITY_ATTRIBUTES saAttr;

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
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;
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
        OS_REPORT_1(OS_ERROR,
                "os_procCreate", 1,
                "GetEnvironmentStrings failed, environment will be inherited from parent-process without modifications.", (int)GetLastError());
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
        const DWORD errorCode= GetLastError();
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

    *procId = (os_procId)process_info.hProcess;

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
        OS_DEBUG_1("os_procCreate", "SetPriorityClass failed with %d", (int)GetLastError());
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
            OS_DEBUG_1("os_procCreate", "SetThreadPriority failed with %d", (int)GetLastError());
        }
    }
    CloseHandle(process_info.hThread);

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
    DWORD err;
    register int callstatus;

    if (procId == OS_INVALID_PID) {
        return os_resultInvalid;
    }
    assert(status != NULL);

    callstatus = GetExitCodeProcess(procId, &tr);

    if (callstatus == 0) {
        err = GetLastError();
        if (err == ERROR_INVALID_HANDLE) {
            return os_resultUnavailable;
        }

        OS_DEBUG_1("os_procCheckStatus", "os_procCheckStatus GetExitCodeProcess Failed %d", err);
        return os_resultFail;
    }

    if (tr == STILL_ACTIVE) {
        return os_resultBusy;
    }

    CloseHandle (procId);
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
    char* errMsg;
    int id = os_procIdToInteger(procId);

    if (procId == OS_INVALID_PID) {
        return os_resultInvalid;
    }
    if (signal == OS_SIGKILL)
    {
        result = (TerminateProcess(procId, 1) == 0 ? os_resultFail : os_resultSuccess);
        if (result == os_resultFail)
        {
            if (GetLastError() == ERROR_ACCESS_DENIED)
            {
                OS_REPORT_1 (OS_ERROR, "os_procDestroy", 0,
                        "OS_SIGKILL / TerminateProcess of PID %d failed - HANDLE procId lacks TERMINATE_PROCESS "
                        OS_REPORT_NL "access right or process has already terminated.", id);
            }
            else
            {
                errMsg = os_reportErrnoToString(GetLastError());
                OS_REPORT_2 (OS_ERROR, "os_procDestroy", 0,
                        "OS_SIGKILL / TerminateProcess of PID %d failed: %s", id, errMsg);
                os_free(errMsg);
            }
        }
        return result;
    }

    if (signal != OS_SIGHUP  &&
        signal != OS_SIGINT  &&
        signal != OS_SIGALRM &&
        signal != OS_SIGTERM) {
       return os_resultInvalid;
    }

    result = os_resultSuccess;
    snprintf(pname, 256, "\\\\.\\pipe\\osplpipe_%d", id);
    ph = CreateFile(pname,
                    GENERIC_WRITE, 0,
                    NULL, OPEN_EXISTING,
                    0, NULL);
    if (ph != INVALID_HANDLE_VALUE) {
       if (GetLastError() != ERROR_PIPE_BUSY) {
          written = 0;
          WriteFile(ph, &signal, sizeof(os_int32), &written, NULL);
          if (written == 0) {
             result = os_resultFail;
          }
       }
       CloseHandle(ph);
    } else {
       result = os_resultFail;
    }

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
    BOOL result;
    HANDLE hProcess;
    os_int32 procResult;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

    if (hProcess == INVALID_HANDLE_VALUE)
    {
        int err = GetLastError();
        return os_resultFail;
    }
    else if (hProcess == NULL)
    {
        if (GetLastError() == ERROR_ACCESS_DENIED)
        {
            OS_REPORT_1(OS_INFO,"ospl",0, "Failed to open process %d no access", pid);
            return os_resultFail;
        }
        /* the process already died so no need to remove it */
    }
    else
    {
        /* remove process*/
        osr = os_procDestroy((os_procId)hProcess, OS_SIGTERM);
        if(osr != os_resultSuccess)
        {
            OS_REPORT_1(OS_INFO,"ospl",0, "Failed to send the SIGTERM signal to the splice daemon process %d", (os_procId)hProcess);
        }
        osr = os_procCheckStatus((os_procId)hProcess, &procResult);

        procBusyChecks = checkcount;
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

                    OS_REPORT_1(OS_INFO,"ospl", 0,
                                            "OpenSplice service process PID %u still running after spliced terminated." OS_REPORT_NL
                                            "Sending SIGKILL signal.",pid);
                    os_procDestroy ((os_procId)hProcess, OS_SIGKILL);

                    CloseHandle(hProcess);
                }

                if (os_procCheckStatus((os_procId)hProcess, &procResult) == os_resultBusy)
                {
                    /* A process is still running with specified PID.

                    /* Process matches. */
                    /* (100 x 0.1) Ten seconds for an orphaned process to expire subject
                    to a *kill* should be way more than enough (?). */
                    procBusyChecks = checkcount;
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
                        OS_REPORT_1(OS_CRITICAL,"ospl", 0,
                                                "OpenSplice service process PID %u could not be terminated.",pid);
                    }
                }
                CloseHandle(hProcess);
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
os_result
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

    return os_resultSuccess;
}

/** \brief Return the integer representation of the given process ID
 *
 * Possible Results:
 * - returns the integer representation of the given process ID
 */
os_int
os_procIdToInteger(os_procId id)
{
   /* arg id has actual of type HANDLE */
   return (int)GetProcessId (id);
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
   return GetCurrentProcess();
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
                process_name, os_procIdToInteger(os_procIdSelf()));
    }
    else {
        /* No processname could be determined, so default to PID */
        size = snprintf(procIdentity, procIdentitySize, "<%d>",
                os_procIdToInteger(os_procIdSelf()));
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

void
os_procSetSignalHandlingEnabled(
    os_uint enabled)
{
    return;
}
