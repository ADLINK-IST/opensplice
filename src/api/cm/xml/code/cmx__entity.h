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
/**@file api/cm/xml/code/cmx__entity.h
 * 
 * Offers internal routines on an entity.
 */
#ifndef CMX__ENTITY_H
#define CMX__ENTITY_H

#include "c_typebase.h"
#include "v_kernel.h"
#include "v_entity.h"
#include "u_user.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_entity.h"

C_CLASS(cmx_entityArg);
C_CLASS(cmx_walkEntityArg);
C_CLASS(cmx_entityKernelArg);
C_CLASS(cmx_entityKindArg);

C_STRUCT(cmx_entityArg){
    u_entity entity;            /*The user entity that matches the kernel entity.*/
    u_participant participant;  /*The user participant of the entity.*/
    c_char* result;             /*The XML representation of the entity.*/
    c_bool create;              /*Whether to actually create the entity or create a proxy.*/
};

C_STRUCT(cmx_walkEntityArg){
    v_kind filter;              /*The filter that was supplied by the user.*/
    c_iter list;                /*The list of XML entities.*/
    c_long length;              /*The total length of all XML in the list*/
    cmx_entityArg entityArg;    /*The entity creation arguments.*/
    c_voidp userData;
};

C_STRUCT(cmx_entityKernelArg){
    v_kernel kernel;            /*The kernel itself.*/
};

C_STRUCT(cmx_entityKindArg){
    v_kind kind;
};

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

/**
 * Constructs an XML representation of the supplied entity. Whether or not
 * a created entity is owned, depends on the create boolean in the args. 
 * 
 * @param entity The kernel entity to create a user and XML entity for.
 * @param args Must be of type cmx_entityArg and supplied information about 
 *             how to create the user and XML entity.
 */
void        cmx_entityNewFromAction         (v_entity entity,
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
c_bool      cmx_entityNewFromWalk           (v_entity entity,
                                             c_voidp args);

/**
 * Initializes the entity type specific part of the XML representation of the 
 * supplied kernel entity. This function should only be used by the 
 * cmx_entityNewFromWalk function.
 * 
 * @param entity The entity to create a XML representation of.
 * @param proxy The proxy user entity that will be initialized.
 * @param init Whether or not to initialize the user entity.
 * @return The entity type specific part of the XML representation of the 
 *         entity.
 */
c_char*     cmx_entityInit                  (v_entity entity, 
                                             u_entity proxy, 
                                             c_bool init);

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
 * Resolves the user entity that matches the supplied XML entity. This is done
 * by casting the contents of the pointer tag in the XML entity to a user 
 * entity.
 * 
 * @param xmlEntity The XML representation of the user entity to resolve.
 * @return The user entity that matches the supplied XML entity.
 */
u_entity    cmx_entityUserEntity            (const c_char* xmlEntity);

/**
 * Resolves the user entities that match the supplied XML entities. This is done
 * by casting the contents of the pointer tags in the XML entity to user
 * entities.
 *
 * @param xmlEntities The XML representation of the user entities to resolve.
 * @return The user entities that match the supplied XML entities.
 */
c_iter   cmx_entityUserEntities            (const c_char* xmlEntities);

/**
 * Frees the supplied user entity. It checks whether the entity is owned. If
 * not, the u_entityFree is called. If so, the entity specific destructor is
 * called. The entity is NOT deregistered from the cmx_factory.
 * 
 * @param entity The entity to free.
 */
void        cmx_entityFreeUserEntity        (u_entity entity);

/**
 * Entity action routine to resolve the current status of the entity.
 * 
 * @param entity The entity to resolve the status of.
 * @param args Must be of type struct cmx_statusArg, which will contain the XML 
 *             representation of the status of the entity, when it is available.
 */
void        cmx_entityStatusAction          (v_entity entity,
                                             c_voidp args);


void        cmx_entityStatisticsAction      (v_entity entity,
                                             c_voidp args);

void        cmx_entityStatisticsResetAction (v_entity entity,
                                             c_voidp args);
                                         
/**
 * Entity action routine to resolve the kind of the entity.
 * 
 * @param entity The entity to resolve the kind of.
 * @param args Must be of type struct cmx_entityKindArg, which will contain the 
 *             XML representation of the kind of the entity when the routine has 
 *             finished.
 */
void        cmx_entityKindAction             (v_entity entity,
                                              c_voidp args);

/**
 * Entity action routine to resolve the kernel where the entity 'lives' in.
 * 
 * @param entity The entity to resolve the kernel of.
 * @param args Must be of type struct cmx_entityKernelArg, which will contain 
 *             the kernel of the entity when the routine has finished.
 */
void        cmx_entityKernelAction          (v_entity entity,
                                             c_voidp args);
#if defined (__cplusplus)
}
#endif

#endif /* CMX__ENTITY_H */
