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
#ifndef MM_ORC_H
#define MM_ORC_H

#include "c_typebase.h"
#include "v_entity.h"

C_CLASS(monitor_orc);
#define monitor_orc(o)     ((monitor_orc)(o))

monitor_orc
monitor_orcNew (
    c_long refCountLimit,
    const char *filterExpression,
    c_bool delta
    );

void
monitor_orcFree (
    monitor_orc o
    );

void
monitor_orcAction (
    v_entity entity,
    c_voidp args
    );

#endif /* MM_ORC_H */
