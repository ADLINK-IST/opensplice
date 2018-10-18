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

/* abstraction layer includes */
#include "vortex_os.h"
#include "os_report.h"
#include "os_version.h"
#include "os_gitrev.h"
#include "os_atomics.h"

/* User layer includes */
#include "u_user.h"
#include "u_group.h"
#include "u_nwbridge.h"
#include "u_service.h"

/* nwbridge service includes */
#include "nb__service.h"
#include "nb__topic.h"
#include "nb__configuration.h"
#include "nb__thread.h"
#include "nb__util.h"
#include "nb__log.h"

#include "v_service.h" /* for service state */
#include "v_builtin.h" /* for built-in partition and topic names */
#include "v_entity.h"
#include "v_event.h"
#include "v_kernel.h"
#include "v_partition.h"
#include "v_topic.h"
#include "v_state.h"
#include "v_dataReaderSample.h"
#include "v_groupSet.h"

#include "ut_collection.h"

/* FIXME: built-in topics for Subscriber and Publisher entities are specific to OpenSplice,
   and we therefore can't really rely on their presence. These two exist to "document" this */
static const c_bool requireContainingEntities = FALSE;

/* When toggled to 1, an exit of the process was requested, meaning all
 * networkings within the process must terminate. */
static int nb_proc_must_exit;

typedef u_result (*nb__serviceHandleTopicObjectFunc)(nb_service _this, nb_topicObject top) __nonnull_all__;

struct nb_serviceInterestTopic {
    const c_char                    *topicName;
    u_dataReader                     reader;
    u_dataReaderAction               action;
    c_iter                           toHandle;
    nb__serviceHandleTopicObjectFunc handleFunc;
    u_writer                         writer;
    c_ulong                          subscriptionsMatched;
    c_bool                           alwaysForward;
    ut_table                         handled;
};

#define NB_SERVICE_NUM_ENTITIES sizeof(struct nb_serviceInterestTopics)/sizeof(struct nb_serviceInterestTopic)



C_STRUCT(nb_service)
{
    C_EXTENDS(nb_object);
    os_char* name;
    u_service service;
    u_subscriber subscriber;
    u_publisher publisher;
    u_writer statusWriter;
    volatile int terminate;
    os_signalHandlerExitRequestHandle erh;
    os_signalHandlerExceptionHandle eh;
    nb_configuration config;
    char ** includes;
    char ** excludes;
    os_mutex configLock;
    u_waitset ws;
    nb_thread leaseRenewalThr;
    struct {
        u_topic serviceStatus;
    } topics;
    struct nb_serviceInterest {
        u_subscriber subscriber; /* Used to subscribe to __BUILT-IN PARTITION__ */
        u_publisher publisher;
        union {
            struct nb_serviceInterestTopics {
                struct nb_serviceInterestTopic DCPSTopic;
                struct nb_serviceInterestTopic DCPSParticipant;
                struct nb_serviceInterestTopic CMParticipant;
                struct nb_serviceInterestTopic CMPublisher;
                struct nb_serviceInterestTopic CMDataWriter;
                struct nb_serviceInterestTopic DCPSPublication;
                struct nb_serviceInterestTopic CMSubscriber;
                struct nb_serviceInterestTopic CMDataReader;
                struct nb_serviceInterestTopic DCPSSubscription;
            } named;
            struct nb_serviceInterestTopic array[NB_SERVICE_NUM_ENTITIES];
        } entities;
    } interest; /* Initialized by nb__serviceInterestInit */
};

#define NB_TOPIC_OBJECT_DCPS_SUBSCRIPTION_INDEX  0
#define NB_TOPIC_OBJECT_CM_READER_INDEX          1
#define NB_TOPIC_OBJECT_CM_SUBSCRIBER_INDEX      2
#define NB_TOPIC_OBJECT_DCPS_PUBLICATION_INDEX   3
#define NB_TOPIC_OBJECT_CM_WRITER_INDEX          4
#define NB_TOPIC_OBJECT_CM_PUBLISHER_INDEX       5
#define NB_TOPIC_OBJECT_DCPS_PARTICIPANT_INDEX   6
#define NB_TOPIC_OBJECT_CM_PARTICIPANT_INDEX     7
#define NB_TOPIC_OBJECT_DCPS_TOPIC_INDEX         8

#define NB_TOPIC_INDEX(top) top##_INDEX


/***************** PRIVATE FUNCTIONS *****************/
/******************** NB_SERVICE *********************/
static nb_service         nb__serviceNew(const os_char* uri,
                                         const os_char* serviceName)
                                    __nonnull((2))
                                    __attribute_malloc__;

static void               nb__serviceDeinit(nb_service _this) __nonnull_all__;

/* Utility functions */
static void               nb__serviceLoadMetaData(v_public vpublic,
                                                  c_voidp arg /* c_bool* */)
                                            __nonnull_all__;

static void               nb__serviceWatchSpliced(v_serviceStateKind state,
                                                  c_voidp userData)
                                            __nonnull_all__;

static os_result          nb__serviceOpenTracingFile(nb_service _this)
                                            __nonnull_all__;

static void*              nb__serviceUpdateLease(c_voidp args)
                                            __nonnull_all__;

static v_serviceStateKind nb__ServiceStateToKernelServiceStateKind(ServiceState state)
                                                            __attribute_const__;

static void               nb__serviceMain(nb_service _this) __nonnull_all__;

static void               nb__serviceCreateGroups(nb_service _this,
                                                  const char * const * partitions,
                                                  c_ulong len,
                                                  const char * topicName,
                                                  const v_builtinTopicKey *pubkey)
                                            __nonnull((1,4,5));

static u_result           nb__serviceHandleServiceInterestTopic(const char *type,
                                                                const struct nb_serviceInterestTopic *top,
                                                                const v_builtinTopicKey *key,
                                                                nb_topicObject toHandle)
                                                            __nonnull_all__;

/* nb__serviceHandleTopicObjectFunc signature wrapper */
static u_result           nb__serviceHandleDcpsTopicFunc(nb_service _this, nb_topicObject top) __nonnull_all__;
static u_result           nb__serviceHandleDcpsTopic(nb_service _this, nb_dcpsTopic top) __nonnull_all__;

/* nb__serviceHandleTopicObjectFunc signature wrapper */
static u_result           nb__serviceHandleDcpsParticipantFunc(nb_service _this, nb_topicObject top) __nonnull_all__;
static u_result           nb__serviceHandleDcpsParticipant(nb_service _this, nb_dcpsParticipant part) __nonnull_all__;

/* nb__serviceHandleTopicObjectFunc signature wrapper */
static u_result           nb__serviceHandleCmParticipantFunc(nb_service _this, nb_topicObject top) __nonnull_all__;
static u_result           nb__serviceHandleCmParticipant(nb_service _this, nb_cmParticipant cmp) __nonnull_all__;

/* nb__serviceHandleTopicObjectFunc signature wrapper */
static u_result           nb__serviceHandleDcpsPublicationFunc(nb_service _this, nb_topicObject top) __nonnull_all__;
static u_result           nb__serviceHandleDcpsPublication(nb_service _this, nb_dcpsPublication pub) __nonnull_all__;

/* nb__serviceHandleTopicObjectFunc signature wrapper */
static u_result           nb__serviceHandleCmWriterFunc(nb_service _this, nb_topicObject top) __nonnull_all__;
static u_result           nb__serviceHandleCmWriter(nb_service _this, nb_cmWriter toHandle) __nonnull_all__;

/* nb__serviceHandleTopicObjectFunc signature wrapper */
static u_result           nb__serviceHandleDcpsSubscriptionFunc(nb_service _this, nb_topicObject top) __nonnull_all__;
static u_result           nb__serviceHandleDcpsSubscription(nb_service _this, nb_dcpsSubscription sub) __nonnull_all__;

/* nb__serviceHandleTopicObjectFunc signature wrapper */
static u_result           nb__serviceHandleCmReaderFunc(nb_service _this, nb_topicObject top) __nonnull_all__;
static u_result           nb__serviceHandleCmReader(nb_service _this, nb_cmReader cmr) __nonnull_all__;

/* nb__serviceHandleTopicObjectFunc signature wrapper */
static u_result           nb__serviceHandleCmPublisherFunc(nb_service _this, nb_topicObject top) __nonnull_all__;
static u_result           nb__serviceHandleCmPublisher(nb_service _this, nb_cmPublisher cmp) __nonnull_all__;

/* nb__serviceHandleTopicObjectFunc signature wrapper */
static u_result           nb__serviceHandleCmSubscriberFunc(nb_service _this, nb_topicObject top) __nonnull_all__;
static u_result           nb__serviceHandleCmSubscriber(nb_service _this, nb_cmSubscriber cms) __nonnull_all__;

static void               nb__serviceHandleBuiltinTopics (nb_service _this) __nonnull_all__;

static c_bool             ServiceStatusCopyIn(
                                        c_type type,
                                        struct ServiceStatus *data,
                                        struct ServiceStatus *to)
                                    __nonnull_all__;

static u_topic            nb__serviceInterestFindTopic(u_participant participant,
                                                       const c_char * name)
                                                    __nonnull_all__;

static u_result           nb__serviceInterestTopicInit(
                                        struct nb_serviceInterestTopic *interest,
                                        u_participant participant,
                                        u_waitset waitset,
                                        u_publisher publisher,
                                        u_writerQos writerQos,
                                        u_subscriber subscriber,
                                        u_readerQos readerQos) __nonnull_all__;

static void               nb__serviceInterestTopicDeinit(
                                        struct nb_serviceInterestTopic *interest,
                                        u_waitset waitset) __nonnull_all__;

static os_result          nb__serviceInterestInit(struct nb_serviceInterest* interest,
                                                  u_participant participant,
                                                  u_waitset waitset) __nonnull_all__;

static os_result          nb__exceptionHandler(void *callingThreadContext,
                                               void * arg);

static v_result           nb__serviceWriterGetMatchedStatus(
                                        c_voidp info /* struct v_topicMatchInfo* */,
                                        c_voidp arg /* c_bool* */) __nonnull_all__;

static const char * const * nb__serviceIncludes(nb_service _this)
                                                    __nonnull_all__
                                                    __attribute_returns_nonnull__;

static const char * const * nb__serviceExcludes(nb_service _this)
                                                    __nonnull_all__
                                                    __attribute_returns_nonnull__;

static void nb__serviceInterestTopicObjectFreeFunc(void *o, void *arg);

/***************** END PRIVATE FUNCTIONS *****************/

