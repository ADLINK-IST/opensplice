#include "v_kernel.h"
#include "v_message.h"
#include "v_time.h"

void
v_messageSetAllocTime(
    v_message _this)
{
    if (_this) {
        _this->allocTime = v_timeGet();
    }
}


