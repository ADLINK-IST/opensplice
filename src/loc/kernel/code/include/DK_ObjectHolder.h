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
#ifndef DLRL_KERNEL_OBJECT_HOLDER_H
#define DLRL_KERNEL_OBJECT_HOLDER_H

/* DLRL util includes */
#include "DLRL_Exception.h"

/* DLRL kernel includes */
#include "DK_ObjectAdmin.h"
#include "DK_Types.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef union DK_Reference_u
{
    DK_ObjectAdmin* target;
    void* values;
} DK_Reference;

struct DK_ObjectHolder_s
{
    LOC_boolean resolved;/* true means target is valid, false means values is valid */
    DK_Reference ref;
    DK_Entity* owner;
    LOC_unsigned_long arraySize;
    LOC_unsigned_long index;
    /* collection specific data of an object holder... should be in a different
     * subclass, but no time right now to make this change.
     */
    void* userData;
    u_instanceHandle elementHandle;
    LOC_long noWritersCount;
    LOC_long disposedCount;
};

DK_ObjectHolder*
DK_ObjectHolder_newResolved(
    DLRL_Exception* exception,
    DK_Entity* owner,
    DK_ObjectAdmin* target,
    u_instanceHandle elementHandle,
    LOC_unsigned_long index);

DK_ObjectHolder*
DK_ObjectHolder_newUnresolved(
    DLRL_Exception* exception,
    DK_Entity* owner,
    void* values,
    LOC_unsigned_long arraySize,
    u_instanceHandle elementHandle,
    LOC_unsigned_long index);

LOC_long
DK_ObjectHolder_us_getNoWritersCount(
    DK_ObjectHolder* _this);

void
DK_ObjectHolder_us_setNoWritersCount(
    DK_ObjectHolder* _this,
    LOC_long noWritersCount);

LOC_long
DK_ObjectHolder_us_getDisposedCount(
    DK_ObjectHolder* _this);

void
DK_ObjectHolder_us_setDisposedCount(
    DK_ObjectHolder* _this,
    LOC_long disposedCount);

void
DK_ObjectHolder_us_destroy(
    DK_ObjectHolder* _this);

/* takes care of release and duplicate, may pass along a null value */
void
DK_ObjectHolder_us_setTarget(
    DK_ObjectHolder* _this,
    DK_ObjectAdmin* target);

void*
DK_ObjectHolder_us_getValues(
    DK_ObjectHolder* _this);

LOC_unsigned_long
DK_ObjectHolder_us_getValuesSize(
    DK_ObjectHolder* _this);

void
DK_ObjectHolder_us_setValues(
    DK_ObjectHolder*_this,
    void* values,
    LOC_unsigned_long arraySize);

/* no duplicate done */
DK_Entity*
DK_ObjectHolder_us_getOwner(
    DK_ObjectHolder* _this);

/* may return null */
u_instanceHandle
DK_ObjectHolder_us_getHandle(
    DK_ObjectHolder* _this);

/* no duplicate / copy made, may return null. */
void
DK_ObjectHolder_us_setUserData(
    DK_ObjectHolder* _this,
    void* userData);

void
DK_ObjectHolder_us_setHandle(
    DK_ObjectHolder* _this,
    u_instanceHandle elementHandle);

/* NOT IN DESIGN */
void
DK_ObjectHolder_us_setHandleNil(
    DK_ObjectHolder* _this);

LOC_unsigned_long
DK_ObjectHolder_us_getIndex(
    DK_ObjectHolder* _this);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_OBJECT_HOLDER_H */