static u_result
nb__serviceHandleServiceInterestTopic(
        const char *type,
        const struct nb_serviceInterestTopic *top,
        const v_builtinTopicKey *key,
        nb_topicObject toHandle)
{
    v_builtinTopicKey *k;
    v_state s;
    u_result uresult = U_RESULT_OK;
    void *removed;

    s = nb_topicObjectState(toHandle);
    k = (v_builtinTopicKey *) /* discard const */ key;

    if(v_stateTest(s, L_DISPOSED)) {
        /* It is not guaranteed that a builtin-topic stored in the collection
         * has been written yet. This means that potentially a dispose is
         * sent for a topic that was never written, but this shouldn't hurt. */
        if((removed = ut_remove((ut_collection)top->handled, k)) != NULL){
            NB_TRACE(("Removed %s %s={key="NB_KEYFMT"}\n",
                       nb_topicObjectName(toHandle), type,
                       v_gidSystemId(*k), v_gidLocalId(*k)));
            nb_objectFree(removed);

            /* Forward the dispose */
            uresult = nb_topicObjectWrite(top->writer, toHandle);
        }
    } else if(v_stateTest(s, L_WRITE)){
        if(ut_tableInsert(top->handled, k, toHandle)){
            NB_TRACE(("Stored %s %s={key="NB_KEYFMT"}\n",
                       nb_topicObjectName(toHandle), type,
                       v_gidSystemId(*k), v_gidLocalId(*k)));
            nb_objectKeep(toHandle);

            if (top->alwaysForward) {
                NB_TRACE(("Forwarding %s %s={key="NB_KEYFMT"}\n",
                          nb_topicObjectName(toHandle), type,
                          v_gidSystemId(*k), v_gidLocalId(*k)));
                uresult = nb_topicObjectWrite(top->writer, toHandle);
            }
        }
    }

    return uresult;
}


static u_result nb__serviceHandleDcpsTopicFunc(nb_service _this, nb_topicObject top) { return nb__serviceHandleDcpsTopic(_this, nb_dcpsTopic(top)); }

static u_result
nb__serviceHandleDcpsTopic(
        nb_service _this,
        nb_dcpsTopic toHandle)
{
    u_result ures;
    u_writer writer;
    const char * const partitions[] = {"*"};
    v_state s;

    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);
    nb_objectIsValidKind(toHandle, NB_TOPIC_OBJECT_DCPS_TOPIC);

    s = nb_topicObjectState(nb_topicObject(toHandle));
    writer = _this->interest.entities.named.DCPSTopic.writer;

    if(v_stateTest(s, L_VALIDDATA)){
        if(nb_match(partitions, 1, nb_dcpsTopicTopicName(toHandle), nb__serviceIncludes(_this), nb__serviceExcludes(_this))){
            /* Currently all DCPSTopics that potentially match a filter are forwarded,
             * so no need to store them in _this->interest.entities.named.DCPSTopic.handled
             * for now. */
            ures = nb_topicObjectWrite(writer, nb_topicObject(toHandle));
            NB_TRACE(("Written (%s) %s for topic '%s'\n",
                       u_resultImage(ures),
                       nb_topicObjectName(nb_topicObject(toHandle)),
                       nb_dcpsTopicTopicName(toHandle)));
            assert(ures == U_RESULT_OK);
        } else {
            NB_TRACE(("Ignored %s for topic '%s'\n",
                       nb_topicObjectName(nb_topicObject(toHandle)),
                       nb_dcpsTopicTopicName(toHandle)));

            ures = U_RESULT_OK;
        }
    } else {
        ures = nb_topicObjectWrite(writer, nb_topicObject(toHandle));
        assert(ures == U_RESULT_OK);
    }


    return ures;
}

static u_result nb__serviceHandleDcpsParticipantFunc(nb_service _this, nb_topicObject top) { return nb__serviceHandleDcpsParticipant(_this, nb_dcpsParticipant(top)); }

static u_result
nb__serviceHandleDcpsParticipant(
        nb_service _this,
        nb_dcpsParticipant toHandle)
{
    u_result ur;

    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);
    nb_objectIsValidKind(toHandle, NB_TOPIC_OBJECT_DCPS_PARTICIPANT);

    ur = nb__serviceHandleServiceInterestTopic(
            "nb_dcpsParticipant",
            &_this->interest.entities.named.DCPSParticipant,
            nb_dcpsParticipantKey(toHandle),
            nb_topicObject(toHandle));

    return ur;
}

static u_result nb__serviceHandleCmParticipantFunc(nb_service _this, nb_topicObject top) { return nb__serviceHandleCmParticipant(_this, nb_cmParticipant(top)); }

static u_result
nb__serviceHandleCmParticipant(
        nb_service _this,
        nb_cmParticipant toHandle)
{
    u_result ur;

    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);
    nb_objectIsValidKind(toHandle, NB_TOPIC_OBJECT_CM_PARTICIPANT);

    ur = nb__serviceHandleServiceInterestTopic(
            "nb_cmParticipant",
            &_this->interest.entities.named.CMParticipant,
            nb_cmParticipantKey(toHandle),
            nb_topicObject(toHandle));

    return ur;
}

static u_result nb__serviceHandleDcpsPublicationFunc(nb_service _this, nb_topicObject top) { return nb__serviceHandleDcpsPublication(_this, nb_dcpsPublication(top)); }

static u_result
nb__serviceHandleDcpsPublication(
        nb_service _this,
        nb_dcpsPublication pub)
{
    u_result ures;
    u_topic topic;
    const char * const *partitions;
    c_ulong len;
    void *entry;
    nb_dcpsParticipant part;
    nb_cmParticipant cmpart;
    nb_cmPublisher cmp;
    nb_cmWriter cmw;
    u_writer writer;
    const v_builtinTopicKey *key;
    const v_builtinTopicKey *pubkey;
    const char * topObjName;
    v_state s;
    void * removed;
    nb_dcpsPublication oldPub;

    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);
    nb_objectIsValidKind(pub, NB_TOPIC_OBJECT_DCPS_PUBLICATION);

    s = nb_topicObjectState(nb_topicObject(pub));
    pubkey = nb_dcpsPublicationKey(pub);
    topObjName = nb_topicObjectName(nb_topicObject(pub));

    if(v_stateTest(s, L_DISPOSED)){
        removed = ut_remove((ut_collection)_this->interest.entities.named.DCPSPublication.handled, (void *) /* remove const */ pubkey);
        if(removed){
            oldPub = nb_dcpsPublication(removed);
            NB_TRACE(("Removed %s nb_dcpsPublication={key="NB_KEYFMT"}\n",
                   topObjName, v_gidSystemId(*pubkey), v_gidLocalId(*pubkey)));
            if(nb_dcpsPublicationGetInterested(oldPub)){
               /* Forward the dispose */
               ures = nb_topicObjectWrite(_this->interest.entities.named.DCPSPublication.writer, nb_topicObject(pub));
               if(ures != U_RESULT_OK) NB_TRACE(("nb_topicObjectWrite returned %s for %s\n", u_resultImage(ures), nb_topicObjectName(nb_topicObject(pub))));
            }
            nb_objectFree(oldPub);
        }
        return U_RESULT_OK;
    } else if(v_stateTest(s, L_WRITE)){
        const char * topicName = nb_dcpsPublicationTopicName(pub);

        /* Set interest */
        nb_dcpsPublicationSetInterested(pub, nb__serviceIncludes(_this), nb__serviceExcludes(_this));

        if(nb_dcpsPublicationGetInterested(pub)){
            /* Check pre-conditions */
            NB_TRACE(("Checking pre-conditions for nb_dcpsPublication={key="NB_KEYFMT"}"
                      " for forwarding of %s for topic \'%s\'\n",
                                v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                                topObjName, topicName
                ));

            key = nb_dcpsPublicationParticipantKey(pub);
            entry = ut_get((ut_collection)_this->interest.entities.named.DCPSParticipant.handled, (void*) /* discard const */ key);
            if(entry == NULL && requireContainingEntities){
                NB_TRACE(("nb_dcpsPublication={key="NB_KEYFMT"}: Can't find nb_dcpsParticipant={"
                            "key="NB_KEYFMT"} needed for forwarding of %s for topic '%s' yet\n",
                            v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                            v_gidSystemId(*key), v_gidLocalId(*key),
                            topObjName, topicName
                    ));
                goto err_lookup;
            }
            part = entry ? nb_dcpsParticipant(entry) : NULL;

            key = nb_dcpsPublicationParticipantKey(pub);
            entry = ut_get((ut_collection)_this->interest.entities.named.CMParticipant.handled, (void*) /* discard const */ key);
            if(entry == NULL && requireContainingEntities){
                NB_TRACE(("nb_dcpsPublication={key="NB_KEYFMT"}: Can't find nb_cmParticipant={"
                            "key="NB_KEYFMT"} needed for forwarding of %s for topic '%s' yet\n",
                            v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                            v_gidSystemId(*key), v_gidLocalId(*key),
                            topObjName, topicName
                    ));
                goto err_lookup;
            }
            cmpart = entry ? nb_cmParticipant(entry) : NULL;

            key = nb_dcpsPublicationKey(pub);
            entry = ut_get((ut_collection)_this->interest.entities.named.CMDataWriter.handled, (void*) /* discard const */ key);
            if(entry == NULL){
                NB_TRACE(("nb_dcpsPublication={key="NB_KEYFMT"}: Can't find nb_cmWriter={"
                            "key="NB_KEYFMT"} needed for forwarding of %s for topic '%s' yet\n",
                            v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                            v_gidSystemId(*key), v_gidLocalId(*key),
                            topObjName, topicName
                    ));
                goto err_lookup;
            }
            cmw = entry ? nb_cmWriter(entry) : NULL;

            if (cmw == NULL) {
                cmp = NULL;
            } else {
                key = nb_cmWriterPublisherKey(cmw);
                entry = ut_get((ut_collection)_this->interest.entities.named.CMPublisher.handled, (void*) /* discard const */ key);
                if(entry == NULL && requireContainingEntities){
                    NB_TRACE(("nb_dcpsPublication={key="NB_KEYFMT"}: Can't find nb_cmPublisher={"
                              "key="NB_KEYFMT"} needed for forwarding of %s for topic '%s' yet\n",
                              v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                              v_gidSystemId(*key), v_gidLocalId(*key),
                              topObjName, topicName
                              ));
                    goto err_lookup;
                }
                cmp = entry ? nb_cmPublisher(entry) : NULL;
            }

            if((topic = nb__serviceInterestFindTopic(nb_serviceParticipant(_this), topicName)) == NULL){
                NB_TRACE(("nb_dcpsPublication={key="NB_KEYFMT"}: Can't find u_topic '%s'\n",
                        v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                        topicName
                    ));
                goto err_lookup;
            }

            NB_TRACE(("nb_dcpsPublication={key="NB_KEYFMT"}: Found all entities needed to forward %s for topic '%s'\n",
                    v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                    topObjName, topicName
                ));

            /* The topic-definition is available, so for all absolute partition-
             * expressions u_groupNew should succeed. */
            partitions = nb_dcpsPublicationPartitions(pub, &len);
            nb__serviceCreateGroups(_this, partitions, len, topicName, pubkey);

            if (!_this->interest.entities.named.DCPSParticipant.alwaysForward && part != NULL) {
                writer = _this->interest.entities.named.DCPSParticipant.writer;
                ures = nb_topicObjectWrite(writer, nb_topicObject(part));
                if(ures != U_RESULT_OK) NB_TRACE(("nb_topicObjectWrite returned %s for %s\n", u_resultImage(ures), nb_topicObjectName(nb_topicObject(part))));
            }

            if (!_this->interest.entities.named.CMParticipant.alwaysForward && cmpart != NULL) {
                writer = _this->interest.entities.named.CMParticipant.writer;
                ures = nb_topicObjectWrite(writer, nb_topicObject(cmpart));
                if(ures != U_RESULT_OK) NB_TRACE(("nb_topicObjectWrite returned %s for %s\n", u_resultImage(ures), nb_topicObjectName(nb_topicObject(cmpart))));
            }

            if (!_this->interest.entities.named.CMDataWriter.alwaysForward && cmw != NULL) {
                writer = _this->interest.entities.named.CMDataWriter.writer;
                ures = nb_topicObjectWrite(writer, nb_topicObject(cmw));
                if(ures != U_RESULT_OK) NB_TRACE(("nb_topicObjectWrite returned %s for %s\n", u_resultImage(ures), nb_topicObjectName(nb_topicObject(cmw))));
            }

            if (!_this->interest.entities.named.CMPublisher.alwaysForward && cmp != NULL) {
                writer = _this->interest.entities.named.CMPublisher.writer;
                ures = nb_topicObjectWrite(writer, nb_topicObject(cmp));
                if(ures != U_RESULT_OK) NB_TRACE(("nb_topicObjectWrite returned %s for %s\n", u_resultImage(ures), nb_topicObjectName(nb_topicObject(cmp))));
            }

            writer = _this->interest.entities.named.DCPSPublication.writer;
            ures = nb_topicObjectWrite(writer, nb_topicObject(pub));
            if(ures != U_RESULT_OK) NB_TRACE(("nb_topicObjectWrite returned %s for %s\n", u_resultImage(ures), nb_topicObjectName(nb_topicObject(pub))));

            (void)u_objectFree(u_object(topic));
        }

        key = nb_dcpsPublicationKey(pub);
        if(ut_tableInsert(_this->interest.entities.named.DCPSPublication.handled, (void*) /* discard const */ key, pub)){
            NB_TRACE(("nb_dcpsPublication={key="NB_KEYFMT"}: Stored and processed %s nb_dcpsPublication={key="NB_KEYFMT", topic_name=\"%s\"}\n",
                    v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                    topObjName,
                    v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                    topicName
                ));
            nb_objectKeep(pub);
        } else {
            /* An update to an existing publication is not stored, just forwarded. */
            NB_TRACE(("nb_dcpsPublication={key="NB_KEYFMT"}: Processed %s nb_dcpsPublication={key="NB_KEYFMT", topic_name=\"%s\"}\n",
                    v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                    topObjName,
                    v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                    topicName
                ));
        }
        return U_RESULT_OK;

        /* Error handling */
    err_lookup:
        NB_TRACE(("nb_dcpsPublication={key="NB_KEYFMT"}: Handling of %s deferred for nb_dcpsPublication={key="NB_KEYFMT", topic_name=\"%s\"}\n",
                  v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                  topObjName,
                  v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                  topicName
                  ));
        return U_RESULT_TIMEOUT;
    } else {
        return U_RESULT_OK;
    }
}

