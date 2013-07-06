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

/** \file os/common/code/os_cond_attr.c
 *  \brief Common condition variable attributes
 *
 * Implements os_condAttrInit and sets attributes
 * to platform independent values:
 * - scope is OS_SCOPE_SHARED
 */

#include <assert.h>
#include "os_cond.h"

/** \brief Initialize condition variable attribute
 *
 * Set \b condAttr->scopeAttr to \b OS_SCOPE_SHARED
 */
os_result
os_condAttrInit (
    os_condAttr *condAttr)
{
    assert (condAttr != NULL);
    condAttr->scopeAttr = OS_SCOPE_SHARED;
    return os_resultSuccess;
}

