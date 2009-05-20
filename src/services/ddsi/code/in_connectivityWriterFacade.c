/* Abstraction layer includes */
#include "os_heap.h"

/* collection includes */
#include "Coll_List.h"
#include "Coll_Compare.h"

/* DDSi includes */
#include "in_connectivityWriterFacade.h"
#include "in_connectivityEntityFacade.h"
#include "in_connectivityParticipantFacade.h"
#include "in_locator.h"
#include "in_report.h"
#include "in__ddsiSubscription.h"

OS_STRUCT(in_connectivityWriterFacade)
{
    OS_EXTENDS(in_connectivityEntityFacade);
    /* A set containing all matched peers, in_connectivityPeerReader objects */
    Coll_Set matchedReaders;
    /* A list containing a subset of all locators known by the matchedReaders.
     * This subset will ensure each reader will receive the data at least
     * once.
     */

    struct v_publicationInfo info;
    Coll_List locators;
    os_uint32 partitionCount;
    OS_STRUCT(in_ddsiSequenceNumber) sequenceNumber;
};

OS_CLASS(in_LocatorStoreElement);
OS_STRUCT(in_LocatorStoreElement)
{
    in_locator locator;
    Coll_Set readers;
};
#define in_LocatorStoreElement(elem) ((in_LocatorStoreElement)elem)


static in_result
in_connectivityWriterFacadeBuildLocatorList(
    in_connectivityWriterFacade _this);

static in_result
in_connectivityWriterFacade_addUniqueLocatorsToStore(
    Coll_Set* locatorStore,
    Coll_List* locators,
    in_connectivityPeerReader reader);

static int
in_connectivityWriterFacade_locatorIsLessThen(
    void *left,
    void *right);

static os_boolean
in_connectivityWriterFacadeInit(
    in_connectivityWriterFacade _this,
    struct v_publicationInfo *info,
    os_boolean hasKey,
    in_ddsiSequenceNumber seq,
    in_connectivityParticipantFacade  participant);

static void
in_connectivityWriterFacadeDeinit(
    in_object obj);

in_connectivityWriterFacade
in_connectivityWriterFacadeNew(
    struct v_publicationInfo *info,
    os_boolean hasKey,
    in_ddsiSequenceNumber seq,
    in_connectivityParticipantFacade  participant)
{
    os_boolean success;
    in_connectivityWriterFacade _this;

    _this = in_connectivityWriterFacade(os_malloc(sizeof(OS_STRUCT(in_connectivityWriterFacade))));

    if(_this)
    {
        success = in_connectivityWriterFacadeInit(_this,info,hasKey, seq, participant);

        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }
    return _this;
}

