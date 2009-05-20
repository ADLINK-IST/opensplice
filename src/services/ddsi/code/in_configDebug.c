#include "in__configDebug.h"

OS_STRUCT(in_configDebug)
{
    os_char* pathName;
    os_uint32 wait;
};

os_uint32
in_configDebugGetWaitTime(
    in_configDebug _this)
{
    assert(_this);

    return _this->wait;
}

os_char*
in_configDebugGetPathName(
    in_configDebug _this)
{
    assert(_this);

    return _this->pathName;
}
