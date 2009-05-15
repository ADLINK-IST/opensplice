#include "in__configTracing.h"
#include "os_heap.h"

static os_boolean
in_configTracingInit(
    in_configTracing _this);

OS_STRUCT(in_configTracing)
{
    os_char* pathName;
    os_boolean isEnabled;
    os_char* outputFileName;
    os_uint32 initLevel;
    os_uint32 configurationLevel;
    os_uint32 deinitLevel;
    os_uint32 mainloopLevel;
    os_uint32 groupsLevel;
    os_uint32 writingLevel;
    os_uint32 readingLevel;
    os_uint32 testLevel;
    os_uint32 discoveryLevel;
    in_configTimestamps timestamps;
};

in_configTracing
in_configTracingNew(
    )
{
    in_configTracing _this;
    os_boolean success;

    _this = os_malloc(sizeof(OS_STRUCT(in_configTracing)));
    if(_this)
    {
        success = in_configTracingInit(_this);
        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }

    return _this;
}

os_boolean
in_configTracingInit(
    in_configTracing _this)
{
    assert(_this);

    _this->pathName = NULL;
    /* TODO should use default values as defined */
    _this->isEnabled = OS_TRUE;
    _this->outputFileName = NULL;
    _this->initLevel = 1;
    _this->configurationLevel = 1;
    _this->deinitLevel = 1;
    _this->mainloopLevel = 1;
    _this->groupsLevel = 1;
    _this->writingLevel = 1;
    _this->readingLevel = 1;
    _this->testLevel = 1;
    _this->discoveryLevel = 1;
    _this->timestamps = NULL;

    return OS_TRUE;
}

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

void
in_configTracingSetDefaultLevel(
    in_configTracing _this,
    os_uint32 level)
{
    assert(_this);

    _this->configurationLevel = level;
    _this->initLevel = level;
    _this->deinitLevel = level;
    _this->mainloopLevel = level;
    _this->groupsLevel = level;
    _this->writingLevel = level;
    _this->readingLevel = level;
    _this->testLevel = level;
    _this->discoveryLevel = level;
}

void
in_configTracingSetConfigurationLevel(
    in_configTracing _this,
    os_uint32 level)
{
    assert(_this);

    _this->configurationLevel = level;
}

void
in_configTracingSetInitLevel(
    in_configTracing _this,
    os_uint32 level)
{
    assert(_this);

    _this->initLevel = level;
}

void
in_configTracingSetDeinitLevel(
    in_configTracing _this,
    os_uint32 level)
{
    assert(_this);

    _this->deinitLevel = level;
}

void
in_configTracingSetMainloopLevel(
    in_configTracing _this,
    os_uint32 level)
{
    assert(_this);

    _this->mainloopLevel = level;
}

void
in_configTracingSetGroupsLevel(
    in_configTracing _this,
    os_uint32 level)
{
    assert(_this);

    _this->groupsLevel = level;
}

void
in_configTracingSetWritingLevel(
    in_configTracing _this,
    os_uint32 level)
{
    assert(_this);

    _this->writingLevel = level;
}

void
in_configTracingSetReadingLevel(
    in_configTracing _this,
    os_uint32 level)
{
    assert(_this);

    _this->readingLevel = level;
}

void
in_configTracingSetTestLevel(
    in_configTracing _this,
    os_uint32 level)
{
    assert(_this);

    _this->testLevel = level;
}

void
in_configTracingSetDiscoveryLevel(
    in_configTracing _this,
    os_uint32 level)
{
    assert(_this);

    _this->discoveryLevel = level;
}

void
in_configTracingSetOutputFile(
    in_configTracing _this,
    os_char* outputFileName)
{
    assert(_this);

    _this->outputFileName = outputFileName;
}

void
in_tracingSetEnabled(
    in_configTracing _this,
    os_boolean isEnabled)
{
    assert(_this);

    _this->isEnabled = isEnabled;

}
