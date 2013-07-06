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
#ifndef DLRL_KERNEL_CACHE_BASE_H
#define DLRL_KERNEL_CACHE_BASE_H

/* DLRL util includes */
#include "DLRL_Types.h"

/* DLRL kernel includes */
#include "DK_Entity.h"
#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

/* \brief The abstract base class of the Cache and CacheAccess classes. May not be created 'stand-alone'.
 */
struct DK_CacheBase_s
{
    /* The base class of the <code>DK_CacheBase</code> class which manages the reference count.
     */
    DK_Entity entity;
    /* Indicates whether this <code>DK_CacheBase</code> object is 'alive' (<code>TRUE</code>) or not
     * (<code>FALSE</code>). Objects that are no longer alive have been properly deleted and accessing operations
     * on these objects will raise <code>DLRL_ALREADY_DELETED</code> exceptions.
     */
    LOC_boolean alive;
    /* Indicates the usage of the <code>DK_CacheBase</code> object.
     */
    DK_Usage usage;
    /* The language specific representative of this <code>DK_CacheBase</code> object.
     */
    DLRL_LS_object ls_cacheBase;
};

/* \brief Returns the usage of this <code>DK_CacheBase</code> object.
 *
 * Preconditions:<ul>
 * <li>Must ensure thread safety</li>
 * <li>Must verify the <code>DK_CacheBase</code> is still alive.</li></ul>
 *
 * \param _this The <code>DK_CacheBase</code> entity that is the target of this operation
 *
 * \return the usage of this <code>DK_CacheBase</code> object.
 */
DK_Usage
DK_CacheBase_us_getCacheUsage(
    DK_CacheBase* _this);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_CACHE_BASE_H */
