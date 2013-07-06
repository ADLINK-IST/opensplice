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

#ifndef D_OBJECT_H
#define D_OBJECT_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef enum d_kind{
    D_BAD_TYPE, D_DURABILITY, D_CONFIGURATION, D_FELLOW, D_ADMIN, D_GROUP,
    D_LISTENER, D_POLICY, D_NAMESPACE, D_PUBLISHER, D_SUBSCRIBER, D_TABLE, D_CHAIN,
    D_EVENT_LISTENER, D_ADMIN_EVENT, D_STORE, D_WAITSET, D_WAITSET_ENTITY,
    D_ACTION, D_ACTION_QUEUE, D_DISPOSE_HELPER, D_GROUP_CREATION_QUEUE,
    D_READER_REQUEST, D_MERGE_ACTION,
    D_KINDCOUNT /*Must be the last element! */
}d_kind;

C_CLASS(d_object);

typedef void (*d_objectDeinitFunc) (d_object object);

C_STRUCT(d_object){
    c_ulong confidence;
    d_kind kind;
    os_uint32 refCount;
    d_objectDeinitFunc deinit;
};

#define d_object(d) ((d_object)(d))

void        d_objectInit        (d_object object,
                                 d_kind kind,
                                 d_objectDeinitFunc deinit);

void        d_objectFree        (d_object object,
                                 d_kind kind);

c_bool      d_objectIsValid     (d_object object,
                                 d_kind kind);

d_object    d_objectKeep        (d_object object);

c_ulong     d_objectGetRefCount (d_object object);

c_bool      d_objectValidate    (c_ulong expected);

#if defined (__cplusplus)
}
#endif

#endif /* D_OBJECT_H */
