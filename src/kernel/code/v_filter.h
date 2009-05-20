/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef V_FILTER_H
#define V_FILTER_H

#include <v_kernel.h>

#define v_filter(f) (C_CAST(f,v_filter))

v_filter
v_filterNew (
    v_topic t,
    q_expr e,
    c_value params[]);

c_bool
v_filterEval (
    v_filter _this,
    c_object o);

#endif

