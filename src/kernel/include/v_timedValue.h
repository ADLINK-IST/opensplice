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
#ifndef V_TIMEDVALUE_H
#define V_TIMEDVALUE_H

#include "v_time.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API void    v_timedValueInit      (v_timedValue *_this);
OS_API void    v_timedValueReset     (v_timedValue *_this);
OS_API c_ulong v_timedValueGetValue  (v_timedValue *_this);
OS_API c_time  v_timedValueGetTime   (v_timedValue *_this);
OS_API void    v_timedValueSetValue  (v_timedValue *_this, c_ulong value);
OS_API void    v_timedValueValuePlus (v_timedValue *_this);
OS_API void    v_timedValueValueMin  (v_timedValue *_this);

#undef OS_API

#endif
