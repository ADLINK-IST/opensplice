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
#ifndef MM_MS_H
#define MM_MS_H

#include "c_typebase.h"
#include "v_entity.h"

C_CLASS(monitor_ms);
#define monitor_ms(o)     ((monitor_ms)(o))

monitor_ms
monitor_msNew (
    c_bool extendedMode,
    c_bool rawMode,
    c_bool delta,
    c_bool preallocated
    );

void
monitor_msFree (
    monitor_ms o
    );

void
monitor_msAction (
    v_entity entity,
    c_voidp args
    );

#endif /* MM_MS_H */
