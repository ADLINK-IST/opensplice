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
#ifndef NB__TOPIC_H
#define NB__TOPIC_H

#include "kernelModule.h"

#include "nb__object.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifndef NDEBUG
#define nb_dcpsTopic(o) (assert(nb__objectKind(nb_object(o)) == NB_TOPIC_OBJECT_DCPS_TOPIC), (nb_dcpsTopic)(o))
#else
#define nb_dcpsTopic(o) ((nb_dcpsTopic)(o))
#endif

nb_dcpsTopic                nb_dcpsTopicNew(void) __attribute_returns_nonnull__
                                                  __attribute_malloc__;

nb_dcpsTopic                nb_dcpsTopicAlloc(void) __attribute_returns_nonnull__
                                                    __attribute_malloc__;

void                        nb_dcpsTopicInit(nb_dcpsTopic _this) __nonnull_all__;

void                        nb_dcpsTopicDeinit(nb_object _this) __nonnull_all__;

const c_char *              nb_dcpsTopicTopicName(nb_dcpsTopic _this) __nonnull_all__;

const v_builtinTopicKey *   nb_dcpsTopicKey(nb_dcpsTopic _this)
                                                    __nonnull_all__
                                                    __attribute_const__
                                                    __attribute_returns_nonnull__;

v_actionResult              nb_dcpsTopicReaderAction(c_object o,
                                                     c_voidp copyArg /* c_iter<nb_topicObject> * */)
                                                __nonnull((2));

#ifndef NDEBUG
void                        nb_dcpsTopicFree (nb_dcpsTopic _this);
#else
#define                     nb_dcpsTopicFree(s) nb_objectFree(s)
#endif

#ifndef NDEBUG
#define nb_dcpsParticipant(o) (assert(nb__objectKind(nb_object(o)) == NB_TOPIC_OBJECT_DCPS_PARTICIPANT), (nb_dcpsParticipant)(o))
#else
#define nb_dcpsParticipant(o) ((nb_dcpsParticipant)(o))
#endif

nb_dcpsParticipant                nb_dcpsParticipantNew(void) __attribute_returns_nonnull__
                                                              __attribute_malloc__;

nb_dcpsParticipant                nb_dcpsParticipantAlloc(void) __attribute_returns_nonnull__
                                                                __attribute_malloc__;

void                              nb_dcpsParticipantInit(nb_dcpsParticipant _this) __nonnull_all__;

void                              nb_dcpsParticipantDeinit(nb_object _this) __nonnull_all__;

const v_builtinTopicKey *         nb_dcpsParticipantKey(nb_dcpsParticipant _this)
                                                    __nonnull_all__
                                                    __attribute_const__
                                                    __attribute_returns_nonnull__;

v_actionResult                    nb_dcpsParticipantReaderAction(c_object o,
                                                     c_voidp copyArg /* c_iter<nb_topicObject> * */)
                                                __nonnull((2));

#ifndef NDEBUG
void                              nb_dcpsParticipantFree (nb_dcpsParticipant _this);
#else
#define                           nb_dcpsParticipantFree(s) nb_objectFree(s)
#endif

#ifndef NDEBUG
#define nb_cmParticipant(o) (assert(nb__objectKind(nb_object(o)) == NB_TOPIC_OBJECT_CM_PARTICIPANT), (nb_cmParticipant)(o))
#else
#define nb_cmParticipant(o) ((nb_cmParticipant)(o))
#endif

nb_cmParticipant                nb_cmParticipantNew(void) __attribute_returns_nonnull__
                                                          __attribute_malloc__;

nb_cmParticipant                nb_cmParticipantAlloc(void) __attribute_returns_nonnull__
                                                            __attribute_malloc__;

void                            nb_cmParticipantInit(nb_cmParticipant _this) __nonnull_all__;

void                            nb_cmParticipantDeinit(nb_object _this) __nonnull_all__;

const v_builtinTopicKey *       nb_cmParticipantKey(nb_cmParticipant _this)
                                                    __nonnull_all__
                                                    __attribute_const__
                                                    __attribute_returns_nonnull__;

v_actionResult                  nb_cmParticipantReaderAction(c_object o,
                                                     c_voidp copyArg /* c_iter<nb_topicObject> * */)
                                                __nonnull((2));

#ifndef NDEBUG
void                            nb_cmParticipantFree (nb_cmParticipant _this);
#else
#define                         nb_cmParticipantFree(s) nb_objectFree(s)
#endif


#ifndef NDEBUG
#define nb_dcpsSubscription(o) (assert(nb__objectKind(nb_object(o)) == NB_TOPIC_OBJECT_DCPS_SUBSCRIPTION), (nb_dcpsSubscription)(o))
#else
#define nb_dcpsSubscription(o) ((nb_dcpsSubscription)(o))
#endif

nb_dcpsSubscription         nb_dcpsSubscriptionNew(void) __attribute_returns_nonnull__
                                                         __attribute_malloc__;

