#include "in__configDiscoveryChannel.h"
#include "in__configChannel.h"
#include "os_heap.h"
#include "in__config.h"

static os_boolean
in_configDiscoveryChannelInit(
    in_configDiscoveryChannel _this,
    os_boolean isEnabled,
    in_configDdsiService owningService);

OS_STRUCT(in_configDiscoveryChannel)
{
    OS_EXTENDS(in_configChannel);
};

in_configDiscoveryChannel
in_configDiscoveryChannelNew(
    os_boolean isEnabled,
    in_configDdsiService owningService)
{
    in_configDiscoveryChannel _this;
    os_boolean success;

    _this = os_malloc(sizeof(OS_STRUCT(in_configDiscoveryChannel)));
    if(_this)
    {
        success = in_configDiscoveryChannelInit(_this, isEnabled, owningService);
        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }
    return _this;
}


os_boolean
in_configDiscoveryChannelInit(
    in_configDiscoveryChannel _this,
    os_boolean isEnabled,
    in_configDdsiService owningService)
{
    assert(_this);

    return in_configChannelInit(in_configChannel(_this), IN_CONFIG_CHANNEL_DISCOVERY, isEnabled, owningService);
}
