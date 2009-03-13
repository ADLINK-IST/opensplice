#include "in_transport.h"
#include "os_heap.h"

void
in_transportFree(
    in_transport _this)
{
    assert(_this);

    /* TODO add call to final class free function */
    os_free(_this);
}
