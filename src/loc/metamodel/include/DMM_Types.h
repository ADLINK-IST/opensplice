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
#ifndef DLRL_META_MODEL_TYPES_H
#define DLRL_META_MODEL_TYPES_H

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

/* for C++ compatibility */
struct DMM_DLRLClass_s;
typedef struct DMM_DLRLClass_s DMM_DLRLClass;

struct DMM_DCPSField_s;
typedef struct DMM_DCPSField_s DMM_DCPSField;

struct DMM_DCPSTopic_s;
typedef struct DMM_DCPSTopic_s DMM_DCPSTopic;

struct DMM_DLRLAttribute_s;
typedef struct DMM_DLRLAttribute_s DMM_DLRLAttribute;

struct DMM_DLRLMultiAttribute_s;
typedef struct DMM_DLRLMultiAttribute_s DMM_DLRLMultiAttribute;

struct DMM_DLRLMultiRelation_s;
typedef struct DMM_DLRLMultiRelation_s DMM_DLRLMultiRelation;

struct DMM_DLRLRelation_s;
typedef struct DMM_DLRLRelation_s DMM_DLRLRelation;

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_META_MODEL_TYPES_H */
