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
