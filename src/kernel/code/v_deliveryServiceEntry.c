/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "v_deliveryServiceEntry.h"
#include "v_kernel.h"
#include "v_reader.h"
#include "v_observer.h"
#include "v_messageQos.h"
#include "v__deliveryService.h"
#include "v__entry.h"

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
