
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

