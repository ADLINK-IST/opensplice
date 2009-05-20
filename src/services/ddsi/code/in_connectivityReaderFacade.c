/* Abstraction layer includes */
#include "os_heap.h"

/* collection includes */
#include "Coll_List.h"
#include "Coll_Compare.h"

/* DDSi includes */
#include "in_connectivityReaderFacade.h"
#include "in_connectivityEntityFacade.h"
#include "in_connectivityParticipantFacade.h"
#include "in__ddsiPublication.h"

static os_boolean
in_connectivityReaderFacadeInit(
    in_connectivityReaderFacade _this,
    struct v_subscriptionInfo *info,
    os_boolean hasKey,
    in_ddsiSequenceNumber seq,
    in_connectivityParticipantFacade  participant);

static void
in_connectivityReaderFacadeDeinit(
    in_object obj);

OS_STRUCT(in_connectivityReaderFacade)
{
    OS_EXTENDS(in_connectivityEntityFacade);
    /* A set containing all matched peers, in_connectivityPeerWriter objects */
    struct v_subscriptionInfo info;
    os_uint32 partitionCount;
    Coll_Set matchedWriters;
    OS_STRUCT(in_ddsiSequenceNumber) sequenceNumber;
};


in_connectivityReaderFacade
in_connectivityReaderFacadeNew(
    struct v_subscriptionInfo *info,
    os_boolean hasKey,
    in_ddsiSequenceNumber seq,
    in_connectivityParticipantFacade  participant)
{
    os_boolean success;
    in_connectivityReaderFacade _this;

    _this = in_connectivityReaderFacade(os_malloc(sizeof(OS_STRUCT(in_connectivityReaderFacade))));

    if(_this)
    {
        success = in_connectivityReaderFacadeInit(_this,info, hasKey, seq, participant);

        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }

    return _this;
}

os_boolean
in_connectivityReaderFacadeInit(
    in_connectivityReaderFacade _this,
    struct v_subscriptionInfo *info,
    os_boolean hasKey,
    in_ddsiSequenceNumber seq,
    in_connectivityParticipantFacade  participant)
{
    os_boolean success;
    OS_STRUCT(in_ddsiGuid) guid;
    os_uint32 i;
    in_ddsiGuidPrefixRef prefix;

    assert(_this);

    prefix =in_connectivityParticipantFacadeGetGuidPrefix(participant);

    memcpy(guid.guidPrefix,prefix,in_ddsiGuidPrefixLength);
    guid.entityId.entityKey[0] = info->key.localId & 0xFF;
    guid.entityId.entityKey[1] = (info->key.localId >> 8) & 0xFF;
    guid.entityId.entityKey[2] = (info->key.localId >> 16) & 0xFF;

    if(hasKey)
    {
        guid.entityId.entityKind = IN_ENTITYKIND_APPDEF_READER_WITH_KEY;
    } else
    {
        guid.entityId.entityKind = IN_ENTITYKIND_APPDEF_READER_NO_KEY;
    }

    success = in_connectivityEntityFacadeInit(
        OS_SUPER(_this),
        IN_OBJECT_KIND_READER_FACADE,
        in_connectivityReaderFacadeDeinit,
        &guid);

    if(success)
    {
        _this->info = *info;
        _this->info.type_name = os_strdup(info->type_name);
        _this->info.topic_name = os_strdup(info->topic_name);
        Coll_Set_init(&_this->matchedWriters, pointerIsLessThen, TRUE);
        _this->sequenceNumber = *seq;

        _this->partitionCount = (os_uint32)c_arraySize(info->partition.name);
		_this->info.partition.name = os_malloc(_this->partitionCount*
			sizeof(c_string));

		for(i=0; i<_this->partitionCount; i++)
		{
			_this->info.partition.name[i] = os_strdup(info->partition.name[i]);
		}
    }
    return success;
}