static u_result nb__serviceHandleCmWriterFunc(nb_service _this, nb_topicObject top) { return nb__serviceHandleCmWriter(_this, nb_cmWriter(top)); }

static u_result
nb__serviceHandleCmWriter(
        nb_service _this,
        nb_cmWriter toHandle)
{
    u_result ur;

    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);
    nb_objectIsValidKind(toHandle, NB_TOPIC_OBJECT_CM_WRITER);

    ur = nb__serviceHandleServiceInterestTopic(
            "nb_cmWriter",
            &_this->interest.entities.named.CMDataWriter,
            nb_cmWriterKey(toHandle),
            nb_topicObject(toHandle));

    return ur;
}

static u_result nb__serviceHandleDcpsSubscriptionFunc(nb_service _this, nb_topicObject top) { return nb__serviceHandleDcpsSubscription(_this, nb_dcpsSubscription(top)); }

static u_result
nb__serviceHandleDcpsSubscription(
        nb_service _this,
        nb_dcpsSubscription sub)
{
    u_result ures;
    u_topic topic;
    const char * const *partitions;
    c_ulong len;
    void *entry;
    nb_dcpsParticipant part;
    nb_cmParticipant cmpart;
    nb_cmSubscriber cms;
    nb_cmReader cmr;
    u_writer writer;
    const v_builtinTopicKey *key;
    const v_builtinTopicKey *subkey;
    const char * topObjName;
    v_state s;
    void * removed;
    nb_dcpsSubscription oldSub;

    s = nb_topicObjectState(nb_topicObject(sub));
    topObjName = nb_topicObjectName(nb_topicObject(sub));
    subkey = nb_dcpsSubscriptionKey(sub);

    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);
    nb_objectIsValidKind(sub, NB_TOPIC_OBJECT_DCPS_SUBSCRIPTION);

    if(v_stateTest(s, L_DISPOSED)){
        removed = ut_remove((ut_collection)_this->interest.entities.named.DCPSSubscription.handled, (void *) /* remove const */ subkey);
        if(removed){
            oldSub = nb_dcpsSubscription(removed);
            NB_TRACE(("Removed %s nb_dcpsSubscription={key="NB_KEYFMT"}\n",
                      topObjName, v_gidSystemId(*subkey), v_gidLocalId(*subkey)));
            if(nb_dcpsSubscriptionGetInterested(oldSub)){
                /* Forward the dispose */
                ures = nb_topicObjectWrite(_this->interest.entities.named.DCPSSubscription.writer, nb_topicObject(sub));
                if(ures != U_RESULT_OK) NB_TRACE(("nb_topicObjectWrite returned %s for %s\n", u_resultImage(ures), nb_topicObjectName(nb_topicObject(sub))));
            }
            nb_objectFree(oldSub);
        }
        return U_RESULT_OK;
    } else if(v_stateTest(s, L_WRITE)){
        const char * topicName = nb_dcpsSubscriptionTopicName(sub);

        /* Set interest */
        nb_dcpsSubscriptionSetInterested(sub, nb__serviceIncludes(_this), nb__serviceExcludes(_this));

        if(nb_dcpsSubscriptionGetInterested(sub)){
            /* Check pre-conditions */
            NB_TRACE(("Checking pre-conditions for nb_dcpsSubscription={key="NB_KEYFMT"}"
                      " for forwarding of %s for topic \'%s\'\n",
                      v_gidSystemId(*subkey), v_gidLocalId(*subkey),
                      topObjName, topicName
                      ));

            key = nb_dcpsSubscriptionParticipantKey(sub);
            entry = ut_get((ut_collection)_this->interest.entities.named.DCPSParticipant.handled, (void*) /* discard const */ key);
            if(entry == NULL && requireContainingEntities){
                NB_TRACE(("nb_dcpsSubscription={key="NB_KEYFMT"}: Can't find nb_dcpsParticipant={"
                          "key="NB_KEYFMT"} needed for forwarding of %s for topic '%s' yet\n",
                          v_gidSystemId(*subkey), v_gidLocalId(*subkey),
                          v_gidSystemId(*key), v_gidLocalId(*key),
                          topObjName, topicName
                          ));
                goto err_lookup;
            }
            part = entry ? nb_dcpsParticipant(entry) : NULL;

            key = nb_dcpsSubscriptionParticipantKey(sub);
            entry = ut_get((ut_collection)_this->interest.entities.named.CMParticipant.handled, (void*) /* discard const */ key);
            if(entry == NULL && requireContainingEntities){
                NB_TRACE(("nb_dcpsSubscription={key="NB_KEYFMT"}: Can't find nb_cmParticipant={"
                          "key="NB_KEYFMT"} needed for forwarding of %s for topic '%s' yet\n",
                          v_gidSystemId(*subkey), v_gidLocalId(*subkey),
                          v_gidSystemId(*key), v_gidLocalId(*key),
                          topObjName, topicName
                          ));
                goto err_lookup;
            }
            cmpart = entry ? nb_cmParticipant(entry) : NULL;

            key = nb_dcpsSubscriptionKey(sub);
            entry = ut_get((ut_collection)_this->interest.entities.named.CMDataReader.handled, (void*) /* discard const */ key);
            if(entry == NULL){
                NB_TRACE(("nb_dcpsSubscription={key="NB_KEYFMT"}: Can't find nb_cmReader={"
                          "key="NB_KEYFMT"} needed for forwarding of %s for topic '%s' yet\n",
                          v_gidSystemId(*subkey), v_gidLocalId(*subkey),
                          v_gidSystemId(*key), v_gidLocalId(*key),
                          topObjName, topicName
                          ));
                goto err_lookup;
            }
            cmr = entry ? nb_cmReader(entry) : NULL;

            if (cmr == NULL) {
                cms = NULL;
            } else {
                key = nb_cmReaderSubscriberKey(cmr);
                entry = ut_get((ut_collection)_this->interest.entities.named.CMSubscriber.handled, (void*) /* discard const */ key);
                if(entry == NULL && requireContainingEntities){
                    NB_TRACE(("nb_dcpsSubscription={key="NB_KEYFMT"}: Can't find nb_cmSubscriber={"
                              "key="NB_KEYFMT"} needed for forwarding of %s for topic '%s' yet\n",
                              v_gidSystemId(*subkey), v_gidLocalId(*subkey),
                              v_gidSystemId(*key), v_gidLocalId(*key),
                              topObjName, topicName
                              ));
                    goto err_lookup;
                }
                cms = entry ? nb_cmSubscriber(entry) : NULL;
            }

            if((topic = nb__serviceInterestFindTopic(nb_serviceParticipant(_this), topicName)) == NULL){
                NB_TRACE(("nb_dcpsSubscription={key="NB_KEYFMT"}: Can't find u_topic '%s'\n",
                          v_gidSystemId(*subkey), v_gidLocalId(*subkey),
                          topicName
                          ));
                goto err_lookup;
            }

            NB_TRACE(("nb_dcpsSubscription={key="NB_KEYFMT"}: Found all entities needed to forward %s for topic '%s'\n",
                      v_gidSystemId(*subkey), v_gidLocalId(*subkey),
                      topObjName, topicName
                      ));

            /* The topic-definition is available, so for all absolute partition-
             * expressions u_groupNew should succeed. */
            partitions = nb_dcpsSubscriptionPartitions(sub, &len);
            nb__serviceCreateGroups(_this, partitions, len, topicName, subkey);

            if (!_this->interest.entities.named.DCPSParticipant.alwaysForward && part != NULL) {
                writer = _this->interest.entities.named.DCPSParticipant.writer;
                ures = nb_topicObjectWrite(writer, nb_topicObject(part));
                if(ures != U_RESULT_OK) NB_TRACE(("nb_topicObjectWrite returned %s for %s\n", u_resultImage(ures), nb_topicObjectName(nb_topicObject(part))));
            }

            if (!_this->interest.entities.named.CMParticipant.alwaysForward && cmpart != NULL) {
                writer = _this->interest.entities.named.CMParticipant.writer;
                ures = nb_topicObjectWrite(writer, nb_topicObject(cmpart));
                if(ures != U_RESULT_OK) NB_TRACE(("nb_topicObjectWrite returned %s for %s\n", u_resultImage(ures), nb_topicObjectName(nb_topicObject(cmpart))));
            }

            if (!_this->interest.entities.named.CMDataReader.alwaysForward && cmr != NULL) {
                writer = _this->interest.entities.named.CMDataReader.writer;
                ures = nb_topicObjectWrite(writer, nb_topicObject(cmr));
                if(ures != U_RESULT_OK) NB_TRACE(("nb_topicObjectWrite returned %s for %s\n", u_resultImage(ures), nb_topicObjectName(nb_topicObject(cmr))));
            }

            if (!_this->interest.entities.named.CMSubscriber.alwaysForward && cms != NULL) {
                writer = _this->interest.entities.named.CMSubscriber.writer;
                ures = nb_topicObjectWrite(writer, nb_topicObject(cms));
                if(ures != U_RESULT_OK) NB_TRACE(("nb_topicObjectWrite returned %s for %s\n", u_resultImage(ures), nb_topicObjectName(nb_topicObject(cms))));
            }

            writer = _this->interest.entities.named.DCPSSubscription.writer;
            ures = nb_topicObjectWrite(writer, nb_topicObject(sub));
            if(ures != U_RESULT_OK) NB_TRACE(("nb_topicObjectWrite returned %s for %s\n", u_resultImage(ures), nb_topicObjectName(nb_topicObject(sub))));

            (void)u_objectFree(u_object(topic));
        }

        key = nb_dcpsSubscriptionKey(sub);
        if(ut_tableInsert(_this->interest.entities.named.DCPSSubscription.handled, (void*) /* discard const */ key, sub)){
            NB_TRACE(("nb_dcpsSubscription={key="NB_KEYFMT"}: Stored and processed %s nb_dcpsSubscription={key="NB_KEYFMT", topic_name=\"%s\"}\n",
                      v_gidSystemId(*subkey), v_gidLocalId(*subkey),
                      nb_topicObjectName(nb_topicObject(sub)),
                      v_gidSystemId(*subkey), v_gidLocalId(*subkey),
                      topicName
                      ));
            nb_objectKeep(sub);
        } else {
            /* An update to an existing subscription is not stored, just forwarded. */
            NB_TRACE(("nb_dcpsSubscription={key="NB_KEYFMT"}: Processed %s nb_dcpsSubscription={key="NB_KEYFMT", topic_name=\"%s\"}\n",
                      v_gidSystemId(*subkey), v_gidLocalId(*subkey),
                      nb_topicObjectName(nb_topicObject(sub)),
                      v_gidSystemId(*subkey), v_gidLocalId(*subkey),
                      topicName
                      ));
        }
        return U_RESULT_OK;

        /* Error handling */
    err_lookup:
        NB_TRACE(("nb_dcpsSubscription={key="NB_KEYFMT"}: Handling of %s deferred for nb_dcpsSubscription={key="NB_KEYFMT", topic_name=\"%s\"}\n",
                  v_gidSystemId(*subkey), v_gidLocalId(*subkey),
                  nb_topicObjectName(nb_topicObject(sub)),
                  v_gidSystemId(*subkey), v_gidLocalId(*subkey),
                  topicName
                  ));
        return U_RESULT_TIMEOUT;
    } else {
        return U_RESULT_OK;
    }
}

