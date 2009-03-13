
#ifndef V__SPLICED_H
#define V__SPLICED_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_spliced.h"

v_spliced
v_splicedNew (
    v_kernel kernel);

void
v_splicedInit (
    v_spliced _this);

void
v_splicedDeinit (
    v_spliced _this);

void
v_splicedHeartbeat (
    v_spliced _this);

void
v_splicedCheckHeartbeats (
    v_spliced _this);

#if defined (__cplusplus)
}
#endif

#endif /* V__SPLICED_H */
