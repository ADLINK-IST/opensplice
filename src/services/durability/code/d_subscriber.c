/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "vortex_os.h"
#include "d__subscriber.h"
#include "d__subscriber.h"
#include "d__admin.h"
#include "d__durability.h"
#include "d__configuration.h"
#include "d__listener.h"
#include "d__nameSpace.h"
#include "d__waitset.h"
#include "d__misc.h"
#include "d__deleteDataListener.h"
#include "d__statusListener.h"
#include "d__sampleRequestListener.h"
#include "d__sampleChainListener.h"
#include "d__persistentDataListener.h"
#include "d__nameSpacesRequestListener.h"
#include "d__nameSpacesListener.h"
#include "d__groupsRequestListener.h"
#include "d__groupRemoteListener.h"
#include "d__groupLocalListener.h"
#include "d__readerListener.h"
#include "d__remoteReaderListener.h"
#include "d__historicalDataRequestListener.h"
#include "d__durabilityStateRequestListener.h"
#include "d__dcpsHeartbeatListener.h"
#include "d__dcpsPublicationListener.h"
#include "d__capabilityListener.h"
#include "d__conflictResolver.h"
#include "d_qos.h"
#include "d_store.h"
#include "d_store.h"
#include "u_dataReader.h"
#include "v_event.h"
#include "v_builtin.h"
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
    size_t length;
    c_ulong i, j;
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
            "Persistent partition expression is: '%s'\n", result);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINER,
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
             d_printTimedEvent(durability, D_LEVEL_FINEST,
              "Initial quality for nameSpace %d is %"PA_PRItime"\n",
              i, OS_TIMEW_PRINT(quality));
         } else {
              d_printTimedEvent(durability, D_LEVEL_FINEST,
              "Unable to get quality from persistent store for nameSpace %d.\n",
              i);
         }
     } else {
         d_printTimedEvent(durability, D_LEVEL_FINEST,
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

    assert(d_adminIsValid(admin));

    subscriber = NULL;
    /* Allocate subscriber object */
    subscriber = d_subscriber(os_malloc(C_SIZEOF(d_subscriber)));
    if (subscriber) {
        memset(subscriber, 0, (os_uint32)sizeof(C_STRUCT(d_subscriber)));
        /* Call super-init */
        d_objectInit(d_object(subscriber), D_SUBSCRIBER,
                     (d_objectDeinitFunc)d_subscriberDeinit);
        /* Initialize the subscriber */
        subscriber->admin = admin;
        durability        = d_adminGetDurability(admin);
        config            = d_durabilityGetConfiguration(durability);
        /* Create the subscriber for the durability protocol */
        subscriberQos     = d_subscriberQosNew(config->partitionName);
        if (subscriberQos == NULL) {
            goto err_allocSubscriberQos;
        }
        subscriber->subscriber = u_subscriberNew (u_participant(d_durabilityGetService(durability)),
                                                  config->subscriberName,
                                                  subscriberQos);
        d_subscriberQosFree(subscriberQos);
        if (subscriber->subscriber == NULL) {
            goto err_allocSubscriber;
        }

        if(u_entityEnable(u_entity(subscriber->subscriber)) != U_RESULT_OK) {
            goto err_allocSubscriber;
        }

        /* Create the subscriber for client durability, but only
         * if client durability is enabled. */
        if (config->clientDurabilityEnabled) {
            subscriberQos         = d_subscriberQosNew(config->clientDurabilityPartitionName);
            if (subscriberQos == NULL) {
                goto err_allocSubscriberQos2;
            }
            subscriber->subscriber2 = u_subscriberNew(u_participant(d_durabilityGetService(durability)),
                                                  D_CLIENT_DURABILITY_SUBSCRIBER_NAME,
                                                  subscriberQos);
            d_subscriberQosFree(subscriberQos);
            if (subscriber->subscriber2 == NULL) {
                goto err_allocSubscriber2;
            }
            if(u_entityEnable(u_entity(subscriber->subscriber2)) != U_RESULT_OK) {
                goto err_allocSubscriber2;
            }
        } else {
            subscriber->subscriber2 = NULL;
        }
        /* Create the builtin subscriber for DCPSSubscriptions, DCPSHeartbeats and
         * DCPSPublications. */
        subscriberQos         = d_subscriberQosNew(V_BUILTIN_PARTITION);
        if (subscriberQos == NULL) {
            goto err_allocSubscriberQos3;
        }
        subscriber->builtinSubscriber = u_subscriberNew(u_participant(d_durabilityGetService(durability)),
                                              D_DURABILITY_BUILTIN_SUBSCRIBER_NAME,
                                              subscriberQos);
        d_subscriberQosFree(subscriberQos);
        if (subscriber->builtinSubscriber == NULL) {
            goto err_builtinSubscriber;
        }
        if(u_entityEnable(u_entity(subscriber->builtinSubscriber)) != U_RESULT_OK) {
            goto err_builtinSubscriber;
        }
        /* Create the waitset */
        subscriber->waitset = d_waitsetNew(subscriber);
        if (subscriber->waitset == NULL) {
            goto err_allocWaitset;
        }
        /* Open the persistent store (if any) */
        subscriber->persistentStore = d_storeOpen(durability, config->persistentStoreMode);
        if (subscriber->persistentStore) {
            partitionExpr     = getPersistentPartitionExpression(admin, durability);
            psubscriberQos    = d_subscriberQosNew(partitionExpr);
            if (psubscriberQos == NULL) {
                os_free(partitionExpr);
                partitionExpr = NULL;
                goto err_allocPSubscriberQos;
            }
            os_free(partitionExpr);
            partitionExpr = NULL;
            if (psubscriberQos->partition.v) {
                /* Collect nameSpaces from admin. */
                nameSpaces = d_adminNameSpaceCollect(admin);
                /* Loop nameSpaces */
                while ((nameSpace = c_iterTakeFirst(nameSpaces))) {
                    dkind = d_nameSpaceGetDurabilityKind(nameSpace);
                    /* Walk only over persistent nameSpaces */
                    if ((dkind == D_DURABILITY_PERSISTENT) || (dkind == D_DURABILITY_ALL)) {
                        /* If persistent nameSpace is not complete, restore backup */
                        result = d_storeNsIsComplete (subscriber->persistentStore, nameSpace, &nsComplete);
                        if ( (result == D_STORE_RESULT_OK) && !nsComplete) {
                            /* Incomplete namespace, restore backup. */
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                 "Namespace '%s' is incomplete, trying to restore backup.\n",
                                d_nameSpaceGetName(nameSpace));
                            if (d_storeRestoreBackup (subscriber->persistentStore, nameSpace) != D_STORE_RESULT_OK) {
                                d_printTimedEvent(durability, D_LEVEL_WARNING,
                                    "Backup for namespace '%s' could not be restored as no complete backup did exist on disk. Marking namespace as incomplete and continuing.\n",
                                    d_nameSpaceGetName(nameSpace));
                                OS_REPORT(OS_WARNING, D_CONTEXT_DURABILITY, 0,
                                    "Backup for namespace '%s' could not be restored as no complete backup did exist on disk. Marking namespace as incomplete and continuing.\n",
                                    d_nameSpaceGetName (nameSpace));
                                /* If backup fails, mark master state for nameSpace !D_STATE_COMPLETE */
                                d_nameSpaceSetMasterState (nameSpace, D_STATE_INIT);
                            }
                        }
                    }
                    d_nameSpaceFree(nameSpace);
                } /* while */
                /* Free nameSpaces iterator */
                assert(c_iterLength(nameSpaces) == 0);
                c_iterFree(nameSpaces);
                /* create the persistent subscriber */
                subscriber->persistentSubscriber = u_subscriberNew(u_participant(d_durabilityGetService(durability)),
                                                                   config->subscriberName,
                                                                   psubscriberQos);
                if (!subscriber->persistentSubscriber) {
                    goto err_allocPersistentSubscriber;
                }
                if(u_entityEnable(u_entity(subscriber->persistentSubscriber)) != U_RESULT_OK) {
                    goto err_allocPersistentSubscriber;
                }
            } else {
                subscriber->persistentSubscriber = NULL;
            }
            walkData.subscriber = subscriber;
            walkData.i = 0;
            d_adminNameSpaceWalk(admin, nsInitialQualityWalk, &walkData);
            d_subscriberQosFree(psubscriberQos);
        } else {
            subscriber->persistentSubscriber = NULL;
        }
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
        subscriber->remoteReaderListener      = NULL;
        subscriber->historicalDataRequestListener = NULL;
        subscriber->durabilityStateRequestListener = NULL;
        subscriber->dcpsHeartbeatListener     = NULL;
        subscriber->dcpsPublicationListener   = NULL;
        subscriber->capabilityListener        = NULL;
    }
    return subscriber;

err_allocPersistentSubscriber:
err_allocPSubscriberQos:
err_allocWaitset:
err_builtinSubscriber:
err_allocSubscriberQos3:
err_allocSubscriber2:
err_allocSubscriberQos2:
err_allocSubscriber:
err_allocSubscriberQos:
    d_subscriberFree(subscriber);
    return NULL;
}


