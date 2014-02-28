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

/** \file os/linux/code/os_rwlock.c
 *  \brief Linux multiple reader writer lock
 *
 * Implements multiple reader writer lock for Linux
 * by including the POSIX implementation
 */

#include "../common/code/os_rwlock_by_mutex.c"
#include "../common/code/os_rwlock_attr.c"

