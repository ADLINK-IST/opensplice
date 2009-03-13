
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
