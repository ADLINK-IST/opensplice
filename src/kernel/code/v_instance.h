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


#ifndef V_INSTANCE_H
#define V_INSTANCE_H

#include "v_kernel.h"

#define v_instance(o) (C_CAST(o,v_instance))

void
v_instanceInit (
    v_instance _this);

void
v_instanceDeinit (
    v_instance _this);

void
v_instanceInsert (
    v_instance _this,
    v_instance prev);
void
v_instanceAppend (
    v_instance _this,
    v_instance next);

void
v_instanceRemove (
    v_instance _this);

c_bool
v_instanceAlone (
    v_instance _this);

void
v_instanceUpdate (
    v_instance _this,
    c_time timestamp);

v_writeResult
v_instanceWrite (
    v_instance _this,
    v_message message);

/* registration indicates the specific registration that needs to be unregistered. */
void
v_instanceUnregister (
    v_instance instance,
    v_registration registration,
    c_time timestamp);

#endif
