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

/** \file os/solaris10/code/os_thread.c
 *  \brief Solaris thread management
 *
 * Implements thread management for Solaris
 * by including the POSIX implementation
 */

/* PR_SET_NAME doesn't seem to be available on Solaris */
#define OS_HAS_NO_SET_NAME_PRCTL

#include "../posix/code/os_thread.c"
#include "os_thread_attr.c"
