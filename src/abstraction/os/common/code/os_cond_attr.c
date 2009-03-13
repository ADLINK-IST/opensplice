
/** \file os/common/code/os_cond_attr.c
 *  \brief Common condition variable attributes
 *
 * Implements os_condAttrInit and sets attributes
 * to platform independent values:
 * - scope is OS_SCOPE_SHARED
 */

#include <assert.h>

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

