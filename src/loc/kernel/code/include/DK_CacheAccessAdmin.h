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
#ifndef DLRL_KERNEL_CACHE_ACCESS_ADMIN_H
#define DLRL_KERNEL_CACHE_ACCESS_ADMIN_H

/* OS abstraction layer includes */
#include "os_mutex.h"

/* DLRL util includes */
#include "DLRL_Types.h"

/* Collection includes */
#include "Coll_List.h"
#include "Coll_Set.h"

/* DLRL Kernel includes */
#include "DK_CacheBase.h"
#include "DK_Entity.h"
#include "DK_Types.h"
#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* NOT IN DESIGN -check entire struct... */
struct DK_CacheAccessAdmin_s
{
    DK_CacheBase base;
    os_mutex mutex;
    Coll_List contracts;
    DK_CacheAdmin* owner;
    LOC_long* containedTypes;/* array of longs */
    LOC_unsigned_long maxContainedTypes;
    LOC_unsigned_long currentNrOfTypes;/* NOT IN DESIGN */
    DK_CacheAccessTypeRegistry** registry;
#if 0
    LOC_unsigned_long invalidLinks;/* NOT IN DESIGN */
#endif
};

/* NOT IN DESIGN */
DK_CacheAccessAdmin*
DK_CacheAccessAdmin_new(
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object ls_cacheAccess,
    DK_Usage usage,
    DK_CacheAdmin* cache);

void
DK_CacheAccessAdmin_ts_delete(
    DK_CacheAccessAdmin* _this,
    void* userData);

/* no homes may be locked! */
void
DK_CacheAccessAdmin_us_delete(
    DK_CacheAccessAdmin* _this,
    void* userData);

/* NOT IN DESIGN */
void
DK_CacheAccessAdmin_us_markObjectAsChanged(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* changedAdmin);

/* NOT IN DESIGN */
void
DK_CacheAccessAdmin_us_resolveElements(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectAdmin* objectAdmin);

/* NOT IN DESIGN */
LOC_boolean
DK_CacheAccessAdmin_us_isAlive(
    DK_CacheAccessAdmin* _this);

/* NOT IN DESIGN */
/* expects an object admin which has already been ref counted */
void
DK_CacheAccessAdmin_us_registerUnregisteredObjectAdmin(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin);


/* NOT IN DESIGN */
/* expects an object admin which has already been ref counted */
void
DK_CacheAccessAdmin_us_registerObjectAdmin(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin);

/* NOT IN DESIGN */
void
DK_CacheAccessAdmin_us_unregisterRegisteredObjectAdmin(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin);

/* NOT IN DESIGN */
/* expects an object admin which has already been ref counted */
void
DK_CacheAccessAdmin_us_addUnregisteredObjectAdmin(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin);

/* NOT IN DESIGN */
void
DK_CacheAccessAdmin_us_removeUnregisteredObject(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin);

void
DK_CacheAccessAdmin_us_unregisterUnresolvedElement(
    DK_CacheAccessAdmin* _this,
    void* userData,
    DK_ObjectHomeAdmin* targetHome,
    DK_ObjectHolder* holder);

#if 0
/* NOT IN DESIGN */
void
DK_CacheAccessAdmin_us_increaseInvalidLinksWithAmount(
    DK_CacheAccessAdmin* _this,
    LOC_unsigned_long amount);

/* NOT IN DESIGN */
void
DK_CacheAccessAdmin_us_decreaseInvalidLinks(
    DK_CacheAccessAdmin* _this);
#endif
/* NOT IN DESIGN */
DK_CacheAdmin*
DK_CacheAccessAdmin_us_getOwner(
    DK_CacheAccessAdmin* _this);


#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_CACHE_ACCESS_ADMIN_H */
