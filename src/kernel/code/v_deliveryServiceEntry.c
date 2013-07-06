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
#include "v_deliveryServiceEntry.h"
#include "v_kernel.h"
#include "v_reader.h"
#include "v_observer.h"
#include "v_state.h"

#include "v__deliveryService.h"
#include "v__entry.h"
#include "v__messageQos.h"

#include "os_report.h"

/**************************************************************
 * Private functions
 **************************************************************/
/**************************************************************
 * constructor/destructor
 **************************************************************/
v_deliveryServiceEntry
v_deliveryServiceEntryNew(
    v_deliveryService deliveryService,
    v_topic topic)
{
    v_kernel kernel;
    v_deliveryServiceEntry e;

    assert(C_TYPECHECK(deliveryService,v_deliveryService));
    assert(C_TYPECHECK(topic,v_topic));

    kernel = v_objectKernel(deliveryService);
    e = v_deliveryServiceEntry(v_objectNew(kernel,K_DELIVERYSERVICEENTRY));
    v_entryInit(v_entry(e), v_reader(deliveryService));
    e->topic = c_keep(topic);

    return e;
}

v_writeResult
v_deliveryServiceEntryWrite(
    v_deliveryServiceEntry _this,
    v_message message,
    v_instance *instancePtr)
{
    v_deliveryInfoTemplate ackMsg;
    v_writeResult result = V_WRITE_REJECTED;
    v_reader reader;

    OS_UNUSED_ARG(instancePtr);
    assert(C_TYPECHECK(_this,v_deliveryServiceEntry));
    assert(message != NULL);

    /* Only write if the message is not produced by an incompatible writer. */
    reader = v_entryReader(_this);
    v_observerLock(v_observer(reader));

    /* Filter-out all QoS-incompatible messages. */
    if (!v_messageQos_isReaderCompatible(message->qos,reader)) {
        v_observerUnlock(v_observer(reader));
        return V_WRITE_SUCCESS;
    }

    /* If Alive then claim instance and trigger with sample event.
     */
    ackMsg = (v_deliveryInfoTemplate)message;

    result = v_deliveryServiceWrite(v_deliveryService(reader),ackMsg);

    v_observerUnlock(v_observer(reader));

    return result;
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
