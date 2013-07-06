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

/** \file os/darwin/code/os_thread.c
 *  \brief Darwin thread management
 *
 * Implements thread management for Darwin 
 * by including the POSIX implementation
 */

#define OS_HAS_NO_SET_NAME_PRCTL

#include <../posix/code/os_thread.c>
#include <code/os_thread_attr.c>
