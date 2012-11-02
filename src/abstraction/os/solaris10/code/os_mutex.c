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

/** \file os/solaris/code/os_mutex.c
 *  \brief Solaris mutual exclusion semaphores
 *
 * Implements mutual exclusion semaphores for Solaris
 * by including the POSIX implementation
 */

#include "../posix/code/os_mutex.c"
#include "../common/code/os_mutex_attr.c"
