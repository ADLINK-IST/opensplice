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
#ifndef DLRL_KERNEL_CACHE_FACTORY_ADMIN_H
#define DLRL_KERNEL_CACHE_FACTORY_ADMIN_H

/* OS abstraction layer includes */
#include "os_mutex.h"

/* DLRL util includes */
#include "DLRL_Exception.h"
#include "DLRL_Types.h"

/* collection includes */
#include "Coll_Map.h"

/* DLRL Kernel includes */
#include "DK_CacheAdmin.h"
#include "DK_Types.h"
#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* \brief The <code>DK_CacheFactoryAdmin</code> singleton. Take note that this object isnt a singleton if you simply use
 * it on C level, you need to make some arrangements on the language specific level to make it a 'real' singleton.
 */
struct DK_CacheFactoryAdmin_s
{
    /* The map of all <code>DK_CacheAdmin</code> objects (the values) with a string as key. The String key is also
     * used as the name of the cache. But in essense the key is managed by the <code>DK_CacheFactoryAdmin</code> object.
     */
    Coll_Map caches;
    /* The language specific representative of this <code>DK_CacheFactoryAdmin</code> object.
     */
    DLRL_LS_object ls_cacheFactory;
    /* This mutex is locked during all actions upon this <code>DK_CacheAdmin</code> object. Each
     * operation defined for this class will note whether or not this mutex is locked.
     */
    os_mutex mutex;
};

/* \brief Locks the <code>DK_CacheFactoryAdmin</code>.
 *
 * NOTE: This function may ONLY be called after the <code>DK_CacheFactoryAdmin_ts_getInstance(...)</code> has been
 * called at least once. As that operation initializes the <code>DK_CacheFactoryAdmin</code> singleton.
 */
void
DK_CacheFactoryAdmin_lock();

/* \brief Unlocks the CacheFactoryAdmin.
 *
 * NOTE: This function may ONLY be called after the <code>DK_CacheFactoryAdmin_ts_getInstance(...)</code> has been
 * called at least once. As that operation initializes the <code>DK_CacheFactoryAdmin</code> singleton.
 */
void
DK_CacheFactoryAdmin_unlock();

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_CACHE_FACTORY_ADMIN_H */
