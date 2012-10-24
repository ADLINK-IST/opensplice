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
#include "ospl_proc.h"

#include "os.h"
#include "os_iterator.h"

#include <tlhelp32.h>

#define MAX_STATUSCHECKS (30)

#define procInfo(o) ((procInfo)(o))
OS_CLASS(procInfo);

OS_STRUCT(procInfo) {
    char *exename;
    DWORD pid;
    DWORD ppid;
    os_iter children;
    int inserted;
};

procInfo
procInfoNew(
    const char *exename,
    DWORD pid,
    DWORD ppid)
{
    procInfo newProc = os_malloc(OS_SIZEOF(procInfo));

    if (newProc) {
        newProc->exename = os_strdup(exename);
        newProc->pid = pid;
        newProc->ppid = ppid;
        newProc->inserted = 0;
        newProc->children = os_iterNew(NULL);
    }
    return newProc;
}

void
insertChild(
    procInfo proc,
    procInfo newProc)
{
    if (proc->pid == newProc->ppid) {
        proc->children = os_iterAppend(proc->children, newProc);
        newProc->inserted = 1;
    } else {
    	if (os_iterLength(proc->children) > 0) {
            os_iterWalk(proc->children, insertChild, newProc);
        }
    }
}

os_equality
childOfProc(
    procInfo proc,
    procInfo newProc)
{
    if (proc->ppid == newProc->pid) {
        return OS_EQ;
    }
    
    return OS_NE;
}

void
procInsert(
    os_iter processes,
    procInfo proc)
{
    procInfo p;

    os_iterWalk(processes, insertChild, proc);
    if (proc->inserted == 0) {
        os_iterAppend(processes, proc);
        proc->inserted = 1;
    }
    while ((p = os_iterResolve(processes, childOfProc, proc))) {
        os_iterAppend(proc->children, p);
        os_iterTake(processes, p);
    }    
}

void show_procs(os_iter processes, int *depth);

void
show_procInfo(
    procInfo proc, 
    int *depth)
{
    int i;

    for (i = 0; i < *depth; i++) {
        printf ("\t");
    }
    printf ("%d-%d\t%s\n", (int)proc->pid, (int)proc->ppid, proc->exename);
    if (os_iterLength(proc->children) > 0) {
        (*depth)++;
        show_procs(proc->children, depth);
	    (*depth)--;
    }
}

void
show_procs(
    os_iter processes,
    int *depth)
{
    os_iterWalk(processes, show_procInfo, depth);
}

static int
read_proc_tree(
    DWORD uid,
    os_iter processes)
{
    procInfo proc;
    HANDLE snapshot;
    PROCESSENTRY32* processInfo;

    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot != INVALID_HANDLE_VALUE)
    {
       processInfo = malloc(sizeof(PROCESSENTRY32));
       processInfo->dwSize = sizeof(PROCESSENTRY32);

       while(Process32Next(snapshot, processInfo) != FALSE)
       {
          if (processInfo->th32ParentProcessID == uid)
          {
             proc = procInfoNew (processInfo->szExeFile,
                                 processInfo->th32ProcessID,
                                 processInfo->th32ParentProcessID);
             procInsert (processes, proc);
          }
       }
    }

    CloseHandle(snapshot);
    free(processInfo);
    
    return(1);
}

void
signal_related_processes(
    os_iter processes,
    DWORD pid,
    int signal,
    int related)
{
   int i;
   procInfo proc;
   
   for (i = 0; i < os_iterLength (processes); i++) {
       proc = os_iterObject (processes, i);
       
       if (related) {
          os_result r;
          os_int32 procResult;
          int i = MAX_STATUSCHECKS;
          HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, proc->pid);

          os_procDestroy ((os_procId)handle, signal);
          r = os_procCheckStatus((os_procId)handle, &procResult);
          while ((r == os_resultBusy) && (i > 0)) {
             i--;
             printf (".");
             fflush(stdout);
             Sleep(1000);
             r = os_procCheckStatus((os_procId)handle, &procResult);
          }          
       }
   }
}

void
remove_proc_tree(
    os_iter processes)
{
    procInfo proc;

    while ((proc = os_iterTakeFirst(processes)) != NULL) {
        if (proc->children != NULL) {
            remove_proc_tree(proc->children);
            os_iterFree(proc->children);
        }
        os_free(proc->exename);
        os_free(proc);
    }
}

void
kill_descendents(
    DWORD pid,
    int signal)
{
    os_iter processes;

    processes = os_iterNew(NULL);

    /* collect all processes that uid is parent of */
    read_proc_tree (pid, processes);

    /* kill these processes */
    signal_related_processes(processes, pid, signal, 1);
    
    remove_proc_tree(processes);
    os_iterFree(processes);
}