void
d_subscriberDeinit(
    d_subscriber subscriber)
{
     d_durability durability;
     d_configuration config;

    assert(d_subscriberIsValid(subscriber));

    durability = d_adminGetDurability(subscriber->admin);
    assert(d_durabilityIsValid(durability));
    config = d_durabilityGetConfiguration(durability);
    assert(d_configurationIsValid(config));
    if (subscriber->persistentStore) {
        d_storeClose(subscriber->persistentStore);
        d_printTimedEvent(durability, D_LEVEL_FINEST, "persistent store closed\n");
        subscriber->persistentStore = NULL;
    }
    if (subscriber->waitset) {
        d_waitsetFree(subscriber->waitset);
        d_printTimedEvent(durability, D_LEVEL_FINEST, "waitset freed\n");
        subscriber->waitset = NULL;
    }
    if (subscriber->persistentSubscriber) {
        u_objectFree(u_object(subscriber->persistentSubscriber));
        d_printTimedEvent(durability, D_LEVEL_FINEST, "user persistent subscriber freed\n");
        subscriber->persistentSubscriber = NULL;
    }
    if (subscriber->subscriber2) {
        u_objectFree(u_object(subscriber->subscriber2));
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s freed\n", config->clientDurabilitySubscriberName);
        subscriber->subscriber2 = NULL;
    }
    if (subscriber->subscriber) {
        u_objectFree(u_object(subscriber->subscriber));
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s freed\n", config->subscriberName);
        subscriber->subscriber = NULL;
    }
    if (subscriber->builtinSubscriber) {
        u_objectFree(u_object(subscriber->builtinSubscriber));
        d_printTimedEvent(durability, D_LEVEL_FINEST, "%s freed\n", D_DURABILITY_BUILTIN_SUBSCRIBER_NAME);
        subscriber->builtinSubscriber = NULL;
    }
    /* Call super-deinit */
    d_objectDeinit(d_object(subscriber));
}


