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
#ifndef DLRL_META_MODEL_ATTRIBUTE_TYPE_H
#define DLRL_META_MODEL_ATTRIBUTE_TYPE_H

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_LOC_METAMODEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef enum DMM_AttributeType{
    DMM_ATTRIBUTETYPE_LONG,
    DMM_ATTRIBUTETYPE_FLOAT,
    DMM_ATTRIBUTETYPE_STRING,
    DMM_ATTRIBUTETYPE_DOUBLE,
    DMM_ATTRIBUTETYPE_LONGLONG,
    DMM_ATTRIBUTETYPE_LONGDOUBLE,
    DMM_ATTRIBUTETYPE_CHAR,
    DMM_ATTRIBUTETYPE_BOOLEAN,
    DMM_ATTRIBUTETYPE_ENUM,
    DMM_AttributeType_elements/* much more types need to be defined! */
} DMM_AttributeType;

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_META_MODEL_ATTRIBUTE_TYPE_H */
