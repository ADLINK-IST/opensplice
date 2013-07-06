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
#ifndef MM_TRC_H
#define MM_TRC_H

#include "c_typebase.h"
#include "v_entity.h"

C_CLASS(monitor_trc);
#define monitor_trc(o)     ((monitor_trc)(o))

typedef enum {
    NO_ORDERING,
    ORDER_BY_COUNT, 
    ORDER_BY_SIZE, 
    ORDER_BY_TOTAL
} orderKind;

monitor_trc
monitor_trcNew (
    c_long objectCountLimit,
    const char *filterExpression,
    orderKind oKind,
    int orderCount,
    c_bool delta);

void
monitor_trcFree (
    monitor_trc o
    );

void
monitor_trcAction (
    v_entity entity,
    c_voidp args
    );

#endif /* MM_TRC_H */
