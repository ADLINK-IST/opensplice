/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/**@file api/cm/xml/include/cmx_entity.h
 * @brief XML representation of an user layer entity. All C&M XML entities map
 *        on a user layer entity, except for the snapshots.
 * 
 * The XML entity is associated with an user entity. That means a user entity
 * is created when a new XML entity is created and is freed when the XML entity
 * (and so the user entity) is freed. 
 * 
 * Most of the user entities do not own a kernel entity (except for self
 * created entities, which are created using the cmx_...New routines ), which 
 * means that the kernel entity is not freed when the XML and user entities are 
 * freed.
 * 
 * Each XML entity at least contains the following information:
   @verbatim
   <entity>
       <pointer>...</pointer>
       <handle_index>...</handle_index>
       <handle_serial>..</handle_serial>
       <name>...</name>
       <kind>...</kind>
       <enabled>...</enabled>
       ...
   </entity>
   @endverbatim
 * 
 * - 'pointer' contains the address of its corresponding user entity. This 
 *   pointer is needed to be able to get access to the user and kernel entity
 *   that correspond with the XML entity.
 * - 'handle_index' contains the index of the handle of the kernel entity that 
 *   corresponds with the XML entity.
 * - 'handle_serial' contains the serial of the handle of the kernel entity that 
 *   corresponds with the XML entity.
 * - 'name' contains the name of the kernel entity that corresponds with the 
 *   XML entity.
 * - 'kind' contains the kind of the kernel entity that corresponds with the 
 *   XML entity.
 * - 'enabled' specifies whether the entity is currently enabled.
 * 
 * Just like the user entity, the XML entity has several descendants. Some 
 * entity types contain additional tags that describe information that is 
 * specific for that type of entity.
 */

#ifndef CMX_ENTITY_H
#define CMX_ENTITY_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CMXML
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * @brief Provides access to entities that are owned by the supplied entity and
 * match the supplied filter.
 * 
 * The result of this function is a list of entities in XML format.
 @verbatim
    <list>
        <entity>
            <pointer>...</pointer>
            <handle_index>...</handle_index>
            <handle_serial>..</handle_serial>
            <name>...</name>
            <kind>...</kind>
            <enabled>...</enabled>
            ...
        </entity>
        <entity>
            ...
        </entity>
        ...
    </list>
 @endverbatim
 * 
 * @param entity The entity, which owned entities must be resolved.
 * @param filter The filter that determines the kind of entities that will
 *               be resolved by this function. The filter can only be used to
 *               filter entities by kind and must be one of the following:
 *               ENTITY, KERNEL, TOPIC, QUERY, VIEW, DOMAIN, READER, DATAREADER,
 *               QUEUE, WRITER, SUBSCRIBER, PUBLISHER, PARTICIPANT or SERVICE.
 *               Entities wich are descendants of classes of the supplied kind
 *               will also be resolved. Using any other filter will result in
 *               applying ENTITY as filter. 
 * @return A XML list of entities that are owned by the supplied entity and
 *         match the supplied filter. If the supplied entity is not available 
 *         (anymore), NULL will be returned.
 */
OS_API c_char*         cmx_entityOwnedEntities         (const c_char* entity,
                                                        const c_char* filter);
/**
 * @brief Provides access to entities that depend on the supplied entity and
 * match the supplied filter.
 * 
 * The result of this function is a list of entities in XML format.
 @verbatim
    <list>
        <entity>
            <pointer>...</pointer>
            <handle_index>...</handle_index>
            <handle_serial>..</handle_serial>
            <name>...</name>
            <kind>...</kind>
            <enabled>...</enabled>
            ...
        </entity>
        <entity>
            ...
        </entity>
        ...
    </list>
 @endverbatim
 * 
 * @param entity The entity, which dependant entities must be resolved.
 * @param filter The filter that determines the kind of entities that will
 *               be resolved by this function. The filter can only be used to
 *               filter entities by kind and must be one of the following:
 *               ENTITY, KERNEL, TOPIC, QUERY, VIEW, DOMAIN, READER, DATAREADER,
 *               QUEUE, WRITER, SUBSCRIBER, PUBLISHER, PARTICIPANT or SERVICE.
 *               Entities wich are descendants of classes of the supplied kind
 *               will also be resolved. Using any other filter will result in
 *               applying ENTITY as filter. 
 * @return A XML list of entities that where the supplied entity depends on and
 *         match the supplied filter. If the supplied entity is not available 
 *         (anymore), NULL will be returned.
 */                                  
