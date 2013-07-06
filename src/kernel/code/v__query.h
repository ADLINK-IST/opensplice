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

#ifndef V__QUERY_H
#define V__QUERY_H

#include "v_query.h"

void
v_queryNotify (
    v_query _this,
    v_event event,
    c_voidp userData);

c_bool
v_queryNotifyDataAvailable (
    v_query _this,
    v_event e);

void
v_querySetParamsString (
    v_query _this,
    q_expr predicate,
    c_value params[]);

#endif