static u_result nb__serviceHandleCmReaderFunc(nb_service _this, nb_topicObject top) { return nb__serviceHandleCmReader(_this, nb_cmReader(top)); }

static u_result
nb__serviceHandleCmReader(
        nb_service _this,
        nb_cmReader toHandle)
{
    u_result ur;

    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);
    nb_objectIsValidKind(toHandle, NB_TOPIC_OBJECT_CM_READER);

    ur = nb__serviceHandleServiceInterestTopic(
            "nb_cmReader",
            &_this->interest.entities.named.CMDataReader,
            nb_cmReaderKey(toHandle),
            nb_topicObject(toHandle));

    return ur;
}

static u_result nb__serviceHandleCmPublisherFunc(nb_service _this, nb_topicObject top) { return nb__serviceHandleCmPublisher(_this, nb_cmPublisher(top)); }

static u_result
nb__serviceHandleCmPublisher(
        nb_service _this,
        nb_cmPublisher toHandle)
{
    u_result ur;

    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);
    nb_objectIsValidKind(toHandle, NB_TOPIC_OBJECT_CM_PUBLISHER);

    ur = nb__serviceHandleServiceInterestTopic(
           "nb_cmPublisher",
           &_this->interest.entities.named.CMPublisher,
           nb_cmPublisherKey(toHandle),
           nb_topicObject(toHandle));

    return ur;
}

static u_result nb__serviceHandleCmSubscriberFunc(nb_service _this, nb_topicObject top) { return nb__serviceHandleCmSubscriber(_this, nb_cmSubscriber(top)); }

static u_result
nb__serviceHandleCmSubscriber(
        nb_service _this,
        nb_cmSubscriber toHandle)
{
    u_result ur;

    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);
    nb_objectIsValidKind(toHandle, NB_TOPIC_OBJECT_CM_SUBSCRIBER);

    ur = nb__serviceHandleServiceInterestTopic(
            "nb_cmSubscriber",
            &_this->interest.entities.named.CMSubscriber,
            nb_cmSubscriberKey(toHandle),
            nb_topicObject(toHandle));

    return ur;
}

static void
nb__serviceHandleBuiltinTopics (
        nb_service _this)
{
    size_t i;
    nb_topicObject topObj;
    c_iter toHandle;
    struct nb_serviceInterestTopic *topic;
    u_result ures;
    c_bool somethingChanged;

    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);

    /* The service handles built-in topics by storing all samples with L_WRITE
     * state in the relevant handled-table. The DCPSPublication- and
     * DCPSSubscription-topics are only stored when they match the interest
     * expressions and all other related built-in topics are stored already. If
     * that is the case, the built-in topics are retrieved and forwarded. */
    do {
        somethingChanged = FALSE;
        for(i = 0; i < NB_SERVICE_NUM_ENTITIES; i++){
            topic = &_this->interest.entities.array[i];
            if(topic->subscriptionsMatched){
                toHandle = NULL;
                while((topObj = c_iterTakeFirst(topic->toHandle)) != NULL){
                    ures = topic->handleFunc(_this, topObj);

                    if(ures == U_RESULT_OK){
                        nb_topicObjectFree(topObj);
                        somethingChanged = TRUE;
                    } else {
                        toHandle = c_iterAppend(toHandle, topObj);
                    }
                }
                assert(c_iterLength(topic->toHandle) == 0);
                c_iterFree(topic->toHandle);
                topic->toHandle = toHandle;
            }
        }
    } while (somethingChanged);
}

static os_result
nb__exitRequestHandler(
    os_callbackArg ignore,
    void *callingThreadContext,
    void * arg)
{
    nb_service _this = nb_service(arg);

    OS_UNUSED_ARG(ignore);
    OS_UNUSED_ARG(callingThreadContext);

    assert(_this);

    _this->terminate = 1;
    (void) u_waitsetNotify(_this->ws, NULL);

    return os_resultSuccess; /* the main thread will take care of termination */
}

static os_result
nb__exceptionHandler(
        void *callingThreadContext, void * arg)
{
    nb_service _this = nb_service(arg);

    OS_UNUSED_ARG(callingThreadContext);
    assert(_this);

    (void) u_serviceChangeState(_this->service, STATE_DIED);

    return os_resultSuccess;
}

static v_result
nb__serviceWriterGetMatchedStatus(
        c_voidp info /* struct v_topicMatchInfo* */,
        c_voidp arg /* c_ulong* */)
{
    const struct v_topicMatchInfo *from = (struct v_topicMatchInfo *)info;

    assert(from);
    assert(arg);

    /* currentCount can never become negative, so we can safely cast to c_ulong */
    *(c_ulong *)arg = (c_ulong)from->currentCount;

    return V_RESULT_OK;
}

static const char * const *
nb__serviceIncludes(
        nb_service _this)
{
    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);

    if(!_this->includes){
        _this->includes = nb_configurationIncludes(_this->config);
    }

    return (const char * const *) _this->includes;
}

static const char * const *
nb__serviceExcludes(
        nb_service _this)
{
    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);

    if(!_this->excludes){
        _this->excludes = nb_configurationExcludes(_this->config);
    }

    return (const char * const *) _this->excludes;
}