OS_API c_char*         cmx_entityDependantEntities     (const c_char* entity,
                                                        const c_char* filter);

/**
 * @brief Frees the supplied entity and its associated user entity.
 * 
 * After calling the function both the user entity as the supplied XML entity
 * are freed.
 * 
 * @param entity The XML entity that needs to be freed.
 */
OS_API void            cmx_entityFree                  (c_char* entity);

/**
 * @brief Provides access to the current status of the supplied entity.
 * 
 * The layout of the status depends on the type of the entity that is
 * supplied.
 * 
 * @param entity The entity, which status must be resolved.
 * @return The XML representation of the current status of the supplied
 *         entity.
 * 
 */
OS_API c_char*         cmx_entityStatus                (const c_char* entity);

/**
 * @brief Resolves the current statistics of the supplied entity.
 * 
 * The layout of the statistics depends on the type of the entity.
 * 
 * @param entity The entity, which statistics must be resolved.
 * @return The statistics of the supplied entity or NULL if the entity has no
 *         statistics.
 */
OS_API c_char*         cmx_entityStatistics            (const c_char* entity);

/**
 * @brief Resets (a part of) the statistics of the supplied entity.
 * 
 * If a fieldName is provided, it is looked up and only that field of the 
 * statistics is resetted. If NULL is provided as fieldName, the complete
 * statistics are resetted.
 * 
 * @param entity The entity where to reset (a part of) the statistics of.
 * @param fieldName The fieldName within the statistics to reset, or NULL
 *                  if all statistics are to be resetted.
 * @return The result of the reset action. If succeeded 
 *         @verbatim<result>OK</result>@endverbatim is returned, 
 *         @verbatim<result>...</result>@endverbatim otherwise.
 */
OS_API const c_char*   cmx_entityResetStatistics       (const c_char* entity,
                                                        const c_char* fieldName);

/**
 * @brief Provides access to the current qos of the supplied entity.
 * 
 * The layout of the qos depends on the type of the entity that is
 * supplied.
 * 
 * @param entity The entity, which qos must be resolved.
 * @return The XML representation of the current qos of the supplied
 *         entity.
 * 
 */
OS_API c_char*         cmx_entityQoS                   (const c_char* entity);


/**
 * @brief Tries to apply the supplied qos of the supplied entity.
 * 
 * The layout of the qos depends on the type of the entity that is
 * supplied.
 * 
 * @param entity The entity, which qos must be set.
 * @param qos The qos, which to apply to the entity.
 * @return The result of the attempt to apply the qos on the entity. If 
 *         succeeded @verbatim<result>OK</result>@endverbatim is returned, 
 *         @verbatim<result>...</result>@endverbatim otherwise.
 * 
 */
OS_API const c_char*   cmx_entitySetQoS                (const c_char* entity,
                                                        const c_char* qos);

/**
 * @brief Enables the supplied entity.
 * 
 * Subsequent calls to this function have no effect.
 * 
 * @param entity The entity to enable.
 * @return The result of the attempt to enable the entity. If 
 *         succeeded @verbatim<result>OK</result>@endverbatim is returned, 
 *         @verbatim<result>...</result>@endverbatim otherwise.
 */
OS_API const c_char*   cmx_entityEnable                (const c_char* entity);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_ENTITY_H */