void
d_subscriberFree(
    d_subscriber subscriber)
{
    assert(d_subscriberIsValid(subscriber));

    d_objectFree(d_object(subscriber));
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

#define _INIT_LISTENER_(listener) \
        if (!subscriber->listener) { \
            subscriber->listener = d_##listener##New(subscriber); \
            assert(subscriber->listener); \
        }

#define _START_LISTENER_(listener) \
        if (subscriber->listener) { \
            if (!d_##listener##Start(subscriber->listener)) { \
                result = FALSE; \
                assert(0); \
            } \
        }

c_bool
d_subscriberStartListeners(
    d_subscriber subscriber)
{
    d_durability durability;
    c_bool result = TRUE;
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);
    if (subscriber) {
        durability = d_adminGetDurability(subscriber->admin);
        if (durability->configuration->capabilitySupport) {
            _INIT_LISTENER_(capabilityListener);
        }
        _INIT_LISTENER_(nameSpacesListener);
        _INIT_LISTENER_(nameSpacesRequestListener);
        _INIT_LISTENER_(statusListener);
        _INIT_LISTENER_(groupsRequestListener);
        if(subscriber->persistentSubscriber) {
            _INIT_LISTENER_(persistentDataListener);
        }
        _INIT_LISTENER_(deleteDataListener);
        _INIT_LISTENER_(groupRemoteListener);
        _INIT_LISTENER_(sampleRequestListener);
        _INIT_LISTENER_(sampleChainListener);
        _INIT_LISTENER_(groupLocalListener);
        _INIT_LISTENER_(dcpsHeartbeatListener);
        if (durability->configuration->waitForRemoteReaders) {
            _INIT_LISTENER_(remoteReaderListener);
        }
        if (durability->configuration->clientDurabilityEnabled) {
            _INIT_LISTENER_(dcpsPublicationListener);
            _INIT_LISTENER_(durabilityStateRequestListener);
            _INIT_LISTENER_(historicalDataRequestListener);
        }
        if (durability->splicedRunning == TRUE) {
            subscriber->admin->conflictResolver = d_conflictResolverNew(subscriber->admin);
            assert(subscriber->admin->conflictResolver);
            _START_LISTENER_(capabilityListener);
            _START_LISTENER_(groupsRequestListener);
            _START_LISTENER_(nameSpacesListener);
            _START_LISTENER_(nameSpacesRequestListener);
            _START_LISTENER_(statusListener);
            _START_LISTENER_(persistentDataListener);
            _START_LISTENER_(groupRemoteListener);
            _START_LISTENER_(sampleChainListener);
            _START_LISTENER_(sampleRequestListener);
            _START_LISTENER_(deleteDataListener);
            _START_LISTENER_(dcpsHeartbeatListener);
            _START_LISTENER_(remoteReaderListener);
            _START_LISTENER_(dcpsPublicationListener);
            _START_LISTENER_(durabilityStateRequestListener);
            _START_LISTENER_(historicalDataRequestListener);
        }
    }
    return result;
}

