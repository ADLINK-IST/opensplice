/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include "os.h"
#include "d__subscriber.h"
#include "d_subscriber.h"
#include "d_admin.h"
#include "d_durability.h"
#include "d__durability.h"
#include "d_configuration.h"
#include "d_listener.h"
#include "d_nameSpace.h"
#include "d_waitset.h"
#include "d_groupsRequestListener.h"
#include "d_groupRemoteListener.h"
#include "d_groupLocalListener.h"
#include "d_statusListener.h"
#include "d_sampleRequestListener.h"
#include "d_sampleChainListener.h"
#include "d_nameSpacesRequestListener.h"
#include "d_nameSpacesListener.h"
#include "d_readerListener.h"
#include "d_persistentDataListener.h"
#include "d_deleteDataListener.h"
#include "d_qos.h"
#include "d_store.h"
#include "d_misc.h"
#include "d_store.h"
#include "u_dataReader.h"
#include "u_dispatcher.h"
#include "v_event.h"
#include "os_heap.h"
#include "os_report.h"

static c_char*
getPersistentPartitionExpression(
    d_admin admin,
    d_durability durability)
{
    c_char *result, *expr;
    d_nameSpace ns;
    d_durabilityKind dkind;
    c_ulong length;
    c_long i, j;
    c_iter nameSpaces;

    result = NULL;

    assert (admin);

    /* Collect namespaces */
    nameSpaces = d_adminNameSpaceCollect(admin);

    if(admin){
        length = 0;
        j = 0;

        for(i=0; i<c_iterLength(nameSpaces); i++){
            ns    = d_nameSpace(c_iterObject(nameSpaces, i));
            dkind = d_nameSpaceGetDurabilityKind(ns);

            if((dkind == D_DURABILITY_PERSISTENT) || (dkind == D_DURABILITY_ALL)){
                expr = d_nameSpaceGetPartitions(ns);
                if(j==0){
                    length += strlen(expr);
                } else {
                    length += strlen(expr) + 1; /*for the comma*/
                }
                os_free(expr);
                j++;
            }
        }

        if(length > 0){
            result = (c_char*)(os_malloc(length + 1));
            result[0] = '\0';
            j = 0;

            for(i=0; i<c_iterLength(nameSpaces); i++){
                ns    = d_nameSpace(c_iterObject(nameSpaces, i));
                dkind = d_nameSpaceGetDurabilityKind(ns);

                if((dkind == D_DURABILITY_PERSISTENT) || (dkind == D_DURABILITY_ALL)){
                    expr = d_nameSpaceGetPartitions(ns);

                    if(j != 0){
                        os_strcat(result, ",");
                    }
                    os_strcat(result, expr);
                    os_free(expr);
                    j++;
                }
            }
        }

        d_adminNameSpaceCollectFree(admin, nameSpaces);
    }

    if(result){
        d_printTimedEvent(durability, D_LEVEL_FINE,
            D_THREAD_PERISTENT_DATA_LISTENER,
            "Persistent partition expression is: '%s'\n", result);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINE,
            D_THREAD_PERISTENT_DATA_LISTENER,
            "Persistent partition expression is empty.\n");
    }
    return result;
}

struct initialQualityWalkData
{
    d_subscriber subscriber;
    unsigned int i;
};

static void
nsInitialQualityWalk(
    d_nameSpace ns,
    void* userData)
{
    d_durability    durability;
    d_admin         admin;
    d_subscriber    subscriber;
    d_quality       quality;
    d_storeResult   result;
    c_ulong         i;

    subscriber = d_subscriber(((struct initialQualityWalkData*)userData)->subscriber);
    admin = subscriber->admin;
    durability = d_adminGetDurability(admin);
    i = ((struct initialQualityWalkData*)userData)->i;

     if( (d_nameSpaceGetDurabilityKind(ns) == D_DURABILITY_PERSISTENT) ||
         (d_nameSpaceGetDurabilityKind(ns) == D_DURABILITY_ALL))
     {
         result = d_storeGetQuality(subscriber->persistentStore, ns, &quality);

         if(result == D_STORE_RESULT_OK){
             d_nameSpaceSetInitialQuality(ns, quality);
             d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN,
              "Initial quality for nameSpace %d is %d.%u.\n",
              i, quality.seconds, quality.nanoseconds);
         } else {
              d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN,
              "Unable to get quality from persistent store for nameSpace %d.\n",
              i);
         }
     } else {
         d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN,
              "nameSpace %d does not hold persistent data.\n", i);
     }
     ((struct initialQualityWalkData*)userData)->i++;
}

