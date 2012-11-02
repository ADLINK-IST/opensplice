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

/**@file api/cm/common/include/cm_typebase.h
 * @brief Contains all common functionality for the C&M API.
 */
#ifndef CM_TYPEBASE_H
#define CM_TYPEBASE_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "c_typebase.h"

/**@brief All kinds of the possible types.
 */
typedef enum cm_kind {
    CMC_ENTITY_SET, /*!< C language entity set.*/
    CMJ_ENTITY_SET, /*!< Java language entity set.*/
    CMC_ENTITY,     /*!< C language entity.*/
    CMJ_ENTITY,     /*!< Java language entity.*/
    CMC_QOS,        /*!< C language entity QoS policy.*/
    CMJ_QOS,        /*!< Java language entity QoS policy.*/
    CMC_TYPE,       /*!< C language entity type.*/
    CMJ_TYPE,       /*!< Java language entity type.*/
    CMC_STATUS,     /*!< C language entity status.*/
    CMJ_STATUS      /*!< Java language entity status.*/
} cm_kind;

/**@brief The C&M result type.
 */
typedef enum cm_result {
    CM_RESULT_OK,               /*!< Everything went fine.*/
    CM_RESULT_ERROR,            /*!< Unspecified generic error.*/
    CM_RESULT_BAD_PARAMETER,    /*!< Supplied parameter is not valid.*/
    CM_RESULT_UNSUPPORTED,      /*!< Functionality not implemented.*/
    CM_RESULT_UNDEFINED         /*!< Undefined error.*/
} cm_result;

C_CLASS(cm_baseObject);
C_CLASS(cm_entity);
C_CLASS(cm_entitySet);
C_CLASS(cm_qos);
C_CLASS(cm_status);
C_CLASS(cm_type);

/**@brief Base object that contains the kind.
 */
C_STRUCT(cm_baseObject){
    cm_kind kind;   /*!The kind of the object.*/
};

/**@brief Wrapper for entity objects.
 */
C_STRUCT(cm_entity){
   C_EXTENDS(cm_baseObject);
};

/**@brief Wrapper for entitySet objects.
 */
C_STRUCT(cm_entitySet){
   C_EXTENDS(cm_baseObject);
};

/**@brief Wrapper for entity qos objects.
 */
C_STRUCT(cm_qos){
    C_EXTENDS(cm_baseObject);
};

/**@brief Wrapper for entity status objects.
 */
C_STRUCT(cm_status){
    C_EXTENDS(cm_baseObject);
};

/**@brief Wrapper for entity type objects.
 */
C_STRUCT(cm_type){
    C_EXTENDS(cm_baseObject);
};

#define cm_baseObject(o) ((cm_baseObject)(o))
#define cm_entity(o) ((cm_entity)(o))
#define cm_entitySet(o) ((cm_entitySet)(o))
#define cm_qos(o) ((cm_qos)(o))
#define cm_status(o) ((cm_status)(o))
#define cm_type(o) ((cm_type)(o))

#if defined (__cplusplus)
}
#endif

#endif /* CM_TYPEBASE_H */