static void
in_connectivityReaderFacadeDeinit(
    in_object obj)
{
	os_uint32 i;
    in_connectivityPeerWriter writer;
    Coll_Iter* iterator;
    in_connectivityReaderFacade _this;

    assert(in_connectivityReaderFacadeIsValid(obj));
    _this = in_connectivityReaderFacade(obj);

    iterator = Coll_Set_getFirstElement(&_this->matchedWriters);
    while(iterator)
    {
        writer = in_connectivityPeerWriter(Coll_Iter_getObject(iterator));
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(&_this->matchedWriters, writer);
        in_connectivityPeerWriterFree(writer);
    }
    os_free(_this->info.type_name);
	os_free(_this->info.topic_name);

    for(i=0; i<_this->partitionCount; i++)
	{
		os_free(_this->info.partition.name[i]);
	}
	os_free(_this->info.partition.name);
    in_connectivityEntityFacadeDeinit(obj);
}

in_result
in_connectivityReaderFacadeAddMatchedPeer(
    in_connectivityReaderFacade _this,
    in_connectivityPeerWriter peer)
{
    in_result result;
    os_uint32 collResult;

    assert(in_connectivityReaderFacadeIsValid(_this));
    assert(in_connectivityPeerWriterIsValid(peer));

    collResult = Coll_Set_add(&_this->matchedWriters, in_connectivityPeerWriterKeep(peer));
    IN_COLLRESULT_TO_RESULT(collResult, result);

    if(result != IN_RESULT_OK)
    {
        /* undo the keep! */
        in_connectivityPeerWriterFree(peer);
    }
    return result;
}

in_result
in_connectivityReaderFacadeRemoveMatchedPeer(
    in_connectivityReaderFacade _this,
    in_connectivityPeerWriter peer)
{
    in_result result = IN_RESULT_OK;
    in_connectivityPeerWriter removedWriter;

    assert(in_connectivityReaderFacadeIsValid(_this));
    assert(in_connectivityPeerWriterIsValid(peer));

    removedWriter = Coll_Set_remove(&_this->matchedWriters, peer);

    if(!removedWriter)
    {
        result = IN_RESULT_NOT_FOUND;
    } else
    {
        /* undo the keep of the collection! */
        in_connectivityPeerWriterFree(peer);
    }

    return result;
}

struct v_subscriptionInfo *
in_connectivityReaderFacadeGetInfo(
    in_connectivityReaderFacade _this)
{
    assert(in_connectivityReaderFacadeIsValid(_this));

    return &_this->info;
}



Coll_Set*
in_connectivityReaderFacadeGetMatchedWriters(
    in_connectivityReaderFacade _this)
{
    assert(in_connectivityReaderFacadeIsValid(_this));

    return &_this->matchedWriters;
}

in_ddsiSequenceNumber
in_connectivityReaderFacadeGetSequenceNumber(
    in_connectivityReaderFacade _this)
{
    assert(in_connectivityReaderFacadeIsValid(_this));

    return &_this->sequenceNumber;
}

os_uint32
in_connectivityReaderFacadeGetPartitionCount(
	in_connectivityReaderFacade _this)
{
	assert(in_connectivityReaderFacadeIsValid(_this));

	return _this->partitionCount;
}

os_boolean
in_connectivityReaderFacadeMatchesPeerWriter(
	in_connectivityReaderFacade _this,
	in_connectivityPeerWriter writer)
{
	os_int32 i, j;
	os_boolean match;
	struct v_publicationInfo* rinfo;

	assert(in_connectivityReaderFacadeIsValid(_this));
	assert(in_connectivityPeerWriterIsValid(writer));

	if(_this && writer)
	{
		rinfo = &(in_connectivityPeerWriterGetInfo(writer)->topicData.info);

		if(strcmp(_this->info.topic_name, rinfo->topic_name) == C_EQ )
		{
			if((_this->partitionCount == 0) &&
				(c_arraySize(rinfo->partition.name) == 0))
			{
				match = OS_TRUE;
			} else
			{
				match = OS_FALSE;

				for(i=0; (i<(os_int32)_this->partitionCount) && !match; i++)
				{
					for(j=0; j<c_arraySize(rinfo->partition.name) && !match; j++)
					{
						if(strcmp(_this->info.partition.name[i],
							(c_string)(rinfo->partition.name[j])) == C_EQ)
						{
							match = OS_TRUE;
						}
					}
				}
			}
			if(match)
			{
				  /* TODO: QOS checks must be performed here */
			}
		} else
		{
			match = OS_FALSE;
		}
	} else
	{
		match = OS_FALSE;
	}
	return match;
}

