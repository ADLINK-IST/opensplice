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

/** \file os/darwin/code/os_rwlock.c
 *  \brief Darwin multiple reader writer lock
 *
 * Implements multiple reader writer lock for Darwin
 * by including the POSIX implementation
 */

#include <os_mutex.h>
#include <../common/code/os_rwlock_by_mutex.c>
#include <../common/code/os_rwlock_attr.c>

