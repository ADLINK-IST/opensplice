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

/**@file api/cm/common/include/cm_api.h
 * 
 * @brief The cm_api contains the common functionality for the Control & Monitoring 
 * API. 
 * 
 * All language specific Control & Monitoring API's share the functionality 
 * within this component.
 */
#ifndef CM_API_H
#define CM_API_H

#include "cm_typebase.h"
#include "u_user.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**@brief Initializes the Control & Monitoring API.
 * 
 * Prior to using the Control & Monitoring API this function must be called.
 * 
 * @return CM_RESULT_OK if succeeded, any other result otherwise.
 */
cm_result       cm_init();

/**@brief Detaches the Control & Monitoring API.
 * 
 * This function should be called after using the Control & Monitoring API. It
 * cleans up all used resources properly.
 * 
 * @return CM_RESULT_OK if succeeded, any other result otherwise.
 */
cm_result       cm_detach();

/**@brief Retrieves the kernel URI associated with the supplied domain ID.
 * 
 * This function uses the nameservice to look up the kernel URI.
 * 
 * @param domainId The ID of the domain the look up the URI for.
 * 
 * @return The associated kernel URI, NULL if it does not exist.
 */
const c_char*   cm_getKernelURI     (long domainId);

/**@brief Retrieves the root entity of the supplied domain.
 * 
 * The supplied copy function will be called in protected area.
 * 
 * @param kernel_uri The URI of the domain to look in.
 * @param func The copy function that will be called with the kernel root entity
 * as argument.
 * @param fArg The argument that will be passed on to the supplied copy 
 * function.
 * 
 * @return The result of the copy function.
 */
c_voidp         cm_getRootEntity    (const c_char* kernel_uri,
                                     const c_voidp (*func)(v_entity entity, c_voidp funcArg),
                                     c_voidp fArg);

/**@brief Retrieves entities within the supplied entity.
 * 
 * The supplied copy function will be called in protected area.
 * 
 * @param index The index of the handle of the entity to look in.
 * @param serial The serial of the handle of the entity to look in.
 * @param kernel_uri The kernel to find the entity in.
 * @param kind The kind of the entities to retrieve. All subkinds of the
 * supplied kind are also included. That means K_ENTITY gets all entities.
 * @param func The copy function to call when the entities have been retrieved.
 * @param fArg The argument that will be passed on to the copy function.
 * 
 * @return The result of the copy function.
 */
c_voidp         cm_getEntities      (c_long index,
                                     c_long serial,
                                     const c_char* kernel_uri,
                                     v_kind kind,
                                     const c_voidp (*func)(c_iter entities, c_voidp funcArg),
                                     const c_voidp fArg);

/**@brief Retrieves the type of the supplied entity.
 * 
 * @param index The index of the handle of the entity.
 * @param serial The serial of the handle of the entity.
 * @param kernel_uri The URI of the kernel to look in.
 * 
 * @return The type of the entity.
 */
c_type          cm_getEntityType    (c_long index,
                                     c_long serial,
                                     const c_char* kernel_uri);

#if defined (__cplusplus)
}
#endif

#endif /* CM_API_H */
