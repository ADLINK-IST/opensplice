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
#ifndef V_MAXVALUE_H
#define V_MAXVALUE_H

#include "c_typebase.h"
#include "v_time.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API void    v_maxValueInit       (v_maxValue *_this);
OS_API void    v_maxValueReset      (v_maxValue *_this);
OS_API c_time  v_maxValueGetTime    (v_maxValue *_this);
OS_API c_ulong v_maxValueGetValue   (v_maxValue *_this);
OS_API void    v_maxValueSetValue   (v_maxValue *_this, c_ulong value);

#undef OS_API

#endif
