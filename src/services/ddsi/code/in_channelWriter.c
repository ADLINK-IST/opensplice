#include "in_channel.h"
#include "in_report.h"
#include "os_heap.h"

os_boolean
in_channelWriterInit(
    in_channelWriter _this,
    in_channel channel,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    const os_char* name,
    const os_char* pathName,
    const in_runnableMainFunc runnableMainFunc,
    const in_runnableTriggerFunc triggerFunc)
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

    IN_TRACE_1(Construction,2,"in_channelWriterInit success:%d",success);
    return success;
}

void
in_channelWriterDeinit(
    in_object _this)
{
    assert(_this);
    assert(in_channelWriterIsValid(_this));

    in_runnableDeinit(_this);
}

in_channel
in_channelWriterGetChannel(
    in_channelWriter _this)
{
    assert(in_channelWriterIsValid(_this));

    return in_channelKeep(in_objectFromObjectRef(_this->channel));
}
