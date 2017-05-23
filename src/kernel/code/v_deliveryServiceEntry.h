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
#ifndef V_DELIVERYSERVICEENTRY_H
#define V_DELIVERYSERVICEENTRY_H

/** \file kernel/include/v_deliveryServiceEntry.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_entry.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_deliveryServiceEntry</code> cast method.
 *
 * This method casts an object to a <code>v_deliveryServiceEntry</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_deliveryServiceEntry</code> or
 * one of its subclasses.
 */
#define v_deliveryServiceEntry(e) (C_CAST(e,v_deliveryServiceEntry))

#define v_deliveryServiceEntryTopic(_this) \
        v_topic(v_deliveryServiceEntry(_this)->topic)

v_deliveryServiceEntry
v_deliveryServiceEntryNew(
    v_deliveryService deliveryService,
    v_topic topic);
    
void               
v_deliveryServiceEntryFree(
    v_deliveryServiceEntry _this);
    
v_writeResult      
v_deliveryServiceEntryWrite(
    v_deliveryServiceEntry _this, 
    v_message o, 
    v_instance *instance);
    
#if defined (__cplusplus)
}
#endif

#endif
