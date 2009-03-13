

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
    v_instance _this);

v_writeResult
v_instanceWrite (
    v_instance _this,
    v_message message);

/* param count specifies the number of registrations that are unregisters */
void
v_instanceUnregister (
    v_instance _this,
    c_long count);

#endif
