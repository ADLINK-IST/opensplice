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

#include "d__dcpsPublicationListener.h"
#include "d__subscriber.h"
#include "d__admin.h"
#include "d__listener.h"
#include "d__durability.h"
#include "d_qos.h"
#include "d__waitset.h"
#include "d__misc.h"
#include "d__fellow.h"
#include "u_participant.h"
#include "u_subscriber.h"
#include "u_dataReader.h"
#include "u_observable.h"
#include "u_group.h"
#include "v_builtin.h"
#include "v_event.h"
#include "v_readerSample.h"
#include "v_message.h"
#include "v_kernel.h"
#include "v_observer.h"
#include "os_heap.h"
#include "os_report.h"
#include "v_dataReaderSample.h"

#define d_dcpsPublicationListener(_this) ((d_dcpsPublicationListener)(_this))
#define d_dcpsPublicationListenerIsValid(_this)   \
    d_listenerIsValidKind(d_listener(_this), D_DCPS_PUBLICATION_LISTENER)


/**
 * The dcpsPublicationListener listens to DCPSPublication messages.
 * The dcpsPublicationListener is used to learn about new group when
 * client durability is enabled
 */
C_STRUCT(d_dcpsPublicationListener){
    C_EXTENDS(d_listener);
    d_waitsetEntity waitsetData;
    u_dataReader dataReader;
    d_subscriber subscriber;
};

static u_actionResult processPublication(c_object object,c_voidp copyArg);

static c_ulong
d_dcpsPublicationListenerAction(
    u_object o,
    u_waitsetEvent event,
    c_voidp usrData)
{
    d_dcpsPublicationListener listener;
    u_result result;
    d_admin admin;
    d_durability durability;

    listener = d_dcpsPublicationListener(usrData);

    assert(d_dcpsPublicationListenerIsValid(listener));

    d_listenerLock(d_listener(listener));
    /* Read the latest DCPSPublication and process it */
    result = u_dataReaderTake(u_dataReader(o), U_STATE_ANY, processPublication, listener, OS_DURATION_ZERO);

    if (result != U_RESULT_OK && result != U_RESULT_NO_DATA) {
        admin = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);
        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, result,
                "Failed to read data from dcpsPublicationReader (result: %s)", u_resultImage(result));
        d_durabilityTerminate(durability, TRUE);
    }
    d_listenerUnlock(d_listener(listener));

    return event->kind;
}


static void
d_dcpsPublicationListenerDeinit(
    d_dcpsPublicationListener listener)
{
    assert(d_dcpsPublicationListenerIsValid(listener));

    /* Stop the listener */
    d_dcpsPublicationListenerStop(listener);

    if (listener->waitsetData) {
        d_waitsetEntityFree(listener->waitsetData);
        listener->waitsetData = NULL;
    }
    if (listener->dataReader) {
        u_objectFree(u_object(listener->dataReader));
        listener->dataReader = NULL;
    }
    listener->subscriber = NULL;
    /* Call super-deinit */
    d_listenerDeinit(d_listener(listener));

    return;
}

