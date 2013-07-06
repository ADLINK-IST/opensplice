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
#ifndef DLRL_META_MODEL_INHERITANCE_TABLE_H
#define DLRL_META_MODEL_INHERITANCE_TABLE_H

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

#define DMM_DLRL_RELATION_CLASS               0x20000000u
#define DMM_DLRL_MULTI_RELATION_CLASS         (DMM_DLRL_RELATION_CLASS + 1)

#define DMM_DLRL_ATTRIBUTE_CLASS              0x4000000u
#define DMM_DLRL_MULTI_ATTRIBUTE_CLASS        (DMM_DLRL_ATTRIBUTE_CLASS + 1)

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_TYPES_H */
