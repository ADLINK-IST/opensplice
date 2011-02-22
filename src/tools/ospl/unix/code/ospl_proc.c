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
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "os_classbase.h"
#include "os_iterator.h"
#include "os_defs.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include <assert.h>

#ifdef OS_SOLARIS_DEFS_H
#include <procfs.h>
#endif

#ifdef OS_AIX_DEFS_H
#include <sys/procfs.h>
#endif

#include "ospl_proc.h"

#define PROCFS	"/proc"

#define procInfo(o) ((procInfo)(o))
OS_CLASS(procInfo);

OS_STRUCT(procInfo) {
    char *name;
    char *cmdLine;
    pid_t pid;
    pid_t ppid;
    os_iter children;
    int inserted;
};

procInfo
procInfoNew (
    const char *name,
    pid_t pid,
    pid_t ppid,
    const char *cmdline
    )
{
    procInfo newProc = os_malloc (OS_SIZEOF(procInfo));

    if (newProc) {
	newProc->name = os_strdup(name);
	newProc->cmdLine = os_strdup(cmdline);
	newProc->pid = pid;
	newProc->ppid = ppid;
	newProc->inserted = 0;
	newProc->children = os_iterNew (NULL);
    }
    return newProc;
}

void
insertChild (
    procInfo proc,
    procInfo newProc
    )
{
    if (proc->pid == newProc->ppid) {
	proc->children = os_iterAppend (proc->children, newProc);
	newProc->inserted = 1;
    } else if (os_iterLength (proc->children)) {
	os_iterWalk (proc->children, insertChild, newProc);
    }
}

os_equality
childOfProc (
    procInfo proc,
    procInfo newProc
    )
{
    if (proc->ppid == newProc->pid) {
	return OS_EQ;
    }
    return OS_NE;
}

void
procInsert (
    os_iter processes,
    procInfo proc
    )
{
    procInfo p;

    os_iterWalk (processes, insertChild, proc);
    if (proc->inserted == 0) {
	os_iterAppend (processes, proc);
	proc->inserted = 1;
    }
    while ((p = os_iterResolve (processes, childOfProc, proc))) {
	os_iterAppend (proc->children, p);
	os_iterTake (processes, p);
    }    
}

void show_procs (os_iter processes, int *depth);

void
show_procInfo (procInfo proc, int *depth)
{
    int i;

    for (i = 0; i < *depth; i++) {
        printf ("\t");
    }
    printf ("%d-%d\t%s\t%s\n", (int)proc->pid, (int)proc->ppid, proc->name, proc->cmdLine);
    if (os_iterLength(proc->children)) {
	(*depth)++;
	show_procs (proc->children, depth);
	(*depth)--;
    }
}

void show_procs (os_iter processes, int *depth)
{
    os_iterWalk (processes, show_procInfo, depth);
}

#ifndef OS_LINUX_DEFS_H

/* for Solaris and AIX */
static int
read_proc_tree (
    uid_t uid,
    os_iter processes
    )
{
    DIR *dir;
    struct dirent *de;
    char path[PATH_MAX + 1];
    pid_t pid;
    psinfo_t proc_status;
    int status;
    int fdd;
    procInfo proc;
    
    assert(processes);
    /*processes = os_iterNew (NULL);*/
    
    if ((dir = opendir (PROCFS)) != NULL) {
        while ((de = readdir (dir)) != NULL) {
            if ((pid = atoi (de->d_name)) != 0) {
                os_sprintf (path, "%s/%d/psinfo", PROCFS, (int)pid);
                
                if ((fdd = open (path, O_RDONLY)) != -1) {
                    status = read (fdd, &proc_status, sizeof(proc_status));
                    
                    if ((status > 0) && (proc_status.pr_uid == uid)) {
                        proc_status.pr_fname[PRFNSZ-1] = '\0';
			            proc = procInfoNew (proc_status.pr_fname,
                        proc_status.pr_pid,
                        proc_status.pr_ppid,
                        proc_status.pr_psargs);
                        procInsert (processes, proc);
                    }
                    (void) close (fdd);
                }
            }
        }
        (void) closedir (dir);
    }
        
    return(1);
}
#else
/* for linux */
static int
read_proc_tree (
    uid_t uid,
    os_iter processes
    )
{
    DIR *dir;
    struct dirent *de;
    char stat_line[100];
    char cmd_line[100];
    char fname[100];
    char path[PATH_MAX + 1];
    pid_t pid;
    pid_t ppid;
    uid_t puid;
    FILE* fdd;
    procInfo proc;

    /*processes = os_iterNew (NULL);*/
    
    if ((dir = opendir (PROCFS)) != NULL) {
        while ((de = readdir (dir)) != NULL) {
            if ((pid = atoi (de->d_name)) != 0) {
                os_sprintf (path, "%s/%d/status", PROCFS, pid);
                if ((fdd = fopen (path, "r")) != NULL) {
		    while (fgets (stat_line, sizeof(stat_line)-1, fdd) != NULL) {
			if (strncmp (stat_line, "PPid:", 5) == 0) {
			    sscanf (&stat_line[5], "%d", &ppid);
			} else if (strncmp (stat_line, "Uid:", 4) == 0) {
			    sscanf (&stat_line[4], "%d", (int *)&puid);
			} else if (strncmp (stat_line, "Name:", 5) == 0) {
			    sscanf (&stat_line[5], "%s", fname);
			}
		    }
                    (void) fclose (fdd);
                    os_sprintf (path, "%s/%d/cmdline", PROCFS, pid);
                    if ((fdd = fopen (path, "r")) != NULL) {
		        fgets (cmd_line, sizeof(cmd_line), fdd);
		    }
                    (void) fclose (fdd);
		    if (uid == puid) {
		        proc = procInfoNew (fname, pid, ppid, cmd_line);
			procInsert (processes, proc);
                    }
                }
            }
	}
        (void) closedir (dir);
    }
    return(1);
}
#endif

void
signal_related_processes (
    os_iter processes,
    pid_t pid,
    int signal,
    int related
    )
{
    int i;
    procInfo proc;

    for (i = 0; i < os_iterLength (processes); i++) {
	proc = os_iterObject (processes, i);
	if (proc->pid == pid) {
	    signal_related_processes (proc->children, pid, signal, 1);
	} else if (proc->children) {
	    signal_related_processes (proc->children, pid, signal, 0);
	}
	if (related) {
	    printf (" %d\n", (int)proc->pid); fflush (stdout);
	    kill (proc->pid, signal);
	}
    }
}

void
remove_proc_tree (
    os_iter processes
    )
{
    procInfo proc;

    while ((proc = os_iterTakeFirst (processes)) != NULL) {
	if (proc->children) {
	    remove_proc_tree (proc->children);
	    os_iterFree (proc->children);
	}
	os_free (proc->name);
	os_free (proc->cmdLine);
	os_free (proc);
    }
}

void
kill_descendents (
    pid_t pid,
    int signal
    )
{
    os_iter processes;

    processes = os_iterNew (NULL);
    read_proc_tree (getuid(), processes);
    printf ("(%d", (int)pid); fflush (stdout);
    kill (pid, signal);
    signal_related_processes (processes, pid, signal, 0);
    printf (")"); fflush (stdout);
    remove_proc_tree (processes);
    os_iterFree (processes);
}
