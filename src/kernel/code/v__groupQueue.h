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

#ifndef V__GROUPQUEUE_H
#define V__GROUPQUEUE_H

#include "v_groupQueue.h"
#include "kernelModule.h"

#if defined (__cplusplus)
extern "C" {
#endif

v_writeResult
v_groupQueueWrite (
    v_groupQueue _this,
    v_groupAction action);

void
v_groupQueueInit (
    v_groupQueue _this,
    v_subscriber subscriber,
    const c_char *name,
    c_ulong size,
    v_readerQos qos,
    v_statistics qstat,
    c_iter expr);

void
v_groupQueueDeinit (
    v_groupQueue _this);

#if defined (__cplusplus)
}
#endif

#endif
