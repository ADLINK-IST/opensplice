#include "os_defs.h"
#include "os_heap.h"

#include "in__ddsiParticipant.h"

#include "in_connectivityPeerParticipant.h"
#include "in_report.h"

OS_STRUCT(in_connectivityPeerParticipant)
{
    OS_EXTENDS(in_connectivityPeerEntity);
    in_ddsiDiscoveredParticipantData info;
    OS_STRUCT(in_ddsiSequenceNumber) lastWriterSN;
    OS_STRUCT(in_ddsiSequenceNumber) lastReaderSN;   
};

/* ------------- private  ------------------- */
static os_boolean
in_connectivityPeerParticipantInit(
    in_connectivityPeerParticipant _this,
    in_ddsiDiscoveredParticipantData info);

static void
in_connectivityPeerParticipantDeinit(
    in_object obj);

/* ------------- public ----------------------- */
in_connectivityPeerParticipant
in_connectivityPeerParticipantNew(
    in_ddsiDiscoveredParticipantData info)
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
    in_ddsiDiscoveredParticipantData info)
{
    os_boolean success;

    assert(_this);

    success = in_connectivityPeerEntityInit(
        OS_SUPER(_this),
        IN_OBJECT_KIND_PEER_PARTICIPANT,
        in_connectivityPeerParticipantDeinit);

    if(success)
    {
        _this->info = in_ddsiDiscoveredParticipantDataKeep(info);
        in_ddsiSequenceNumberInitNative(&_this->lastWriterSN,0); 
        in_ddsiSequenceNumberInitNative(&_this->lastReaderSN,0); 
    }
    return success;

}

static void
in_connectivityPeerParticipantDeinit(
    in_object obj)
{
    in_connectivityPeerParticipant _this;
    assert(in_connectivityPeerParticipantIsValid(obj));
    _this = in_connectivityPeerParticipant(obj);


    in_ddsiDiscoveredParticipantDataFree(_this->info);
    in_connectivityPeerEntityDeinit(in_connectivityPeerEntity(obj));
}

Coll_List*
in_connectivityPeerParticipantGetUnicastLocators(
    in_connectivityPeerParticipant _this)
{
    assert(in_connectivityPeerParticipantIsValid(_this));
    return &(_this->info->proxy.metatrafficUnicastLocatorList);
}

Coll_List*
in_connectivityPeerParticipantGetMulticastLocators(
    in_connectivityPeerParticipant _this)
{
    assert(in_connectivityPeerParticipantIsValid(_this));

    return &(_this->info->proxy.metatrafficMulticastLocatorList);
}

Coll_List*
in_connectivityPeerParticipantGetDefaultUnicastLocators(
    in_connectivityPeerParticipant _this)
{
    assert(in_connectivityPeerParticipantIsValid(_this));
    return &(_this->info->proxy.defaultUnicastLocatorList);
}

Coll_List*
in_connectivityPeerParticipantGetDefaultMulticastLocators(
    in_connectivityPeerParticipant _this)
{
    assert(in_connectivityPeerParticipantIsValid(_this));

    return &(_this->info->proxy.defaultMulticastLocatorList);
}

in_ddsiDiscoveredParticipantData
in_connectivityPeerParticipantGetInfo(
    in_connectivityPeerParticipant _this)
{
    assert(in_connectivityPeerParticipantIsValid(_this));

    return _this->info;
}

in_ddsiGuidPrefixRef
in_connectivityPeerParticipantGetGuidPrefix(
    in_connectivityPeerParticipant _this)
{
    assert(in_connectivityPeerParticipantIsValid(_this));

    return _this->info->proxy.guidPrefix;
}


in_ddsiSequenceNumber
in_connectivityPeerParticipantGetLastWriterSNRef(    
    in_connectivityPeerParticipant _this)
{
    assert(in_connectivityPeerParticipantIsValid(_this));

    return &_this->lastWriterSN;
}

in_ddsiSequenceNumber
in_connectivityPeerParticipantGetLastReaderSNRef(    
    in_connectivityPeerParticipant _this)
{
    assert(in_connectivityPeerParticipantIsValid(_this));

    return &_this->lastReaderSN;
}

void
in_connectivityPeerParticipantAddWriterSN(    
    in_connectivityPeerParticipant _this,
    in_ddsiSequenceNumber seq)
{
    assert(in_connectivityPeerParticipantIsValid(_this));
    assert(in_ddsiSequenceNumberIsValid(seq));
    
    if ( in_ddsiSequenceNumberCompare(seq, &_this->lastWriterSN) == C_GT ) {
      _this->lastWriterSN = *seq;
    }
}

void
in_connectivityPeerParticipantAddReaderSN(    
    in_connectivityPeerParticipant _this,
    in_ddsiSequenceNumber seq)
{
    assert(in_connectivityPeerParticipantIsValid(_this));
    assert(in_ddsiSequenceNumberIsValid(seq));
    
    if ( in_ddsiSequenceNumberCompare(seq, &_this->lastReaderSN) == C_GT ) {
      _this->lastReaderSN = *seq;
    }
}



