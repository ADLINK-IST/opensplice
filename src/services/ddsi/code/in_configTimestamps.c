#include "in__configTimestamps.h"

OS_STRUCT(in_configTimestamps)
{
    os_char* pathName;
    os_boolean isAbsolute;
};


os_char*
in_configTimestampsGetPathName(
    in_configTimestamps _this)
{
    assert(_this);

    return _this->pathName;
}

os_boolean
in_configTimestampsIsAbsolute(
    in_configTimestamps _this)
{
    assert(_this);

    return _this->isAbsolute;
}
