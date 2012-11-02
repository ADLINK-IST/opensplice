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
#ifndef V_MINVALUE_H
#define V_MINVALUE_H

#include "c_typebase.h"
#include "v_time.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API void    v_minValueInit       (v_minValue *_this);
OS_API void    v_minValueReset      (v_minValue *_this);
OS_API c_time  v_minValueGetTime    (v_minValue *_this);
OS_API c_ulong v_minValueGetValue   (v_minValue *_this);
OS_API void    v_minValueSetValue   (v_minValue *_this, c_ulong value);

#undef OS_API

#endif
