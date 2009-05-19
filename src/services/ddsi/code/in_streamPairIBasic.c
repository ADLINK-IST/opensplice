/* interfaces */
#include "in_streamPairIBasic.h"
#include "in_stream.h"
#include "in_ddsiStreamReaderImpl.h"
#include "in_ddsiStreamWriterImpl.h"
#include "in_transport.h"
#include "in_report.h"
#include "in__plugKernel.h"
/* implemenation */


static void
in_streamPairIBasicDeinit(
    in_object _this);

static os_boolean
in_streamPairIBasicInit(
    in_streamPairIBasic    _this,
    in_configChannel       channelConfig,
    in_transport           transport,
    in_plugKernel          plug);

OS_STRUCT(in_streamPairIBasic)
{
    OS_EXTENDS(in_stream);
};

in_streamPairIBasic
in_streamPairIBasicNew(
    const in_configChannel channelConfig,
    in_transport           transport,
    in_plugKernel          plug)
{
    in_streamPairIBasic _this;
    assert(channelConfig);

    _this = in_streamPairIBasic(os_malloc(OS_SIZEOF(in_streamPairIBasic)));

    if (_this)
    {
        os_boolean success;

        success = in_streamPairIBasicInit(_this,
                channelConfig,
                transport,
                plug);
        if (!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }
    IN_TRACE_1(Construction,2,"in_streamPairIBasic created = %x",_this);

    return _this;
}

void
in_streamPairIBasicFree(
    in_streamPairIBasic _this)
{
    assert(_this);

    in_objectFree(in_object(_this));
}

static os_boolean
in_streamPairIBasicInit(
    in_streamPairIBasic _this,
    in_configChannel channelConfig,
    in_transport           transport,
    in_plugKernel          plug)
{
    os_boolean result = OS_TRUE;
    in_ddsiStreamReaderImpl reader = NULL;
    in_ddsiStreamWriterImpl writer = NULL;

    /* managed objects: get-er increments the refcounter */
    in_transportReceiver receiver =
        in_transportGetReceiver(transport);
    in_transportSender sender =
        in_transportGetSender(transport);

    assert(_this);
    assert(channelConfig);
    assert(receiver);
    assert(sender);

    reader = in_ddsiStreamReaderImplNew(channelConfig,
            receiver, plug);

    writer = in_ddsiStreamWriterImplNew(channelConfig,
            sender, plug);

    if (!writer || !reader)
    {
        if (writer)
        {
        	in_ddsiStreamWriterImplFree(writer);
        }
        if (reader)
        {
        	in_ddsiStreamReaderImplFree(reader);
        }
        result = OS_FALSE;
    } else
    {
        result = in_streamInit(
            OS_SUPER(_this),
            IN_OBJECT_KIND_STREAM_PAIR_BASIC,
            in_streamPairIBasicDeinit,
            in_streamReader(reader),
            in_streamWriter(writer));

    	in_ddsiStreamReaderImplFree(reader);
    	in_ddsiStreamWriterImplFree(writer);
    }

    /* decrement refcount */
    in_transportSenderFree(sender);
    in_transportReceiverFree(receiver);

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


    in_streamDeinit(_this);
}

