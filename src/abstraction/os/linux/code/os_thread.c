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

/** \file
 *  \brief Linux thread management
 *
 * Implements thread management for Linux
 * by including the POSIX implementation
 */

#ifndef OS_HAS_NO_SET_NAME_PRCTL
#include <sys/prctl.h>
#ifndef PR_SET_NAME
#define OS_HAS_NO_SET_NAME_PRCTL
#endif
#endif

#include "../posix/code/os_thread.c"
#include "code/os_thread_attr.c"