#define _STOP_LISTENER_(listener) \
        if (subscriber->listener) { \
            (void)d_##listener##Stop(subscriber->listener); \
        }

#define _FREE_LISTENER_(listener) \
        if (subscriber->listener) { \
            d_##listener##Free(subscriber->listener); \
            subscriber->listener = NULL; \
        }

void d_subscriberStopListeners(
    d_subscriber subscriber)
{
    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        _STOP_LISTENER_(capabilityListener);
        _STOP_LISTENER_(dcpsPublicationListener);
        _STOP_LISTENER_(durabilityStateRequestListener);
        _STOP_LISTENER_(historicalDataRequestListener);
        _STOP_LISTENER_(remoteReaderListener);
        _STOP_LISTENER_(deleteDataListener);
        _STOP_LISTENER_(sampleRequestListener);
        _STOP_LISTENER_(sampleChainListener);
        _STOP_LISTENER_(groupRemoteListener);
        _STOP_LISTENER_(statusListener);
        _STOP_LISTENER_(nameSpacesRequestListener);
        _STOP_LISTENER_(nameSpacesListener);
        _STOP_LISTENER_(groupsRequestListener);
        _STOP_LISTENER_(persistentDataListener);
        _STOP_LISTENER_(groupLocalListener);

        _FREE_LISTENER_(capabilityListener);
        _FREE_LISTENER_(dcpsPublicationListener);
        _FREE_LISTENER_(durabilityStateRequestListener);
        _FREE_LISTENER_(historicalDataRequestListener);
        _FREE_LISTENER_(remoteReaderListener);
        _FREE_LISTENER_(deleteDataListener);
        _FREE_LISTENER_(sampleRequestListener);
        _FREE_LISTENER_(sampleChainListener);
        _FREE_LISTENER_(groupRemoteListener);
        _FREE_LISTENER_(statusListener);
        _FREE_LISTENER_(nameSpacesRequestListener);
        _FREE_LISTENER_(nameSpacesListener);
        _FREE_LISTENER_(groupsRequestListener);
        _FREE_LISTENER_(persistentDataListener);
        _FREE_LISTENER_(groupLocalListener);
    }
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


d_historicalDataRequestListener
d_subscriberGetHistoricalDataRequestListener(
    d_subscriber subscriber)
{
    assert(d_subscriberIsValid(subscriber));

    return subscriber->historicalDataRequestListener;
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
