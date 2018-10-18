/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef D__READERLISTENER_H
#define D__READERLISTENER_H

#include "d__types.h"
#include "d__listener.h"
#include "d__subscriber.h"
#include "u_object.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * \brief The d_readerListener cast macro.
 *
 * This macro casts an object to a d_readerListener object.
 */
#define d_readerListener(_this) ((d_readerListener)(_this))

C_STRUCT(d_readerListener){
    C_EXTENDS(d_listener);
    u_dataReader        dataReader;
    os_size_t           fieldOffset;
    c_iter              takenSamples;
    c_object            value;
    c_bool              processMessage;
    d_waitsetEntity     waitsetData;
    os_threadAttr       attr;
    c_char*             name;
    c_ulong             samplesTotal;
    c_ulong             samplesFromMe;
    c_ulong             samplesForMe;
    os_timeW            lastInsertTime;
    os_timeW            lastSourceTime;
    c_ulong             myAddr;
};

c_bool              d_readerListenerStart           (d_readerListener listener);
c_bool              d_readerListenerStop            (d_readerListener listener);
void                d_readerListenerDeinit          (d_readerListener listener);
void                d_readerListenerInit            (d_readerListener listener,
                                                     d_listenerKind kind,
                                                     d_listenerAction action,
                                                     d_subscriber subscriber,
                                                     const c_char* topicName,
                                                     const c_char* fieldName,
                                                     v_reliabilityKind reliability,
                                                     v_historyQosKind historyKind,
                                                     c_long historyDepth,
                                                     os_threadAttr attr,
                                                     d_objectDeinitFunc deinit);

#if defined (__cplusplus)
}
#endif

#endif /* D__READERLISTENER_H */
