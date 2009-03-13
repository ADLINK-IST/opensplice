#ifndef V_NETWORKMESSAGE_H
#define V_NETWORKMESSAGE_H

#include "v_kernel.h"

#define v_networkMessage(o) (C_CAST(o,v_networkMessage))

v_networkMessage
v_networkMessageNew (
    v_kernel kernel);

void
v_networkMessageFree (
    v_networkMessage _this);

#endif
