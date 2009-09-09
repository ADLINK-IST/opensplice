#include "in__configDdsiService.h"
#include "os_heap.h"

static os_boolean
in_configDdsiServiceInit(
    in_configDdsiService _this,
    os_char* name);

OS_STRUCT(in_configDdsiService)
{
    os_char* name;
    os_char* interfaceId;
    os_char* pathName;
    in_configPartitioning partitioning;
    in_configDebug debugging;
    in_configTracing tracing;
    in_configDiscoveryChannel discoveryChannel;
    Coll_List channels;
    os_boolean hasDefaultChannel;
};

in_configDdsiService
in_configDdsiServiceNew(
    os_char* name)
{
    in_configDdsiService _this;
    os_boolean success;

    assert(name);

    _this = os_malloc(sizeof(OS_STRUCT(in_configDdsiService)));
    if(_this)
    {
        success = in_configDdsiServiceInit(_this, name);
        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }

    return _this;
}

os_boolean
in_configDdsiServiceInit(
    in_configDdsiService _this,
    os_char* name)
{
    os_boolean success;

    assert(_this);
    assert(name);

    Coll_List_init(&(_this->channels));
    _this->name = os_strdup(name);
    if(!_this->name)
    {
        success = OS_FALSE;
    } else
    {
        success = OS_TRUE;
    }
    _this->interfaceId = NULL;
    _this->pathName = NULL;
    _this->partitioning = NULL;
    _this->debugging = NULL;
    _this->tracing = NULL;
    _this->discoveryChannel = NULL;
    _this->hasDefaultChannel = OS_FALSE;

    return success;
}

void
in_configDdsiServiceFree(
    in_configDdsiService _this)
{
    if (_this)
    {
        /* Free all resources for the in_configDdsiService */
        if (_this->tracing)
        {
            in_configTracingFree (_this->tracing);
        }
        if (_this->debugging) {
			in_configDebugFree(_this->debugging);
        }
        /* TODO : free rest of resources here */
    }
}

/* takes ownership of the discoverychannel */
void
in_configDdsiServiceSetDiscoveryChannel(
    in_configDdsiService _this,
    in_configDiscoveryChannel discoveryChannel)
{
    assert(_this);
    assert(discoveryChannel);

    _this->discoveryChannel = discoveryChannel;
}

void
in_configDdsiServiceSetTracing(
    in_configDdsiService _this,
    in_configTracing tracing)
{
    assert(_this);
    assert(tracing);

    _this->tracing = tracing;
}

void
in_configDdsiServiceAddDataChannelConfig(
    in_configDdsiService _this,
    in_configDataChannel channel)
{
    os_uint32 result;

    assert(_this);
    assert(channel);

    result = Coll_List_pushBack(&_this->channels, channel);
    if(result != COLL_ERROR_ALLOC)
    {
        /* TODO report out of memory error */
    }
}

void
in_configDdsiServiceSetNetworkId(
    in_configDdsiService _this,
    os_char* networkId)
{
    assert(_this);
    assert(networkId);

	/* free previous definition  */
    if (_this->interfaceId)
    {
    	os_free(_this->interfaceId);
    	_this->interfaceId = NULL;
    }
    _this->interfaceId = os_strdup(networkId);
    if(!_this->interfaceId)
    {
        /* TODO report out of memory error */
    }
}

os_boolean
in_configDdsiServiceHasDefaultChannel(
    in_configDdsiService _this)
{
    assert(_this);

    return _this->hasDefaultChannel;
}

void
in_configDdsiServiceSetHasDefaultChannel(
    in_configDdsiService _this,
    os_boolean hasDefaultChannel)
{
    assert(_this);

    _this->hasDefaultChannel = hasDefaultChannel;
}

os_char*
in_configDdsiServiceGetName(
    in_configDdsiService _this)
{
    assert(_this);

    return _this->name;
}

os_char*
in_configDdsiServiceGetPathName(
    in_configDdsiService _this)
{
    assert(_this);

    return _this->pathName;
}

os_char*
in_configDdsiServiceGetInterfaceId(
    in_configDdsiService _this)
{
    assert(_this);

    return _this->interfaceId;
}

/* List contains 'in_configDataChannel' object */
Coll_List*
in_configDdsiServiceGetChannels(
    in_configDdsiService _this)
{
    assert(_this);

    return &(_this->channels);
}


/* return first object matching the name */
in_configDataChannel
in_configDdsiServiceGetChannel(
    in_configDdsiService _this,
    const os_char *name)
{
	in_configDataChannel result = NULL;

	Coll_List *list = NULL;
	Coll_Iter *iter = NULL;
	assert(_this);

	list = &(_this->channels);

	if (list) {
		iter = Coll_List_getFirstElement(list);
		while(iter && !result) {
			in_configDataChannel channel =
				in_configDataChannel(Coll_Iter_getObject(iter));
			assert(channel!=NULL);
			if (in_configDataChannelHasName(channel, name)) {
				result = channel;
			}
			iter = Coll_Iter_getNext(iter); /* may be NULL */
		}
	}

	/* result may be NULL */
	return result;
}

in_configDiscoveryChannel
in_configDdsiServiceGetDiscoveryChannel(
    in_configDdsiService _this)
{
    assert(_this);

    return _this->discoveryChannel;
}

in_configTracing
in_configDdsiServiceGetTracing(
    in_configDdsiService _this)
{
    assert(_this);

    return _this->tracing;
}

in_configPartitioning
in_configDdsiServiceGetPartitioning(
    in_configDdsiService _this)
{
    assert(_this);

    return _this->partitioning;
}

in_configDebug
in_configDdsiServiceGetDebugging(
    in_configDdsiService _this)
{
    assert(_this);

    return _this->debugging;
}

void
in_configDdsiServiceSetDebugging(
    in_configDdsiService _this,
    in_configDebug config)
{
    assert(_this);
    assert(config);

    _this->debugging = config;
}

void
in_configDdsiServiceSetPartitioning(
    in_configDdsiService _this,
    in_configPartitioning partitioning)
{
    assert(_this);
    assert(partitioning);

    _this-> partitioning = partitioning;
}