static u_result
nb__serviceHandleEvents (
    nb_service _this,
    u_waitsetEvent event)
{
    u_result uresult = U_RESULT_OK;
    size_t i;

    if (v_eventTest(event->kind, V_EVENT_DATA_AVAILABLE)) {
        for(i = 0; i < NB_SERVICE_NUM_ENTITIES; i++){
            struct nb_serviceInterestTopic *topic = &_this->interest.entities.array[i];
            if (event->userData == u_entity(topic->reader)) {
                uresult = u_dataReaderTake(topic->reader, V_MASK_ANY, topic->action, &topic->toHandle, OS_DURATION_ZERO);
                if (uresult == U_RESULT_NO_DATA) {
                    uresult = U_RESULT_OK;
                }
                /* TODO: flush? */
                break;
            }
        }
        nb__serviceHandleBuiltinTopics(_this);
    } else if (v_eventTest(event->kind, V_EVENT_PUBLICATION_MATCHED)) {
        for(i = 0; i < NB_SERVICE_NUM_ENTITIES; i++){
            struct nb_serviceInterestTopic *topic = &_this->interest.entities.array[i];
            if (event->userData == u_entity(topic->writer)) {
                uresult = u_writerGetPublicationMatchStatus(topic->writer, TRUE, nb__serviceWriterGetMatchedStatus, &topic->subscriptionsMatched);
                NB_TRACE(("%s-writer has %u matching subscription(s), so will %spublish %s's now.\n",
                          topic->topicName,
                          topic->subscriptionsMatched,
                          topic->subscriptionsMatched ? "" : "not ",
                          topic->topicName));
                break;
            }
        }
    } else if (v_eventTest(event->kind, V_EVENT_TRIGGER)) {
        _this->terminate = 1;
    } else {
        NB_ERROR_1("nb__serviceMain",
                   "Received unexpected event %d\n",
                   (int) event->kind);
        uresult = U_RESULT_INTERNAL_ERROR;
    }
    if(uresult != U_RESULT_OK){
        if(uresult == U_RESULT_ALREADY_DELETED){
            nb_log(LOG_WARNING, "nb__serviceMain: Processing of event failed: %s\n", u_resultImage(uresult));
        } else {
            NB_WARNING_1("nb__serviceMain", "Processing of event failed: %s\n", u_resultImage(uresult));
        }
    }
    return uresult;
}

struct nb__serviceHandleEventsActionArg {
    nb_service service;
    u_result result;
};

static void
nb__serviceHandleEventsAction (
    u_waitsetEvent event,
    void *varg)
{
    struct nb__serviceHandleEventsActionArg *arg = varg;
    arg->result = nb__serviceHandleEvents (arg->service, event);
}

static void
nb__serviceMain(
        nb_service _this)
{
    size_t i;
    u_result uresult;
    struct nb__serviceHandleEventsActionArg argument;

    argument.service = _this;

    nb_log(LOG_INFO, "nb__serviceMain started\n");

    nb__serviceChangeState(_this, SERVICE_OPERATIONAL);

    /* Workaround for OSPL-5132. These calls can be removed when OSPL-5132 is fixed. */
    for(i = 0; i < NB_SERVICE_NUM_ENTITIES; i++){
        struct nb_serviceInterestTopic *topic = &_this->interest.entities.array[i];
        uresult = u_dataReaderTake(topic->reader, V_MASK_ANY, topic->action, &topic->toHandle, OS_DURATION_ZERO);
        assert(uresult == U_RESULT_OK || uresult == U_RESULT_NO_DATA);
        uresult = u_writerGetPublicationMatchStatus(topic->writer, TRUE, nb__serviceWriterGetMatchedStatus, &topic->subscriptionsMatched);
        assert(uresult == U_RESULT_OK);
    }
    /* End of workaround. */

    while (uresult == U_RESULT_OK && !_this->terminate) {
        /* Handle pending builtin topics */
        nb__serviceHandleBuiltinTopics(_this);

        /* Wait for events to occur */
        uresult = u_waitsetWaitAction (_this->ws, nb__serviceHandleEventsAction, &argument, OS_DURATION_INFINITE);

        if (uresult == U_RESULT_OK) {
            uresult = argument.result;
        } else if (uresult == U_RESULT_DETACHING) {
            NB_INFO("nb__serviceMain", "u_waitsetTimedWaitEvents detected termination\n");
            _this->terminate = 1;
        } else if (uresult == U_RESULT_TIMEOUT) {
            uresult = U_RESULT_OK;
        } else {
            NB_ERROR_1("nb__serviceMain",
                "u_waitsetTimedWaitEvents failed (%s)\n", u_resultImage(uresult));
        }
    }

    nb_log(LOG_INFO, "nb__serviceMain finished\n");

    return;
}

static void
nb__serviceCreateGroups(
        nb_service _this,
        const char * const * partitions,
        c_ulong len,
        const char * topicName,
        const v_builtinTopicKey *pubkey)
{
    c_ulong i;
    const char * partition;
    u_group group;

    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);

    assert(partitions || len == 0);
    assert(topicName);
    assert(pubkey);

    for(i = 0; i < len; i++){
        partition = partitions[i];

        /* Check that the partition-expression is absolute; wildcards are not
         * supported during creation. */
        if (strchr(partition, '*') == NULL && strchr(partition, '?') == NULL) {
            if(nb_match(&partition, 1, topicName, nb__serviceIncludes(_this), nb__serviceExcludes(_this))){
                group = u_groupNew(nb_serviceParticipant(_this), partition, topicName, 0);
                assert(group); /* u_participantFindTopic passed, so this one should as well */
                NB_TRACE(("nb_dcpsPublication={key="NB_KEYFMT"}: New group created based on DCPSPublication: Group<%s,%s>\n",
                        v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                        partition, topicName
                    ));
                NB_TRACE(("nb_serviceCreateGroups: enabling forwarding on Group<%s,%s>\n", partition, topicName));
                u_groupSetRoutingEnabled(group, TRUE, NULL);
                u_objectFree((u_object) group);
            } else {
                NB_TRACE(("nb_dcpsPublication={key="NB_KEYFMT"}: New group not created for DCPSPublication: Group<%s,%s>\n",
                        v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                        partition, topicName));
            }
        } else {
            NB_TRACE(("nb_dcpsPublication={key="NB_KEYFMT"}: Ignored wildcard partition-expression: '%s' for topic '%s' is not supported\n",
                    v_gidSystemId(*pubkey), v_gidLocalId(*pubkey),
                    partition, topicName
                ));
        }
    }
}

OPENSPLICE_SERVICE_ENTRYPOINT (ospl_nwbridge, nwbridge)
{
    /* Exit status: 0: OK, 1: abnormal termination.
     * Default to OK (normal termination), then override if
     * that is inappropriate.
     */
    os_int32 exitStatus = EXIT_SUCCESS;
    nb_service service;
    os_char* uri = NULL;
    os_char* serviceName;
    u_result uresult;
    C_STRUCT(nb_logConfig) gc;
    nb_threadStates ts;

    os_osInit();

#ifdef EVAL_V
    OS_REPORT(OS_INFO, "Networking Bridge Service", 0,
        "++++++++++++++++++++++++++++++++++++++++++++++++++" OS_REPORT_NL
        "++ Networking Bridge Service EVALUATION VERSION ++" OS_REPORT_NL
        "++++++++++++++++++++++++++++++++++++++++++++++++++");
#endif

    if(argc <= 1) {
        OS_REPORT(OS_WARNING, "Networking Bridge Service", 0,
            "No service-name supplied, defaulting to '%s'. Usage: '%s <service-name> (<URI/domain-name>)'.", argv[0], argv[0]);
        serviceName = argv[0];
    }
    if(argc > 1) {
        serviceName = argv[1];
    }
    if(argc > 2) {
        uri = argv[2];
    }

    ts = nb_threadStatesAlloc();

    nb_logConfigInit(&gc);

    /* Create the threads administration (and promote the main thread) */
    if (nb_threadStatesInit(ts, NB_MAXTHREADS, &gc) != os_resultSuccess) { /* todo make max number of threads configurable? */
        OS_REPORT(OS_FATAL, "ospl_nwbridge", 0,
                "Failed to initialize threads administration");
        goto err_thrstatesinit;
    }
    nb_threadUpgrade(ts);

    /* Create the nwbridge service object. This will create all needed DDS entities and insert the
     * nwbridge topic meta data into the domain. */
    service = nb__serviceNew(uri, serviceName);
    if(!service) {
        fprintf(stderr, "Failed to create the service. Is the domain running?\n");
        /* Error reported by nb_serviceNew */
        goto err_serviceNew;
    }

    /* Write our init-status; no way to detect failure... */
    nb__serviceChangeState(service, SERVICE_INITIALISING);

    /* The service must monitor the splice daemon for liveliness. If the SPLICE-daemon disappears, the service should
     * react to this. */
    uresult = u_serviceWatchSpliceDaemon(service->service, nb__serviceWatchSpliced, (c_voidp)service);
    if(uresult != U_RESULT_OK){
        /* Error reported by u_serviceWatchSpliceDaemon; TODO: check whether report contains enough info. */
        nb__serviceChangeState(service, SERVICE_TERMINATING);
        goto err_serviceWatchSpliced;
    }

    /* Now the main thread of the nwbridge service will wait until the terminate flag of the nwbridge service has been set to true. */
    nb__serviceMain(service);

    nb__serviceChangeState(service, SERVICE_TERMINATING);

    u_serviceWatchSpliceDaemon(service->service, NULL, (c_voidp)service);

err_serviceWatchSpliced:
    nb_serviceFree(service);
err_serviceNew:
    nb_threadDowngrade();
    nb_threadStatesDeinit(ts);
err_thrstatesinit:
    nb_logConfigDeinit(&gc);
    nb_threadStatesDealloc(ts);

    os_osExit();
    return exitStatus;
}

const os_char *
nb_serviceName(
    nb_service _this)
{
    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);

    return _this->name;
}

u_service
nb_serviceService(
    nb_service _this)
{
    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);

    return _this->service;
}

void
nb__serviceLoadMetaData(
    v_public vpublic,
    c_voidp arg /* c_bool* */)
{
    c_base base;
    c_bool *result = (c_bool*)arg;

    assert(vpublic);
    assert(result);

    base = c_getBase((c_object) vpublic);
    *result = loadNetworkingBridge(base);

    if (!*result) {
        NB_ERROR("nb_serviceLoadMetaData", "Failed to load NetworkingBridge-module\n");
    }
}

