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

/** \file os/solaris/code/os_rwlock.c
 *  \brief Solaris multiple reader writer lock
 *
 * Implements multiple reader writer lock for Solaris
 * by including the POSIX implementation
 */

#if 0

#include "../posix/code/os_rwlock.c"
#include "../common/code/os_rwlock_attr.c"

#else

#include "../common/code/os_rwlock_by_mutex.c"
#include "../common/code/os_rwlock_attr.c"

#endif
