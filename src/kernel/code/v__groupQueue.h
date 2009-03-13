
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
    v_readerQos qos);

void
v_groupQueueDeinit (
    v_groupQueue _this);

#if defined (__cplusplus)
}
#endif

#endif
