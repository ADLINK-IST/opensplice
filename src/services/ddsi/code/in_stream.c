#include "in_stream.h"
#include "os_heap.h"

os_boolean
in_streamInit(
    in_stream _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_streamReader reader,
    in_streamWriter writer)
{
    os_boolean success;
    assert(_this);
    assert(in_streamReaderIsValid(reader));
    assert(in_streamWriterIsValid(writer));
    assert(kind > IN_OBJECT_KIND_INVALID);
    assert(kind < IN_OBJECT_KIND_COUNT);

    success = in_objectInit(in_object(_this), kind, deinit);

    if(success)
    {
        _this->reader = in_streamReaderKeep(reader);
        _this->writer = in_streamWriterKeep(writer);
    }
    return success;
}

void
in_streamDeinit(
    in_object obj)
{
    in_stream _this;

    assert(in_streamIsValid(obj));

    _this = in_stream(obj);
    in_objectFree(in_object(_this->reader));
    in_objectFree(in_object(_this->writer));
    in_objectDeinit(obj);
}

in_streamReader
in_streamGetReader(
    in_stream _this)
{
    assert(_this);

    return in_streamReaderKeep(_this->reader);
}

in_streamWriter
in_streamGetWriter(
    in_stream _this)
{
    assert(_this);

    return in_streamWriterKeep(_this->writer);
}

