/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include <string.h>
#include "u_user.h"
#include "u_observable.h"
#include "Entity.h"
#include "DomainParticipant.h"
#include "Constants.h"
#include "QosUtils.h"
#include "StatusUtils.h"
#include "ReportUtils.h"
#include "Publisher.h"
#include "Subscriber.h"
#include "TopicDescription.h"
#include "Topic.h"
#include "MultiTopic.h"
#include "ContentFilteredTopic.h"
#include "TypeSupportMetaHolder.h"
#include "ListenerDispatcher.h"
#include "SequenceUtils.h"
#include "MiscUtils.h"
#include "dds_builtinTopicsSplDcps.h"
#include "c_stringSupport.h"
#include "ListenerDispatcher.h"



DDS::OpenSplice::DomainParticipant::DomainParticipant() :
    DDS::OpenSplice::Entity(DDS::OpenSplice::DOMAINPARTICIPANT),
    factory(NULL),
    defaultPublisherQos(PUBLISHER_QOS_DEFAULT),
    defaultSubscriberQos(SUBSCRIBER_QOS_DEFAULT),
    defaultTopicQos(TOPIC_QOS_DEFAULT),
    builtinSubscriber(NULL),
    publisherList(new ObjSet(TRUE)),
    subscriberList(new ObjSet(TRUE)),
    topicList(new ObjSet(TRUE)),
    cfTopicList(new ObjSet(TRUE)),
    multiTopicList(new ObjSet(TRUE)),
    builtinTopicList(new ObjSet(TRUE)),
    typeMetaHolders(new StrObjMap(TRUE)),
    factoryAutoEnable(FALSE),
    listenerInterest(0)
{
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::init(
    DDS::DomainParticipantFactory_ptr factory,
    DDS::DomainId_t domainId,
    const DDS::DomainParticipantQos &qos)
{
    DDS::ReturnCode_t result;

    result = this->nlReq_init(factory, domainId, qos);
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::nlReq_init(
    DDS::DomainParticipantFactory_ptr factory,
    DDS::DomainId_t domainId,
    const DDS::DomainParticipantQos &qos)
{
    DDS::ReturnCode_t result;
    u_participantQos pQos = NULL;
    u_participant uParticipant;
    c_long timeout = 1;
    char *participantName;

    /* Check and copy the QoS to a user layer QoS. */
    result = DDS::OpenSplice::Utils::qosIsConsistent(qos);
    if (result == DDS::RETCODE_OK) {
        pQos = u_participantQosNew(NULL);
        if (pQos == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
        } else {
            result = DDS::OpenSplice::Utils::copyQosIn(qos, pQos);
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = topicList->init();
    }

    if (result == DDS::RETCODE_OK) {
        result = cfTopicList->init();
    }

    if (result == DDS::RETCODE_OK) {
        result = multiTopicList->init();
    }

    if (result == DDS::RETCODE_OK) {
        result = builtinTopicList->init();
    }

    if (result == DDS::RETCODE_OK) {
        result = typeMetaHolders->init();
    }

    if (result == DDS::RETCODE_OK) {
        result = publisherList->init();
    }

    if (result == DDS::RETCODE_OK) {
        result = subscriberList->init();
    }

    if (result == DDS::RETCODE_OK) {
        /* Create the user layer object and initialize this entity with it. */
        participantName = u_userGetProcessName();
        uParticipant = u_participantNew(NULL, domainId, timeout, participantName, pQos, FALSE);
        if (uParticipant != NULL) {
            result = DDS::OpenSplice::Entity::nlReq_init(u_entity(uParticipant));
            setDomainId(u_participantGetDomainId(uParticipant));
        } else {
            result = DDS::RETCODE_ERROR;
        }
        os_free(participantName);
    }

    if (result == DDS::RETCODE_OK) {
        this->factory = factory;
        this->factoryAutoEnable = qos.entity_factory.autoenable_created_entities;
    }

    /* Create, init and set the listener dispatcher. */
    if (result == DDS::RETCODE_OK) {
        cmn_listenerDispatcher dispatcher;
        os_schedClass schedulingClass;
        os_int32 schedulingPriority;

        schedulingClass = DDS::OpenSplice::ListenerDispatcher::scheduling_class (
            qos.listener_scheduling);
        schedulingPriority = DDS::OpenSplice::ListenerDispatcher::scheduling_priority (
            qos.listener_scheduling);

        dispatcher = cmn_listenerDispatcher_new (
            uParticipant,
            schedulingClass,
            schedulingPriority,
            &DDS::OpenSplice::ListenerDispatcher::event_handler,
            NULL, OS_TRUE);

        if (dispatcher != NULL) {
            /* We are in the init, so only one thread will access this,
             * meaning we don't really have to lock (despite the wlReq). */
            this->wlReq_set_listenerDispatcher(dispatcher);
        } else {
            result = DDS::RETCODE_ERROR;
        }
    }

    /* Cleanup */
    if (pQos != NULL) {
        u_participantQosFree(pQos);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::wlReq_deinit()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    /* Only deinit when there's no contained entities. */
    if (publisherList->getNrElements() != 0) {
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
        CPP_REPORT(result, "DomainParticipant still contains '%d' Publisher entities.",
            publisherList->getNrElements());
    } else if (subscriberList->getNrElements() != 0) {
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
        CPP_REPORT(result, "DomainParticipant still contains '%d' Subscriber entities.",
            subscriberList->getNrElements());
    } else if (topicList->getNrElements() != 0) {
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
        CPP_REPORT(result, "DomainParticipant still contains '%d' Topic entities.",
            topicList->getNrElements());
    } else if (cfTopicList->getNrElements() != 0) {
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
        CPP_REPORT(result, "DomainParticipant still contains '%d' ContentFilteredTopic entities.",
            cfTopicList->getNrElements());
    } else if (multiTopicList->getNrElements() != 0) {
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
        CPP_REPORT(result, "DomainParticipant still contains '%d' MultiTopic entities.",
            multiTopicList->getNrElements());
    } else {
        this->disable_callbacks();

        /* Cleanup dispatcher. */
        cmn_listenerDispatcher dispatcher = this->rlReq_get_listenerDispatcher();
        this->wlReq_set_listenerDispatcher(NULL);
        result = cmn_listenerDispatcher_free (dispatcher);
    }

    if (result == DDS::RETCODE_OK) {
        /* Cleanup builtin entities. */
        result = this->wlReq_deleteBuiltinSubscriber();
    }

    if (result == DDS::RETCODE_OK) {
        /* Cleanup empty lists. */
        delete publisherList;
        publisherList = NULL;

        delete subscriberList;
        subscriberList = NULL;

        delete typeMetaHolders;
        typeMetaHolders = NULL;

        delete topicList;
        topicList = NULL;

        delete cfTopicList;
        cfTopicList = NULL;

        delete multiTopicList;
        multiTopicList = NULL;

        delete builtinTopicList;
        builtinTopicList = NULL;

        /* This will also free the user layer object. */
        result = DDS::OpenSplice::Entity::wlReq_deinit();
    }

    return result;
}

DDS::OpenSplice::DomainParticipant::~DomainParticipant()
{
}

DDS::Boolean
DDS::OpenSplice::DomainParticipant::rlReq_fnFindTopicDescription(
    DDS::Object_ptr td,
    struct findObjectArg *arg)
{
    DDS::Boolean result = TRUE;

    DDS::OpenSplice::TopicDescription *index = dynamic_cast<DDS::OpenSplice::TopicDescription *>(td);
    assert (index != NULL);
    if (strcmp(index->topic_name.in(), arg->key) == 0) {
        arg->match = index;
        result = FALSE;
    }
    return result;
}

DDS::OpenSplice::TopicDescription *
DDS::OpenSplice::DomainParticipant::rlReq_findTopicDescription(
    const char *topicName)
{
    struct findObjectArg arg;
    DDS::Boolean notFound;

    arg.key = topicName;
    arg.match = NULL;

    /* Look in regular topics */
    notFound = this->topicList->walk(
        (DDS::OpenSplice::ObjSet::ObjSetActionFunc)rlReq_fnFindTopicDescription,
        &arg);
    if (notFound) {
        /* Look in content-filtered topics */
        assert(!arg.match);
        notFound = this->cfTopicList->walk(
            (DDS::OpenSplice::ObjSet::ObjSetActionFunc)rlReq_fnFindTopicDescription,
            &arg);
        if (notFound) {
            /* Look in multi topics */
            assert(!arg.match);
            notFound = this->multiTopicList->walk(
                (DDS::OpenSplice::ObjSet::ObjSetActionFunc)rlReq_fnFindTopicDescription,
                &arg);
            if (notFound) {
                /* Look in builtin topics */
                assert(!arg.match);
                notFound = this->builtinTopicList->walk(
                    (DDS::OpenSplice::ObjSet::ObjSetActionFunc)rlReq_fnFindTopicDescription,
                    &arg);
            }
        }
    }

    return dynamic_cast<DDS::OpenSplice::TopicDescription *>(arg.match);
}

const DDS::OpenSplice::TypeSupportMetaHolder *
DDS::OpenSplice::DomainParticipant::wlReq_insertMetaHolder(
    const char *typeName,
    DDS::OpenSplice::TypeSupportMetaHolder *tsMetaHolder)
{
    return dynamic_cast<const DDS::OpenSplice::TypeSupportMetaHolder *>(
            typeMetaHolders->insertElement(typeName, tsMetaHolder));
}

DDS::Boolean
DDS::OpenSplice::DomainParticipant::rlReq_fnFindMetaHolderByInternalTypeName(
    const char *key,
    DDS::Object_ptr mh,
    struct findObjectArg *arg)
{
    DDS::Boolean result = TRUE;
    OS_UNUSED_ARG(key);

    DDS::OpenSplice::TypeSupportMetaHolder *index = dynamic_cast<DDS::OpenSplice::TypeSupportMetaHolder *>(mh);
    assert(index);

    if ((strcmp(arg->key, index->get_internal_type_name()) == 0) ){
        arg->match = index;
        result = FALSE;
    }

    return result;
}

DDS::OpenSplice::TypeSupportMetaHolder *
DDS::OpenSplice::DomainParticipant::rlReq_findMetaHolderByInternalTypeName(
    const char *idlTypeName)
{
    struct findObjectArg arg;

    arg.key   = idlTypeName;
    arg.match = NULL;

    typeMetaHolders->walk((DDS::OpenSplice::StrObjMap::StrObjMapActionFunc) rlReq_fnFindMetaHolderByInternalTypeName, &arg);

    DDS::Object::_duplicate(arg.match);
    return dynamic_cast<DDS::OpenSplice::TypeSupportMetaHolder *>(arg.match);
}

DDS::OpenSplice::TypeSupportMetaHolder *
DDS::OpenSplice::DomainParticipant::rlReq_findMetaHolder(
    const char *typeName)
{
    return dynamic_cast<DDS::OpenSplice::TypeSupportMetaHolder *>(
            typeMetaHolders->findElement(typeName));
}


DDS::Boolean
DDS::OpenSplice::DomainParticipant::wlReq_insertPublisher(
    DDS::OpenSplice::Publisher *pub)
{
    return publisherList->insertElement(pub);
}

DDS::Boolean
DDS::OpenSplice::DomainParticipant::wlReq_removePublisher(
    DDS::OpenSplice::Publisher *pub)
{
    return publisherList->removeElement(pub);
}

DDS::Boolean
DDS::OpenSplice::DomainParticipant::wlReq_insertSubscriber(
    DDS::OpenSplice::Subscriber *sub)
{
    return subscriberList->insertElement(sub);
}

DDS::Boolean
DDS::OpenSplice::DomainParticipant::wlReq_removeSubscriber(
    DDS::OpenSplice::Subscriber *sub)
{
    return subscriberList->removeElement(sub);
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::nlReq_builtinTopicRegisterTypeSupport()
{
    DDS::ParticipantBuiltinTopicDataTypeSupport_var  dpTS;
    DDS::TopicBuiltinTopicDataTypeSupport_var        tTS;
    DDS::PublicationBuiltinTopicDataTypeSupport_var  pTS;
    DDS::SubscriptionBuiltinTopicDataTypeSupport_var sTS;
    DDS::ReturnCode_t result;

    dpTS = new DDS::ParticipantBuiltinTopicDataTypeSupport();
    if (dpTS.in() != NULL) {
        result = dpTS->register_type(this, NULL);
    } else {
        result = DDS::RETCODE_OUT_OF_RESOURCES;
    }

    if (result == DDS::RETCODE_OK) {
        tTS = new DDS::TopicBuiltinTopicDataTypeSupport();
        if (tTS.in() != NULL) {
            result = tTS->register_type(this, NULL);
        } else {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
        }
    }

    if (result == DDS::RETCODE_OK) {
        pTS = new DDS::PublicationBuiltinTopicDataTypeSupport();
        if (pTS.in() != NULL) {
            result = pTS->register_type(this, NULL);
        } else {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
        }
    }

    if (result == DDS::RETCODE_OK)  {
        sTS = new DDS::SubscriptionBuiltinTopicDataTypeSupport();
        if (sTS.in() != NULL) {
            result = sTS->register_type(this, NULL);
        } else {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
        }
    }

    return result;
}

DDS::Publisher_ptr
DDS::OpenSplice::DomainParticipant::create_publisher (
    const DDS::PublisherQos &qos,
    DDS::PublisherListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    const DDS::PublisherQos *tmpQos;
    DDS::OpenSplice::Publisher *pub = NULL;
    DDS::Boolean inserted = FALSE;
    os_char *name;

    CPP_REPORT_STACK();

    result = DDS::OpenSplice::Utils::qosIsConsistent(qos);
    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
    }

    if (result == DDS::RETCODE_OK) {
        /* Determine which QoS to use (prevent deep copy). */
        if (&qos == &PUBLISHER_QOS_DEFAULT) {
            tmpQos = &(this->defaultPublisherQos);
        } else {
            tmpQos = &qos;
        }

        /* Create, initialize and store the Publisher object. */
        pub = new DDS::OpenSplice::Publisher();
        if (pub == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not create Publisher.");
        }

        if (result == DDS::RETCODE_OK) {
            name = this->rlReq_getChildName("publisher");
            result = pub->init(this, name, *tmpQos);
            os_free(name);
            if (result == DDS::RETCODE_OK) {
                /* Duplicate and store the created Publisher object. */
                inserted = wlReq_insertPublisher(pub);
                assert(inserted);
            }
        }

        if (result == DDS::RETCODE_OK) {
            pub->wlReq_set_listenerDispatcher(this->rlReq_get_listenerDispatcher());
            result = pub->set_listener(a_listener, mask);
        }

        if (result == DDS::RETCODE_OK) {
            if (this->factoryAutoEnable) {
                result = pub->enable();
            }
            if (result != DDS::RETCODE_OK) {
                (void)pub->set_listener(NULL, 0);
                pub->wlReq_set_listenerDispatcher(NULL);
            }
        }

        if (result != DDS::RETCODE_OK) {
            /* After a failure: remove the created object
             * from the list, deinit() it and destroy it . */
            if (inserted) {
                DDS::Boolean removed = wlReq_removePublisher(pub);
                assert(removed);
                OS_UNUSED_ARG(removed);
            }
            (void) pub->deinit();
            DDS::release(pub);
            pub = NULL;
        }

        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return pub;
}


DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::delete_publisher (
    DDS::Publisher_ptr p
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::Publisher *pub;

    CPP_REPORT_STACK();

    if (p == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "publisher '<NULL>' is invalid.");
    }

    if (result == DDS::RETCODE_OK) {
        pub = dynamic_cast<DDS::OpenSplice::Publisher*>(p);
        if (pub == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "Publisher is invalid, not of type '%s'.",
                "DDS::OpenSplice::Publisher");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
    }

    if (result == DDS::RETCODE_OK) {
        /* Be sure that this Publisher is created by this Participant. */
        if (wlReq_removePublisher(pub) == FALSE) {
            result = DDS::RETCODE_PRECONDITION_NOT_MET;
            CPP_REPORT(result, "Publisher not created by DomainParticipant.");
        }

        if (result == DDS::RETCODE_OK) {
            /* Destroy the Publisher. */
            (void)pub->set_listener(NULL, 0);
            result = pub->deinit();
            if (result != DDS::RETCODE_OK) {
                if (result == DDS::RETCODE_PRECONDITION_NOT_MET) {
                    /* Re-store when the deinit didn't work. */
                    DDS::Boolean reinserted = wlReq_insertPublisher(pub);
                    assert(reinserted);
                    OS_UNUSED_ARG(reinserted);
                }
            }
        }

        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}


DDS::Subscriber_ptr
DDS::OpenSplice::DomainParticipant::create_subscriber (
    const DDS::SubscriberQos &qos,
    DDS::SubscriberListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    const DDS::SubscriberQos *tmpQos;
    DDS::OpenSplice::Subscriber *sub = NULL;
    DDS::Boolean inserted = FALSE;
    os_char *name;

    CPP_REPORT_STACK();

    result = DDS::OpenSplice::Utils::qosIsConsistent(qos);
    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
    }

    if (result == DDS::RETCODE_OK) {
        /* Determine which QoS to use (prevent deep copy). */
        if (&qos == &(SUBSCRIBER_QOS_DEFAULT)) {
            tmpQos = &(this->defaultSubscriberQos);
        } else {
            tmpQos = &qos;
        }

        /* Create, initialize and store the Subscriber object. */
        sub = new DDS::OpenSplice::Subscriber();
        if (sub == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not create Subscriber.");
        }

        if (result == DDS::RETCODE_OK) {
            name = this->rlReq_getChildName("subscriber");
            result = sub->init(this, name, *tmpQos);
            os_free(name);
            if (result == DDS::RETCODE_OK) {
                /* Duplicate and store the created Subscriber object. */
                inserted = wlReq_insertSubscriber(sub);
                assert(inserted);
            }
        }

        if (result == DDS::RETCODE_OK) {
            sub->wlReq_set_listenerDispatcher(this->rlReq_get_listenerDispatcher());
            result = sub->set_listener(a_listener, mask);
        }

        if (result == DDS::RETCODE_OK) {
            /* Now check factory enable policy and enable the subscriber */
            if (this->factoryAutoEnable) {
                result = sub->enable();
            }
            if (result != DDS::RETCODE_OK) {
                (void)sub->set_listener(NULL, 0);
                sub->wlReq_set_listenerDispatcher(NULL);
            }
        }

        if (result != DDS::RETCODE_OK) {
            /* After a failure: remove the created object
             * from the list, deinit() it and destroy it . */
            if (inserted) {
                DDS::Boolean removed = wlReq_removeSubscriber(sub);
                assert(removed);
                OS_UNUSED_ARG(removed);
            }
            (void) sub->deinit();
            DDS::release(sub);
            sub = NULL;
        }

        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return sub;
}


DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::delete_subscriber (
    DDS::Subscriber_ptr s
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::Subscriber *sub;

    CPP_REPORT_STACK();

    if (s == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "subscriber '<NULL>' is invalid.");
    }

    if (result == DDS::RETCODE_OK) {
        sub = dynamic_cast<DDS::OpenSplice::Subscriber*>(s);
        if (sub == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "subscriber is invalid, not of type '%s'.",
                "DDS::OpenSplice::Subscriber");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
    }

    if (result == DDS::RETCODE_OK) {
        /* The builtin subscriber is a special case. */
        if (sub == this->builtinSubscriber) {
            result = this->wlReq_deleteBuiltinSubscriber();
        } else {
            /* Be sure that this Subscriber is created by this Participant. */
            if (this->wlReq_removeSubscriber(sub) == FALSE) {
                result = DDS::RETCODE_PRECONDITION_NOT_MET;
                CPP_REPORT(result, "Subscriber not created by DomainParticipant.");
            }

            if (result == DDS::RETCODE_OK) {
                /* Destroy the Subscriber. */
                (void)sub->set_listener(NULL, 0);
                result = sub->deinit();
                if (result != DDS::RETCODE_OK) {
                    if (result == DDS::RETCODE_PRECONDITION_NOT_MET) {
                        /* Re-store when the deinit didn't work. */
                        DDS::Boolean reinserted = wlReq_insertSubscriber(sub);
                        assert(reinserted);
                        OS_UNUSED_ARG(reinserted);
                    }
                }
            }
        }

        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::Subscriber_ptr
DDS::OpenSplice::DomainParticipant::get_builtin_subscriber (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::Subscriber_ptr sub = NULL;
    DDS::SubscriberQos sQos;

    /* Use _var object to automatically release the builtin readers. */
    DDS::DataReader_var reader;

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        if (this->rlReq_is_enabled()) {
            if (this->builtinSubscriber == NULL) {
                this->nlReq_initBuiltinSubscriberQos(&sQos);

                /* Create, initialize and store the Subscriber object. */
                this->builtinSubscriber = new DDS::OpenSplice::Subscriber();
                if (this->builtinSubscriber == NULL) {
                    result = DDS::RETCODE_OUT_OF_RESOURCES;
                    CPP_REPORT(result, "Could not create Subscriber.");
                }
                if (result == DDS::RETCODE_OK) {
                    result = this->builtinSubscriber->init(this, "BuiltinSubscriber", sQos);
                    if (result == DDS::RETCODE_OK) {
                        this->builtinSubscriber->wlReq_set_listenerDispatcher(this->rlReq_get_listenerDispatcher());
                        if (this->factoryAutoEnable) {
                            result = this->builtinSubscriber->enable();
                        }
                    }

                    if (result != DDS::RETCODE_OK) {
                        delete this->builtinSubscriber;
                        this->builtinSubscriber = NULL;
                    }
                }
            }
            if (result == DDS::RETCODE_OK) {
                sub = DDS::Subscriber::_duplicate(this->builtinSubscriber);
            }
        } else {
            result = DDS::RETCODE_NOT_ENABLED,
            CPP_REPORT(result, "DomainParticipant is disabled.");
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return sub;
}


DDS::Topic_ptr
DDS::OpenSplice::DomainParticipant::create_topic (
    const char * topic_name,
    const char * type_name,
    const DDS::TopicQos &qos,
    DDS::TopicListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::Topic_ptr topic = NULL;

    CPP_REPORT_STACK();

    if (topic_name == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "topic_name '<NULL>' is invalid.");
    } else if (type_name == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "type_name '<NULL>' is invalid.");
    } else {
        topic = this->nlReq_createTopic(topic_name,
                                        type_name,
                                        qos,
                                        a_listener,
                                        mask,
                                        *(this->topicList));
        if (topic) {
            /* set participant listener interest */
            DDS::OpenSplice::set_topic_listener_mask(topic, &listenerInterest);
        } else {
            result = DDS::RETCODE_PRECONDITION_NOT_MET;
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return topic;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::delete_topic (
    DDS::Topic_ptr a_topic
) THROW_ORB_EXCEPTIONS
{
    DDS::OpenSplice::Topic *topic;
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    CPP_REPORT_STACK();

    if (a_topic == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_topic '<NULL>' is invalid.");
    } else {
        /* Cast to Topic* to be able to remove the listeners. */
        topic = dynamic_cast<DDS::OpenSplice::Topic *>(a_topic);
        if (topic == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "a_topic is invalid, not of type '%s'.",
                "DDS::OpenSplice::Topic");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            result = topic->write_lock();
            if (result == DDS::RETCODE_OK) {
                if (topic->rlReq_getNrUsers() > 0) {
                    result = DDS::RETCODE_PRECONDITION_NOT_MET;
                    CPP_REPORT(result, "Topic is still in use.");
                }

                if (result == DDS::RETCODE_OK) {
                    /* Remove and destroy the TopicDescription (map decreases refCount). */
                    DDS::Boolean removed = this->topicList->removeElement(topic);
                    if (removed) {
                        (void)topic->wlReq_set_listener(NULL, 0);
                        result = topic->wlReq_deinit();
                        if (result != DDS::RETCODE_OK) {
                            if (result == DDS::RETCODE_PRECONDITION_NOT_MET) {
                                /* Re-store when the deinit didn't work. */
                                DDS::Boolean firstTime = this->topicList->insertElement(topic);
                                assert(firstTime);
                                OS_UNUSED_ARG(firstTime);
                            }
                        }
                    } else {
                        result = DDS::RETCODE_PRECONDITION_NOT_MET;
                        CPP_REPORT(result, "Topic not registered to DomainParticipant.");
                    }
                }
                topic->unlock();
            } else if (result == DDS::RETCODE_ALREADY_DELETED) {
                result = DDS::RETCODE_PRECONDITION_NOT_MET;
            }

            this->unlock();
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

static bool
keyCompare(
    const char *key1,
    const char *key2)
{
    c_iter key1Names, key2Names;
    c_char *name1, *name2;
    bool consistent;

    key1Names = c_splitString(key1,", \t");
    key2Names = c_splitString(key2,", \t");

    consistent = (c_iterLength(key1Names) == c_iterLength(key2Names));
    if (consistent) {
        name1 = (c_char *)c_iterTakeFirst(key1Names);
        name2 = (c_char *)c_iterTakeFirst(key2Names);
        while ((name1 != NULL) && (name2 != NULL) && consistent) {
            consistent = (strcmp(name1,name2) == 0);
            os_free(name1);
            os_free(name2);
            if (consistent) {
                name1 = (c_char *)c_iterTakeFirst(key1Names);
                name2 = (c_char *)c_iterTakeFirst(key2Names);
            }
        }
    }
    name1 = (c_char *)c_iterTakeFirst(key1Names);
    while (name1 != NULL) {
        os_free(name1);
        name1 = (c_char *)c_iterTakeFirst(key1Names);
    }
    c_iterFree(key1Names);
    name2 = (c_char *)c_iterTakeFirst(key2Names);
    while (name2 != NULL) {
        os_free(name2);
        name2 = (c_char *)c_iterTakeFirst(key2Names);
    }
    c_iterFree(key2Names);

    return consistent;
}


DDS::Topic_ptr
DDS::OpenSplice::DomainParticipant::find_topic (
    const char *topic_name,
    const DDS::Duration_t &timeout
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::String_var type_name;
    c_iter list = NULL;
    u_topic uTopic = NULL;
    os_char *uTypeName = NULL;
    os_duration duration;
    u_participant uParticipant = NULL;
    DDS::OpenSplice::Topic *topic = NULL;
    DDS::OpenSplice::TypeSupportMetaHolder *typeMetaHolder = NULL;
    DDS::TypeSupport_var typeSupport;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        if (topic_name == NULL || strchr(topic_name, '*') || strchr(topic_name, '?')) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "topic_name '%s' is invalid.",
                    topic_name ? topic_name : "<NULL>");
        } else {
            result = DDS::OpenSplice::Utils::durationIsValid(timeout);
            if (result == DDS::RETCODE_OK) {
                result = DDS::OpenSplice::Utils::copyDurationIn(timeout, duration);
            }
        }
    }

    if (result == DDS::RETCODE_OK) {
        /* No lock because user entity attribute is valid as long this DomainParticipant exists
         * and because searching can take quite a long time.
         */
        uParticipant = u_participant(this->rlReq_get_user_entity());
        if (uParticipant != NULL) {
            /* Find the Topic in the user layer. */
            list = u_participantFindTopic(uParticipant, topic_name, duration);
            if (c_iterLength(list) != 0) {
                assert(c_iterLength(list) == 1); /* Accept zero or one topic. */
                uTopic = u_topic(c_iterTakeFirst(list));
                if (uTopic == NULL) {
                    result = DDS::RETCODE_ERROR;
                }
            } else {
                result = DDS::RETCODE_PRECONDITION_NOT_MET;
                CPP_REPORT(result, "Failed to resolve Topic \"%s\".", topic_name?topic_name:"NULL");
            }
            c_iterFree(list);
            list = NULL;
        } else {
            result = DDS::RETCODE_ERROR;
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
    }

    if (result == DDS::RETCODE_OK) {
        if (uTopic != NULL) {
            /* Find the related typeMetaHolder for this topic. */
            uTypeName = u_topicTypeName(uTopic);
            type_name = DDS::string_dup(uTypeName);
            os_free(uTypeName);
            typeMetaHolder = rlReq_findMetaHolderByInternalTypeName(type_name);

            if (typeMetaHolder == NULL) {
                /* There is not TypeSupport registered for this Topic.
                 * Now check if a builtin Topic is requested, in that case the
                 * TypeSupport class is available and can be registered along the road.
                 */
                if (strcmp(topic_name, "DCPSParticipant") == 0) {
                    typeSupport = new DDS::ParticipantBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "DCPSTopic") == 0) {
                    typeSupport = new DDS::TopicBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "DCPSPublication") == 0) {
                    typeSupport = new DDS::PublicationBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "DCPSSubscription") == 0) {
                    typeSupport = new DDS::SubscriptionBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "CMParticipant") == 0) {
                    typeSupport = new DDS::CMParticipantBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "CMPublisher") == 0) {
                    typeSupport = new DDS::CMPublisherBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "CMSubscriber") == 0) {
                    typeSupport = new DDS::CMSubscriberBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "CMDataWriter") == 0) {
                    typeSupport = new DDS::CMDataWriterBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "CMDataReader") == 0) {
                    typeSupport = new DDS::CMDataReaderBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "DCPSType") == 0) {
                    typeSupport = new DDS::TypeBuiltinTopicDataTypeSupport();
                } else {
                    typeSupport = NULL;
                }
                if (typeSupport && typeSupport.in()) {
                    DDS::OpenSplice::TypeSupport *ts = dynamic_cast<DDS::OpenSplice::TypeSupport *>(typeSupport.in());
                    if (ts) {
                        typeMetaHolder = ts->get_metaHolder();
                        type_name = typeSupport->get_type_name();
                        result = wlReq_load_type_support_meta_holder(typeMetaHolder, type_name);
                    } else {
                        result = DDS::RETCODE_ERROR;
                    }
                }
            } else {
                /* Release reference to typeMetaHolder, as long as this DomainParticipant
                 * exists and typeMetaHolder is used within lock a reference will always
                 * exist within the DomainParticipant. */
                DDS::release(typeMetaHolder);
            }

            if (result == DDS::RETCODE_OK && typeMetaHolder != NULL) {
                const char * typeKeyList = typeMetaHolder->get_key_list();
                char * topicKeyList = u_topicKeyExpr(uTopic);
                if (typeKeyList == NULL || topicKeyList == NULL) {
                    if (typeKeyList != topicKeyList) {
                        CPP_REPORT_WARNING(
                                "TypeSupport (%s) key \"%s\" doesn't match Topic (%s) key \"%s\".",
                                type_name.in(), typeKeyList?typeKeyList:"NULL",
                                topic_name, topicKeyList?topicKeyList:"NULL");
                    }
                } else if (keyCompare(typeKeyList, topicKeyList) == OS_FALSE) {
                    CPP_REPORT_WARNING(
                            "TypeSupport (%s) key \"%s\" doesn't match Topic (%s) key \"%s\".",
                            type_name.in(), typeKeyList, topic_name, topicKeyList);
                }
                os_free(topicKeyList);
            }

            if (result == DDS::RETCODE_OK) {
                /*
                 * typeMetaHolder == NULL is allowed when creating a topic by means of a find.
                 *
                 * The Topic will start off with no typeMetaHolder.
                 * When the related TypeSupport is registered, the typeMetaHolder of it will be
                 * stored in this participant.
                 * When, after that, a DataReader is created, then participant::find_type_support_factory()
                 * will be called and the result will be added to the related Topic, which was
                 * supplied with the create_reader() call.
                 * If the type wasn't registered, then the DataReader creation will fail.
                 */
                /* Create, initialize and store the Topic object. */
                topic = new DDS::OpenSplice::Topic();
                if (topic == NULL) {
                    result = DDS::RETCODE_OUT_OF_RESOURCES;
                    CPP_REPORT(result, "Could not create Topic '%s'.", topic_name);
                }
                if (result == DDS::RETCODE_OK) {
                    result = topic->init(uTopic, this, topic_name, type_name, typeMetaHolder);
                    if (result == DDS::RETCODE_OK) {
                        /* Store the created Topic object (map performs the duplicate). */
                        DDS::Boolean firstTime = this->topicList->insertElement(topic);
                        assert(firstTime);
                        OS_UNUSED_ARG(firstTime);
                        /* Set the listener dispatcher. */
                        topic->wlReq_set_listenerDispatcher(this->rlReq_get_listenerDispatcher());
                    } else {
                        /* Destroy created object after init failure. */
                        DDS::release(topic);
                        topic = NULL;
                    }
                }
            }
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return topic;
}



DDS::Topic_ptr
DDS::OpenSplice::DomainParticipant::find_builtin_topic (
    const char *topic_name
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::String_var type_name;
    c_iter list = NULL;
    u_topic uTopic = NULL;
    os_char *uTypeName = NULL;
    u_participant uParticipant = NULL;
    DDS::OpenSplice::Topic *topic = NULL;
    DDS::OpenSplice::TypeSupportMetaHolder *typeMetaHolder = NULL;
    DDS::TypeSupport_var typeSupport;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        if (topic_name == NULL || strchr(topic_name, '*') || strchr(topic_name, '?')) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "topic name '%s' is invalid.",
                    topic_name ? topic_name : "<NULL>");
        }
    }

    if (result == DDS::RETCODE_OK) {
        /* No lock because user entity attribute is valid as long this DomainParticipant exists
         * and because searching can take quite a long time.
         */
        uParticipant = u_participant(this->rlReq_get_user_entity());
        if (uParticipant != NULL) {
            /* Find the Topic in the user layer. */
            list = u_participantFindTopic(uParticipant, topic_name, 0);
            if (c_iterLength(list) != 0) {
                assert(c_iterLength(list) == 1); /* Accept zero or one topic. */
                uTopic = u_topic(c_iterTakeFirst(list));
                if (uTopic == NULL) {
                    result = DDS::RETCODE_ERROR;
                }
            } else {
                result = DDS::RETCODE_PRECONDITION_NOT_MET;
                CPP_REPORT(result, "Failed to resolve Topic \"%s\".", topic_name?topic_name:"NULL");
            }
            c_iterFree(list);
            list = NULL;
        } else {
            result = DDS::RETCODE_ERROR;
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
    }

    if (result == DDS::RETCODE_OK) {
        if (uTopic != NULL) {
            /* Find the related typeMetaHolder for this topic. */
            uTypeName = u_topicTypeName(uTopic);
            type_name = DDS::string_dup(uTypeName);
            os_free(uTypeName);
            typeMetaHolder = rlReq_findMetaHolderByInternalTypeName(type_name);
            if (typeMetaHolder == NULL) {
                /* There is not TypeSupport registered for this Topic.
                 * Now check if a builtin Topic is requested, in that case the
                 * TypeSupport class is available and can be registered along the road.
                 */
                if (strcmp(topic_name, "DCPSParticipant") == 0) {
                    typeSupport = new DDS::ParticipantBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "DCPSTopic") == 0) {
                    typeSupport = new DDS::TopicBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "DCPSPublication") == 0) {
                    typeSupport = new DDS::PublicationBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "DCPSSubscription") == 0) {
                    typeSupport = new DDS::SubscriptionBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "CMParticipant") == 0) {
                    typeSupport = new DDS::CMParticipantBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "CMPublisher") == 0) {
                    typeSupport = new DDS::CMPublisherBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "CMSubscriber") == 0) {
                    typeSupport = new DDS::CMSubscriberBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "CMDataWriter") == 0) {
                    typeSupport = new DDS::CMDataWriterBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "CMDataReader") == 0) {
                    typeSupport = new DDS::CMDataReaderBuiltinTopicDataTypeSupport();
                } else if (strcmp(topic_name, "DCPSType") == 0) {
                    typeSupport = new DDS::TypeBuiltinTopicDataTypeSupport();
                } else {
                    typeSupport = NULL;
                }
                if (typeSupport && typeSupport.in()) {
                    DDS::OpenSplice::TypeSupport *ts = dynamic_cast<DDS::OpenSplice::TypeSupport *>(typeSupport.in());
                    if (ts) {
                        typeMetaHolder = ts->get_metaHolder();
                        type_name = typeSupport->get_type_name();
                        result = wlReq_load_type_support_meta_holder(typeMetaHolder, type_name);
                    } else {
                        result = DDS::RETCODE_ERROR;
                    }
                }
            } else {
                /* Release reference to typeMetaHolder, as long as this DomainParticipant
                 * exists and typeMetaHolder is used within lock a reference will always
                 * exist within the DomainParticipant. */
                DDS::release(typeMetaHolder);
            }

            if (result == DDS::RETCODE_OK) {

                /*
                 * typeMetaHolder == NULL is allowed when creating a topic by means of a find.
                 *
                 * The Topic will start off with no typeMetaHolder.
                 * When the related TypeSupport is registered, the typeMetaHolder of it will be
                 * stored in this participant.
                 * When, after that, a DataReader is created, then participant::find_type_support_factory()
                 * will be called and the result will be added to the related Topic, which was
                 * supplied with the create_reader() call.
                 * If the type wasn't registered, then the DataReader creation will fail.
                 */
                /* Create, initialize and store the Topic object. */
                topic = new DDS::OpenSplice::Topic();
                if (topic == NULL) {
                    result = DDS::RETCODE_OUT_OF_RESOURCES;
                    CPP_REPORT(result, "Could not create Topic '%s'.", topic_name);
                }
                if (result == DDS::RETCODE_OK) {
                    result = topic->init(uTopic, this, topic_name, type_name, typeMetaHolder);
                    if (result == DDS::RETCODE_OK) {
                        /* Store the created Topic object (map performs the duplicate). */
                        DDS::Boolean firstTime = this->builtinTopicList->insertElement(topic);
                        assert(firstTime);
                        OS_UNUSED_ARG(firstTime);
                        /* Set the listener dispatcher. */
                        topic->wlReq_set_listenerDispatcher(this->rlReq_get_listenerDispatcher());
                    } else {
                        /* Destroy created object after init failure. */
                        DDS::release(topic);
                        topic = NULL;
                    }
                }
            }
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return topic;
}

