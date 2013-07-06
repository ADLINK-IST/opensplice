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

#ifndef V__GROUPSTREAM_H
#define V__GROUPSTREAM_H

#include "v_group.h"
#include "v_groupStream.h"
#include "kernelModule.h"

#if defined (__cplusplus)
extern "C" {
#endif

void
v_groupStreamConnectNewGroups (
    v_groupStream _this,
    v_group group);

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
    v_partition partition);
                                                         
c_bool
v_groupStreamUnSubscribe (
    v_groupStream _this,
    v_partition partition);
                                                         
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