static c_bool
d_dcpsPublicationListenerInit(
    d_dcpsPublicationListener listener,
    d_subscriber subscriber)
{
    d_admin admin;
    d_durability durability;
    u_participant participant;
    c_bool result;
    v_readerQos readerQos;
    os_threadAttr attr;
    u_result ur;
    v_gid gid;
    c_value ps[1];

    assert(d_subscriberIsValid(subscriber));
    assert(listener);

    /* Call super-init */
    d_listenerInit(d_listener(listener), D_DCPS_PUBLICATION_LISTENER, subscriber, NULL,
                          (d_objectDeinitFunc)d_dcpsPublicationListenerDeinit);
    result = FALSE;
    admin = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    participant = u_participant(d_durabilityGetService(durability));
    if (participant != NULL) {
        listener->subscriber = subscriber;
        if (subscriber->builtinSubscriber != NULL) {
            /* Create a reader that listens to DCPSPublication
             * topics. The reader uses the builtin subscriber, and
             * only listens to topics that have a different systemId
             * than myself. */
            if ((readerQos = d_readerQosNew(V_DURABILITY_TRANSIENT, V_RELIABILITY_RELIABLE)) != NULL) {
                readerQos->history.v.kind = V_HISTORY_KEEPLAST;
                readerQos->history.v.depth = 1;
                gid = u_observableGid((u_observable)subscriber->builtinSubscriber);
                ps[0].kind = V_ULONG;
                ps[0].is.ULong = gid.systemId;
                listener->dataReader = u_subscriberCreateDataReader(
                    subscriber->builtinSubscriber,
                    "DCPSPublicationReader",
                    "select * from " V_PUBLICATIONINFO_NAME " where key.systemId <> %0",
                    ps, 1,
                    readerQos);
                if (listener->dataReader != NULL) {
                    ur = u_entityEnable(u_entity(listener->dataReader));

                    if (ur == U_RESULT_OK) {
                        os_threadAttrInit(&attr);
                        listener->waitsetData = d_waitsetEntityNew(
                                        "dcpsPublicationListener",
                                        u_object(listener->dataReader),
                                        d_dcpsPublicationListenerAction,
                                        V_EVENT_DATA_AVAILABLE,
                                        attr, listener);
                        assert(listener->waitsetData);
                        (void) u_dataReaderWaitForHistoricalData(listener->dataReader, OS_DURATION_ZERO);
                        result = TRUE;
                    } else {
                        u_objectFree(u_object(listener->dataReader));
                        listener->dataReader = NULL;
                    }
                }
                u_readerQosFree(readerQos);
            }
         }
     }
     return result;
}

d_dcpsPublicationListener
d_dcpsPublicationListenerNew(
    d_subscriber subscriber)
{
    d_dcpsPublicationListener listener;
    c_bool success;

    assert(d_subscriberIsValid(subscriber));

    listener = d_dcpsPublicationListener(os_malloc(C_SIZEOF(d_dcpsPublicationListener)));
    if (listener) {
        success = d_dcpsPublicationListenerInit(listener, subscriber);
        if (success != TRUE) {
            os_free(listener);
            listener = NULL;
        }
    }
    return listener;
}

void
d_dcpsPublicationListenerFree(
    d_dcpsPublicationListener listener)
{
    assert(d_dcpsPublicationListenerIsValid(listener));

    d_objectFree(d_object(listener));
}

c_bool
d_dcpsPublicationListenerStop(
    d_dcpsPublicationListener listener)
{
    c_bool result;
    d_admin admin;
    d_subscriber subscriber;
    d_waitset waitset;

    assert(d_dcpsPublicationListenerIsValid(listener));

    result = FALSE;

    if (listener) {
        d_listenerLock(d_listener(listener));

        if (d_listener(listener)->attached == TRUE) {
            admin = d_listenerGetAdmin(d_listener(listener));
            subscriber = d_adminGetSubscriber(admin);
            waitset = d_subscriberGetWaitset(subscriber);
            d_listenerUnlock(d_listener(listener));
            result = d_waitsetDetach(waitset, listener->waitsetData);
            d_listenerLock(d_listener(listener));

            if (result == TRUE) {
                d_listener(listener)->attached = FALSE;
            }
        } else {
            result = TRUE;
        }
        d_listenerUnlock(d_listener(listener));
    }
    return result;
}

c_bool
d_dcpsPublicationListenerStart(
    d_dcpsPublicationListener listener)
{
    c_bool result;
    d_waitset waitset;
    u_result ur;

    assert(d_dcpsPublicationListenerIsValid(listener));

    result = FALSE;

    if (listener) {
        d_listenerLock(d_listener(listener));

        if (d_listener(listener)->attached == FALSE) {
            waitset = d_subscriberGetWaitset(listener->subscriber);
            result = d_waitsetAttach(waitset, listener->waitsetData);

            if (result == TRUE) {
                /* A V_DATA_AVAILABLE event is only generated when new data
                 * arrives. It is NOT generated when historical data is inserted.
                 * In case this durability service receives historical
                 * DCPSPublication from another durability service that
                 * was started earlier, all these DCPSPublications are
                 * inserted in the reader at creation time. To read these
                 * we must explicitly trigger a take action. */
                ur = u_dataReaderTake(listener->dataReader, U_STATE_ANY, processPublication, listener, OS_DURATION_ZERO);

                if (ur != U_RESULT_OK && ur != U_RESULT_NO_DATA) {
                    OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, ur,
                            "Failed to read data from dcpsPublicationReader (result: %s)",
                            u_resultImage(ur));
                } else {
                    d_listener(listener)->attached = TRUE;
                    result = TRUE;
                }
            }
        } else {
            result = TRUE;
        }
        d_listenerUnlock(d_listener(listener));
    }
    return result;
}

