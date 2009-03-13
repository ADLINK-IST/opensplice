/* Abstraction layer includes */
#include "os_heap.h"

/* collection includes */
#include "Coll_List.h"
#include "Coll_Compare.h"

/* DDSi includes */
#include "in_connectivityParticipantFacade.h"
#include "in_connectivityEntityFacade.h"

OS_STRUCT(in_connectivityParticipantFacade)
{
    OS_EXTENDS(in_connectivityEntityFacade);
    struct v_participantInfo info;
    Coll_Set matchedPeerParticipants;
};

static void
in_connectivityParticipantFacadeDeinit(
    in_object obj);

static os_boolean
in_connectivityParticipantFacadeInit(
    in_connectivityParticipantFacade _this,
    struct v_participantInfo *info);

in_connectivityParticipantFacade
in_connectivityParticipantFacadeNew(
    struct v_participantInfo *info)
{
    in_connectivityParticipantFacade _this;
    os_boolean success;

    _this = in_connectivityParticipantFacade(
            os_malloc(sizeof(OS_STRUCT(in_connectivityParticipantFacade))));

    if(_this)
    {
        success = in_connectivityParticipantFacadeInit(_this,info );

        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }

    return _this;
}

static os_boolean
in_connectivityParticipantFacadeInit(
    in_connectivityParticipantFacade _this,
    struct v_participantInfo *info)
{
    os_boolean success;
    assert(_this);

    success = in_connectivityEntityFacadeInit(
        OS_SUPER(_this),
        IN_OBJECT_KIND_PARTICIPANT_FACADE,
        in_connectivityParticipantFacadeDeinit);

    if(success)
    {
        _this->info = *info;
        Coll_Set_init(&_this->matchedPeerParticipants, pointerIsLessThen, TRUE);
    }
    return success;
}

static void
in_connectivityParticipantFacadeDeinit(
    in_object obj)
{
    in_connectivityPeerParticipant peerParticipant;
    in_connectivityParticipantFacade _this;
    Coll_Iter* iterator;

    assert(in_connectivityParticipantFacadeIsValid(obj));

    _this = (in_connectivityParticipantFacade)obj;
    iterator = Coll_Set_getFirstElement(&_this->matchedPeerParticipants);

    while(iterator)
    {
        peerParticipant = in_connectivityPeerParticipant(
                Coll_Iter_getObject(iterator));
        in_connectivityPeerParticipantFree(peerParticipant);
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(&_this->matchedPeerParticipants, peerParticipant);
    }
    /*Call parent deinit*/
    in_connectivityEntityFacadeDeinit(obj);
}

in_result
in_connectivityParticipantFacadeAddMatchedPeer(
    in_connectivityParticipantFacade _this,
    in_connectivityPeerParticipant peer)
{
    in_result result;
    os_uint32 collResult;

    assert(in_connectivityParticipantFacadeIsValid(_this));
    assert(in_connectivityPeerParticipantIsValid(peer));

    collResult = Coll_Set_add(&_this->matchedPeerParticipants,
            in_connectivityPeerParticipantKeep(peer));
    IN_COLLRESULT_TO_RESULT(collResult, result);

    if(result != IN_RESULT_OK)
    {
        /* undo the keep! */
        in_connectivityPeerParticipantFree(peer);
    }
    return result;
}

in_result
in_connectivityParticipantFacadeRemoveMatchedPeer(
    in_connectivityParticipantFacade _this,
    in_connectivityPeerParticipant peer)
{
    in_result result = IN_RESULT_OK;
    in_connectivityPeerParticipant removedPeer;

    assert(in_connectivityParticipantFacadeIsValid(_this));
    assert(in_connectivityPeerParticipantIsValid(peer));

    removedPeer = Coll_Set_remove(&_this->matchedPeerParticipants, peer);

    if(!removedPeer)
    {
        result = IN_RESULT_NOT_FOUND;
    } else
    {
        /* undo the keep of the collection! */
        in_connectivityPeerParticipantFree(peer);
    }

    return result;
}

struct v_participantInfo *
in_connectivityParticipantFacadeGetInfo(
    in_connectivityParticipantFacade _this)
{
    assert(in_connectivityParticipantFacadeIsValid(_this));

    return &_this->info;
}

Coll_Set*
in_connectivityParticipantFacadeGetMatchedPeerParticipants(
    in_connectivityParticipantFacade _this)
{
    assert(in_connectivityParticipantFacadeIsValid(_this));

    return &_this->matchedPeerParticipants;
}
