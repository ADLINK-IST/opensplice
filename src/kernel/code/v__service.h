
#ifndef V__SERVICE_H
#define V__SERVICE_H

#include "v_service.h"
#include "v_event.h"

void
v_serviceNotify(
    v_service _this,
    v_event event,
    c_voidp userData);

#endif /* V__SERVICE_H */
