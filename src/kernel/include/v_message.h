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

#ifndef V_MESSAGE_H
#define V_MESSAGE_H

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_message</code> cast method.
 *
 * This method casts an object to a <code>v_message</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_message</code> or
 * one of its subclasses.
 */
#define v_message(o) (C_CAST(o,v_message))

#define v_messageState(_this) \
        (v_nodeState(v_message(_this)))

#define v_messageStateTest(_this,_state) \
        (v_stateTest(v_messageState(_this),_state))

#ifdef _MSG_STAMP_

#define V_MESSAGE_INIT(_this) \
            v_message(_this)->hops = 0

#define V_MESSAGE_STAMP(_this,_id) { \
            os_time c_time = os_hrtimeGet(); \
            v_message(_this)->_id[v_message(_this)->hops] = \
                (v_hrtime)c_time.tv_sec * (v_hrtime)1000000000 + \
                (v_hrtime)c_time.tv_nsec; \
        }

#define V_MESSAGE_SETSTAMP(_this,_id,_time) { \
            v_message(_this)->_id[v_message(_this)->hops] = \
                (v_hrtime)_time.tv_sec * (v_hrtime)1000000000 + \
                (v_hrtime)_time.tv_nsec; \
        }

#define V_MESSAGE_HOPINC(_this) \
            v_message(_this)->hops++

#define V_MESSAGE_HOPDEC(_this) \
            v_message(_this)->hops++

#define V_MESSAGE_REPORT(_this,reader) \
            v_dataReaderLogMessage(reader,_this)

#else
#define V_MESSAGE_INIT(_this)
#define V_MESSAGE_STAMP(_this,_id)
#define V_MESSAGE_HOPINC(_this)
#define V_MESSAGE_HOPDEC(_this)
#define V_MESSAGE_REPORT(_this,reader)
#endif

OS_API void
v_messageSetAllocTime(
    v_message _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
