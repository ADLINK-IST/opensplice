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
/**
 * @file api/cm/xml/code/cmx__qos.h
 * 
 * Offers internal routines on a qos.
 */
#ifndef CMX__QOS_H
#define CMX__QOS_H

#include "c_typebase.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Struct used used by the cmx_qosAction routine to resolve the 
 * kind of an entity and the database.
 */
struct cmx_qosArg { 
    v_kind kind;
    c_base base;
};

/**
 * Constructs a new XML qos from the supplied entity. This is realized
 * by serializing the kernel qos using the XML serializer.
 * 
 * @param entity the entity to create a new XML qos from.
 * @param qos    location where the XML representation of the entity qos is stored.
 */
void cmx_qosNew                  (v_entity e, c_char **xmlqos);

/**
 * Constructs a new kernel qos from the supplied XML qos and XML entity. This
 * is realized by deserializing the XML qos using the XML deserializer.
 * 
 * @param entity The XML entity used to resolve the kind of the qos.
 * @param qos The XML qos where to construct a kernel qos for.
 * @return The kernel representation of the XML qos.
 */
v_qos   cmx_qosKernelQos            (const c_char* entity,
                                     const c_char* qos);

v_qos   cmx_qosKernelQosFromKind    (const c_char* qos,
                                     v_kind entityKind,
                                     c_base base);

/**
 * Entity action routine to resolve the kind of the entity as well as the 
 * database.
 * 
 * @param entity The entity to resolve the kind of.
 * @param args Actual type is cmx_qosArg. The result of the function is stored
 *             in here.
 */
void    cmx_qosAction               (v_entity entity, 
                                     c_voidp args);


#if defined (__cplusplus)
}
#endif

#endif /* CMX__QOS_H */
