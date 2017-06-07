/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/**@file api/cm/xml/code/cmx__entity.h
 *
 * Offers internal routines on an entity.
 */
#ifndef CMX__ENTITY_H
#define CMX__ENTITY_H

#include "os_abstract.h"
#include "c_typebase.h"
#include "v_kernel.h"
#include "v_entity.h"
#include "u_user.h"
#include "cmx__factory.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_entity.h"

C_CLASS(cmx_entityArg);
C_CLASS(cmx_walkEntityArg);
C_CLASS(cmx_entityKernelArg);
C_CLASS(cmx_entityKindArg);

C_STRUCT(cmx_entityArg){
    cmx_entity entity;          /*The entity that matches the kernel entity.*/
    c_char* result;             /*The XML representation of the entity.*/
    c_bool create;              /*Whether to actually create the entity or create a proxy.*/
};

C_STRUCT(cmx_walkEntityArg){
    v_kind filter;              /*The filter that was supplied by the user.*/
    c_iter list;                /*The list of XML entities.*/
    c_ulong length;             /*The total length of all XML in the list*/
    C_STRUCT(cmx_entityArg) entityArg;    /*The entity creation arguments.*/
    c_voidp userData;
};

C_STRUCT(cmx_entityKernelArg){
    v_kernel kernel;            /*The kernel itself.*/
};

C_STRUCT(cmx_entityKindArg){
    v_kind kind;
};

#define cmx_entity(a) ((cmx_entity)(a))
#define cmx_entityArg(a) ((cmx_entityArg)(a))
#define cmx_walkEntityArg(a) ((cmx_walkEntityArg)(a))
#define cmx_entityKernelArg(a) ((cmx_entityKernelArg)(a))
#define cmx_entityKindArg(a) ((cmx_entityKindArg)(a))

#define CMX_RESULT_OK "<result>OK</result>"
#define CMX_RESULT_FAILED "<result>FAILED</result>"
#define CMX_RESULT_NOT_IMPLEMENTED "<result>Not implemented (yet)</result>"
#define CMX_RESULT_ILL_PARAM "<result>ILLEGAL PARAMETER</result>"
#define CMX_RESULT_ENTITY_NOT_AVAILABLE "<result>ENTITY NOT AVAILABLE</result>"
#define CMX_RESULT_IMMUTABLE_POLICY "<result>IMMUTABLE POLICY</result>"
#define CMX_RESULT_INCONSISTENT_QOS "<result>INCONSISTENT QOS</result>"
#define CMX_RESULT_TIMEOUT "<result>TIMEOUT</result>"
#define CMX_RESULT_PRECONDITION_NOT_MET "<result>PRECONDITION NOT MET</result>"

/**
 * Constructs an XML representation of the supplied entity. Whether or not
 * a created entity is owned, depends on the create boolean in the args.
 *
 * @param entity The kernel entity to create a user and XML entity for.
 * @param args Must be of type cmx_entityArg and supplied information about
 *             how to create the user and XML entity.
 */
void        cmx_entityNewFromAction         (v_public entity,
                                             c_voidp args);

/**
 * Constructs an XML representation of the supplied entity. Whether or not
 * a created entity is owned, depends on the create boolean in the args.
 *
 * @param entity The kernel entity to create a user and XML entity for.
 * @param args Must be of type cmx_entityArg and supplied information about
 *             how to create the user and XML entity.
 * @return TRUE if succeeded, FALSE otherwise.
 */
c_bool      cmx_entityNewFromWalk           (v_public entity,
                                             c_voidp args);

/**
 * Initializes the entity type specific part of the XML representation of the
 * supplied kernel entity. This function should only be used by the
 * cmx_entityNewFromWalk function.
 *
 * @param entity The entity to create a XML representation of.
 * @return The entity type specific part of the XML representation of the
 *         entity.
 */
c_char*     cmx_entityGetTypeXml            (v_public entity);

/**
 * Applies the entity filter on the supplied entity. If it matches the filter,
 * the entity is added to the list of resolved entities, if not it is ignored.
 *
 * @param e The resolved entity, that was resolved during the walk action.
 * @param args Must be of type cmx_walkEntityArg, it supplies the filter as well
 *             as the list of resolved entities until now.
 * @return Always TRUE.
 */
c_bool      cmx_entityWalkAction            (v_entity e,
                                             c_voidp args);

/**
 * Creates the entity XML according to the given arguments.
 *
 * @param name     Entity name (maybe NULL).
 * @param proxy    Address of the Entity proxy (maybe NULL).
 * @param handle   Kernel handle of the Entity (maybe NULL).
 * @param enabled  Indicates if Entity is enabled (or not).
 * @param special  The specifics of this Entity (XML format, not NULL).
 * @return XML that represents the object as Entity.
 */
c_char*     cmx_entityXml                   (const c_string  name,
                                             const c_address proxy,
                                             const v_handle *handle,
                                             const c_bool    enabled,
                                             const c_string  special);

cmx_entity  cmx_entityClaim(const c_char* xmlEntity);

#define     cmx_entityRelease(entity) cmx_factoryReleaseEntity(entity)

/**
 * Entity action routine to resolve the current status of the entity.
 *
 * @param entity The entity to resolve the status of.
 * @param args Must be of type struct cmx_statusArg, which will contain the XML
 *             representation of the status of the entity, when it is available.
 */
void        cmx_entityStatusAction          (v_public entity,
                                             c_voidp args);


void        cmx_entityStatisticsAction      (v_public entity,
                                             c_voidp args);

void        cmx_entityStatisticsResetAction (v_public entity,
                                             c_voidp args);

void        cmx_entityStatisticsFieldResetAction (
                                             v_public entity,
                                             c_voidp args);

/**
 * Entity action routine to resolve the kind of the entity.
 *
 * @param entity The entity to resolve the kind of.
 * @param args Must be of type struct cmx_entityKindArg, which will contain the
 *             XML representation of the kind of the entity when the routine has
 *             finished.
 */
void        cmx_entityKindAction             (v_public entity,
                                              c_voidp args);

/**
 * Entity action routine to resolve the kernel where the entity 'lives' in.
 *
 * @param entity The entity to resolve the kernel of.
 * @param args Must be of type struct cmx_entityKernelArg, which will contain
 *             the kernel of the entity when the routine has finished.
 */
void        cmx_entityKernelAction          (v_public entity,
                                             c_voidp args);

u_result    cmx_entityRegister              (u_object object,
                                             cmx_entity participant,
                                             c_char **xml);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__ENTITY_H */
