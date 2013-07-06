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

/** \file os/common/code/os_mutex_attr.c
 *  \brief Common mutual exclusion semaphore attributes
 *
 * Implements os_mutexAttrInit and sets attributes
 * to platform independent values:
 * - scope is OS_SCOPE_SHARED
 */

#include <assert.h>

/** \brief Initialize mutex attribute
 *
 * Set \b mutexAttr->scopeAttr to \b OS_SCOPE_SHARED
 */
os_result
os_mutexAttrInit (
    os_mutexAttr *mutexAttr)
{
    assert (mutexAttr != NULL);
    mutexAttr->scopeAttr = OS_SCOPE_SHARED;
    return os_resultSuccess;
}
