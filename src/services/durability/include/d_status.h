
#ifndef D_STATUS_H
#define D_STATUS_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_status(s) ((d_status)(s))

d_status    d_statusNew     (d_admin admin);

void        d_statusFree    (d_status status);

#if defined (__cplusplus)
}
#endif

#endif /* D_STATUS_H */
