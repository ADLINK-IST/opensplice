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
#ifndef _C_CLONE_H_
#define _C_CLONE_H_


#include "c_typebase.h"
#include "c_metabase.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef struct c_clone_s *c_clone;

OS_API c_clone  c_cloneNew      (c_base destination);
OS_API void     c_cloneFree     (c_clone c);
OS_API c_object c_cloneCloneObject   (c_clone c, c_object obj);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /*_C_CLONE_H_ */
