#include "in__configTracing.h"

OS_STRUCT(in_configTracing)
{
    os_char* pathName;
    os_boolean isEnabled;
    os_char* outputFileName;
    os_uint32 initLevel;
    os_uint32 deinitLevel;
    os_uint32 mainloopLevel;
    os_uint32 groupsLevel;
    os_uint32 writingLevel;
    os_uint32 readingLevel;
    os_uint32 testLevel;
    os_uint32 discoveryLevel;
    in_configTimestamps timestamps;
};

os_char*
in_configTracingGetPathName(
    in_configTracing _this)
{
    assert(_this);

    return _this->pathName;
}

os_boolean
in_configTracingIsEnabled(
    in_configTracing _this)
{
    assert(_this);

    return _this->isEnabled;
}

os_char*
in_configTracingGetOutputFileName(
    in_configTracing _this)
{
    assert(_this);

    return _this->outputFileName;
}

os_uint32
in_configTracingGetInitLevel(
    in_configTracing _this)
{
    assert(_this);

    return _this->initLevel;
}

os_uint32
in_configTracingGetDeinitLevel(
    in_configTracing _this)
{
    assert(_this);

    return _this->deinitLevel;
}

os_uint32
in_configTracingGetMainloopLevel(
    in_configTracing _this)
{
    assert(_this);

    return _this->mainloopLevel;
}

os_uint32
in_configTracingGetGroupsLevel(
    in_configTracing _this)
{
    assert(_this);

    return _this->groupsLevel;
}

os_uint32
in_configTracingGetWritingLevel(
    in_configTracing _this)
{
    assert(_this);

    return _this->writingLevel;
}

os_uint32
in_configTracingGetReadingLevel(
    in_configTracing _this)
{
    assert(_this);

    return _this->readingLevel;
}


os_uint32
in_configTracingGetTestLevel(
    in_configTracing _this)
{
    assert(_this);

    return _this->testLevel;
}

os_uint32
in_configTracingGetDiscoveryLevel(
    in_configTracing _this)
{
    assert(_this);

    return _this->discoveryLevel;
}

in_configTimestamps
in_configTracingGetTimeStamps(
    in_configTracing _this)
{
    assert(_this);

    return _this->timestamps;
}
