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
#ifndef DJA_OBJECT_WRITER_BRIDGE_H
#define DJA_OBJECT_WRITER_BRIDGE_H

/* DLRL includes */
#include "DLRL_Types.h"

u_instanceHandle
DJA_ObjectWriterBridge_us_registerInstance(
    DLRL_Exception* exception,
    void* userData, 
    DK_ObjectWriter* objWriter,
    DK_ObjectAdmin* objectAdmin);

void
DJA_ObjectWriterBridge_us_write(
    DLRL_Exception* exception, 
    void* userData, 
    DK_ObjectWriter* writer, 
    DK_ObjectAdmin* object);

void
DJA_ObjectWriterBridge_us_destroy(
    DLRL_Exception* exception, 
    void* userData, 
    DK_ObjectWriter* writer, 
    DK_ObjectAdmin* object);

#endif /* DJA_OBJECT_WRITER_BRIDGE_H */
