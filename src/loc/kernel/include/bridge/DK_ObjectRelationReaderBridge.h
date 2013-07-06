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
#ifndef DLRL_KERNEL_OBJECT_RELATION_READER_BRIDGE_H
#define DLRL_KERNEL_OBJECT_RELATION_READER_BRIDGE_H

#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* \brief Creates the relation link between two objects on the language specific level to increase performance when
 * navigating relations.
 *
 * This operation is intended to be used for optimization purposes. When a relation is resolved in the kernel
 * this operation is also called to allow the same link to be established on the language specific level. This is not
 * a requirement however, just an optimization. If wished this operation may do nothing.
 *
 * Mutex claims during this operation:<ul>
 * <li>The update mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The update and admin mutexes of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param userData Optional userdata
 * \param ownerObjectHome The object home of the 'owner' object.
 * \param data Data regarding the owner object.
 * \param relationIndex The index of the relation, which is the same index as used within the kernel MetaModel.
 * \param relationObjectAdmin The 'target' object admin of the relation. This can be <code>NULL</code> and can also be
 * the same object as the 'owner' object admin.
 */
typedef void (*DK_ObjectRelationReaderBridge_us_setRelatedObjectForObject)(
    void* userData,
    DK_ObjectHomeAdmin* ownerObjectHome,
    DK_ObjectAdmin* owner,
    LOC_unsigned_long relationIndex,
    DK_ObjectAdmin* relationObjectAdmin,
    LOC_boolean isValid);

typedef struct DK_ObjectRelationReaderBridge_s{
     DK_ObjectRelationReaderBridge_us_setRelatedObjectForObject setRelatedObjectForObject;
} DK_ObjectRelationReaderBridge;

extern DK_ObjectRelationReaderBridge objectRelationReaderBridge;


#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_OBJECT_RELATION_READER_BRIDGE_H */