d_subscriber
d_subscriberNew(
    d_admin admin)
{
    d_subscriber    subscriber;
    d_durability    durability;
    d_configuration config;
    v_subscriberQos subscriberQos, psubscriberQos;
    c_char*         partitionExpr;
    struct initialQualityWalkData walkData;
    d_storeResult       result;
    d_nameSpace         nameSpace;
    c_iter              nameSpaces;
    c_bool              nsComplete;
    d_durabilityKind    dkind;

    subscriber = NULL;

    if(admin){
        subscriber        = d_subscriber(os_malloc(C_SIZEOF(d_subscriber)));
        d_objectInit(d_object(subscriber), D_SUBSCRIBER, d_subscriberDeinit);

        subscriber->admin = admin;
        durability        = d_adminGetDurability(admin);
        config            = d_durabilityGetConfiguration(durability);
        subscriberQos     = d_subscriberQosNew(config->partitionName);
        partitionExpr     = getPersistentPartitionExpression(admin, durability);
        psubscriberQos    = d_subscriberQosNew(partitionExpr);

        os_free(partitionExpr);

        subscriber->subscriber = u_subscriberNew (u_participant(d_durabilityGetService(durability)),
                                                  config->subscriberName,
                                                  subscriberQos,
                                                  TRUE);

        subscriber->waitset         = d_waitsetNew(subscriber, FALSE, FALSE);
        subscriber->persistentStore = d_storeOpen(durability, config->persistentStoreMode);

        if(subscriber->persistentStore) {
            if(psubscriberQos->partition){

                /* Collect nameSpaces from admin. */
                nameSpaces = d_adminNameSpaceCollect(admin);

                /* Loop nameSpaces */
                while((nameSpace = c_iterTakeFirst(nameSpaces))) {
                    dkind = d_nameSpaceGetDurabilityKind(nameSpace);

                    /* Walk only over persistent nameSpaces */
                    if((dkind == D_DURABILITY_PERSISTENT) || (dkind == D_DURABILITY_ALL)){

                        /* If persistent nameSpace is not complete, restore backup */
                        result = d_storeNsIsComplete (subscriber->persistentStore, nameSpace, &nsComplete);
                        if ( (result == D_STORE_RESULT_OK) && !nsComplete)
                        {
                            /* Incomplete namespace, restore backup. */
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                D_THREAD_GROUP_LOCAL_LISTENER,
                                "Namespace '%s' is incomplete, trying to restore backup.\n",
                                d_nameSpaceGetName(nameSpace));

                            if (d_storeRestoreBackup (subscriber->persistentStore, nameSpace) != D_STORE_RESULT_OK)
                            {
                                d_printTimedEvent(durability, D_LEVEL_WARNING,
                                    D_THREAD_GROUP_LOCAL_LISTENER,
                                    "Backup for namespace '%s' could not be restored as no complete backup did exist on disk. Marking namespace as incomplete and continuing.\n",
                                    d_nameSpaceGetName(nameSpace));

                                OS_REPORT_1(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                    "Backup for namespace '%s' could not be restored as no complete backup did exist on disk. Marking namespace as incomplete and continuing.\n",
                                    d_nameSpaceGetName (nameSpace));

                                /* If backup fails, mark master state for nameSpace !D_STATE_COMPLETE */
                                d_nameSpaceSetMasterState (nameSpace, D_STATE_INIT);
                            }
                        }
                    }
                    d_nameSpaceFree(nameSpace);
                }

                /* Free nameSpaces iterator */
                assert(c_iterLength(nameSpaces) == 0);
                c_iterFree(nameSpaces);

                subscriber->persistentSubscriber = u_subscriberNew(u_participant(d_durabilityGetService(durability)),
                                                                   config->subscriberName,
                                                                   psubscriberQos,
                                                                   TRUE);

                assert(subscriber->persistentSubscriber);
            } else {
                subscriber->persistentSubscriber = NULL;
            }

            walkData.subscriber = subscriber;
            walkData.i = 0;
            d_adminNameSpaceWalk(admin, nsInitialQualityWalk, &walkData);

        } else {
            subscriber->persistentSubscriber = NULL;
        }
        assert(subscriber->subscriber);

        if(subscriber->subscriber){
            subscriber->statusListener            = NULL;
            subscriber->groupLocalListener        = NULL;
            subscriber->groupRemoteListener       = NULL;
            subscriber->groupsRequestListener     = NULL;
            subscriber->sampleRequestListener     = NULL;
            subscriber->sampleChainListener       = NULL;
            subscriber->nameSpacesRequestListener = NULL;
            subscriber->nameSpacesListener        = NULL;
            subscriber->persistentDataListener    = NULL;
            subscriber->deleteDataListener        = NULL;
        } else {
            d_subscriberFree(subscriber);
            subscriber = NULL;
        }
        d_subscriberQosFree(subscriberQos);
        d_subscriberQosFree(psubscriberQos);
    }
    return subscriber;
}