static os_boolean
in_connectivityWriterFacadeInit(
    in_connectivityWriterFacade _this,
    struct v_publicationInfo *info,
    os_boolean hasKey,
    in_ddsiSequenceNumber seq,
    in_connectivityParticipantFacade  participant)
{
    os_boolean success;
    OS_STRUCT(in_ddsiGuid) guid;
    in_ddsiGuidPrefixRef prefix;
    os_uint32 i;
    assert(_this);

    prefix =in_connectivityParticipantFacadeGetGuidPrefix(participant);

    memcpy(guid.guidPrefix,prefix,in_ddsiGuidPrefixLength);
    guid.entityId.entityKey[0] = info->key.localId & 0xFF;
    guid.entityId.entityKey[1] = (info->key.localId >> 8) & 0xFF;
    guid.entityId.entityKey[2] = (info->key.localId >> 16) & 0xFF;

    if(hasKey)
    {
        guid.entityId.entityKind = IN_ENTITYKIND_APPDEF_WRITER_WITH_KEY;
    } else
    {
        guid.entityId.entityKind = IN_ENTITYKIND_APPDEF_WRITER_NO_KEY;
    }

    success = in_connectivityEntityFacadeInit(
        OS_SUPER(_this),
        IN_OBJECT_KIND_WRITER_FACADE,
        in_connectivityWriterFacadeDeinit,
        &guid);

    if(success)
    {
        _this->info = *info;
        _this->info.type_name = os_strdup(info->type_name);
        _this->info.topic_name = os_strdup(info->topic_name);
        Coll_List_init(&_this->locators);
        Coll_Set_init(&_this->matchedReaders, pointerIsLessThen, TRUE);
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
in_connectivityWriterFacadeDeinit(
    in_object obj)
{
    os_uint32 i;
    in_connectivityPeerReader reader;
    Coll_Iter* iterator;
    in_connectivityWriterFacade _this;

    assert(in_connectivityWriterFacadeIsValid(obj));
    _this = in_connectivityWriterFacade(obj);

    for(i = Coll_List_getNrOfElements(&_this->locators); i > 0; i--)
    {
        in_locatorFree(in_locator(Coll_List_popBack(&_this->locators)));
    }
    iterator = Coll_Set_getFirstElement(&_this->matchedReaders);
    while(iterator)
    {
        reader = in_connectivityPeerReader(Coll_Iter_getObject(iterator));
        in_connectivityPeerReaderFree(reader);
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(&_this->matchedReaders, reader);
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

os_boolean
in_connectivityWriterFacadeMatchesPartition(
	in_connectivityWriterFacade _this,
	os_char* partition)
{
	os_uint32 i;
	os_boolean result = OS_FALSE;

	if(_this && partition)
	{
		for(i=0; i<_this->partitionCount && !result; i++)
		{
			if(strcmp(partition, _this->info.partition.name[i]) == 0)
			{
				result = OS_TRUE;
			}
		}
	}
	return result;
}

struct v_publicationInfo *
in_connectivityWriterFacadeGetInfo(
    in_connectivityWriterFacade _this)
{
    assert(in_connectivityWriterFacadeIsValid(_this));

    return &(_this->info);
}


Coll_List*
in_connectivityWriterFacadeGetLocators(
    in_connectivityWriterFacade _this)
{
    assert(in_connectivityWriterFacadeIsValid(_this));

    return &(_this->locators);
}

in_result
in_connectivityWriterFacadeAddMatchedPeer(
    in_connectivityWriterFacade _this,
    in_connectivityPeerReader peer)
{
    in_result result;
    os_uint32 collResult;

    assert(in_connectivityWriterFacadeIsValid(_this));
    assert(in_connectivityPeerReaderIsValid(peer));

    collResult = Coll_Set_add(&_this->matchedReaders, in_connectivityPeerReaderKeep(peer));
    IN_COLLRESULT_TO_RESULT(collResult, result);

    if(result == IN_RESULT_OK)
    {
        IN_TRACE(Send, 2, ">>> in_connectivityWriterFacadeAddMatchedPeer - entering build alg ");
        in_connectivityWriterFacadeBuildLocatorList(_this);
    } else
    {
        /* undo the keep! */
        in_connectivityPeerReaderFree(peer);
    }
    return result;
}

in_result
in_connectivityWriterFacadeRemoveMatchedPeer(
    in_connectivityWriterFacade _this,
    in_connectivityPeerReader peer)
{
    in_result result = IN_RESULT_OK;
    in_connectivityPeerReader removedReader;

    assert(in_connectivityWriterFacadeIsValid(_this));
    assert(in_connectivityPeerReaderIsValid(peer));

    removedReader = Coll_Set_remove(&_this->matchedReaders, peer);
    if(!removedReader)
    {
        result = IN_RESULT_NOT_FOUND;
    } else
    {
        /* undo the keep of the collection! */
        in_connectivityPeerReaderFree(peer);
    }

    return result;
}

Coll_Set*
in_connectivityWriterFacadeGetMatchedReaders(
    in_connectivityWriterFacade _this)
{
    assert(in_connectivityWriterFacadeIsValid(_this));

    return &_this->matchedReaders;
}

static in_result
in_connectivityWriterFacadeBuildLocatorList(
    in_connectivityWriterFacade _this)
{
    Coll_Iter* iterator;
    Coll_Set locatorStore;
    Coll_List* locators;
    in_result result = IN_RESULT_OK;
    os_uint32 collResult = COLL_OK;
    os_uint32 i;
    os_uint32 size;
    in_locator loc;

    assert(in_connectivityWriterFacadeIsValid(_this));

    /* Step 1: Clear the 'old' locators in the list, if any */
    size = Coll_List_getNrOfElements(&_this->locators);
    for(i = 0; i < size; i++)
    {
        loc = in_locator(Coll_List_popBack(&_this->locators));
        /* Must release the ref count of the stored locator! */
        in_locatorFree(loc);
    }
    assert(Coll_List_getNrOfElements(&_this->locators) == 0);

    /* Step 2: Init the temporary locator store */
    Coll_Set_init(&locatorStore, in_connectivityWriterFacade_locatorIsLessThen, 1);
    IN_TRACE_1(Send, 2, ">>> in_connectivityWriterFacadeBuildLocatorList - step 1 %d", Coll_Set_getNrOfElements(&_this->matchedReaders));
    /* Step 3: Fill the locator table with locator/readers pairs */
    iterator = Coll_Set_getFirstElement(&_this->matchedReaders);
    while(iterator && result == IN_RESULT_OK)
    {
        in_connectivityPeerReader reader;

        reader = in_connectivityPeerReader(Coll_Iter_getObject(iterator));

        locators = in_connectivityPeerReaderGetUnicastLocators(reader);
        IN_TRACE_1(Send, 2, ">>> in_connectivityWriterFacadeBuildLocatorList - step 2 %d", Coll_List_getNrOfElements(locators));
        in_connectivityWriterFacade_addUniqueLocatorsToStore(
            &locatorStore,
            locators,
            reader);
        locators = in_connectivityPeerReaderGetMulticastLocators(reader);
        IN_TRACE_1(Send, 2, ">>> in_connectivityWriterFacadeBuildLocatorList - step 3 %d", Coll_List_getNrOfElements(locators));
        in_connectivityWriterFacade_addUniqueLocatorsToStore(
            &locatorStore,
            locators,
            reader);
        iterator = Coll_Iter_getNext(iterator);
    }
    /* Step 4: Now that we have a filled locator store, we can easily see
     * which locator has the most readers associated with it. This locator
     * will be added to the final list of locators and removed from the store.
     * The locator store will then be updated so that all covered readers are
     * removed from each locator/reader pair. If a locator has no more readers
     * associated with it, then that locator will also be removed from the
     * store. Then the process starts all over again.
     * When the store is empty, then we have found our set of locators which
     * reach all readers. This set does not neccesarily need to be the smallest
     * possible subset for all scenarios, but in most cases it will be.
     */

    while(Coll_Set_getNrOfElements(&locatorStore) > 0)
    {
        os_int32 nrOfReaders = 0;
        in_LocatorStoreElement bestLocatorStoreElement = NULL;
        in_LocatorStoreElement locatorStoreElement;

        /* Step 4.1 Now find the locator with the most readers attached */
        iterator = Coll_Set_getFirstElement(&locatorStore);
        while(iterator)
        {
            os_int32 readerElementsSize;

            locatorStoreElement = in_LocatorStoreElement(
                Coll_Iter_getObject(iterator));
            readerElementsSize = Coll_Set_getNrOfElements(
                &(locatorStoreElement->readers));
            if(readerElementsSize > nrOfReaders)
            {
                nrOfReaders = readerElementsSize;

                bestLocatorStoreElement = locatorStoreElement;
            }
            iterator = Coll_Iter_getNext(iterator);
        }
        assert(bestLocatorStoreElement);
        /* Step 4.2:  Remove that locator from the store and add it to the list
         * of 'best' locators
         */
        Coll_Set_remove(&locatorStore, bestLocatorStoreElement);
        collResult = Coll_List_pushBack(
            &_this->locators,
            in_locatorKeep(bestLocatorStoreElement->locator));
/*TODO check result*/
        /* Step 4.3: Now remove the readers we have covered by the previously
         * found locator from the remaining locators. If a locator now has no
         * more readers attached, then remove that locator from the store.
         */
        iterator = Coll_Set_getFirstElement(&locatorStore);
        while(iterator)
        {
            os_boolean empty = FALSE;
            Coll_Iter* readerIterator;
            void* reader;

            locatorStoreElement = in_LocatorStoreElement(
                Coll_Iter_getObject(iterator));
            readerIterator = Coll_Set_getFirstElement(
                (&bestLocatorStoreElement->readers));
            while(readerIterator)
            {
                reader = Coll_Iter_getObject(readerIterator);
                readerIterator = Coll_Iter_getNext(readerIterator);
                /* wont remove anything if the reader was not contained... */
                Coll_Set_remove(&(locatorStoreElement->readers), reader);
                if(Coll_Set_getNrOfElements(&(locatorStoreElement->readers)) == 0)
                {
                    empty = TRUE;
                }
            }
            /* Increase iterator to the next element BEFORE we possibly
             * remove the current element
             */
            iterator = Coll_Iter_getNext(iterator);
            /* If the locator has no more readers associated with it, remove
             * the locator from the store
             */
            if(empty)
            {
                Coll_Set_remove(&locatorStore, locatorStoreElement);
                assert(Coll_Set_getNrOfElements(
                    &(locatorStoreElement->readers)) == 0);
                os_free(locatorStoreElement);
            }
        }
        os_free(bestLocatorStoreElement);
    }
    return result;
}

static in_result
in_connectivityWriterFacade_addUniqueLocatorsToStore(
    Coll_Set* locatorStore,
    Coll_List* locators,
    in_connectivityPeerReader reader)
{
    Coll_Iter* iterator;
    Coll_Iter* locatorElementIterator;
    in_locator locator;
    in_LocatorStoreElement locatorStoreElement;
    OS_STRUCT(in_LocatorStoreElement) locatorStoreElementFind;
    os_uint32 collResult = COLL_OK;
    in_result result = IN_RESULT_OK;

    assert(locatorStore);
    assert(locators);

    iterator = Coll_List_getFirstElement(locators);
    while(iterator)
    {
        locator = in_locator(Coll_Iter_getObject(iterator));
        locatorStoreElementFind.locator = locator;
        locatorElementIterator = Coll_Set_find(
            locatorStore,
            &locatorStoreElementFind);
        if(!locatorElementIterator)
        {
            locatorStoreElement = os_malloc(
                sizeof(OS_STRUCT(in_LocatorStoreElement)));
            locatorStoreElement->locator = locator;
            Coll_Set_init(
                &(locatorStoreElement->readers),
                pointerIsLessThen,
                0);

            collResult = Coll_Set_add(locatorStore, locatorStoreElement);
            if(collResult == COLL_ERROR_ALLOC)
            {
                result = IN_RESULT_OUT_OF_MEMORY;
            }
        } else
        {
            locatorStoreElement = in_LocatorStoreElement(
                Coll_Iter_getObject(locatorElementIterator));
        }
        if(result == IN_RESULT_OK)
        {
            collResult = Coll_Set_add(&(locatorStoreElement->readers), reader);
            if(collResult == COLL_ERROR_ALLOC)
            {
                result = IN_RESULT_OUT_OF_MEMORY;
            }/* ignore already existing error codes */
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    return result;
}

static int
in_connectivityWriterFacade_locatorIsLessThen(
    void *left,
    void *right)
{
    int lt;
    in_LocatorStoreElement locatorStoreElement1;
    in_LocatorStoreElement locatorStoreElement2;
    in_locator locator1;
    in_locator locator2;
    /*in_address locator1IP;
    in_address locator2IP;
    os_uint32 i;*/


    locatorStoreElement1 = in_LocatorStoreElement(left);
    locatorStoreElement2 = in_LocatorStoreElement(right);
    locator1 = locatorStoreElement1->locator;
    locator2 = locatorStoreElement2->locator;

    lt = (in_locatorCompare(locator1, locator2) == OS_LT);
    /* was before */
    /*
     *  lt = in_locatorGetKind(locator1) < in_locatorGetKind(locator2);
     * if(!lt)
     * {
     *   lt = in_locatorGetPort(locator1) < in_locatorGetPort(locator2);
     *   if(!lt)
     *   {
     *       locator1IP = in_locatorGetIp(locator1);
     *       locator2IP = in_locatorGetIp(locator2);
     *       for(i = 0; i < IP_ADDRESS_SIZE && !lt; i++)
     *       {
     *           lt = (*locator1IP)[i] < (*locator2IP)[i];
     *       }
     *   }
     * }
     * */

    return lt;
}

in_ddsiSequenceNumber
in_connectivityWriterFacadeGetSequenceNumber(
    in_connectivityWriterFacade _this)
{
    assert(in_connectivityWriterFacadeIsValid(_this));

    return &_this->sequenceNumber;
}

os_uint32
in_connectivityWriterFacadeGetPartitionCount(
	in_connectivityWriterFacade _this)
{
	assert(in_connectivityWriterFacadeIsValid(_this));

	return _this->partitionCount;
}

os_boolean
in_connectivityWriterFacadeMatchesPeerReader(
	in_connectivityWriterFacade _this,
	in_connectivityPeerReader reader)
{
	os_int32 i, j;
	os_boolean match;
	struct v_subscriptionInfo* rinfo;

	assert(in_connectivityWriterFacadeIsValid(_this));
	assert(in_connectivityPeerReaderIsValid(reader));

	if(_this && reader)
	{
		rinfo = &(in_connectivityPeerReaderGetInfo(reader)->topicData.info);

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

