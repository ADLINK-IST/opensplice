/* OS abstraction layer includes */
#include "os_heap.h"

/* DDSi includes */
#include "in__messageTransformer.h"

os_boolean
in_messageTransformerInit(
    in_messageTransformer _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_messageTransformerGetBufferFunc getBufferFunc,
    c_voidp getBufferFuncArg)
{
    os_boolean success;

    assert(_this);
    assert(kind < IN_OBJECT_KIND_COUNT);
    assert(kind > IN_OBJECT_KIND_INVALID);
    assert(deinit);
    assert(getBufferFunc);

    success = in_objectInit(in_object(_this), kind, deinit);

    if(success)
    {
        /* default to copy data instead of swap, runtime this will be
         * determined per CDR encapsulation
         */
        _this->copyKind = IN_MESSAGE_TRANSFORMER_CM_KIND_COPY;
        _this->curCdrIndex = 0;
        _this->cdrLength = 0;
        _this->getBufferFunc = getBufferFunc;
        _this->getBufferFuncArg = getBufferFuncArg;
        _this->fragmented = OS_FALSE;
    }
    return success;
}

void
in_messageTransformerDeinit(
    in_object _this)
{
    assert(_this);
    assert(in_messageTransformerIsValid(_this));

    in_objectDeinit(_this);
}

void
in_messageTransformerBegin(
        in_messageTransformer _this)
{
    assert(_this);
    assert(in_messageTransformerIsValid(_this));

    _this->curCdrIndex = 0;
}

void
in_messageTransformerSetLength(
    in_messageTransformer _this,
    os_uint32 length)
{
    assert(_this);
    assert(in_messageTransformerIsValid(_this));

    _this->length = length;
}
void
in_messageTransformerSetBuffer(
    in_messageTransformer _this,
    in_data* buffer)
{
    assert(_this);
    assert(in_messageTransformerIsValid(_this));

    _this->bufferPtr = *buffer;
}

in_data*
in_messageTransformerGetBuffer(
        in_messageTransformer _this)
{
    assert(_this);
    assert(in_messageTransformerIsValid(_this));

    return &(_this->bufferPtr);
}

void
in_messageTransformerSetCopyKind(
    in_messageTransformer _this,
    in_messageTransformerCMKind kind)
{
    assert(_this);
    assert(in_messageTransformerIsValid(_this));

    _this->copyKind = kind;
}

void
in_messageTransformerSetCdrLength(
    in_messageTransformer _this,
    os_ushort length)
{
    assert(_this);
    assert(in_messageTransformerIsValid(_this));

    _this->cdrLength = length;
}
