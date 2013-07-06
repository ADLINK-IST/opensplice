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
/* NOT IN DESIGN -  entire class */
#ifndef DLRL_KERNEL_OBJECT_WRITER_BRIDGE_H
#define DLRL_KERNEL_OBJECT_WRITER_BRIDGE_H

#include "kernelModule.h"
#include "DLRL_Kernel.h"


#if defined (__cplusplus)
extern "C" {
#endif

/* NOT IN DESIGN */
typedef u_instanceHandle (*DK_ObjectWriterBridge_us_registerInstance)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectWriter* objWriter,
    DK_ObjectAdmin* objectAdmin);

/* NOT IN DESIGN */
typedef void (*DK_ObjectWriterBridge_us_write)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectWriter* writer,
    DK_ObjectAdmin* object);

/* NOT IN DESIGN */
typedef void (*DK_ObjectWriterBridge_us_destroy)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectWriter* writer,
    DK_ObjectAdmin* object);

typedef struct DK_ObjectWriterBridge_s{
     DK_ObjectWriterBridge_us_registerInstance registerInstance;
     DK_ObjectWriterBridge_us_write write;
     DK_ObjectWriterBridge_us_destroy destroy;
} DK_ObjectWriterBridge;

extern DK_ObjectWriterBridge objectWriterBridge;

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_OBJECT_WRITER_BRIDGE_H */
