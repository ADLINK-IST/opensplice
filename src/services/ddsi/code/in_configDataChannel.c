#include "in__configDataChannel.h"
#include "in__configChannel.h"
#include "os_heap.h"
#include "in__config.h"

static os_boolean
in_configDataChannelInit(
    in_configDataChannel _this,
    os_char* name,
    os_uint32 priority,
    os_boolean isDefault,
    os_boolean isEnabled,
    in_configDdsiService owningService);

OS_STRUCT(in_configDataChannel)
{
    OS_EXTENDS(in_configChannel);
    os_char* name;
    os_uint32 priority;
    os_boolean isDefault;
    os_uint32 groupQueueSize;
};

in_configDataChannel
in_configDataChannelNew(
    os_char* name,
    os_uint32 priority,
    os_boolean isDefault,
    os_boolean isEnabled,
    in_configDdsiService owningService)
{
    in_configDataChannel _this;
    os_boolean success;

    assert(name);

    _this = os_malloc(sizeof(OS_STRUCT(in_configDataChannel)));
    if(_this)
    {
        success = in_configDataChannelInit(
            _this,
            name,
            priority,
            isDefault,
            isEnabled,
            owningService);
        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }
    return _this;
}

os_boolean
in_configDataChannelInit(
    in_configDataChannel _this,
    os_char* name,
    os_uint32 priority,
    os_boolean isDefault,
    os_boolean isEnabled,
    in_configDdsiService owningService)
{
    os_boolean success;

    assert(_this);
    assert(name);
    success = in_configChannelInit(in_configChannel(_this), IN_CONFIG_CHANNEL_DATA, isEnabled, owningService);
    if(success)
    {
        _this->priority = priority;
        _this->isDefault = isDefault;
        _this->name = os_strdup(name);
        _this->groupQueueSize = 500;/* TODO should use default!! */
        if(!_this->name)
        {
            in_configChannelDeinit(in_configChannel(_this));
            success = OS_FALSE;
        }
    }

    return success;
}

void
in_configDataChannelSetGroupQueueSize(
    in_configDataChannel _this,
    os_uint32 queueSize)
{
    assert(_this);

    _this->groupQueueSize = queueSize;
}

os_char*
in_configDataChannelGetName(
    in_configDataChannel _this)
{
    assert(_this);

    return _this->name;
}

os_boolean
in_configDataChannelHasName(
    in_configDataChannel _this,
    const os_char *name)
{
	os_boolean result;
    assert(_this);

	result = strcmp(_this->name, name)==0 ? OS_TRUE : OS_FALSE;
	return result;
}

os_uint32
in_configDataChannelGetPriority(
    in_configDataChannel _this)
{
    assert(_this);

    return _this->priority;
}

os_boolean
in_configDataChannelGetIsDefault(
    in_configDataChannel _this)
{
    assert(_this);

    return _this->isDefault;
}


os_uint32
in_configDataChannelGetGroupQueueSize(
    in_configDataChannel _this)
{
    assert(_this);

    return _this->groupQueueSize;
}
