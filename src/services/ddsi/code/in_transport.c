#include "in_transport.h"

os_boolean
in_transportInit(
    in_transport _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_transportReceiver receiver,
    in_transportSender sender)
{
    os_boolean success;
    assert(_this);
    assert(in_transportReceiverIsValid(receiver));
    assert(in_transportSenderIsValid(sender));
    assert(kind > IN_OBJECT_KIND_INVALID);
    assert(kind < IN_OBJECT_KIND_COUNT);

    success = in_objectInit(in_object(_this), kind, deinit);

    if(success)
    {
        _this->receiver = in_transportReceiverKeep(receiver);
        _this->sender = in_transportSenderKeep(sender);
    }
    return success;
}

void
in_transportDeinit(
    in_object obj)
{
    in_transport _this;

    assert(in_transportIsValid(obj));

    _this = in_transport(obj);
    in_objectFree(in_object(_this->receiver));
    in_objectFree(in_object(_this->sender));
    in_objectDeinit(obj);
}

in_transportReceiver
in_transportGetReceiver(
    in_transport _this)
{
    assert(_this);

    return in_transportReceiverKeep(_this->receiver);
}

in_transportSender
in_transportGetSender(
    in_transport _this)
{
    assert(_this);

    return in_transportSenderKeep(_this->sender);
}

