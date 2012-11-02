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
#ifndef V_FULLCOUNTER_H
#define V_FULLCOUNTER_H

/** \file kernel/include/v_fullCounter.h
 *  \brief This file defines the interface
 *
 */

#include "v_time.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API void    v_fullCounterInit       (v_fullCounter *_this);
OS_API void    v_fullCounterReset      (v_fullCounter *_this);
OS_API c_ulong v_fullCounterGetValue   (v_fullCounter *_this);
OS_API void    v_fullCounterSetValue   (v_fullCounter *_this, c_ulong value);
OS_API void    v_fullCounterValueInc   (v_fullCounter *_this);
OS_API void    v_fullCounterValueDec   (v_fullCounter *_this);
OS_API c_time  v_fullCounterGetMaxTime (v_fullCounter *_this);
OS_API c_ulong v_fullCounterGetMax     (v_fullCounter *_this);
OS_API c_time  v_fullCounterGetMinTime (v_fullCounter *_this);
OS_API c_ulong v_fullCounterGetMin     (v_fullCounter *_this);
OS_API c_float v_fullCounterGetAvg     (v_fullCounter *_this);

#undef OS_API

#endif