nb_dcpsSubscription         nb_dcpsSubscriptionAlloc(void) __attribute_returns_nonnull__
                                                           __attribute_malloc__;

void                        nb_dcpsSubscriptionInit(nb_dcpsSubscription _this) __nonnull_all__;

void                        nb_dcpsSubscriptionDeinit(nb_object _this) __nonnull_all__;

const c_char *              nb_dcpsSubscriptionTopicName(nb_dcpsSubscription _this) __nonnull_all__;

const v_builtinTopicKey *   nb_dcpsSubscriptionKey(nb_dcpsSubscription _this)
                                                    __nonnull_all__
                                                    __attribute_const__
                                                    __attribute_returns_nonnull__;

const v_builtinTopicKey *   nb_dcpsSubscriptionParticipantKey(nb_dcpsSubscription _this)
                                                    __nonnull_all__
                                                    __attribute_const__
                                                    __attribute_returns_nonnull__;

const char * const *        nb_dcpsSubscriptionPartitions(nb_dcpsSubscription _this,
                                                          c_ulong *len) __nonnull_all__;

c_bool                      nb_dcpsSubscriptionGetInterested(
                                                    nb_dcpsSubscription _this)
                                                __nonnull_all__;

void                        nb_dcpsSubscriptionSetInterested(
                                                    nb_dcpsSubscription _this,
                                                    const char * const * includes,
                                                    const char * const * excludes)
                                                __nonnull_all__;

v_actionResult              nb_dcpsSubscriptionReaderAction(c_object o,
                                                            c_voidp copyArg /* c_iter<nb_topicObject> * */)
                                                    __nonnull((2));

#ifndef NDEBUG
void                        nb_dcpsSubscriptionFree (nb_dcpsSubscription _this);
#else
#define                     nb_dcpsSubscriptionFree(s) nb_objectFree(s)
#endif

#ifndef NDEBUG
#define nb_dcpsPublication(o) (assert(nb__objectKind(nb_object(o)) == NB_TOPIC_OBJECT_DCPS_PUBLICATION), (nb_dcpsPublication)(o))
#else
#define nb_dcpsPublication(o) ((nb_dcpsPublication)(o))
#endif

nb_dcpsPublication      nb_dcpsPublicationNew(void) __attribute_returns_nonnull__
                                                    __attribute_malloc__;

nb_dcpsPublication      nb_dcpsPublicationAlloc(void) __attribute_returns_nonnull__
                                                      __attribute_malloc__;

void                    nb_dcpsPublicationInit(nb_dcpsPublication _this) __nonnull_all__;

void                    nb_dcpsPublicationDeinit(nb_object _this) __nonnull_all__;

const c_char *          nb_dcpsPublicationTopicName(nb_dcpsPublication _this) __nonnull_all__;

const v_builtinTopicKey * nb_dcpsPublicationKey(nb_dcpsPublication _this)
                                                    __nonnull_all__
                                                    __attribute_const__
                                                    __attribute_returns_nonnull__;

const v_builtinTopicKey * nb_dcpsPublicationParticipantKey(nb_dcpsPublication _this)
                                                    __nonnull_all__
                                                    __attribute_const__
                                                    __attribute_returns_nonnull__;

const char * const *    nb_dcpsPublicationPartitions(nb_dcpsPublication _this,
                                                     c_ulong *len) __nonnull_all__;

c_bool                  nb_dcpsPublicationGetInterested(nb_dcpsPublication _this) __nonnull_all__;

void                    nb_dcpsPublicationSetInterested(nb_dcpsPublication _this,
                                                        const char * const * includes,
                                                        const char * const * excludes) __nonnull_all__;

v_actionResult          nb_dcpsPublicationReaderAction(c_object o,
                                                       c_voidp copyArg /* c_iter<nb_topicObject> * */)
                                                __nonnull((2));

#ifndef NDEBUG
void                    nb_dcpsPublicationFree (nb_dcpsPublication _this);
#else
#define                 nb_dcpsPublicationFree(s) nb_objectFree(s)
#endif

#ifndef NDEBUG
#define nb_cmReader(o) (assert(nb__objectKind(nb_object(o)) == NB_TOPIC_OBJECT_CM_READER), (nb_cmReader)(o))
#else
#define nb_cmReader(o) ((nb_cmReader)(o))
#endif

nb_cmReader                 nb_cmReaderNew(void) __attribute_returns_nonnull__
                                                 __attribute_malloc__;

nb_cmReader                 nb_cmReaderAlloc(void) __attribute_returns_nonnull__
                                                   __attribute_malloc__;

void                        nb_cmReaderInit(nb_cmReader _this) __nonnull_all__;

void                        nb_cmReaderDeinit(nb_object _this) __nonnull_all__;

const v_builtinTopicKey *   nb_cmReaderKey(nb_cmReader _this) __nonnull_all__
                                                              __attribute_const__
                                                              __attribute_returns_nonnull__;

