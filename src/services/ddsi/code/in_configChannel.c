#include "in__configChannel.h"
#include "in__config.h"
#include "in_report.h"
#include "in_ddsiDefinitions.h"

os_boolean
in_configChannelInit(
    in_configChannel _this,
    in_configChannelKind kind,
    os_boolean isEnabled,
    in_configDdsiService owningService)
{
    assert(_this);

    _this->kind = kind;
    _this->isEnabled = isEnabled;
    _this->owningService = owningService;
    if(kind == IN_CONFIG_CHANNEL_DISCOVERY)
    {
        _this->portNr = INCF_DEF_DISCOVERY_CHANNEL_PORT;
    } else
    {
        assert(kind == IN_CONFIG_CHANNEL_DATA);
        _this->portNr = INCF_DEF_DATA_CHANNEL_PORT;
    }
    _this->pathName = NULL;
    _this->fragmentSize = INCF_DEF_FRAGMENT_SIZE;
    /* receiving parameters */
    _this->receiveBufferSize = INCF_DEF_RECEIVE_BUFFER_SIZE;
    /* sending parameters */
    _this->differentiatedServicesField= INCF_DEF_DIFFERENTIATED_SERVICES_FIELD;

    return OS_TRUE;
}

void
in_configChannelDeinit(
    in_configChannel _this)
{
    /* no-op */
}

os_boolean
in_configChannelIsEnabled(
    in_configChannel _this)
{
    assert(_this);

    return _this->isEnabled;
}

in_configDdsiService
in_configChannelGetDdsiService(
    in_configChannel _this)
{
    assert(_this);

    return _this->owningService;
}

os_ushort
in_configChannelGetPortNr(
    in_configChannel _this)
{
    assert(_this);

    return _this->portNr;
}

void
in_configChannelSetPortNr(
    in_configChannel _this,
    os_ushort portNr)
{
    assert(_this);

    _this->portNr = portNr;
}

os_char*
in_configChannelGetPathName(
    in_configChannel _this)
{
    assert(_this);

    return _this->pathName;
}

os_uint32
in_configChannelGetFragmentSize(
    in_configChannel _this)
{
    assert(_this);

    return _this->fragmentSize;
}

in_configChannelKind
in_configChannelGetKind(
    in_configChannel _this)
{
    assert(_this);

    return _this->kind;
}

void
in_configChannelSetFragmentSize(
    in_configChannel _this,
    os_uint32 fragmentSize)
{
    _this->fragmentSize = fragmentSize;
}

os_boolean
in_configChannelSupportsControl(
    in_configChannel _this)
{
    assert(_this);

    /* TODO make configurable */
    return OS_FALSE;
}

/** shortcuts, mapping to in_configPartitioningGetGlobalPartitionAddress
 * TODO: add to design document */
os_char*
in_configChannelGetGlobalPartitionAddress(
		in_configChannel _this)
{
	os_char *result;
	in_configDdsiService configDdsi;
    in_configPartitioning configPartitioning;

    assert(_this);

    configDdsi = in_configChannelGetDdsiService(_this);
    assert(configDdsi); /* should always be present */
    configPartitioning = in_configDdsiServiceGetPartitioning(configDdsi);
    assert(configPartitioning); /* should always be present */
    result = in_configPartitioningGetGlobalPartitionAddress(configPartitioning);
    assert(result); /* should always be present */

	return result;
}

/** shortcuts, mapping to in_configDdsiServiceGetInterfaceId
 * TODO: add to design document */
os_char*
in_configChannelGetInterfaceId(
		in_configChannel _this)
{
	os_char *result;
	in_configDdsiService configDdsi;

    assert(_this);

    configDdsi = in_configChannelGetDdsiService(_this);
    assert(configDdsi);/* should always be present */
    result = in_configDdsiServiceGetInterfaceId(configDdsi);
    assert(result);/* should always be present */

	return result;
}

/** TODO: add to design document */
os_uint32
in_configChannelGetReceiveBufferSize(
		in_configChannel _this)
{
	assert(_this);

	return _this->receiveBufferSize;
}

/** TODO: add to design document */
os_uint
in_configChannelGetDifferentiatedServicesField(
		in_configChannel _this)
{
	assert(_this);

	return _this->differentiatedServicesField;
}

os_time
in_configChannelGetIOTimeout(
		in_configChannel _this)
{
    /* TODO can't add a default value here, wrong place!! */
	os_time result = { 0 /*secs*/, 50*1000*1000 /* 50 millis */};

    assert(_this);

	return result;
}
