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

#include "v__groupEntry.h"
#include "v__dataReader.h"
#include "v__reader.h"
#include "v_cache.h"
#include "v__messageQos.h"
#include "v_groupCache.h"
#include "v__groupInstance.h"
#include "v_observer.h"

/*
 * Provide a target walk function and corresponding arg for collecting
 * all connectionCache items that are registered by the writer specified
 * by the writerGID.
 */
struct CacheItemWalkArg
{
    v_registration registration;
    c_iter matchingInstances;
};

static c_bool
collectWhenMatchingRegistration(
    v_cacheNode node,
    c_voidp arg)
{
    v_dataReaderInstance instance;
    struct CacheItemWalkArg *ciwArg = (struct CacheItemWalkArg *) arg;
    v_groupCacheItem gcItem = v_groupCacheItem(node);
    v_groupInstance grInst = v_groupInstance(gcItem->groupInstance);

    if (v_groupInstanceHasRegistration(grInst, ciwArg->registration))
    {
        instance = v_cacheItem(gcItem)->instance;
        ciwArg->matchingInstances = c_iterInsert(ciwArg->matchingInstances, instance);
    }

    return TRUE;
}

/*
 * This function disconnects the writer specified in the registration and the
 * unregister message from the specified entry. For a DataReaderEntry that
 * has matching Qos, this means that all instances registered by that Writer
 * need to be unregistered using the provided unregister message.
 */
c_bool
v_groupEntryApplyUnregisterMessage(
    v_groupEntry _this,
    v_message unregisterMsg,
    v_registration registration)
{
    struct CacheItemWalkArg ciwArg;
    v_entry entry;

    /* Process only DataReaderEntries and skip all others. */
    entry = _this->entry;
    if (v_object(entry)->kind == K_DATAREADERENTRY)
    {
        v_reader reader = v_reader(entry->reader);

        /* Determine whether reader- and writer-qos match for the given entry. */
        v_dataReaderLock(reader);
        if (v_messageQos_isReaderCompatible(unregisterMsg->qos, reader))
        {
            ciwArg.registration = registration;
            ciwArg.matchingInstances = c_iterNew(NULL);

            /* Walk over the connectionCache. */
            v_cacheWalk(_this->connectionCache, collectWhenMatchingRegistration, &ciwArg);
            v_dataReaderUnLock(reader);

            /* Check whether any matching instances have been found. */
            if (c_iterLength(ciwArg.matchingInstances) > 0)
            {
                /* For each matching instance, send an UNREGISTER message. */
                v_dataReaderEntryApplyUnregisterMessageToInstanceList(
                        v_dataReaderEntry(_this->entry),
                        unregisterMsg,
                        ciwArg.matchingInstances);
            }
            c_iterFree(ciwArg.matchingInstances);
        }
        else
        {
            v_dataReaderUnLock(reader);
        }
    }

    return TRUE;
}

