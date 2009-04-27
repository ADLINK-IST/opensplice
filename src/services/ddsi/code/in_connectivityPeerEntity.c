#include "os_defs.h"
#include "os_heap.h"

#include "in_connectivityPeerEntity.h"
#include "in_connectivityPeerReader.h"
#include "in_connectivityPeerWriter.h"
#include "in_connectivityPeerParticipant.h"
#include "in_report.h"

os_boolean
in_connectivityPeerEntityInit(
    in_connectivityPeerEntity _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit)
{
    os_boolean success;

    assert(_this);

    success = in_objectInit(
        in_object(_this),
        kind,
        deinit);

    return success;
}

void
in_connectivityPeerEntityDeinit(
    in_connectivityPeerEntity _this)
{
    assert(_this);

    in_objectDeinit(_this);
}


Coll_List*
in_connectivityPeerEntityGetUnicastLocators(
    in_connectivityPeerEntity _this)
{
    assert(_this);
    switch (in_objectGetKind(in_object(_this))) {
        case IN_OBJECT_KIND_PEER_READER:
            return in_connectivityPeerReaderGetUnicastLocators(in_connectivityPeerReader(_this));
        case IN_OBJECT_KIND_PEER_WRITER:
            return in_connectivityPeerWriterGetUnicastLocators(in_connectivityPeerWriter(_this));
        case IN_OBJECT_KIND_PEER_PARTICIPANT:
            return in_connectivityPeerParticipantGetUnicastLocators(in_connectivityPeerParticipant(_this));
        default:
            assert(FALSE); /* one of the above should be valid */
    }
    return NULL;
}

Coll_List*
in_connectivityPeerEntityGetMulticastLocators(
    in_connectivityPeerEntity _this)
{
    assert(_this);

    switch (in_objectGetKind(in_object(_this))) {
        case IN_OBJECT_KIND_PEER_READER:
            return in_connectivityPeerReaderGetMulticastLocators(in_connectivityPeerReader(_this));
        case IN_OBJECT_KIND_PEER_WRITER:
            return in_connectivityPeerWriterGetMulticastLocators(in_connectivityPeerWriter(_this));
        case IN_OBJECT_KIND_PEER_PARTICIPANT:
            return in_connectivityPeerParticipantGetMulticastLocators(in_connectivityPeerParticipant(_this));
        default:
            assert(TRUE); /* one of the above should be valid */
    }

    return NULL;
}