void
nb__serviceWatchSpliced(
    v_serviceStateKind state,
    c_voidp userData /* nb_service */)
{
    nb_service _this;

    nb_objectIsValidKind(userData, NB_OBJECT_SERVICE);

    _this = nb_service(userData);

    switch(state)
    {
        case STATE_TERMINATING:
        case STATE_TERMINATED:
        case STATE_DIED:
            NB_TRACE(("nb__serviceWatchSpliced: spliced is terminating. nwbridge service '%s' %s.",
                    _this->name, _this->terminate ? "is already terminating" : "will terminate too"));
            /* Set the terminate flag  */
            _this->terminate = 1;
            break;
        default:
            /* do nothing */
            break;
    }
}

static v_serviceStateKind
nb__ServiceStateToKernelServiceStateKind(
    ServiceState state)
{
    v_serviceStateKind kind;

    switch(state){
        case SERVICE_INITIALISING:
            kind = STATE_INITIALISING;
            break;
        case SERVICE_OPERATIONAL:
            kind = STATE_OPERATIONAL;
            break;
        case SERVICE_TERMINATING:
            kind = STATE_TERMINATING;
            break;
        case SERVICE_TERMINATED:
            kind = STATE_TERMINATED;
            break;
        default:
            assert(FALSE);
            kind = STATE_NONE;
            break;
    }
    return kind;
}

c_bool
nb__serviceChangeState(
    nb_service _this,
    ServiceState state)
{
    c_bool stateChanged;

    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);
    assert(_this->service);

    /* TODO: fix u_serviceChangeState; currently it cannot be distinguished whether the
     * change failed or the state just didn't change. */
    stateChanged = u_serviceChangeState(_this->service, nb__ServiceStateToKernelServiceStateKind(state));
    if (stateChanged) {
        u_result result;
        struct ServiceStatus status;

        status.serviceId = _this->name;

        status.state = state;

        assert(_this->statusWriter);

        if (state != SERVICE_TERMINATED) {
            result = u_writerWrite (
                _this->statusWriter,
                (u_writerCopy)ServiceStatusCopyIn,
                (void*)&status,
                OS_TIMEW_INVALID,
                U_INSTANCEHANDLE_NIL);
        } else {
            result = u_writerWriteDispose (
                _this->statusWriter,
                (u_writerCopy)ServiceStatusCopyIn,
                (void*)&status,
                OS_TIMEW_INVALID,
                U_INSTANCEHANDLE_NIL);
        }

        if(result != U_RESULT_OK){
            OS_REPORT(OS_WARNING, "nb__serviceChangeState", result,
                "Failed to publish service state-change; write returned '%s'.", u_resultImage(result));
        }
    }
    return stateChanged;
}

static c_bool
ServiceStatusCopyIn(
    c_type type,
    struct ServiceStatus *data,
    struct ServiceStatus *to)
{
    assert(type);
    assert(data);
    assert(to);

    memcpy(to, data, sizeof(*to));
    to->serviceId = c_stringNew(c_getBase(type), data->serviceId);

    return TRUE;
}

static u_topic
nb__serviceInterestFindTopic(
        u_participant participant,
        const c_char * name)
{
    c_iter topics;
    u_topic topic;

    assert(participant);
    assert(name);

    topics = u_participantFindTopic(participant, name, 0);
    assert(c_iterLength(topics) <= 1);
    topic = u_topic(c_iterTakeFirst(topics));
    c_iterFree(topics);

    return topic;
}

static u_result
nb__serviceInterestTopicInit(
        struct nb_serviceInterestTopic *interest,
        u_participant participant,
        u_waitset waitset,
        u_publisher publisher,
        u_writerQos writerQos,
        u_subscriber subscriber,
        u_readerQos readerQos)
{
    u_result result = U_RESULT_INTERNAL_ERROR;
    c_char *entityName;
    c_char *expression;
    u_topic topic;

    assert(interest);
    assert(interest->topicName);
    assert(participant);
    assert(waitset);
    assert(publisher);
    assert(writerQos);
    assert(subscriber);
    assert(readerQos);

    interest->toHandle = NULL;
    interest->alwaysForward = FALSE;

    interest->handled = (ut_table)ut_tableNew(nb_compareByGid, NULL, 0, NULL, nb__serviceInterestTopicObjectFreeFunc, NULL);
    if(!interest->handled){
        OS_REPORT(OS_FATAL, "nb__serviceInterestTopicInit", 0, "Out of resources. Could not allocate table.");
        goto err_tableNew;
    }

    topic = nb__serviceInterestFindTopic(participant, interest->topicName);
    if(!topic){
        /* Out-of-memory and no topics found cannot be distinguished currently.
         * Both are wrong however, so report and fail. */
        OS_REPORT(OS_FATAL, "nb__serviceInterestTopicInit", 0, "Out of resources or failed to retrieve %s-topic.", interest->topicName);
        goto err_findTopic;
    }

    entityName = os_malloc(strlen(interest->topicName) + strlen(NB_INTEREST_WRITER_SUFFIX) + 1);
    (void) sprintf(entityName, "%s" NB_INTEREST_WRITER_SUFFIX, interest->topicName);

        /* Create the DCPSSubscription Writer and Reader */
    interest->writer = u_writerNew(
                          publisher,
                          entityName,
                          topic,
                          writerQos);

    os_free(entityName);
    u_objectFree(u_object(topic));

    if(!interest->writer){
        /* Error reported by u_writerNew */
        goto err_writerNew;
    }
    u_writerEnable (interest->writer);
    interest->subscriptionsMatched = 0;

    result = u_waitsetAttach(waitset, u_observable(interest->writer), u_entity(interest->writer));
    if(result != U_RESULT_OK){
       OS_REPORT(OS_FATAL, "nb__serviceInterestTopicInit", result, "Could not attach writer to waitset: result %s",
               u_resultImage(result));
       goto err_waitsetattachWriter;
    }

    entityName = os_malloc(strlen(interest->topicName) + strlen(NB_INTEREST_READER_SUFFIX) + 1);
    (void) sprintf(entityName, "%s" NB_INTEREST_READER_SUFFIX, interest->topicName);

    expression = os_malloc(strlen(interest->topicName) + strlen("select * from ") + 1);
    (void) sprintf(expression, "select * from %s", interest->topicName);

    interest->reader = u_subscriberCreateDataReader(
            subscriber,
            entityName,
            expression,
            NULL, 0,
            readerQos);

    os_free(entityName);
    os_free(expression);

    if(!interest->reader){
        /* Error reported by u_subscriberCreateDataReader */
        goto err_readerNew;
    }

    if(u_entityEnable(u_entity(interest->reader)) != U_RESULT_OK) {
        goto err_readerEnable;
    }

    result = u_waitsetAttach(waitset, u_observable(interest->reader), u_entity(interest->reader));
    if(result != U_RESULT_OK){
       OS_REPORT(OS_FATAL, "nb__serviceInterestTopicInit", result, "Could not attach reader to waitset: result %s",
               u_resultImage(result));
       goto err_waitsetattachReader;
    }

    return result;

/* Error-handling */
err_waitsetattachReader:
err_readerEnable:
    u_objectFree(u_object(interest->reader));
err_readerNew:
    u_waitsetDetach(waitset, u_observable(interest->writer));
err_waitsetattachWriter:
    u_objectFree(u_object(interest->writer));
err_writerNew:
err_findTopic:
    ut_tableFree(interest->handled);
err_tableNew:
    return result;
}

/* ut_freeElementFunc wrapper */
static void
nb__serviceInterestTopicObjectFreeFunc(
        void *o,
        void *arg)
{
    OS_UNUSED_ARG(arg);

    nb_topicObjectFree(nb_topicObject(o));
}


static void
nb__serviceInterestTopicDeinit(
        struct nb_serviceInterestTopic *interest,
        u_waitset waitset)
{
    nb_topicObject topObj;

    assert(interest);
    assert(waitset);

    /* Free remaining builtin topics */
    while((topObj = c_iterTakeFirst(interest->toHandle)) != NULL) {
        NB_TRACE(("Could not handle %s for topic %s\n", nb_topicObjectName(topObj), interest->topicName));
        nb_topicObjectFree(topObj);
    }
    c_iterFree(interest->toHandle);

    ut_tableFree(interest->handled);

    (void) u_waitsetDetach(waitset, u_observable(interest->reader));
    (void) u_objectFree(u_object(interest->reader));
    (void) u_waitsetDetach(waitset, u_observable(interest->writer));
    (void) u_objectFree(u_object(interest->writer));
}

