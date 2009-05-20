#include "in_channel.h"
#include "os_heap.h"

os_boolean
in_channelInit(
    in_channel _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_stream stream,
    in_plugKernel plug,
    in_configChannel config)
{
    os_boolean success;

    assert(_this);
    assert(kind < IN_OBJECT_KIND_COUNT);
    assert(kind > IN_OBJECT_KIND_INVALID);
    assert(deinit);
    assert(stream);
    assert(plug);
    assert(config);

    success = in_objectInit(in_object(_this), kind, deinit);

    if(success)
    {
        _this->stream = in_streamKeep(stream);
        _this->plug = in_plugKernelKeep(plug);
        _this->config = config;
        _this->reader = NULL;
        _this->writer = NULL;
    }

    return success;
}

void
in_channelDeinit(
    in_object obj)
{
    in_channel _this = in_channel(obj);

    assert(_this);
    assert(in_channelIsValid(_this));

    if(_this->stream)
    {
        in_streamFree(_this->stream);
        _this->stream = NULL;
    }
    if(_this->writer)
    {
        in_channelWriterFree(_this->writer);
        _this->writer = NULL;
    }
    if(_this->reader)
    {
        in_channelReaderFree(_this->reader);
        _this->reader = NULL;
    }
    if(_this->plug)
    {
        in_plugKernelFree(_this->plug);
        _this->plug = NULL;
    }
}

in_configChannel
in_channelGetConfig(
    in_channel _this)
{
    assert(_this);

    return _this->config;
}

in_channelReader
in_channelGetReader(
    in_channel _this)
{
    assert(_this);

    return in_channelReaderKeep(_this->reader);
}

in_plugKernel
in_channelGetPlugKernel(
    in_channel _this)
{
    assert(_this);

    return in_plugKernelKeep(_this->plug);
}

in_channelWriter
in_channelGetWriter(
    in_channel _this)
{
    assert(_this);

    return in_channelWriterKeep(_this->writer);
}

void
in_channelSetReader(
    in_channel _this,
    in_channelReader reader)
{
    assert(_this);
    assert(reader);

    _this->reader = in_channelReaderKeep(reader);
}

void
in_channelSetWriter(
    in_channel _this,
    in_channelWriter writer)
{
    assert(_this);
    assert(writer);

    _this->writer = in_channelWriterKeep(writer);
}

in_stream
in_channelGetStream(
    in_channel _this)
{
    assert(_this);

    return in_streamKeep(_this->stream);
}

void
in_channelActivate(
    in_channel _this)
{
    assert(_this);

    in_runnableStart(in_runnable(_this->writer));
    in_runnableStart(in_runnable(_this->reader));
}

void
in_channelDeactivate(
    in_channel _this)
{
    assert(_this);

    in_runnableStop(in_runnable(_this->writer));
    in_runnableStop(in_runnable(_this->reader));
}
