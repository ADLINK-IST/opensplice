#include "in_channel.h"
#include "os_heap.h"

os_boolean
in_channelReaderInit(
    in_channelReader _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    const os_char* name,
    const os_char* pathName,
    const in_runnableMainFunc runnableMainFunc,
    const in_runnableTriggerFunc triggerFunc,
    in_channel channel)
{
    os_boolean success;

    assert(_this);
    assert(kind < IN_OBJECT_KIND_COUNT);
    assert(kind > IN_OBJECT_KIND_INVALID);
    assert(deinit);
    assert(name);
    assert(runnableMainFunc);

    success = in_runnableInit(
        in_runnable(_this),
        kind,
        deinit,
        name,
        pathName,
        runnableMainFunc,
        triggerFunc);

    if(success)
    {
        _this->channel = in_objectRefFromObject(in_object(channel));
    }
    return success;
}

void
in_channelReaderDeinit(
    in_object _this)
{
    assert(in_channelReaderIsValid(_this));

    in_runnableDeinit(_this);
}

in_channel
in_channelReaderGetChannel(
    in_channelReader _this)
{
    assert(in_channelReaderIsValid(_this));

    return in_channelKeep(in_objectFromObjectRef(_this->channel));
}