void
d_subscriberDeinit(
    d_object object)
{
    d_subscriber subscriber;
    d_durability durability;

    assert(d_objectIsValid(object, D_SUBSCRIBER) == TRUE);

    if(object){
        subscriber = d_subscriber(object);
        durability = d_adminGetDurability(subscriber->admin);

        if(subscriber->statusListener){
            d_statusListenerFree(subscriber->statusListener);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "statusListener freed\n");
            subscriber->statusListener = NULL;
        }
        if(subscriber->groupLocalListener){
            if(subscriber->sampleChainListener){
                d_sampleChainListenerStop(subscriber->sampleChainListener);
            }
            d_groupLocalListenerFree(subscriber->groupLocalListener);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "groupLocalListener freed\n");
            subscriber->groupLocalListener = NULL;
        }
        if(subscriber->groupRemoteListener){
            d_groupRemoteListenerFree(subscriber->groupRemoteListener);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "groupRemoteListener freed\n");
            subscriber->groupRemoteListener = NULL;
        }
        if(subscriber->groupsRequestListener){
            d_groupsRequestListenerFree(subscriber->groupsRequestListener);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "groupsRequestListener freed\n");
            subscriber->groupsRequestListener = NULL;
        }
       if(subscriber->sampleRequestListener){
            d_sampleRequestListenerFree(subscriber->sampleRequestListener);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "sampleRequestListener freed\n");
            subscriber->sampleRequestListener = NULL;
        }
        if(subscriber->sampleChainListener){
            d_sampleChainListenerFree(subscriber->sampleChainListener);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "sampleChainListener freed\n");
            subscriber->sampleChainListener = NULL;
        }
        if(subscriber->nameSpacesRequestListener){
            d_nameSpacesRequestListenerFree(subscriber->nameSpacesRequestListener);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "nameSpacesRequestListener freed\n");
            subscriber->nameSpacesRequestListener = NULL;
        }
        if(subscriber->nameSpacesListener){
            d_nameSpacesListenerFree(subscriber->nameSpacesListener);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "nameSpacesListener freed\n");
            subscriber->nameSpacesListener = NULL;
        }
        if(subscriber->deleteDataListener){
            d_deleteDataListenerFree(subscriber->deleteDataListener);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "deleteDataListener freed\n");
            subscriber->deleteDataListener = NULL;
        }
        if(subscriber->persistentDataListener){
            d_persistentDataListenerFree(subscriber->persistentDataListener);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "persistentDataListener freed\n");
            subscriber->persistentDataListener = NULL;
        }
        if(subscriber->persistentStore){
            d_storeClose(subscriber->persistentStore);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "persistent store closed\n");
            subscriber->persistentStore = NULL;
        }
        if(subscriber->waitset) {
            d_waitsetFree(subscriber->waitset);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "waitset freed\n");
            subscriber->waitset = NULL;
        }
        if(subscriber->persistentSubscriber){
            u_subscriberFree(subscriber->persistentSubscriber);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "user persistent subscriber freed\n");
            subscriber->persistentSubscriber = NULL;
        }
        if(subscriber->subscriber){
            u_subscriberFree(subscriber->subscriber);
            d_printTimedEvent(durability, D_LEVEL_FINEST, D_THREAD_MAIN, "user subscriber freed\n");
            subscriber->subscriber = NULL;
        }
    }
}

