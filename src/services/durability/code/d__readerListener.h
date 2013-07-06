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

#ifndef D__READERLISTENER_H
#define D__READERLISTENER_H

#include "d__types.h"
#include "d__listener.h"
#include "u_dispatcher.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_readerListener){
    C_EXTENDS(d_listener);
    u_dataReader        dataReader;
    c_ulong             fieldOffset;
    d_message           message;
    c_object            value;
    c_bool              processMessage;
    d_waitsetEntity     waitsetData;
    os_threadAttr       attr;
    c_char*             name;
    c_ulong             samplesTotal;
    c_ulong             samplesFromMe;
    c_ulong             samplesForMe;
    c_time              lastInsertTime;
    c_time              lastSourceTime;
    c_ulong             myAddr;
    d_objectDeinitFunc  deinit;
};

#define D_NO_ADDRESS            (d_networkAddress(0x0u))  /**< no address defined (yet) */
#define D_SELECT_ALL            "select * from %s"
#define D_STRLEN_SELECT_ALL     ((size_t)(16))

c_ulong             d_readerListenerAction          (u_dispatcher o,
                                                     u_waitsetEvent event,
                                                     c_voidp usrData);

void                d_readerListenerProcessAction   (d_message message,
                                                     c_voidp copyArg);

void                d_readerListenerInitView        (d_readerListener listener,
                                                     d_subscriber subscriber,
                                                     const c_char* topicName);

void                d_readerListenerInitDataReader  (d_readerListener listener,
                                                     d_subscriber subscriber,
                                                     const c_char* name,
                                                     v_reliabilityKind reliability,
                                                     v_historyQosKind historyKind,
                                                     c_long historyDepth);

void                d_readerListenerInitField       (d_readerListener listener,
                                                     d_subscriber   subscriber,
                                                     const c_char * typeName);

c_bool              d_readerListenerStart           (d_readerListener listener);

c_bool              d_readerListenerStop            (d_readerListener listener);

v_actionResult      d_readerListenerCopy            (c_object object,
                                                     c_voidp copyArg);

void                d_readerListenerDeinit          (d_object object);

#if defined (__cplusplus)
}
#endif

#endif /* D__READERLISTENER_H */
