#include "in_connectivityPeerParticipant.h"
#include "os_defs.h"
#include "os_heap.h"

OS_STRUCT(in_connectivityPeerParticipant)
{
    OS_EXTENDS(in_connectivityPeerEntity);
    struct v_participantInfo info;
};

/* ------------- private  ------------------- */
static os_boolean
in_connectivityPeerParticipantInit(
    in_connectivityPeerParticipant _this,
    struct v_participantInfo *info);

static void
in_connectivityPeerParticipantDeinit(
    in_object obj);

/* ------------- public ----------------------- */
in_connectivityPeerParticipant
in_connectivityPeerParticipantNew(
    struct v_participantInfo *info)
{
    in_connectivityPeerParticipant _this;
    os_boolean success;

    _this = in_connectivityPeerParticipant(os_malloc(sizeof(OS_STRUCT(in_connectivityPeerParticipant))));

    if(_this)
    {
        success = in_connectivityPeerParticipantInit(_this, info);

        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }
    return _this;
}

os_boolean
in_connectivityPeerParticipantInit(
    in_connectivityPeerParticipant _this,
    struct v_participantInfo *info)
{
    os_boolean success;

    assert(_this);

    success = in_connectivityPeerEntityInit(
        OS_SUPER(_this),
        IN_OBJECT_KIND_PEER_PARTICIPANT,
        in_connectivityPeerParticipantDeinit);

    if(success)
    {
        _this->info = *info;
    }
    return success;

}

static void
in_connectivityPeerParticipantDeinit(
    in_object obj)
{
    assert(in_connectivityPeerParticipantIsValid(obj));
    in_connectivityPeerEntityDeinit(in_connectivityPeerEntity(obj));
}

in_result
in_connectivityPeerParticipantAddUnicastLocator(
    in_connectivityPeerParticipant _this,
    in_locator locator)
{
    assert(in_connectivityPeerParticipantIsValid(_this));

    return  in_connectivityPeerEntityAddUnicastLocator( in_connectivityPeerEntity(_this), locator);
}

in_result
in_connectivityPeerParticipantAddMulticastLocator(
    in_connectivityPeerParticipant _this,
    in_locator locator)
{
    assert(in_connectivityPeerParticipantIsValid(_this));

    return in_connectivityPeerEntityAddMulticastLocator( in_connectivityPeerEntity(_this), locator);
}

Coll_List*
in_connectivityPeerParticipantGetUnicastLocators(
    in_connectivityPeerParticipant _this)
{
    assert(in_connectivityPeerParticipantIsValid(_this));

    return in_connectivityPeerEntityGetUnicastLocators( in_connectivityPeerEntity(_this));
}

Coll_List*
in_connectivityPeerParticipantGetMulticastLocators(
    in_connectivityPeerParticipant _this)
{
    assert(in_connectivityPeerParticipantIsValid(_this));

    return in_connectivityPeerEntityGetMulticastLocators( in_connectivityPeerEntity(_this));
}

struct v_participantInfo *
in_connectivityPeerParticipantGetInfo(
    in_connectivityPeerParticipant _this)
{
    assert(in_connectivityPeerParticipantIsValid(_this));

    return &_this->info;
}