void
d_subscriberFree(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        d_objectFree(d_object(subscriber), D_SUBSCRIBER);
    }
}


d_admin
d_subscriberGetAdmin(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);
    return subscriber->admin;
}

d_waitset
d_subscriberGetWaitset(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);
    return subscriber->waitset;
}

u_subscriber
d_subscriberGetSubscriber(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);
    return subscriber->subscriber;
}

u_subscriber
d_subscriberGetPersistentSubscriber(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);
    return subscriber->persistentSubscriber;
}

void
d_subscriberInitStatusListener(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        if(!subscriber->statusListener){
            subscriber->statusListener = d_statusListenerNew(subscriber);
            assert(subscriber->statusListener);
        }
    }
}

c_bool
d_subscriberSetStatusListenerEnabled(
    d_subscriber subscriber,
    c_bool enable)
{
    c_bool result = FALSE;
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        if(enable == TRUE){
            result = d_statusListenerStart(subscriber->statusListener);
        } else {
            result = d_statusListenerStop(subscriber->statusListener);
        }
    }
    return result;
}

void
d_subscriberInitGroupLocalListener(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(!subscriber->groupLocalListener){
        assert(subscriber->sampleChainListener);
        subscriber->groupLocalListener = d_groupLocalListenerNew(subscriber,
                                                subscriber->sampleChainListener);
        assert(subscriber->groupLocalListener);
    }
}

c_bool
d_subscriberSetGroupLocalListenerEnabled(
    d_subscriber subscriber,
    c_bool enable)
{
    c_bool result = FALSE;
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        if(enable == TRUE){
            result = d_groupLocalListenerStart(subscriber->groupLocalListener);
        } else {
            result = d_groupLocalListenerStop(subscriber->groupLocalListener);
        }
    }
    return result;
}

void
d_subscriberInitGroupRemoteListener(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(!subscriber->groupRemoteListener){
        subscriber->groupRemoteListener = d_groupRemoteListenerNew(subscriber);
        assert(subscriber->groupRemoteListener);
    }
}

c_bool
d_subscriberSetGroupRemoteListenerEnabled(
    d_subscriber subscriber,
    c_bool enable)
{
    c_bool result = FALSE;
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        if(enable == TRUE){
            result = d_groupRemoteListenerStart(subscriber->groupRemoteListener);
        } else {
            result = d_groupRemoteListenerStop(subscriber->groupRemoteListener);
        }
    }
    return result;
}

void
d_subscriberInitGroupsRequestListener(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(!subscriber->groupsRequestListener){
        subscriber->groupsRequestListener = d_groupsRequestListenerNew(subscriber);
        assert(subscriber->groupsRequestListener);
    }
}

c_bool
d_subscriberSetGroupsRequestListenerEnabled(
    d_subscriber subscriber,
    c_bool enable)
{
    c_bool result = FALSE;
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        if(enable == TRUE){
            result = d_groupsRequestListenerStart(subscriber->groupsRequestListener);
        } else {
            result = d_groupsRequestListenerStop(subscriber->groupsRequestListener);
        }
    }
    return result;
}

void
d_subscriberInitSampleRequestListener(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(!subscriber->sampleRequestListener){
        subscriber->sampleRequestListener = d_sampleRequestListenerNew(subscriber);
        assert(subscriber->sampleRequestListener);
    }
}

c_bool
d_subscriberSetSampleRequestListenerEnabled(
    d_subscriber subscriber,
    c_bool enable)
{
    c_bool result = FALSE;
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        if(enable == TRUE){
            result = d_sampleRequestListenerStart(subscriber->sampleRequestListener);
        } else {
            result = d_sampleRequestListenerStop(subscriber->sampleRequestListener);
        }
    }
    return result;
}

void
d_subscriberInitSampleChainListener(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(!subscriber->sampleChainListener){
        subscriber->sampleChainListener = d_sampleChainListenerNew(subscriber);
        assert(subscriber->sampleChainListener);
    }
}

