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

/****************************************************************
 * Implementation for thread management conforming to           *
 * OpenSplice requirements                                      *
 ****************************************************************/

/** \file os/code/os_thread.c
 *  \brief Thread management - create threads
 *
 * A thread is a unit of execution. Threads can compete with each
 * other within the context of a process for the CPU (unbounded
 * threads, scheduling is on process scope) or threads can compete
 * with other processes or threads within the same or other processes
 * for the CPU (bounded threads, scheduling is on system scope).
 * A thread has its own stack, but shares the address space with
 * other threads within a process.
 *
 * \par Thread model on Solaris
 * Solaris supports bouded and unbounded threads, the OS layer
 * will only support bouded threads
 *
 * \par Thread model on Linux
 * Linux only supports bounded threads
 */

#include "os_thread.h"

/* include OS specific thread implementation			*/
#include "code/os_thread.c"