static c_bool
create_local_group(
    d_durability durability,
    char *partition,
    char *topic,
    v_durabilityKind kind)
{
    u_group ugroup;
    c_bool result = TRUE;

    assert(d_durabilityIsValid(durability));

    OS_UNUSED_ARG(kind);

    ugroup = u_groupNew(
            u_participant(d_durabilityGetService(durability)),
            partition, topic, 10*OS_DURATION_MILLISECOND);
    if (ugroup) {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Group '%s.%s' created locally.\n",
                        partition, topic);
        /* FIXME: enable again after fix is in place
        u_objectFree(u_object(ugroup));*/
    } else {
        d_printTimedEvent(durability, D_LEVEL_WARNING,
            "Group '%s.%s' could NOT be created locally.\n",
            partition, topic);
    }
    return result;
}


static u_actionResult
processPublication(
    c_object object,
    c_voidp copyArg)
{
    d_dcpsPublicationListener listener;
    d_admin admin;
    d_durability durability;
    v_message message;
    v_publicationInfoTemplate template;
    const struct v_publicationInfo* publication;
    c_iter partitions;
    v_durabilityKind kind;
    d_durabilityKind dkind;
    char *topic;
    char *partition;
    d_group group;

    if (object != NULL) {
        listener = d_dcpsPublicationListener(copyArg);
        admin = d_listenerGetAdmin(d_listener(listener));
        durability = d_adminGetDurability(admin);
        message = v_dataReaderSampleTemplate(object)->message;
        template = (v_publicationInfoTemplate)message;

        if (template) {
            publication = &template->userData;
            d_printTimedEvent(durability, D_LEVEL_FINE,
                                      "Received DCPSPublication for writer (%u,%u,%u) (state=%lu)\n",
                                      publication->key.systemId,
                                      publication->key.localId,
                                      publication->key.serial,
                                      v_messageState(message));
            /* Create local groups for each partition/topic
             * combination that matches one or more aligner
             * namespaces. Ignore groups for partitions that
             * contain matching wildcards. Volatile groups can
             * also be ignored, because no the durability service
             * will not maintain any historical data for these
             * groups. */
            kind = publication->durability.kind;
            dkind = d_durabilityKindFromKernel(kind);
            topic = publication->topic_name;
            partitions = c_iterNew(NULL);
            if (!sequenceOfStringCopyOut(&partitions, publication->partition.name)) {
                OS_REPORT(OS_WARNING, "processPublication", 0,
                            "Unable to retrieve the list of partitions from DCPSPublication for writer (%u,%u,%u), skipping",
                                      publication->key.systemId,
                                      publication->key.localId,
                                      publication->key.serial);
                c_iterFree(partitions);
                return V_PROCEED;
            }
            while ((partition = c_iterTakeFirst(partitions)) != NULL) {
                if ((group = d_adminGetLocalGroup(admin, partition, topic, dkind)) != NULL) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                              "Group '%s.%s' already locally known, no need to create locally\n",
                              partition, topic);
                } else if (kind == V_DURABILITY_VOLATILE) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                              "Group '%s.%s' is volatile, no need to create locally\n",
                              partition, topic);
                } else if (!d_adminGroupInAligneeNS(admin, partition,topic)) {
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                              "Group '%s.%s' not in alignee namespace, no need to create locally\n",
                              partition,topic);
                } else {
                    d_printTimedEvent(durability, D_LEVEL_FINER,
                              "Creating group '%s.%s' with kind %d locally\n",
                              partition, topic, kind);
                    /* create a local group */
                    (void)create_local_group(durability, partition, topic, kind);
                }
                os_free(partition);
            } /* while */
            c_iterFree(partitions);
        }
    }
    return V_PROCEED;
}
