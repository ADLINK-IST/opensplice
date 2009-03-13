/* interfaces */
#include "in_streamPairIBasic.h"
#include "in__streamPair.h"

/* implemenation */
#ifdef STREAM_DUMMY
#include "in_streamReaderIBasic.h"
#include "in_streamWriterIBasic.h"
#endif
#include "os_heap.h"

#ifndef STREAM_DUMMY
#define in_streamReaderIBasic void*
#define in_streamReader_cast(o) ((void*)(o))
#define in_streamWriter_cast(o) ((void*)(o))
#define in_streamReaderIBasicNew(o) NULL
#define in_streamWriterIBasicNew(o) NULL
#define in_streamWriterIBasicFree(writer)
#define in_streamReaderIBasicFree(reader)
#define in_streamWriterIBasic void*
#endif

static in_streamReader
in_streamPairGetReaderImpl(
    in_streamPair _this);

static in_streamWriter
in_streamPairGetWriterImpl(
    in_streamPair _this);

static void
in_streamPairIBasicDeinit(
    in_object _this);

static os_boolean
in_streamPairIBasicInit(
    in_streamPairIBasic _this,
    in_configChannel channelConfig);

OS_STRUCT(in_streamPairIBasic)
{
    OS_EXTENDS(in_streamPair);
    in_streamReaderIBasic reader;
    in_streamWriterIBasic writer;
};

in_streamPairIBasic
in_streamPairIBasicNew(
    const in_configChannel channelConfig)
{
    in_streamPairIBasic _this;

    assert(channelConfig);

    _this = in_streamPairIBasic(os_malloc(OS_SIZEOF(in_streamPairIBasic)));

    if (_this)
    {
        os_boolean success;

        success = in_streamPairIBasicInit(_this, channelConfig);
        if (!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }

    return _this;
}

void
in_streamPairIBasicFree(
    in_streamPairIBasic _this)
{
    assert(_this);

    in_objectFree(in_object(_this));
}

os_boolean
in_streamPairIBasicInit(
    in_streamPairIBasic _this,
    in_configChannel channelConfig)
{
    os_boolean result = OS_TRUE;
    in_streamReaderIBasic reader = NULL;
    in_streamWriterIBasic writer = NULL;

    assert(_this);
    assert(channelConfig);

    reader = in_streamReaderIBasicNew(channelConfig);
    writer = in_streamWriterIBasicNew(channelConfig);
    if (!writer || !reader)
    {
        if (writer)
        {
            in_streamWriterIBasicFree(writer);
        }
        if (reader)
        {
            in_streamReaderIBasicFree(reader);
        }
        result = OS_FALSE;
    } else
    {
        result = in_streamPairInit(
            OS_SUPER(_this),
            IN_OBJECT_KIND_STREAM_PAIR_BASIC,
            in_streamPairIBasicDeinit,
            in_streamPairGetReaderImpl,
            in_streamPairGetWriterImpl);
        _this->reader = reader;
        _this->writer = writer;
    }

    return result;
}

void
in_streamPairIBasicDeinit(
    in_object _this)
{
    in_streamPairIBasic obj;

    assert(_this);

    /* narrow interface */
    obj = in_streamPairIBasic(_this);

    in_streamReaderIBasicFree(obj->reader);
    in_streamWriterIBasicFree(obj->writer);
    obj->reader = NULL;
    obj->writer = NULL;

    in_streamPairDeinit(OS_SUPER(obj));
}

in_streamReader
in_streamPairGetReaderImpl(
    in_streamPair _this)
{
    in_streamReader result;
    in_streamPairIBasic obj;

    assert(_this);

    /* narrow interface */
    obj = in_streamPairIBasic(_this);

    /* generalize */
    result = in_streamReader_cast(obj->reader);

    return result;
}

in_streamWriter
in_streamPairGetWriterImpl(
    in_streamPair _this)
{
    in_streamWriter result;
    in_streamPairIBasic obj;

    assert(_this);

    /* narrow interface */
    obj = in_streamPairIBasic(_this);

    /* generalize */
    result = in_streamWriter_cast(obj->writer);

    return result;
}
