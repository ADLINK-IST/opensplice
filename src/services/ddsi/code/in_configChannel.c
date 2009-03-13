#include "in__configChannel.h"
#include "in__config.h"

os_boolean
in_configChannelInit(
    in_configChannel _this,
    os_boolean isEnabled,
    in_configDdsiService owningService,
    os_ushort defaultPortNr)
{
    assert(_this);

    _this->isEnabled = isEnabled;
    _this->owningService = owningService;
    _this->portNr = defaultPortNr;
    _this->pathName = NULL;
    _this->fragmentSize = 0;

    /* receiving parameters */
    _this->receiveBufferSize =
    	INCF_DEF_RECEIVE_BUFFER_SIZE;

    /* sending parameters */
    _this->differentiatedServicesField=
    	INCF_DEF_DIFFERENTIATED_SERVICES_FIELD;

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

void
in_configChannelSetFragmentSize(
    in_configChannel _this,
    os_uint32 fragmentSize)
{
    assert(_this);

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
	os_char *result = NULL;

	in_configDdsiService configDdsi =
		in_configChannelGetDdsiService(_this);

	if (!configDdsi || !in_configDdsiServiceGetPartitioning(configDdsi)) {
		/* for some reason no global partition object defined */
		result = INCF_DEF_GLOBAL_PARTITON; /* const */
	} else {
		in_configPartitioning configPartitioning =
			in_configDdsiServiceGetPartitioning(configDdsi);
		result =
			in_configPartitioningGetGlobalPartitionAddress(configPartitioning);
	}

	return result;
}


/** shortcuts, mapping to in_configDdsiServiceGetInterfaceId
 * TODO: add to design document */
os_char*
in_configChannelGetInterfaceId(
		in_configChannel _this)
{
	os_char *result = NULL;

	in_configDdsiService configDdsi =
		in_configChannelGetDdsiService(_this);

	if (!configDdsi) {
		/* for some reason no global partition object defined */
		result = INCF_DEF_INTERFACE; /* const */
	} else {
		result =
			in_configDdsiServiceGetInterfaceId(configDdsi);

	}

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
	os_time result = { 0 /*secs*/, 50*1000*1000 /* 50 millis */};
	return result;
}