static os_result
nb__serviceInterestInit(
    struct nb_serviceInterest* interest,
    u_participant participant,
    u_waitset waitset)
{
    v_subscriberQos subscriberQos;
    v_readerQos readerQos;
    v_publisherQos publisherQos;
    v_writerQos writerQos;
    c_char fedSpecPart[U_DOMAIN_FEDERATIONSPECIFICPARTITIONNAME_MINBUFSIZE];
    u_result result;
    c_ulong mask;
    size_t i;

    {   /* Create and initialize the QoS's */

        /* Publisher-QoS */
        publisherQos = u_publisherQosNew(NULL);
        if (!publisherQos) {
            OS_REPORT(OS_FATAL, "nb__serviceInterestInit", 0, "Out of resources. Could not allocate default publisherQos.");
            goto err_publisherQosNew;
        }

        /* Retrieve the federation specific partition-name */
        /* If bufSize is at least U_DOMAIN_FEDERATIONSPECIFICPARTITIONNAME_MINBUFSIZE,
         * the full partition-name is guaranteed to be returned. */
        result = u_participantFederationSpecificPartitionName (
                    participant, fedSpecPart, sizeof (fedSpecPart));
        assert(result == U_RESULT_OK);

        publisherQos->presentation.v.access_scope = V_PRESENTATION_TOPIC;
        os_free(publisherQos->partition.v);
        publisherQos->partition.v = os_strdup(fedSpecPart);

        /* Writer-QoS */
        writerQos = u_writerQosNew(NULL);
        if(!writerQos){
            OS_REPORT(OS_FATAL, "nb__serviceInterestInit", 0, "Out of resources. Could not allocate default writerQos.");
            goto err_writerQosNew;
        }

        /* Set interest writer QoS. Should be VOLATILE and autodispose = TRUE */
        assert(writerQos->durability.v.kind == V_DURABILITY_VOLATILE);
        /* This writer is meant to be attached to a node-local partition, so
         * reliability shouldn't really matter. */
        writerQos->reliability.v.kind = V_RELIABILITY_RELIABLE;
        assert(writerQos->lifecycle.v.autodispose_unregistered_instances == TRUE);


        /* Subscriber-QoS */
        subscriberQos = u_subscriberQosNew(NULL);
        if (!subscriberQos) {
            OS_REPORT(OS_FATAL, "nb__serviceInterestInit", 0, "Out of resources. Could not allocate default subscriberQos.");
            goto err_subscriberQosNew;
        }
        os_free(subscriberQos->partition.v);
        subscriberQos->presentation.v.access_scope = V_PRESENTATION_TOPIC;
        subscriberQos->partition.v = os_strdup(V_BUILTIN_PARTITION);

        /* Reader-QoS */
        readerQos = u_readerQosNew (NULL);
        if (!readerQos){
            OS_REPORT(OS_FATAL, "nb__serviceInterestInit", 0, "Out of resources. Could not allocate default readerQos.");
            goto err_readerQosNew;
        }

        /* Set the proper QoS for the DCPSPublication-topic */
        readerQos->durability.v.kind = V_DURABILITY_TRANSIENT;
        readerQos->reliability.v.kind = V_RELIABILITY_RELIABLE;
        readerQos->history.v.kind = V_HISTORY_KEEPLAST;
        readerQos->history.v.depth = 1;

    }

    {   /* Create the Publisher and Subscriber  */

        /* Publisher */
        interest->publisher = u_publisherNew(participant, NB_INTEREST_PUBLISHER, publisherQos, TRUE);
        if (!interest->publisher) {
            /* Error reported by u_publisherNew */
            goto err_publisherNew;
        }

        /* Subscriber */
        interest->subscriber = u_subscriberNew(participant, NB_INTEREST_SUBSCRIBER, subscriberQos);
        if (!interest->subscriber) {
            /* Error reported by u_subscriberNew */
            goto err_subscriberNew;
        }
        if(u_entityEnable(u_entity(interest->subscriber)) != U_RESULT_OK) {
            goto err_subscriberEnable;
        }
    }

    {   /* Set proper masks on WaitSet */

        assert(waitset);

        result = u_waitsetGetEventMask(waitset, &mask);
        if (result != U_RESULT_OK) {
            OS_REPORT(OS_FATAL, "nb__serviceInterestInit", result, "Could not get event mask for waitset: result %s",
                    u_resultImage(result));
            goto err_eventmask;
        }

        /* V_EVENT_DATA_AVAILABLE for the readers; V_EVENT_PUBLICATION_MATCHED for the writers */
        result = u_waitsetSetEventMask(waitset, mask | V_EVENT_DATA_AVAILABLE | V_EVENT_PUBLICATION_MATCHED);
        if (result != U_RESULT_OK) {
            OS_REPORT(OS_FATAL, "nb__serviceInterestInit", result, "Could not set event mask for waitset: result %s",
                    u_resultImage(result));
            goto err_eventmask;
        }
    }

    /* Initializes topic-names and actions */
    interest->entities.named.DCPSTopic.topicName =        V_TOPICINFO_NAME;
    interest->entities.named.DCPSParticipant.topicName =  V_PARTICIPANTINFO_NAME;
    interest->entities.named.CMParticipant.topicName =    V_CMPARTICIPANTINFO_NAME;
    interest->entities.named.DCPSPublication.topicName =  V_PUBLICATIONINFO_NAME;
    interest->entities.named.CMDataWriter.topicName =     V_CMDATAWRITERINFO_NAME;
    interest->entities.named.DCPSSubscription.topicName = V_SUBSCRIPTIONINFO_NAME;
    interest->entities.named.CMDataReader.topicName =     V_CMDATAREADERINFO_NAME;
    interest->entities.named.CMPublisher.topicName =      V_CMPUBLISHERINFO_NAME;
    interest->entities.named.CMSubscriber.topicName =     V_CMSUBSCRIBERINFO_NAME;

    for(i = 0; i < NB_SERVICE_NUM_ENTITIES; i++){
        struct nb_serviceInterestTopic *topic = &interest->entities.array[i];
        /* topicName must be set! */
        result = nb__serviceInterestTopicInit(topic, participant, waitset, interest->publisher, writerQos, interest->subscriber, readerQos);
        if(result != U_RESULT_OK){
            OS_REPORT(OS_FATAL, "nb__serviceInterestInit", result, "Could not create entities for topic %s: result %s", topic->topicName, u_resultImage(result));
            goto err_topic;
        }
    }

    interest->entities.named.DCPSTopic.action =            nb_dcpsTopicReaderAction;
    interest->entities.named.DCPSTopic.handleFunc =        nb__serviceHandleDcpsTopicFunc;
    interest->entities.named.DCPSParticipant.action =      nb_dcpsParticipantReaderAction;
    interest->entities.named.DCPSParticipant.handleFunc =  nb__serviceHandleDcpsParticipantFunc;
    interest->entities.named.CMParticipant.action =        nb_cmParticipantReaderAction;
    interest->entities.named.CMParticipant.handleFunc =    nb__serviceHandleCmParticipantFunc;
    interest->entities.named.DCPSPublication.action =      nb_dcpsPublicationReaderAction;
    interest->entities.named.DCPSPublication.handleFunc =  nb__serviceHandleDcpsPublicationFunc;
    interest->entities.named.CMDataWriter.action =         nb_cmWriterReaderAction;
    interest->entities.named.CMDataWriter.handleFunc =     nb__serviceHandleCmWriterFunc;
    interest->entities.named.DCPSSubscription.action =     nb_dcpsSubscriptionReaderAction;
    interest->entities.named.DCPSSubscription.handleFunc = nb__serviceHandleDcpsSubscriptionFunc;
    interest->entities.named.CMDataReader.action =         nb_cmReaderReaderAction;
    interest->entities.named.CMDataReader.handleFunc =     nb__serviceHandleCmReaderFunc;
    interest->entities.named.CMPublisher.action =          nb_cmPublisherReaderAction;
    interest->entities.named.CMPublisher.handleFunc =      nb__serviceHandleCmPublisherFunc;
    interest->entities.named.CMSubscriber.action =         nb_cmSubscriberReaderAction;
    interest->entities.named.CMSubscriber.handleFunc =     nb__serviceHandleCmSubscriberFunc;

    interest->entities.named.DCPSParticipant.alwaysForward = TRUE;
    interest->entities.named.CMParticipant.alwaysForward   = TRUE;
    interest->entities.named.CMPublisher.alwaysForward     = TRUE;
    interest->entities.named.CMSubscriber.alwaysForward    = TRUE;

    u_writerQosFree(writerQos);
    u_publisherQosFree(publisherQos);
    u_readerQosFree(readerQos);
    u_subscriberQosFree(subscriberQos);

    return os_resultSuccess;

/* Error handling */
err_topic:
    while(i-- > 0){
        nb__serviceInterestTopicDeinit(&interest->entities.array[i], waitset);
    }
    u_waitsetSetEventMask(waitset, mask);
err_eventmask:
    /* No undo for u_entityEnable(...) */
err_subscriberEnable:
    u_objectFree(u_object(interest->subscriber));
err_subscriberNew:
    u_objectFree(u_object(interest->publisher));
err_publisherNew:
    u_readerQosFree(readerQos);
err_readerQosNew:
    u_subscriberQosFree(subscriberQos);
err_subscriberQosNew:
    u_writerQosFree(writerQos);
err_writerQosNew:
    u_publisherQosFree(publisherQos);
err_publisherQosNew:
    return os_resultFail;
}


static void
nb__serviceInterestDeinit(
    struct nb_serviceInterest* interest,
    u_waitset waitset)
{
    c_ulong i;

    assert(interest);
    assert(waitset);

    for(i = 0; i < NB_SERVICE_NUM_ENTITIES; i++){
        nb__serviceInterestTopicDeinit(&interest->entities.array[i], waitset);
    }

    (void) u_objectFree(u_object(interest->publisher));
    (void) u_objectFree(u_object(interest->subscriber));
}