DDS::TopicDescription_ptr
DDS::OpenSplice::DomainParticipant::lookup_topicdescription (
    const char * name
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::TopicDescription *td = NULL;

    CPP_REPORT_STACK();

    if (name == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "name '<NULL>' is invalid.");
    } else {
        result = this->read_lock();
    }

    if (result == DDS::RETCODE_OK) {
        td = rlReq_findTopicDescription(name);
        (void) DDS::TopicDescription::_duplicate(td); /* Allowed also on nil references. */
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return td;
}

DDS::ContentFilteredTopic_ptr
DDS::OpenSplice::DomainParticipant::create_contentfilteredtopic (
    const char * name,
    DDS::Topic_ptr related_t,
    const char * filter_expression,
    const DDS::StringSeq &filter_parameters
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::Topic *related_topic;
    DDS::OpenSplice::ContentFilteredTopic *topic = NULL;

    CPP_REPORT_STACK();

    if (name == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "name '<NULL>' is invalid.");
    } else if (related_t == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "related_topic '<NULL>' is invalid.");
    } else if (filter_expression == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "filter_expression '<NULL>' is invalid.");
    }

    if (result == DDS::RETCODE_OK) {
        related_topic = dynamic_cast<DDS::OpenSplice::Topic*>(related_t);
        if (related_topic == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "related_topic is invalid, not of type '%s'.",
                "DDS::OpenSplice::Topic");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
    }

    if (result == DDS::RETCODE_OK) {
        if (this->rlReq_findTopicDescription(name) == NULL) {
            /* Create, initialize and store the ContentFilteredTopic object. */
            topic = new DDS::OpenSplice::ContentFilteredTopic();
            if (topic == NULL) {
                result = DDS::RETCODE_OUT_OF_RESOURCES;
                CPP_REPORT(result, "Could not create ContentFilteredTopic.");
            } else {
                result = topic->init(this, name, related_topic, filter_expression, filter_parameters);
                if (result == DDS::RETCODE_OK) {
                    /* Store the created ContentFilteredTopic object (map performs the duplicate). */
                    DDS::Boolean firstTime = this->cfTopicList->insertElement(topic);
                    assert(firstTime);
                    OS_UNUSED_ARG(firstTime);
                } else {
                    /* Destroy created object after init failure. */
                    DDS::release(topic);
                    topic = NULL;
                }
            }
        } else {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "Topic '%s' already exists.", name);
        }

        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return topic;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::delete_contentfilteredtopic (
    DDS::ContentFilteredTopic_ptr a_contentfilteredtopic
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::ContentFilteredTopic *topic;

    CPP_REPORT_STACK();

    if (a_contentfilteredtopic == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_contentfilteredtopic '<NULL>' is invalid.");
    }

    if (result == DDS::RETCODE_OK) {
        topic = dynamic_cast<DDS::OpenSplice::ContentFilteredTopic*>(a_contentfilteredtopic);
        if (topic == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "a_contentfilteredtopic is invalid, not of type '%s'.",
                "DDS::OpenSplice::ContentFilteredTopic");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
    }

    if (result == DDS::RETCODE_OK) {
        if (!this->cfTopicList->removeElement(topic)) {
            result = DDS::RETCODE_PRECONDITION_NOT_MET;
            CPP_REPORT(result, "ContentFilteredTopic not registered to DomainParticipant.");
        } else {
            result = topic->deinit();
            if (result != DDS::RETCODE_OK) {
                if (result == DDS::RETCODE_PRECONDITION_NOT_MET) {
                    DDS::Boolean reinserted = this->cfTopicList->insertElement(topic);
                    assert(reinserted);
                    OS_UNUSED_ARG(reinserted);
                }
            }
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::MultiTopic_ptr
DDS::OpenSplice::DomainParticipant::create_multitopic (
    const char * name,
    const char * type_name,
    const char * subscription_expression,
    const DDS::StringSeq &expression_parameters
) THROW_ORB_EXCEPTIONS
{
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(type_name);
    OS_UNUSED_ARG(subscription_expression);
    OS_UNUSED_ARG(expression_parameters);
    CPP_REPORT(DDS::RETCODE_UNSUPPORTED, "create_multitopic is not yet supported.");
    return NULL;

#if 0
    DDS::ReturnCode_t result;
    DDS::OpenSplice::MultiTopic *topic = NULL;

    result = DDS::RETCODE_OK;

    if ((name == NULL) ||
        (type_name == NULL) ||
        (subscription_expression == NULL))
    {
        result = DDS::RETCODE_BAD_PARAMETER;
        OS_REPORT(OS_ERROR,
                  "DomainParticipant::create_multitopic", 0,
                  "Bad parameters");
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
        if (result != DDS::RETCODE_OK) {
            OS_REPORT(OS_ERROR,
                        "DomainParticipant::create_multitopic", 0,
                        "Failed to claim participant [%s]",
                        DDS::OpenSplice::Utils::returnCodeToString(result));
        }
    }

    if (result == DDS::RETCODE_OK) {
        /* Create, initialize and store the MultiTopic object. */
        topic = new DDS::OpenSplice::MultiTopic();
        if (topic == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            OS_REPORT(OS_ERROR,
                      "DomainParticipant::create_multitopic", 0,
                      "Failed to create topic");
        } else {
            result = topic->init(this, name, type_name, subscription_expression, expression_parameters);
            if (result == DDS::RETCODE_OK) {
                /* Store the created MultiTopic object (map performs the duplicate). */
                DDS::Boolean firstTime = this->multiTopicList->insertElement(topic);
                assert(firstTime);
                OS_UNUSED_ARG(firstTime);
            } else {
                DDS::release(topic);
                topic = NULL;
                OS_REPORT(OS_ERROR,
                          "DomainParticipant::create_multitopic", 0,
                          "Failed to initialize topic");
            }
        }

        this->unlock();
    }

    return topic;
#endif
}


DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::delete_multitopic (
    DDS::MultiTopic_ptr a_multitopic
) THROW_ORB_EXCEPTIONS
{
    OS_UNUSED_ARG(a_multitopic);
    CPP_REPORT(DDS::RETCODE_UNSUPPORTED, "delete_multitopic is not yet supported.");
    return DDS::RETCODE_UNSUPPORTED;

#if 0
    DDS::ReturnCode_t result;
    DDS::OpenSplice::MultiTopic *topic;

    result = DDS::RETCODE_OK;

    /* Cast to MultiTopic* because that is what's stored. */
    topic = dynamic_cast<DDS::OpenSplice::MultiTopic *>(a_multitopic);
    if (topic == NULL)
    {
        result = DDS::RETCODE_BAD_PARAMETER;
        OS_REPORT(OS_ERROR,
                  "DomainParticipant::delete_multitopic", 0,
                  "No topic given");
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
        if (result != DDS::RETCODE_OK) {
            OS_REPORT(OS_ERROR,
                        "DomainParticipant::delete_multitopic", 0,
                        "Failed to claim participant [%s]",
                        DDS::OpenSplice::Utils::returnCodeToString(result));
        }
    }

    if (result == DDS::RETCODE_OK) {
        /* Remove and destroy the MultiTopic (map decreases refCount). */
        DDS::Boolean removed = this->multiTopicList->removeElement(topic);
        if (removed) {
            result = topic->deinit();
            if (result != DDS::RETCODE_OK) {
                /* Restore when the deinit didn't work. */
                DDS::Boolean firstTime = this->multiTopicList->insertElement(topic);
                assert(firstTime);
                OS_UNUSED_ARG(firstTime);
                OS_REPORT(OS_ERROR,
                            "DomainParticipant::delete_multitopic", 0,
                            "Deinitializing topic failed [%s]",
                            DDS::OpenSplice::Utils::returnCodeToString(result));
            }
        } else {
            result = DDS::RETCODE_PRECONDITION_NOT_MET;
            OS_REPORT(OS_ERROR,
                        "DomainParticipant::delete_multitopic", 0,
                        "MultiTopic not found in this participant. [%s]",
                        DDS::OpenSplice::Utils::returnCodeToString(result));
        }

        this->unlock();
    }

    return result;
#endif
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::delete_contained_entities (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = wlReq_deleteContainedEntities();
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::set_qos (
    const DDS::DomainParticipantQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::DomainParticipantQos setQos;
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    u_result uResult;
    u_participantQos uQos = NULL;
    u_participant uParticipant = NULL;
    cmn_listenerDispatcher dispatcher;
    os_schedClass schedulingClass, oldSchedulingClass;
    os_int32 schedulingPriority, oldSchedulingPriority;

    CPP_REPORT_STACK();

    /* Get default qos when needed, otherwise check consistency. */
    if (&qos == &PARTICIPANT_QOS_DEFAULT) {
        result = this->factory->get_default_participant_qos(setQos);
    } else {
        result = DDS::OpenSplice::Utils::qosIsConsistent(qos);
        if (result == DDS::RETCODE_OK) {
            setQos = qos;
        }
    }

    /* Copy the QoS into a user layer QoS. */
    if (result == DDS::RETCODE_OK) {
        uQos = u_participantQosNew(NULL);
        if (uQos == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not copy DomainParticipantQos.");
        } else {
            result = DDS::OpenSplice::Utils::copyQosIn(setQos, uQos);
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
    }

    if (result == DDS::RETCODE_OK) {
        dispatcher = this->rlReq_get_listenerDispatcher ();
        schedulingClass = ListenerDispatcher::scheduling_class (
            setQos.listener_scheduling);
        schedulingPriority = ListenerDispatcher::scheduling_priority (
            setQos.listener_scheduling);
        cmn_listenerDispatcher_get_scheduling (
            dispatcher, &oldSchedulingClass, &oldSchedulingPriority);

        result = cmn_listenerDispatcher_set_scheduling (
            dispatcher, schedulingClass, schedulingPriority);
        if (result == DDS::RETCODE_OK) {
            /* Set the QoS on the user layer participant object. */
            uParticipant = u_participant(this->rlReq_get_user_entity());
            uResult = u_participantSetQos(uParticipant, uQos);
            result = uResultToReturnCode(uResult);

            if (result == DDS::RETCODE_OK) {
                /* Remember the possible new autoEnable flag. */
                this->factoryAutoEnable = setQos.entity_factory.autoenable_created_entities;
            } else {
                CPP_REPORT(result, "Could not apply DomainParticipantQos.");

                if (OS_RETCODE_OK != cmn_listenerDispatcher_set_scheduling (
                        dispatcher, schedulingClass, schedulingPriority))
                {
                    CPP_REPORT (
                        DDS::RETCODE_ERROR, "Could not revert scheduling changes.");
                }
            }
        }

        this->unlock();
    }

    /* Cleanup */
    if (uQos != NULL) {
        u_participantQosFree(uQos);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::get_qos (
    DDS::DomainParticipantQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_result uResult;
    u_participantQos uQos = NULL;
    u_participant uParticipant;

    CPP_REPORT_STACK();

    if (&qos == &PARTICIPANT_QOS_DEFAULT) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'PARTICIPANT_QOS_DEFAULT' is read-only.");
    } else {
        result = this->check ();
    }

    if (result == DDS::RETCODE_OK) {
        /* Get the QoS from the user layer participant object. */
        uParticipant = u_participant(this->rlReq_get_user_entity());
        uResult = u_participantGetQos(uParticipant, &uQos);
        if (uResult == U_RESULT_OK) {
            result = DDS::OpenSplice::Utils::copyQosOut(uQos, qos);
            u_participantQosFree(uQos);
        } else {
            result = this->uResultToReturnCode(uResult);
            CPP_REPORT(result, "Could not copy DomainParticipantQos.");
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::set_listener (
    DDS::DomainParticipantListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = DDS::OpenSplice::Entity::nlReq_set_listener(a_listener, mask);
    if (result == DDS::RETCODE_OK) {
        listenerInterest = mask & (DDS::INCONSISTENT_TOPIC_STATUS | DDS::ALL_DATA_DISPOSED_TOPIC_STATUS);
        topicList->walk(set_topic_listener_mask, &listenerInterest);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::DomainParticipantListener_ptr
DDS::OpenSplice::DomainParticipant::get_listener (
) THROW_ORB_EXCEPTIONS
{
    DDS::DomainParticipantListener_ptr listener;

    listener = dynamic_cast<DDS::DomainParticipantListener_ptr>(
        DDS::OpenSplice::Entity::nlReq_get_listener());
    return listener;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::ignore_participant (
    DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    OS_UNUSED_ARG(handle);
    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = DDS::RETCODE_UNSUPPORTED;
        this->unlock();
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::ignore_topic (
    DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    OS_UNUSED_ARG(handle);
    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = DDS::RETCODE_UNSUPPORTED;
        this->unlock();
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::ignore_publication (
    DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    OS_UNUSED_ARG(handle);
    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = DDS::RETCODE_UNSUPPORTED;
        this->unlock();
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::ignore_subscription (
    DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    OS_UNUSED_ARG(handle);
    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = DDS::RETCODE_UNSUPPORTED;
        this->unlock();
    }
    return result;
}

DDS::DomainId_t
DDS::OpenSplice::DomainParticipant::get_domain_id (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::DomainId_t domainId = DDS::DOMAIN_ID_INVALID;
    u_domainId_t uDomainId;
    u_participant uParticipant;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uParticipant = u_participant(this->rlReq_get_user_entity());
        assert (uParticipant != NULL);
        uDomainId = u_participantGetDomainId(uParticipant);
        if (uDomainId != U_DOMAIN_ID_INVALID) {
            domainId = uDomainId;
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return domainId;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::assert_liveliness (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_result uresult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uresult = u_participantAssertLiveliness(u_participant(this->rlReq_get_user_entity()));
        result = this->uResultToReturnCode(uresult);
        if (result != DDS::RETCODE_OK) {
            CPP_REPORT(result, "Could not assert liveliness.");
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::set_default_publisher_qos (
    const DDS::PublisherQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::qosIsConsistent(qos);
        if (result == DDS::RETCODE_OK) {
            /* deep copy */
            this->defaultPublisherQos = qos;
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::get_default_publisher_qos (
    DDS::PublisherQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    if (&qos == &PUBLISHER_QOS_DEFAULT) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'PUBLISHER_QOS_DEFAULT' is read-only.");
    } else {
        result = this->read_lock ();
    }

    if (result == DDS::RETCODE_OK) {
        /* deep copy */
        qos = this->defaultPublisherQos;
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::set_default_subscriber_qos (
    const DDS::SubscriberQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::qosIsConsistent(qos);
        if (result == DDS::RETCODE_OK) {
            /* deep copy */
            this->defaultSubscriberQos = qos;
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::get_default_subscriber_qos (
    DDS::SubscriberQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    if (&qos == &SUBSCRIBER_QOS_DEFAULT) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'SUBSCRIBER_QOS_DEFAULT' is read-only.");
    } else {
        result = this->read_lock ();
    }

    if (result == DDS::RETCODE_OK) {
        /* deep copy */
        qos = this->defaultSubscriberQos;
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::set_default_topic_qos (
    const DDS::TopicQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::qosIsConsistent(qos);
        if (result == DDS::RETCODE_OK) {
            /* deep copy */
            this->defaultTopicQos = qos;
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::get_default_topic_qos (
    DDS::TopicQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    if (&qos == &TOPIC_QOS_DEFAULT) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'TOPIC_QOS_DEFAULT' is read-only.");
    } else {
        result = this->read_lock ();
    }

    if (result == DDS::RETCODE_OK) {
        /* deep copy */
        qos = this->defaultTopicQos;
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}


DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::get_discovered_participants (
    DDS::InstanceHandleSeq &participant_handles
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->nlReq_getDiscoveredEntities(
        "DCPSParticipant",
        "DDS::ParticipantBuiltinTopicData",
        participant_handles);

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::get_discovered_participant_data (
    DDS::ParticipantBuiltinTopicData &participant_data,
    DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->nlReq_getDiscoveredData<
                        DDS::ParticipantBuiltinTopicData,
                        DDS::ParticipantBuiltinTopicDataSeq,
                        DDS::ParticipantBuiltinTopicDataDataReader>(
                                   "DCPSParticipant",
                                   "DDS::ParticipantBuiltinTopicData",
                                   participant_data,
                                   handle);

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::get_discovered_topics (
    DDS::InstanceHandleSeq &topic_handles
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->nlReq_getDiscoveredEntities("DCPSTopic",
                                               "DDS::TopicBuiltinTopicData",
                                               topic_handles);

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::get_discovered_topic_data (
    DDS::TopicBuiltinTopicData &topic_data,
    DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->nlReq_getDiscoveredData<
                        DDS::TopicBuiltinTopicData,
                        DDS::TopicBuiltinTopicDataSeq,
                        DDS::TopicBuiltinTopicDataDataReader>(
                                   "DCPSTopic",
                                   "DDS::TopicBuiltinTopicData",
                                   topic_data,
                                   handle);

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::Boolean
DDS::OpenSplice::DomainParticipant::contains_entity (
    DDS::InstanceHandle_t a_handle
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::Boolean proceed = TRUE;

    CPP_REPORT_STACK();

    result = this->read_lock ();
    if (result == DDS::RETCODE_OK) {
        proceed = publisherList->walk(
                (DDS::OpenSplice::ObjSet::ObjSetActionFunc) DDS::OpenSplice::DomainParticipant::rlReq_checkHandlePublisher,
                &a_handle);
        if (proceed) {
            proceed = subscriberList->walk(
                    (DDS::OpenSplice::ObjSet::ObjSetActionFunc) DDS::OpenSplice::DomainParticipant::rlReq_checkHandleSubscriber,
                    &a_handle);
        }
        if (proceed) {
            proceed = topicList->walk(
                    (DDS::OpenSplice::ObjSet::ObjSetActionFunc) DDS::OpenSplice::DomainParticipant::rlReq_checkHandleTopic,
                    &a_handle);
        }

        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    /* If we still have to proceed with the search, then the entity is not available. */
    return !proceed;
}


DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::get_current_time (
    DDS::Time_t &current_time
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        DDS::OpenSplice::Utils::copyTimeOut(os_timeWGet(), current_time);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::delete_historical_data (
    const char * partition_expression,
    const char * topic_expression
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_participant uParticipant;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        uParticipant = u_participant(this->rlReq_get_user_entity());
        uResult = u_participantDeleteHistoricalData(uParticipant,
                                                    partition_expression,
                                                    topic_expression);
        result = uResultToReturnCode(uResult);
        if (result != DDS::RETCODE_OK) {
            CPP_REPORT(result, "Could not delete historical data.");
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::wlReq_load_type_support_meta_holder (
    DDS::OpenSplice::TypeSupportMetaHolder *metaHolder,
    const DDS::Char *type_name
) THROW_ORB_EXCEPTIONS
{
    DDS::OpenSplice::TypeSupportMetaHolder *inMap;
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    u_participant uParticipant;
    u_domain uDomain;
    u_result uResult;

    assert(type_name != NULL);
    assert(metaHolder != NULL);

    char *metaDescr = metaHolder->get_meta_descriptor();
    uParticipant = u_participant(this->rlReq_get_user_entity());
    assert (uParticipant != NULL);
    uDomain = u_participantDomain(uParticipant);

    inMap = rlReq_findMetaHolder(type_name);
    if ((inMap == metaHolder) || (inMap == NULL)) {
        /* Do the actual loading of the type. */
        uResult = u_domain_load_xml_descriptor(uDomain, metaDescr);
        result = uResultToReturnCode(uResult);
        if (result != DDS::RETCODE_OK) {
            CPP_REPORT(result, "Could not register type '%s'.", type_name);
        }
    } else {
        DDS::OpenSplice::TypeSupportMetaHolder *tst;

        /* We hold the lock to the Map, so the element encountered
         * cannot be freed. Let's study its contents to see if it is
         * compatible with the currently passed metaHolder. */
        tst= const_cast<DDS::OpenSplice::TypeSupportMetaHolder *>(inMap);
        const char *srcKeyList = tst->get_key_list();
        const char *testKeyList = metaHolder->get_key_list();
        char *srcDescr = tst->get_meta_descriptor();
        if (strcmp(srcKeyList, testKeyList) == 0 && strcmp(srcDescr, metaDescr) == 0) {
            uResult = u_domain_load_xml_descriptor(uDomain, metaDescr);
            if (uResult != U_RESULT_OK) {
                result = DDS::RETCODE_PRECONDITION_NOT_MET;
            }
        } else {
            result = DDS::RETCODE_PRECONDITION_NOT_MET;
            CPP_REPORT(result, "Could not register type '%s', type not compatible.",
                    type_name);
        }
        DDS::string_free(srcDescr);
    }

    if (result == DDS::RETCODE_OK) {
        wlReq_insertMetaHolder(type_name, metaHolder);
    }

    DDS::string_free(metaDescr);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::nlReq_load_type_support_meta_holder (
    DDS::OpenSplice::TypeSupportMetaHolder *metaHolder,
    const DDS::Char *type_name
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    assert(type_name != NULL);
    assert(metaHolder != NULL);

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = wlReq_load_type_support_meta_holder(metaHolder, type_name);
        this->unlock();
    }

    return result;
}


DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::nlReq_find_type_support_meta_holder (
    const ::DDS::Char *type_name,
    ::DDS::OpenSplice::TypeSupportMetaHolder *&metaHolder
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    result = DDS::RETCODE_OK;

    if (type_name == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "type_name '<NULL>' is invalid.");
    } else {
        result = this->read_lock();
    }

    if (result == DDS::RETCODE_OK) {
        metaHolder = rlReq_findMetaHolderByInternalTypeName(type_name);
        this->unlock();
    }

    return result;
}

DDS::Boolean
DDS::OpenSplice::DomainParticipant::wlReq_deinitTypeMetaHolders(
    const char *key,
    DDS::Object_ptr element,
    DDS::ReturnCode_t *result)
{
    DDS::OpenSplice::TypeSupportMetaHolder *meta_holder;
    OS_UNUSED_ARG(key);
    meta_holder = dynamic_cast<DDS::OpenSplice::TypeSupportMetaHolder *>(element);
    if (meta_holder) {
        *result = meta_holder->deinit();
    } else {
        *result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(*result, "TypeSupportMetaHolder is invalid, not of type '%s'.",
            "DDS::OpenSplice::TypeSupportMetaHolder");
    }

    return (*result == DDS::RETCODE_OK) ? TRUE : FALSE;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::wlReq_deleteContainedEntities()
{
    DDS::ReturnCode_t result, endResult = DDS::RETCODE_OK;

    /* When an error is detected during the deletion of an entity,
     * than continue deleting the next instances so that you delete
     * as much as possible. However, save the error you encountered
     * in the endResult, so that the caller knows that not everything
     * deleted successfully.
     */

    /* Clear builtin subscriber and topics. */
    result = wlReq_deleteBuiltinSubscriber();
    if (result != DDS::RETCODE_OK) {
        endResult = result;
    }

    /* Clear publishers list. */
    result = wlReq_deleteFactoryList<DDS::OpenSplice::Publisher *>(publisherList);
    if (result != DDS::RETCODE_OK) {
        endResult = result;
    }

    /* Clear subscribers list. */
    result = wlReq_deleteFactoryList<DDS::OpenSplice::Subscriber *>(subscriberList);
    if (result != DDS::RETCODE_OK) {
        endResult = result;
    }

    /* Clear content filtered topics list. */
    result = wlReq_deleteEntityList<DDS::OpenSplice::ContentFilteredTopic *>(cfTopicList);
    if (result != DDS::RETCODE_OK) {
        endResult = result;
    }

    /* Clear multi topics list. */
    result = wlReq_deleteEntityList<DDS::OpenSplice::MultiTopic *>(multiTopicList);
    if (result != DDS::RETCODE_OK) {
        endResult = result;
    }

    /* Clear topics list. */
    result = wlReq_deleteEntityList<DDS::OpenSplice::Topic *>(topicList);
    if (result != DDS::RETCODE_OK) {
        endResult = result;
    }

    return endResult;
}

DDS::Boolean
DDS::OpenSplice::DomainParticipant::rlReq_checkHandlePublisher(
    DDS::Object_ptr object,
    DDS::InstanceHandle_t *hdl)
{
    DDS::OpenSplice::Publisher *pub = dynamic_cast<DDS::OpenSplice::Publisher*>(object);
    DDS::Boolean proceed;

    assert(pub);
    assert(hdl);

    /* Proceed when this publisher isn't the searched for entity. */
    proceed = (pub->get_instance_handle() != *hdl);

    if ( proceed ) {
        /* Proceed when this publisher doesn't contain the searched for entity. */
        proceed = !(pub->contains_entity(*hdl));
    }

    return proceed;
}

DDS::Boolean
DDS::OpenSplice::DomainParticipant::rlReq_checkHandleSubscriber(
    DDS::Object_ptr object,
    DDS::InstanceHandle_t *hdl)
{
    DDS::OpenSplice::Subscriber *sub = dynamic_cast<DDS::OpenSplice::Subscriber*>(object);
    DDS::Boolean proceed;

    assert(sub);
    assert(hdl);

    /* Proceed when this subscriber isn't the searched for entity. */
    proceed = (sub->get_instance_handle() != *hdl);

    if ( proceed ) {
        /* Proceed when this subscriber doesn't contain the searched for entity. */
        proceed = !(sub->contains_entity(*hdl));
    }

    return proceed;
}

DDS::Boolean
DDS::OpenSplice::DomainParticipant::rlReq_checkHandleTopic(
    DDS::Object_ptr object,
    DDS::InstanceHandle_t *hdl)
{
    DDS::OpenSplice::Topic *topic;

    assert(hdl);
    assert(object);

    topic = dynamic_cast<DDS::OpenSplice::Topic*>(object);

    /* Proceed when this topic isn't the searched for entity. */
    return (topic->get_instance_handle() != *hdl);
}

void
DDS::OpenSplice::DomainParticipant::nlReq_notify_listener(
    DDS::OpenSplice::Entity *sourceEntity,
    DDS::ULong               triggerMask,
    void                    *eventData)
{
    DDS::DomainParticipantListener_ptr listener;
    DDS::ReturnCode_t result;

    /* Using _narrow to cast Listener, this increases the refcount to ensure
     * the ListenerObject is not deleted while notifying. */
    listener = DDS::DomainParticipantListener::_narrow(this->listener);
    if (listener) {

        if (triggerMask & V_EVENT_ON_DATA_ON_READERS) {
            result = sourceEntity->reset_on_data_on_readers_status();
            if (result == DDS::RETCODE_OK) {
                listener->on_data_on_readers(dynamic_cast<DDS::Subscriber_ptr>(sourceEntity));
            }
        } else {
            if (triggerMask & V_EVENT_DATA_AVAILABLE) {
                result = sourceEntity->reset_dataAvailable_status();
                if (result == DDS::RETCODE_OK) {
                    listener->on_data_available(dynamic_cast<DDS::DataReader_ptr>(sourceEntity));
                }
            }
        }

        if (triggerMask & V_EVENT_SAMPLE_REJECTED) {
            DDS::SampleRejectedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->sampleRejected,
                                                  status);
            listener->on_sample_rejected(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                         status);
        }

        if (triggerMask & V_EVENT_LIVELINESS_CHANGED) {
            DDS::LivelinessChangedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->livelinessChanged,
                                                  status);
            listener->on_liveliness_changed(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                            status);
        }

        if (triggerMask & V_EVENT_SAMPLE_LOST) {
            DDS::SampleLostStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->sampleLost,
                                                  status);
            listener->on_sample_lost(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                     status);
        }

        if (triggerMask & V_EVENT_LIVELINESS_LOST) {
            DDS::LivelinessLostStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_writerStatus(eventData)->livelinessLost,
                                                  status);
            listener->on_liveliness_lost(dynamic_cast<DDS::DataWriter_ptr>(sourceEntity),
                                         status);
        }

        if (triggerMask & V_EVENT_OFFERED_DEADLINE_MISSED) {
            DDS::OfferedDeadlineMissedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_writerStatus(eventData)->deadlineMissed,
                                                  status);
            listener->on_offered_deadline_missed(dynamic_cast<DDS::DataWriter_ptr>(sourceEntity),
                                                 status);
        }

        if (triggerMask & V_EVENT_REQUESTED_DEADLINE_MISSED) {
            DDS::RequestedDeadlineMissedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->deadlineMissed,
                                                  status);
            listener->on_requested_deadline_missed(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                                   status);
        }

        if (triggerMask & V_EVENT_OFFERED_INCOMPATIBLE_QOS) {
            DDS::OfferedIncompatibleQosStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_writerStatus(eventData)->incompatibleQos,
                                                  status);
            listener->on_offered_incompatible_qos(dynamic_cast<DDS::DataWriter_ptr>(sourceEntity),
                                                  status);
        }

        if (triggerMask & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) {
            DDS::RequestedIncompatibleQosStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->incompatibleQos,
                                                  status);
            listener->on_requested_incompatible_qos(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                                    status);
        }

        if (triggerMask & V_EVENT_PUBLICATION_MATCHED) {
            DDS::PublicationMatchedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_writerStatus(eventData)->publicationMatch,
                                                  status);
            listener->on_publication_matched(dynamic_cast<DDS::DataWriter_ptr>(sourceEntity),
                                             status);
        }

        if (triggerMask & V_EVENT_SUBSCRIPTION_MATCHED) {
            DDS::SubscriptionMatchedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->subscriptionMatch,
                                                  status);
            listener->on_subscription_matched(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                              status);
        }

        if (triggerMask & V_EVENT_INCONSISTENT_TOPIC) {
            DDS::InconsistentTopicStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_topicStatus(eventData)->inconsistentTopic,
                                                  status);
            listener->on_inconsistent_topic(dynamic_cast<DDS::Topic_ptr>(sourceEntity),
                                            status);
        }

        if ( triggerMask & V_EVENT_ALL_DATA_DISPOSED ) {
            DDS::ExtDomainParticipantListener_ptr extListener;
            extListener = dynamic_cast<DDS::ExtDomainParticipantListener_ptr>(listener);
            if (extListener) {
                extListener->on_all_data_disposed(dynamic_cast<DDS::Topic_ptr>(sourceEntity));
            }
        }
        DDS::release(listener);
    }
}

void
DDS::OpenSplice::DomainParticipant::nlReq_initBuiltinSubscriberQos(
    DDS::SubscriberQos *sQos)
{
    *sQos = SUBSCRIBER_QOS_DEFAULT;
    sQos->presentation.access_scope = DDS::TOPIC_PRESENTATION_QOS;
    DDS::OpenSplice::Utils::copySequenceOut("__BUILT-IN PARTITION__",
                                            ",",
                                            sQos->partition.name);
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::nlReq_getDiscoveredEntities (
    const char *topic_name,
    const char *type_name,
    DDS::InstanceHandleSeq &handles)
{
    DDS::ReturnCode_t result = DDS::RETCODE_ERROR;
    DDS::OpenSplice::DataReader *reader;
    DDS::Subscriber_ptr sub;
    OS_UNUSED_ARG(type_name);

    handles.length(0);

    /* By calling get_builtin_subscriber() instead of using builtinSubscriber directly,
     * we make sure that the builtin subscriber is created when it's not available yet. */
    sub = this->get_builtin_subscriber();

    if (sub != NULL) {
        reader = dynamic_cast<DDS::OpenSplice::DataReader*>(sub->lookup_datareader(topic_name));
        if (reader) {
            result = reader->nlReq_get_instance_handles(handles);
            DDS::release(reader);
        } else {
            CPP_REPORT(result, "Could not resolve builtin DataReader for Topic '%s'.",
                topic_name);
        }
        /* The builtinSubscriber was dublicated by the get_builtin_subscriber(), so we have
         * to release it. If the call to get_builtin_subscriber() created the subscriber,
         * then the builtin resources will not be removed until the application does a
         * get_builtin_subscriber() and then a delete_subscriber() with the builtin
         * subscriber or a delete_contained_entities() on this participant. */
        DDS::release(sub);
    }

    return result;
}

template<typename TYPE, typename TYPE_SEQ, typename TYPE_READER>
DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::nlReq_getDiscoveredData(
    const char *topic_name,
    const char *type_name,
    TYPE &data,
    DDS::InstanceHandle_t handle)
{
    TYPE_READER *reader;
    TYPE_SEQ data_seq;
    DDS::SampleInfoSeq info_seq;
    DDS::ReturnCode_t result = DDS::RETCODE_ERROR;
    DDS::Subscriber_ptr sub;
    OS_UNUSED_ARG(type_name);

    sub = this->get_builtin_subscriber();

    if (sub != NULL) {
        reader = dynamic_cast<TYPE_READER*>( sub->lookup_datareader(topic_name));
        if (reader) {
            result = reader->read_instance(data_seq,
                                           info_seq,
                                           DDS::LENGTH_UNLIMITED,
                                           handle,
                                           DDS::ANY_SAMPLE_STATE,
                                           DDS::ANY_VIEW_STATE,
                                           DDS::ANY_INSTANCE_STATE);
            if (result == DDS::RETCODE_OK) {
                if (data_seq.length() == 1) {
                    data = data_seq[0];
                } else {
                    result = DDS::RETCODE_ERROR;
                    CPP_REPORT(result, "Could not get discovered data for instance handle '%lld'.",
                        handle);
                }
                reader->return_loan(data_seq, info_seq);
            }
            DDS::release(reader);
        } else {
            result = DDS::RETCODE_ERROR;
            CPP_REPORT(result, "Could not resolve builtin DataReader for Topic '%s'.",
                topic_name);
        }
        /* The builtinSubscriber was dublicated by the get_builtin_subscriber(), so we have
         * to release it. If the call to get_builtin_subscriber() created the subscriber,
         * then the builtin resources will not be removed until the application does a
         * get_builtin_subscriber() and then a delete_subscriber() with the builtin
         * subscriber or a delete_contained_entities() on this participant. */
        DDS::release(sub);
    }

    return result;
}

DDS::Topic_ptr
DDS::OpenSplice::DomainParticipant::nlReq_createTopic (
    const char * topic_name,
    const char * type_name,
    const DDS::TopicQos &qos,
    DDS::TopicListener_ptr a_listener,
    DDS::StatusMask mask,
    DDS::OpenSplice::ObjSet &dest_list)
{
    DDS::ReturnCode_t result;
    u_topicQos uQos = NULL;
    const DDS::TopicQos *tmpQos;
    const char *typeName;
    const char *key_list;
    u_topic uTopic;
    u_participant uParticipant;
    DDS::OpenSplice::Topic *topic = NULL;
    DDS::OpenSplice::TypeSupportMetaHolder *typeMetaHolder;

    assert(topic_name);
    assert(type_name);

    /*
     * The creation of a Topic is somewhat different then the creation of a Subscriber
     * or Publisher. The reason for this is the special functionality for the function
     * find_topic(). For that, the init of Topic has a user layer object as argument,
     * where Publisher and Subscriber don't (they get that object themselves).
     * So, because of the find_topic() and the related Topic init, we create the user layer
     * topic here and provide it to the init of the new Topic object.
     */

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        /* Find the TypeSupportMetaHolder needed for creation of this Topic. */
        /* Store the created Topic object (map performs the duplicate). */
        typeMetaHolder = this->rlReq_findMetaHolder(type_name);
        if (typeMetaHolder == NULL) {
            result = DDS::RETCODE_PRECONDITION_NOT_MET;
            CPP_REPORT(result, "Could not create Topic '%s', type '%s' not registered.",
                topic_name, type_name);
        }

        if (result == DDS::RETCODE_OK) {
            /* Determine which QoS to use (prevent deep copy). */
            if (&qos == &(TOPIC_QOS_DEFAULT)) {
                tmpQos = &(this->defaultTopicQos);
            } else {
                tmpQos = &qos;
                result = DDS::OpenSplice::Utils::qosIsConsistent(qos);
            }
        }

        if (result == DDS::RETCODE_OK) {
            /* (deep)copy QoS into user layer QoS. */
            uQos = u_topicQosNew(NULL);
            if (uQos != NULL) {
                result = DDS::OpenSplice::Utils::copyQosIn(*tmpQos, uQos);
            } else {
                result = DDS::RETCODE_OUT_OF_RESOURCES;
                CPP_REPORT(result, "Could not copy TopicQos for Topic '%s'.",
                    topic_name);
            }
        }

        if (result == DDS::RETCODE_OK) {
            /* Create and initialize user layer topic. */
            typeName = typeMetaHolder->get_internal_type_name();
            key_list = typeMetaHolder->get_key_list();
            uParticipant = u_participant(this->rlReq_get_user_entity());
            assert (uParticipant != NULL);

            uTopic = u_topicNew(uParticipant,
                                topic_name,
                                typeName,
                                key_list,
                                uQos);
            if (uTopic == NULL) {
                result = DDS::RETCODE_ERROR;
                CPP_REPORT(result, "Could not create Topic '%s'.", topic_name);
            }
        }

        /* Create, initialize and store the Topic object. */
        if (result == DDS::RETCODE_OK) {
            topic = new DDS::OpenSplice::Topic();
            if (topic == NULL) {
                result = DDS::RETCODE_OUT_OF_RESOURCES;
                CPP_REPORT(result, "Could not create Topic '%s'.", topic_name);
            }
        }
        if (result == DDS::RETCODE_OK) {
            result = topic->init(uTopic, this, topic_name, type_name, typeMetaHolder);
            if (result != DDS::RETCODE_OK) {
                DDS::release(topic);
                topic = NULL;
            }
        }

        if (result == DDS::RETCODE_OK) {
            if (dest_list.insertElement(topic) == FALSE) {
                assert(FALSE);
                DDS::release(topic);
                topic = NULL;
                result = DDS::RETCODE_ERROR;
            }
        }

        if (result == DDS::RETCODE_OK) {
            topic->wlReq_set_listenerDispatcher(this->rlReq_get_listenerDispatcher());
            result = topic->set_listener(a_listener, mask);
        }

        if (result == DDS::RETCODE_OK) {
            if (this->factoryAutoEnable) {
                result = topic->enable();
            }
        }

        /* Cleanup */
        if ((result != DDS::RETCODE_OK) && (topic != NULL)) {
            (void)topic->set_listener(NULL, 0);
            topic->wlReq_set_listenerDispatcher(NULL);
            dest_list.removeElement(topic);
            DDS::release(topic);
            topic = NULL;
        }
        if (typeMetaHolder != NULL) {
            DDS::release(typeMetaHolder);
        }
        if (uQos != NULL) {
            u_topicQosFree(uQos);
        }

        this->unlock();

    }

    return topic;
}

DDS::ReturnCode_t
DDS::OpenSplice::DomainParticipant::wlReq_deleteBuiltinSubscriber()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    /* Do we really have to clean up? */
    if (this->builtinSubscriber != NULL) {

        /* Cleanup and release the builtin subscriber. */
        result = this->builtinSubscriber->delete_contained_entities();
        if (result == DDS::RETCODE_OK) {
            result = this->builtinSubscriber->deinit();
            if (result == DDS::RETCODE_OK) {
                DDS::release(this->builtinSubscriber);
                this->builtinSubscriber = NULL;
            }
        }
        /* Cleanup and release the builtin topics. */
        if (result == DDS::RETCODE_OK) {
            result = wlReq_deleteEntityList<DDS::OpenSplice::Topic *>(builtinTopicList);
        }
    }

    return result;
}

os_char*
DDS::OpenSplice::DomainParticipant::rlReq_getChildName(
    const char* prefix)
{
    os_char *child = NULL;
    os_char  pid[25];
    os_char *name;
    os_char *end;
    int length;

    assert(prefix);

    /* Get the participants' name. */
    name = u_entityName(this->rlReq_get_user_entity());
    if (name) {
        /* Remove possibly " <pid>" name extension. */
        length = (int)strlen(name);
        if (length > 3) {
            if (name[length - 1] == '>') {
                snprintf(pid, 25, " <%d>", os_procIdSelf());
                end = strstr(name, pid);
                if (end) {
                    *end = '\0';
                }
            }
        }

        /* Merge prefix and name into the childs' name. */
        length = (int)strlen(prefix) + (int)strlen(name) + 4 /* " <>\0"*/;
        child = (os_char*)os_malloc(length);
        if (child) {
            snprintf(child, length, "%s <%s>", prefix, name);
        }

        os_free(name);
    }

    return child;
}
