/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

/** \file os/darwin/code/os_process.c
 *  \brief Darwin process management
 *
 * Implements process management for Darwin
 * by including the POSIX implementation
 */

#include <unistd.h>
struct sched_param;
int sched_setscheduler (pid_t pid, int sched, const struct sched_param *p);
int sched_getscheduler (pid_t pid);
int sched_getparam (pid_t pid, struct sched_param *p);

#include <../posix/code/os_process.c>
#include <../posix/code/os_process_attr.c>

int sched_setscheduler (pid_t pid, int sched, const struct sched_param *p)
{
  return 0;
}

int sched_getscheduler (pid_t pid)
{
  return SCHED_OTHER;
}

int sched_getparam (pid_t pid, struct sched_param *p)
{
  p->sched_priority = 0;
  return 0;
}
