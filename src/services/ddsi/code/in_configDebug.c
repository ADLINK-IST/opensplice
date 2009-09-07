#include "in__configDebug.h"
#include "in_commonTypes.h"
#include "os.h"
#include "in_report.h"

OS_STRUCT(in_configDebug)
{
    os_char* pathName;
    os_uint32 wait;
};

in_configDebug 
in_configDebugNew(void)
{
    in_configDebug result = NULL;
    
    result = in_configDebug(os_malloc(OS_SIZEOF(in_configDebug)));
    result->pathName = NULL;
    result->wait = 0;
    
    return result;
}


void
in_configDebugFree(in_configDebug _this)
{
    if (_this) {
        if (_this->pathName) os_free(_this->pathName);
        os_free(_this);
    }
}

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

void
in_configDebugSetWaitTime(
    in_configDebug _this, 
    os_uint32 waitTime)
{
    assert(_this);
    
    _this->wait = waitTime;
}