const v_builtinTopicKey *   nb_cmReaderSubscriberKey(nb_cmReader _this)
                                                              __nonnull_all__
                                                              __attribute_const__
                                                              __attribute_returns_nonnull__;

v_actionResult              nb_cmReaderReaderAction(c_object o,
                                                    c_voidp copyArg /* c_iter<nb_topicObject> * */)
                                                __nonnull((2));

#ifndef NDEBUG
void                        nb_cmReaderFree (nb_cmReader _this);
#else
#define                     nb_cmReaderFree(s) nb_objectFree(s)
#endif

#ifndef NDEBUG
#define nb_cmWriter(o) (assert(nb__objectKind(nb_object(o)) == NB_TOPIC_OBJECT_CM_WRITER), (nb_cmWriter)(o))
#else
#define nb_cmWriter(o) ((nb_cmWriter)(o))
#endif

nb_cmWriter                 nb_cmWriterNew(void) __attribute_returns_nonnull__
                                                 __attribute_malloc__;

nb_cmWriter                 nb_cmWriterAlloc(void) __attribute_returns_nonnull__
                                                   __attribute_malloc__;

void                        nb_cmWriterInit(nb_cmWriter _this) __nonnull_all__;

void                        nb_cmWriterDeinit(nb_object _this) __nonnull_all__;

const v_builtinTopicKey *   nb_cmWriterKey(nb_cmWriter _this) __nonnull_all__
                                                              __attribute_const__
                                                              __attribute_returns_nonnull__;

const v_builtinTopicKey *   nb_cmWriterPublisherKey(nb_cmWriter _this) __nonnull_all__
                                                                       __attribute_const__
                                                                       __attribute_returns_nonnull__;

v_actionResult              nb_cmWriterReaderAction(c_object o,
                                                    c_voidp copyArg /* c_iter<nb_topicObject> * */)
                                                __nonnull((2));

#ifndef NDEBUG
void                        nb_cmWriterFree (nb_cmWriter _this);
#else
#define                     nb_cmWriterFree(s) nb_objectFree(s)
#endif


#ifndef NDEBUG
#define nb_cmPublisher(o) (assert(nb__objectKind(nb_object(o)) == NB_TOPIC_OBJECT_CM_PUBLISHER), (nb_cmPublisher)(o))
#else
#define nb_cmPublisher(o) ((nb_cmPublisher)(o))
#endif

nb_cmPublisher            nb_cmPublisherNew(void) __attribute_returns_nonnull__
                                                  __attribute_malloc__;

nb_cmPublisher            nb_cmPublisherAlloc(void) __attribute_returns_nonnull__
                                                    __attribute_malloc__;

void                      nb_cmPublisherInit(nb_cmPublisher _this) __nonnull_all__;

void                      nb_cmPublisherDeinit(nb_object _this) __nonnull_all__;

const v_builtinTopicKey * nb_cmPublisherKey(nb_cmPublisher _this) __nonnull_all__
                                                                  __attribute_const__
                                                                  __attribute_returns_nonnull__;

const v_builtinTopicKey * nb_cmPublisherParticipantKey(nb_cmPublisher _this)
                                                                  __nonnull_all__
                                                                  __attribute_const__
                                                                  __attribute_returns_nonnull__;

v_actionResult            nb_cmPublisherReaderAction(c_object o,
                                                     c_voidp copyArg /* c_iter<nb_topicObject> * */)
                                                __nonnull((2));

#ifndef NDEBUG
void                      nb_cmPublisherFree (nb_cmPublisher _this);
#else
#define                   nb_cmPublisherFree(s) nb_objectFree(s)
#endif


#ifndef NDEBUG
#define nb_cmSubscriber(o) (assert(nb__objectKind(nb_object(o)) == NB_TOPIC_OBJECT_CM_SUBSCRIBER), (nb_cmSubscriber)(o))
#else
#define nb_cmSubscriber(o) ((nb_cmSubscriber)(o))
#endif

nb_cmSubscriber           nb_cmSubscriberNew(void) __attribute_returns_nonnull__
                                                   __attribute_malloc__;

nb_cmSubscriber           nb_cmSubscriberAlloc(void) __attribute_returns_nonnull__
                                                     __attribute_malloc__;

void                      nb_cmSubscriberInit(nb_cmSubscriber _this) __nonnull_all__;

void                      nb_cmSubscriberDeinit(nb_object _this) __nonnull_all__;

const v_builtinTopicKey * nb_cmSubscriberKey(nb_cmSubscriber _this) __nonnull_all__
                                                                    __attribute_const__
                                                                    __attribute_returns_nonnull__;

v_actionResult            nb_cmSubscriberReaderAction(c_object o,
                                                c_voidp copyArg /* c_iter<nb_topicObject> * */)
                                            __nonnull((2));

#ifndef NDEBUG
void                      nb_cmSubscriberFree (nb_cmSubscriber _this);
#else
#define                   nb_cmSubscriberFree(s) nb_objectFree(s)
#endif

#if defined (__cplusplus)
}
#endif

#endif /* NB__TOPIC_H */