nb_service
nb__serviceNew(
    const os_char* uri,
    const os_char* serviceName)
{
    nb_service _this;
    v_publisherQos publisherQos;
    v_topicQos topicQos;
    v_writerQos writerQos;
    u_result uresult;
    c_long timeout = 3; /* TODO: Make this a sensible value from some config? */

    assert(serviceName);

    _this = os_malloc(sizeof *_this);

    /* Super-init */
    nb__objectInit(nb_object(_this), NB_OBJECT_SERVICE, (nb_objectDeinitFunc)nb__serviceDeinit);
    {
        /* Create the service object */
        _this->name = os_strdup(serviceName);

        u_userInitialise();

        _this->terminate = 0;

        /* Create the u_service object; this will attach to the DDS domain */
        _this->service = u_nwbridgeNew(uri, U_DOMAIN_ID_ANY, timeout, serviceName, NULL, TRUE);
        if (!_this->service) {
            /* Error reported by u_serviceNew */
            goto err_uservicenew;
        }

        /* Create the waitset */
        _this->ws = u_waitsetNew();
        if(!_this->ws){
            OS_REPORT(OS_FATAL, "nb_serviceNew", 0, "Could not create waitset.");
            goto err_waitsetnew;
        }

        uresult = u_waitsetSetEventMask(_this->ws, V_EVENT_TRIGGER);
        if (uresult != U_RESULT_OK) {
            OS_REPORT(OS_FATAL, "nb_serviceNew", uresult,
            "Could not set event mask for waitset: result %s",
            u_resultImage(uresult));
            goto err_service_eventmask;
        }

        uresult = u_waitsetAttach(_this->ws, u_observable(_this->service), _this->service);
        if(uresult != U_RESULT_OK){
            OS_REPORT(OS_FATAL, "nb_serviceNew", uresult,
                "Could not attach service to waitset: result %s",
                u_resultImage(uresult));
            goto err_waitsetattachService;
        }

        if (!os_serviceGetSingleProcess()) {
            /* As long as termination doesn't work correctly in SP, don't try to do cleanup. */
            _this->erh = os_signalHandlerRegisterExitRequestCallback(nb__exitRequestHandler, NULL, NULL, NULL, _this);
            _this->eh = os_signalHandlerRegisterExceptionCallback(nb__exceptionHandler, NULL, NULL, NULL, _this);
        } else {
            _this->erh = os_signalHandlerExitRequestHandleNil;
            _this->eh = os_signalHandlerExceptionHandleNil;
        }
    }

    {
        /* Create the publisher */
        publisherQos = u_publisherQosNew(NULL);
        if (!publisherQos) {
            OS_REPORT(OS_FATAL, "nb_serviceNew", 0, "Out of resources. Could not allocate default publisherQos.");
            goto err_pubqosnew;
        }
        os_free(publisherQos->partition.v);
        publisherQos->partition.v = os_strdup(NB_STATUS_PARTITION);

        /* Create the status-publisher */
        _this->publisher = u_publisherNew(u_participant(_this->service), NB_STATUS_PUBLISHER, publisherQos, TRUE);
        if (!_this->publisher) {
            /* Error reported by u_publisherNew */
            goto err_publisher;
        }
    }

    if(nb__serviceInterestInit(&_this->interest, u_participant(_this->service), _this->ws) != os_resultSuccess){
        /* Errors reported by nb__serviceInterestInit */
        goto err_serviceInterestInit;
    }

    { /* Inject the metadata and create the topics used by the service for status
       * changes and the API. This is done here for OSPL-1091; ease of use when
       * the C&M API is used. */
        c_bool result;

        /* Inject metadata */
        uresult = u_observableAction(u_observable(_this->service), nb__serviceLoadMetaData, &result);
        if (uresult != U_RESULT_OK) {
            OS_REPORT(OS_FATAL, "nb_serviceNew", 0,
                    "Unable to load module 'nwbridge', u_entityAction failed: '%s' (%d).",
                    u_resultImage(uresult) /* TODO: Rename u_resultImage; this is odd... */,
                    uresult);
            goto err_loadmeta;
        }
        if (!result) {
            OS_REPORT(OS_FATAL, "nb_serviceNew", 0,
                    "Unable to load module 'nwbridge', loadnwbridge failed.");
            goto err_loadmeta;
        }

        /* Initialize the topic-QoS for status topics */
        topicQos = u_topicQosNew(NULL);
        if (!topicQos) {
            OS_REPORT(OS_FATAL, "nb_serviceNew", 0,
                    "Out of resources. Could not allocate default topicQos.");
            goto err_topicqos_new;
        }

        /* Set proper QoS for state-based data */
        NB_SET_STATEBASED_TOPICQOS(topicQos);

        /* Create the serviceStatus topic */
        _this->topics.serviceStatus = u_topicNew(u_participant(_this->service),
                NB_SERVICE_STATUS_TOPIC_NAME,
                NB_SERVICE_STATUS_TOPIC_TYPE_NAME,
                NB_SERVICE_STATUS_TOPIC_KEY_LIST,
                topicQos);
        if (!_this->topics.serviceStatus) {
            /* Error reported by u_topicNew */
            goto err_serviceStatusTopic_new;
        }

    }

    {
       /* Create status writer */
       writerQos = u_writerQosNew(NULL);
       if(!writerQos){
           OS_REPORT(OS_FATAL, "nb_serviceNew", 0, "Out of resources. Could not allocate default writerQos.");
           goto err_writerqos_new;
       }

       /* Set proper QoS for state-based data */
       NB_SET_STATEBASED_WRITERQOS(writerQos);

       _this->statusWriter = u_writerNew(
                   _this->publisher,
                   NB_SERVICE_STATUS_WRITER,
                   _this->topics.serviceStatus,
                   writerQos);
       if(!_this->statusWriter){
           /* Error reported by u_writerNew */
           goto err_statuswriter;
       }
       u_writerEnable (_this->statusWriter);
    }

    /* Create the initial service configuration  */
    {
        _this->config = nb_configurationNew(_this);
        if (!_this->config) {
            OS_REPORT(OS_FATAL, "nb_serviceNew", 0,
                "Failed to initialize service configuration of Networking Bridge service '%s'",
                serviceName);
            goto err_confignew;
        }

        if (os_mutexInit(&(_this->configLock), NULL) != os_resultSuccess) {
            OS_REPORT(OS_FATAL, "nb_serviceNew", 0,
                "Failed to initialize mutex for configuration of service '%s'", serviceName);
            goto err_cfgmutexinit;
        }

        _this->includes = NULL;
        _this->excludes = NULL;
    }

    /* Open tracing file */
    {
        if (nb__serviceOpenTracingFile(_this) != os_resultSuccess) {
            /* error reported by nb_serviceOpenTracingFile */
            goto err_tracingopen;
        }
#if defined(OSPL_INNER_REV) && defined (OSPL_OUTER_REV)
        nb_log(LOG_INFO, "Started Networking Bridge Service (OpenSplice " OSPL_VERSION_STR ", build " OSPL_INNER_REV_STR "/" OSPL_OUTER_REV_STR ")\n");
#else
        nb_log(LOG_INFO, "Started Networking Bridge Service (OpenSplice " OSPL_VERSION_STR ", non-PrismTech build)\n");
#endif
        nb_configurationPrint(_this->config);
    }

    /* Create service lease renewal thread */
    {
        os_result result = nb_threadCreate("LeaseRenew", &(_this->leaseRenewalThr), nb__serviceUpdateLease, _this);
        if (result != os_resultSuccess) {
            NB_ERROR_1("nb_serviceNew", "Failed to create service lease renewal thread of service '%s'\n", serviceName);
            goto err_leasethread;
        }
    }

    /* Cleanup local resources */
    u_writerQosFree(writerQos);
    u_topicQosFree(topicQos);
    u_publisherQosFree(publisherQos);

    return _this;

    /************************************************/
err_leasethread:
err_tracingopen:
    os_mutexDestroy(&(_this->configLock));
err_cfgmutexinit:
    nb_configurationFree(_this->config);
err_confignew:
    u_objectFree(u_object(_this->statusWriter));
err_statuswriter:
    u_writerQosFree(writerQos);
    writerQos = NULL;
err_writerqos_new:
    u_objectFree(u_object(_this->topics.serviceStatus));
err_serviceStatusTopic_new:
    u_topicQosFree(topicQos);
err_topicqos_new:
    /* Unfortunately no undo for loading of a module */
err_loadmeta:
    nb__serviceInterestDeinit(&_this->interest, _this->ws);
err_serviceInterestInit:
    u_objectFree(u_object(_this->publisher));
err_publisher:
    /* publisherQos->partition is freed by u_publisherQosFree */
    u_publisherQosFree(publisherQos);
err_pubqosnew:
    os_signalHandlerUnregisterExceptionCallback(_this->eh);
    os_signalHandlerUnregisterExitRequestCallback(_this->erh);
    u_waitsetDetach(_this->ws, u_observable(nb_serviceService(_this)));
err_waitsetattachService:
    /* No undo needed for setting the event-mask */
err_service_eventmask:
    u_objectFree(u_object(_this->ws));
err_waitsetnew:
    u_objectFree(u_object(_this->service));
err_uservicenew:
    os_free(_this->name);
    nb__objectDeinit((nb_object) _this);
    os_free(_this);
    return NULL;
}

void
nb__serviceDeinit(
    nb_service _this)
{
    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);

    if (nb_threadJoin(_this->leaseRenewalThr, NULL) != os_resultSuccess) {
        NB_WARNING("nb__serviceDeinit",
            "Failed to join service lease renewal thread\n");
    }

    /* This is our last update on our status */
    nb__serviceChangeState(_this, SERVICE_TERMINATED);

    nb__serviceInterestDeinit(&_this->interest, _this->ws);

    u_objectFree(u_object(_this->statusWriter));
    u_objectFree(u_object(_this->publisher));

    u_objectFree(u_object(_this->topics.serviceStatus));

    /* Free configuration */
    nb_configurationFree(_this->config);

    { /* Free cached includes and excludes */
        if(_this->includes){
            int i = 0;
            while(_this->includes[i]){
                os_free(_this->includes[i]);
                i++;
            }
            os_free(_this->includes);
        }

        if(_this->excludes){
            int i = 0;
            while(_this->excludes[i]){
                os_free(_this->excludes[i]);
                i++;
            }
            os_free(_this->excludes);
        }
    }

    os_signalHandlerUnregisterExceptionCallback(_this->eh);
    os_signalHandlerUnregisterExitRequestCallback(_this->erh);

    u_waitsetDetach(_this->ws, u_observable(_this->service));

    if (u_objectFree_s(u_object(_this->ws)) != U_RESULT_OK) {
        NB_WARNING("nb__serviceDeinit",
            "Failed to free service waitset\n");
    }
    u_objectFree(u_object(_this->service));
    os_free(_this->name);

    /* Call super-deinit */
    nb__objectDeinit((nb_object) _this);
}

static os_result
nb__serviceOpenTracingFile(
    nb_service _this)
{
    os_result result;
    os_char *filename;
    const char *mode;
    nb_logConfig gc = nb_threadLogConfig(nb_threadLookup());

    nb_objectIsValidKind(_this, NB_OBJECT_SERVICE);

    result = os_resultSuccess;
    filename = nb_configurationTracingFileName(_this->config);
    gc->tracing.categories = nb_configurationTracingCategories(_this->config);

    if(!filename || gc->tracing.categories == LOG_NONE) {
        /* Tracing is not enabled */
        gc->tracing.file = NULL;
        gc->tracing.categories = 0;
    } else if (os_strcasecmp(filename, "stdout") == 0) {
        gc->tracing.file = stdout;
    } else if (os_strcasecmp(filename, "stderr") == 0) {
        gc->tracing.file = stderr;
    } else {
        mode = (nb_configurationTracingAppend(_this->config)) ? "a" : "w";
        gc->tracing.file = fopen(filename, mode);
        if (!gc->tracing.file) {
            const os_char *msg = os_getErrno() ? os_strError(os_getErrno()) : NULL;
            NB_ERROR_2("nb_serviceOpenTracingFile", "Cannot open tracing logfile '%s' (%s)\n",
                filename, msg ? msg : "unknown reason");
            result = os_resultFail;
        }
    }
    if(result == os_resultSuccess) {
        NB_TRACE(("Opened NetworkingBridge tracing file '%s'\n", filename));
    }
    return result;
}

static void*
nb__serviceUpdateLease(
    c_voidp arg /* nb_service */)
{
    nb_service _this;
    os_duration expiry;
    os_duration sleep;

    nb_objectIsValidKind(arg, NB_OBJECT_SERVICE);

    _this = nb_service(arg);

    expiry = nb_configurationLeaseExpiryTime(_this->config);
    sleep = nb_configurationLeaseUpdateInterval(_this->config);
    NB_TRACE(("Started service-lease renewal thread\n"));

    while(!_this->terminate && !nb_proc_must_exit) {
        u_serviceRenewLease(nb_serviceService(_this), expiry);
        ospl_os_sleep(sleep);
    }

    /* Set longer expiry for during termination */
    expiry = OS_DURATION_INIT(20, 0);
    u_serviceRenewLease(nb_serviceService(_this), expiry);

    NB_TRACE(("Terminating service-lease renewal thread\n"));

    return NULL;
}