c_bool
d_subscriberSetSampleChainListenerEnabled(
    d_subscriber subscriber,
    c_bool enable)
{
    c_bool result = FALSE;
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        if(enable == TRUE){
            result = d_sampleChainListenerStart(subscriber->sampleChainListener);
        } else {
            result = d_sampleChainListenerStop(subscriber->sampleChainListener);
        }
    }
    return result;
}

void
d_subscriberInitNameSpacesRequestListener(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(!subscriber->nameSpacesRequestListener){
        subscriber->nameSpacesRequestListener = d_nameSpacesRequestListenerNew(subscriber);
        assert(subscriber->nameSpacesRequestListener);
    }
}

c_bool
d_subscriberSetNameSpacesRequestListenerEnabled(
    d_subscriber subscriber,
    c_bool enable)
{
    c_bool result = FALSE;
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        if(enable == TRUE){
            result = d_nameSpacesRequestListenerStart(subscriber->nameSpacesRequestListener);
        } else {
            result = d_nameSpacesRequestListenerStop(subscriber->nameSpacesRequestListener);
        }
    }
    return result;
}

void
d_subscriberInitNameSpacesListener(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(!subscriber->nameSpacesListener){
        subscriber->nameSpacesListener = d_nameSpacesListenerNew(subscriber);
        assert(subscriber->nameSpacesListener);
    }
}

c_bool
d_subscriberSetNameSpacesListenerEnabled(
    d_subscriber subscriber,
    c_bool enable)
{
    c_bool result = FALSE;
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        if(enable == TRUE){
            result = d_nameSpacesListenerStart(subscriber->nameSpacesListener);
        } else {
            result = d_nameSpacesListenerStop(subscriber->nameSpacesListener);
        }
    }
    return result;
}

void
d_subscriberInitDeleteDataListener(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(!subscriber->deleteDataListener){
        subscriber->deleteDataListener = d_deleteDataListenerNew(subscriber);
        assert(subscriber->deleteDataListener);
    }
}

c_bool
d_subscriberSetDeleteDataListenerEnabled(
    d_subscriber subscriber,
    c_bool enable)
{
    c_bool result = FALSE;
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        if(enable == TRUE){
            result = d_deleteDataListenerStart(subscriber->deleteDataListener);
        } else {
            result = d_deleteDataListenerStop(subscriber->deleteDataListener);
        }
    }
    return result;
}

void
d_subscriberInitPersistentDataListener(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(!subscriber->persistentDataListener){
        if(subscriber->persistentSubscriber) {
            subscriber->persistentDataListener = d_persistentDataListenerNew(subscriber);
            assert(subscriber->persistentDataListener);
        }
    }
}

c_bool
d_subscriberSetPersistentDataListenerEnabled(
    d_subscriber subscriber,
    c_bool enable)
{
    c_bool result = FALSE;
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        if(subscriber->persistentDataListener) {
            if(enable == TRUE){
                result = d_persistentDataListenerStart(subscriber->persistentDataListener);
            } else {
                result = d_persistentDataListenerStop(subscriber->persistentDataListener);
            }
        } else {
            result = TRUE;
        }
    }
    return result;
}

d_store
d_subscriberGetPersistentStore(
    d_subscriber subscriber)
{
    d_store store;

    assert(subscriber);
    store = NULL;

    if(subscriber){
        store = subscriber->persistentStore;
    }
    return store;
}

d_groupLocalListener
d_subscriberGetGroupLocalListener(
    d_subscriber subscriber)
{
    d_groupLocalListener listener = NULL;

    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        listener = subscriber->groupLocalListener;
    }
    return listener;
}

d_sampleChainListener
d_subscriberGetSampleChainListener(
    d_subscriber subscriber)
{
    d_sampleChainListener listener = NULL;

    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        listener = subscriber->sampleChainListener;
    }
    return listener;
}

d_nameSpacesRequestListener
d_subscriberGetNameSpacesRequestListener(
    d_subscriber subscriber)
{
    d_nameSpacesRequestListener listener = NULL;

    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        listener = subscriber->nameSpacesRequestListener;
    }
    return listener;
}

c_bool
d_subscriberAreRemoteGroupsHandled(
    d_subscriber subscriber)
{
    c_bool result = FALSE;

    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        if(subscriber->groupRemoteListener) {
            result = d_groupRemoteListenerAreRemoteGroupsHandled(subscriber->groupRemoteListener);
        }
    }
    return result;
}
