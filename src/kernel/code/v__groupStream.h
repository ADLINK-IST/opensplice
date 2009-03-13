
#ifndef V__GROUPSTREAM_H
#define V__GROUPSTREAM_H

#include "v_event.h"
#include "v_groupStream.h"
#include "kernelModule.h"

#if defined (__cplusplus)
extern "C" {
#endif

void
v_groupStreamNotify (
    v_groupStream _this,
    v_event e,
    c_voidp userData);

void
v_groupStreamNotifyDataAvailable (
    v_groupStream _this);

v_writeResult
v_groupStreamWrite (
    v_groupStream _this,
    v_groupAction action);

c_bool
v_groupStreamSubscribe (
    v_groupStream _this,
    v_domain partition);
                                                         
c_bool
v_groupStreamUnSubscribe (
    v_groupStream _this,
    v_domain partition);
                                                         
c_bool
v_groupStreamSubscribeGroup (
    v_groupStream _this, 
    v_group group);
                                             
c_bool
v_groupStreamUnSubscribeGroup (
    v_groupStream _this, 
    v_group group);
                                             
#if defined (__cplusplus)
}
#endif

#endif
