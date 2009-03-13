
/** \file os/common/code/os_rwlock_attr.c
 *  \brief Common multiple reader writer lock attributes
 *
 * Implements os_rwlockAttrInit and sets attributes
 * to platform independent values:
 * - scope is OS_SCOPE_SHARED
 */

#include <assert.h>

/** \brief Initialize rwlock attribute
 *
 * Set \b rwlockAttr->scopeAttr to \b OS_SCOPE_SHARED
 */
os_result
os_rwlockAttrInit (
    os_rwlockAttr *rwlockAttr)
{
    assert (rwlockAttr != NULL);
    rwlockAttr->scopeAttr = OS_SCOPE_SHARED;
    return os_resultSuccess;
}
