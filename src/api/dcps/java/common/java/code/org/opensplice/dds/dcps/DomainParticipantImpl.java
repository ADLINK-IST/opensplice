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


package org.opensplice.dds.dcps;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import DDS.ExtDomainParticipantListener;
import DDS.Topic;
import DDS.TopicQosHolder;

/**
 * Implementation of the {@link DDS.DomainParticipant} interface.
 *
 * The behavior of built-in entities adheres to the following rules:
 * <ul>
 * <li>Built-in entities will be initialized as late as possible
 * <li>Built-in entities do not have to be deleted by the user
 * <li>Built-in entities can be deleted by the user
 * </ul>
 */
public class DomainParticipantImpl extends DomainParticipantBase implements DDS.DomainParticipant {

    private static final long serialVersionUID = 4658964521910484015L;
    private DDS.DomainParticipantListener listener = null;
    private DDS.TopicQos defaultTopicQos = Utilities.defaultTopicQos;
    private DDS.PublisherQos defaultPublisherQos = Utilities.defaultPublisherQos;
    private DDS.SubscriberQos defaultSubscriberQos = Utilities.defaultSubscriberQos;
    private DDS.SchedulingQosPolicy listener_scheduling = Utilities.defaultDomainParticipantQos.listener_scheduling;
    private String baseName;
    private final Map<String, TypeSupportImpl>  typeSupports          = new HashMap<String, TypeSupportImpl>();
    private final Set<ContentFilteredTopicImpl> contentFilteredTopics = new HashSet<ContentFilteredTopicImpl>();
    private final Set<TopicImpl>         topics = new HashSet<TopicImpl>();
    private final Set<PublisherImpl>     publishers = new HashSet<PublisherImpl>();
    private final Set<SubscriberImpl>    subscribers = new HashSet<SubscriberImpl>();
    /* Built-in topics are ALSO in set of topics */
    private final Set<TopicImpl>         builtintopics = new HashSet<TopicImpl>();
    /* Built-in Subscriber is NOT in set of subscribers */
    private SubscriberImpl               builtinSubscriber = null;
    private long participantDataCopyCache;
    private long topicBuiltinTopicDataCopyCache;
    private boolean factoryAutoEnable = false;
    private String[][] builtinTopicNames = new String[][] {
            { "DCPSParticipant", "DDS::ParticipantBuiltinTopicData" },
            { "DCPSTopic", "DDS::TopicBuiltinTopicData" },
            { "DCPSPublication", "DDS::PublicationBuiltinTopicData", },
            { "DCPSSubscription", "DDS::SubscriptionBuiltinTopicData" },
            { "CMParticipant", "DDS::CMParticipantBuiltinTopicData" },
            { "CMPublisher", "DDS::CMPublisherBuiltinTopicData" },
            { "CMSubscriber", "DDS::CMSubscriberBuiltinTopicData" },
            { "CMDataWriter", "DDS::CMDataWriterBuiltinTopicData" },
            { "CMDataReader", "DDS::CMDataReaderBuiltinTopicData" },
            { "DCPSType", "DDS::TypeBuiltinTopicData" } };

    private int listenerInterest = 0;

    private boolean isBuiltinTopic(String topicName) {
        for (int i = 0; i < this.builtinTopicNames.length; i++) {
            if (this.builtinTopicNames[i][0].equals(topicName)) {
                return true;
            }
        }
        return false;
    }

    protected DomainParticipantImpl () { }

    protected int init (String name, int domainId, DDS.DomainParticipantQos a_qos)
    {
        int result = DDS.RETCODE_OK.value;
        ListenerDispatcher dispatcher = null;

        long uParticipant = jniDomainParticipantNew(name, domainId, a_qos);
        if (uParticipant != 0) {
            this.set_user_object(uParticipant);
            this.setDomainId(jniGetDomainId(uParticipant));
            this.baseName = name.replace(" "+DDS.MainClassName.getProcessId(), "");
            this.factoryAutoEnable = a_qos.entity_factory.autoenable_created_entities;
            dispatcher = new ListenerDispatcher (
                uParticipant, a_qos.listener_scheduling);
            this.set_dispatcher(dispatcher);
        } else {
            result = DDS.RETCODE_ERROR.value;
        }

        return result;
    }

    @Override
    protected int deinit ()
    {
        int result = DDS.RETCODE_OK.value;
        long uParticipant;
        ListenerDispatcher dispatcher = null;

        synchronized (this)
        {
            uParticipant = this.get_user_object();
            if (uParticipant != 0) {
                if (this.topics.size() > this.builtintopics.size()) {
                    result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                    ReportStack.report(result,
                            "DomainParticipant still contains '"
                                    + this.topics.size() + "' Topic entities.");
                } else if (this.publishers.size() != 0) {
                    result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                    ReportStack.report(result,
                            "DomainParticipant still contains '"
                                    + this.publishers.size() + "' Publisher entities.");
                } else if (this.subscribers.size() != 0) {
                    result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                    ReportStack.report(result,
                            "DomainParticipant still contains '"
                                    + this.subscribers.size() + "' Subscriber entities.");
                } else if (this.contentFilteredTopics.size() != 0) {
                    result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                    ReportStack.report(result,
                            "DomainParticipant still contains '"
                                    + this.contentFilteredTopics.size() +
                            "' ContentFilteredTopic entities.");
                } else {
                    if (this.listener != null) {
                        set_listener(this.listener, 0);
                    }
                    this.disable_callbacks();
                    result = this.delete_builtin_subscriber();

                    if (result == DDS.RETCODE_OK.value) {
                        dispatcher = this.get_dispatcher();
                        if (dispatcher != null) {
                            this.set_dispatcher(null);
                            result = dispatcher.deinit();
                            dispatcher = null;
                            this.set_dispatcher(dispatcher);
                        }
                        if (result == DDS.RETCODE_OK.value) {
                            result = ((EntityImpl) this).detach_statuscondition();
                        }
                        if (result == DDS.RETCODE_OK.value) {
                            this.builtintopics.clear();
                            result = jniDomainParticipantFree(uParticipant);
                            if (result == DDS.RETCODE_OK.value) {
                                result = super.deinit();
                            }
                        }
                    }
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        return result;
    }

    private boolean add (
            TypeSupportImpl ts,
            String alias_name)
    {
        boolean result;
        synchronized (this)
        {
            if (this.typeSupports.containsKey(alias_name)) {
                result = false;
            } else {
                this.typeSupports.put(alias_name, ts);
                result = true;
            }
        }
        return result;
    }

    private void remove(String alias_name) {
        synchronized (this)
        {
            this.typeSupports.remove(alias_name);
        }
    }

    protected TypeSupportImpl lookup_typeSupport(
            String name)
    {
        synchronized (this)
        {
            return this.typeSupports.get(name);
        }
    }

    @Override
    public DDS.Publisher create_publisher (
            DDS.PublisherQos qos,
            DDS.PublisherListener a_listener,
            int mask)
    {
        int result = DDS.RETCODE_OK.value;
        PublisherImpl pub = null;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos != DDS.PUBLISHER_QOS_DEFAULT.value) {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            synchronized (this)
            {
                if (qos == DDS.PUBLISHER_QOS_DEFAULT.value) {
                    qos = this.defaultPublisherQos;
                }
                pub = new PublisherImpl();
                result = pub.init(this, "publisher <" + this.baseName + ">", qos);
                if (result == DDS.RETCODE_OK.value) {
                    this.publishers.add(pub);
                    ListenerDispatcher dispatcher = this.get_dispatcher();
                    result = pub.set_dispatcher(dispatcher);
                } else {
                    pub = null;
                }
                if (result == DDS.RETCODE_OK.value) {
                    result = pub.set_listener(a_listener, mask);
                }
                if (result == DDS.RETCODE_OK.value) {
                    if ((this.factoryAutoEnable) && (this.is_enabled())) {
                        result = pub.enable();
                    }
                }
                if (result != DDS.RETCODE_OK.value && pub != null) {
                    this.delete_publisher(pub);
                    pub = null;
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return pub;
    }

    @Override
    public int delete_publisher (
            DDS.Publisher pub)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (pub == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "publisher 'null' is invalid.");
        } else {
            synchronized (this)
            {
                PublisherImpl p = (PublisherImpl)pub;
                if (this.publishers.remove(p)) {
                    result = p.deinit();
                    if (result != DDS.RETCODE_OK.value) {
                        this.publishers.add(p);
                    }
                } else {
                    if (p.get_user_object() == 0) {
                        result = DDS.RETCODE_ALREADY_DELETED.value;
                    } else {
                        result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                        ReportStack.report(result,
                                "Publisher not created by DomainParticipant.");
                    }
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public DDS.Subscriber create_subscriber (
            DDS.SubscriberQos qos,
            DDS.SubscriberListener a_listener,
            int mask)
    {
        int result = DDS.RETCODE_OK.value;
        SubscriberImpl sub = null;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos != DDS.SUBSCRIBER_QOS_DEFAULT.value) {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            synchronized (this)
            {
                if (qos == DDS.SUBSCRIBER_QOS_DEFAULT.value) {
                    qos = this.defaultSubscriberQos;
                }
                sub = new SubscriberImpl();
                result = sub.init(this, "subscriber <" + this.baseName + ">", qos);
                if (result == DDS.RETCODE_OK.value) {
                    this.subscribers.add(sub);
                    ListenerDispatcher dispatcher = this.get_dispatcher();
                    result = sub.set_dispatcher(dispatcher);
                } else {
                    sub = null;
                }
                if (result == DDS.RETCODE_OK.value) {
                    result = sub.set_listener(a_listener, mask);
                }
                if (result == DDS.RETCODE_OK.value) {
                    if ((this.factoryAutoEnable) && (this.is_enabled())) {
                        result = sub.enable();
                    }
                }
                if (result != DDS.RETCODE_OK.value && sub != null) {
                    this.delete_subscriber(sub);
                    sub = null;
                }
            }
        }
        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return sub;
    }

    @Override
    public int delete_subscriber (
            DDS.Subscriber sub)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (sub == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "subscriber 'null' is invalid.");
        } else {
            synchronized (this)
            {
                SubscriberImpl s = (SubscriberImpl)sub;
                if (s.equals(this.builtinSubscriber)) {
                    result = delete_builtin_subscriber();
                } else {
                    if (this.subscribers.remove(s)) {
                        result = s.deinit();
                        if (result != DDS.RETCODE_OK.value) {
                            this.subscribers.add(s);
                        }
                    } else {
                        if (s.get_user_object() == 0) {
                            result = DDS.RETCODE_ALREADY_DELETED.value;
                        } else {
                            result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                            ReportStack
                                    .report(result,
                                            "Subscriber not created by DomainParticipant.");
                        }
                    }
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* must only be invoked by get_builtin_subscriber */
    private int create_builtin_subscriber ()
    {
        int result = DDS.RETCODE_OK.value;
        DDS.Topic topic;
        DDS.DataReader reader;
        DDS.TopicQos topicQos;
        DDS.SubscriberQos subscriberQos;
        DDS.DataReaderQos readerQos;

        /* Initialize QoSses to reflect the builtin ones. */
        topicQos = new DDS.TopicQos();
        topicQos = Utilities.deepCopy(Utilities.defaultTopicQos);
        topicQos.durability.kind =
                DDS.DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;
        topicQos.reliability.kind =
                DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;

        /* The following 2 overrides are because the kernel is using
           the same QoS's as the 6.4 for backwards compatibility */
        topicQos.history.kind =
                DDS.HistoryQosPolicyKind.KEEP_ALL_HISTORY_QOS;
        topicQos.history.depth =
                DDS.LENGTH_UNLIMITED.value;

        subscriberQos = new DDS.SubscriberQos();
        subscriberQos = Utilities.deepCopy(Utilities.defaultSubscriberQos);
        subscriberQos.partition.name = new String[1];
        subscriberQos.partition.name[0] = new String("__BUILT-IN PARTITION__");
        subscriberQos.presentation.access_scope =
                DDS.PresentationQosPolicyAccessScopeKind.TOPIC_PRESENTATION_QOS;

        readerQos = new DDS.DataReaderQos();
        readerQos = Utilities.deepCopy(Utilities.defaultDataReaderQos);
        readerQos.durability.kind =
                DDS.DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;
        readerQos.reliability.kind =
                DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;

        this.builtinSubscriber = new SubscriberImpl();
        result = this.builtinSubscriber.init(
                this, "BuiltinSubscriber", subscriberQos);
        if (result == DDS.RETCODE_OK.value) {
            if (this.is_enabled ()) {
                result = this.builtinSubscriber.enable ();
            }

            for (int i = 0; i < builtinTopicNames.length && result == DDS.RETCODE_OK.value; i++) {
                topic = this.create_topic (
                        builtinTopicNames[i][0], builtinTopicNames[i][1], topicQos, null, 0);
                if (topic != null) {
                    this.builtintopics.add ((TopicImpl) topic);
                    reader = this.builtinSubscriber.create_datareader ( topic, readerQos, null, 0);

                    if (reader == null) {
                        this.delete_builtin_subscriber();
                        ReportStack.report(result,
                                "Failed to initialize built-in DataReader for Topic '"
                                        + builtinTopicNames[i][0] + "'.");
                        result = DDS.RETCODE_ERROR.value;
                    }
                } else {
                    this.delete_builtin_subscriber();
                    ReportStack.report(result,
                            "Failed to initialize built-in Topic '"
                                    + builtinTopicNames[i][0] + "'.");
                    result = DDS.RETCODE_ERROR.value;
                }
            }
        } else {
            this.builtinSubscriber = null;
        }

        return result;
    }

    private int delete_builtin_subscriber() {
        int result, endResult = DDS.RETCODE_OK.value;

        if (this.builtinSubscriber != null) {
            result = this.builtinSubscriber.delete_contained_entities();
            if(result == DDS.RETCODE_OK.value){
                result = this.builtinSubscriber.deinit();
                if (result == DDS.RETCODE_OK.value || result == DDS.RETCODE_ALREADY_DELETED.value) {
                    this.builtinSubscriber = null;
                }
                if (result != DDS.RETCODE_OK.value){
                    ReportStack.report (result, "Deletion of built-in Subscriber failed.");
                }
            } else {
                if (result == DDS.RETCODE_ALREADY_DELETED.value) {
                    this.builtinSubscriber = null;
                }
                ReportStack.report (result, "delete_contained_entities failed on built-in Subscriber.");
            }
            if (endResult == DDS.RETCODE_OK.value ) {
                /* Store first encountered error. */
                endResult = result;
            }

            Iterator<TopicImpl> bti = this.builtintopics.iterator();
            while (bti.hasNext()) {
                TopicImpl t = bti.next();
                result = t.deinit();
                if (result == DDS.RETCODE_OK.value || result == DDS.RETCODE_ALREADY_DELETED.value) {
                    /* Use iterator.remove() to remove from the built-in set */
                    bti.remove();
                    /* Also remove the topic from the set of normal topics */
                    this.topics.remove(t);
                }
                if (result != DDS.RETCODE_OK.value){
                    ReportStack.report (result, "Deletion of Topic contained in built-in Subscriber failed.");
                    if (endResult == DDS.RETCODE_OK.value ) {
                        endResult = result;
                    }
                }
            }
        }
        return endResult;
    }

    @Override
    public DDS.Subscriber get_builtin_subscriber ()
    {
        int result = DDS.RETCODE_OK.value;
        DDS.Subscriber sub = null;
        ReportStack.start();

        synchronized (this)
        {
            if (this.is_enabled()) {
                if (this.builtinSubscriber == null) {
                    result = this.create_builtin_subscriber();
                }

                if (result == DDS.RETCODE_OK.value) {
                    sub = this.builtinSubscriber;
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return sub;
    }

    @Override
    public DDS.Topic create_topic (
            String topic_name,
            String type_name,
            DDS.TopicQos qos,
            DDS.TopicListener a_listener,
            int mask)
    {
        TopicImpl topic = null;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (topic_name == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "topic_name 'null' is invalid.");
        } else if (type_name == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "type_name 'null' is invalid.");
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            topic = new TopicImpl();
            synchronized (this)
            {
                if (qos == DDS.TOPIC_QOS_DEFAULT.value) {
                    qos = this.defaultTopicQos;
                }
                result = topic.init(this, topic_name, type_name, qos);
                if (result == DDS.RETCODE_OK.value) {
                    this.topics.add(topic);
                    ListenerDispatcher dispatcher = this.get_dispatcher();
                    result = topic.set_dispatcher(dispatcher);
                } else {
                    topic = null;
                }
                if (result == DDS.RETCODE_OK.value) {
                    result = topic.set_listener(a_listener, mask);
                }
                if (result == DDS.RETCODE_OK.value) {
                    if ((this.factoryAutoEnable) && (this.is_enabled())) {
                        result = topic.enable();
                    }
                    topic.set_participant_listener_mask(this.listenerInterest);
                }
                if (result != DDS.RETCODE_OK.value && topic != null) {
                    this.delete_topic(topic);
                    topic = null;
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return topic;
    }

    @Override
    public int delete_topic (
            DDS.Topic topic)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (topic == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "topic 'null' is invalid.");
        } else {
            synchronized (this)
            {
                TopicImpl t = (TopicImpl) topic;
                if (this.topics.remove(t)) {
                    result = t.deinit();
                    if (result != DDS.RETCODE_OK.value) {
                        this.topics.add(t);
                    } else {
                        /*
                         * In case this is a built-in topic it will be in the
                         * list of built-in topics as well, so removing it from
                         * there too.
                         */
                        this.builtintopics.remove(t);
                    }
                } else {
                    if (t.get_user_object() == 0) {
                        result = DDS.RETCODE_ALREADY_DELETED.value;
                    } else {
                        result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                        ReportStack.report(result,
                                "Topic not created by DomainParticipant.");
                    }
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public DDS.Topic find_topic (
            String topic_name,
            DDS.Duration_t timeout)
    {
        int result = DDS.RETCODE_OK.value;
        Topic topic = null;
        TopicImpl found = null;
        TypeSupportImpl type_support = null;
        long uParticipant = 0;
        long uTopic = 0;
        ReportStack.start();

        if (topic_name == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "topic_name 'null' is invalid.");
        } else if (topic_name.contains ("*") || topic_name.contains ("?")) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                    "topic_name '" + topic_name + "' is invalid.");
        } else {
            result = Utilities.checkDuration (timeout);
        }

        if (result == DDS.RETCODE_OK.value) {
            synchronized (this)
            {
                uParticipant = this.get_user_object();
                if (uParticipant != 0) {
                    /* do a local lookup to find the topic */
                    for(TopicImpl t : topics) {
                        if (t.get_name().equals(topic_name)) {
                            found = t;
                        }
                    }

                    if (found == null && isBuiltinTopic(topic_name)) {
                        result = this.create_builtin_subscriber();

                        if (result == DDS.RETCODE_OK.value) {
                            for(TopicImpl t : topics) {
                                if(t.get_name().equals(topic_name)){
                                    found = t;
                                    break;
                                }
                            }
                        }
                    }

                    if (found != null) {
                        uTopic = found.get_user_object();
                        TopicQosHolder qos = new TopicQosHolder();
                        found.get_qos(qos);
                        String type_alias = found.get_type_name();
                        /* lookup type alias for the found typesupport */
                        for (String alias : typeSupports.keySet()) {
                            TypeSupportImpl tsp = typeSupports.get(alias);
                            if (tsp == found.get_typesupport()) {
                                type_alias = alias;
                                break;
                            }
                        }
                        topic = this.create_topic(topic_name, type_alias, qos.value, null, 0);
                        if (topic != null) {
                            topics.add((TopicImpl)topic);
                        }
                    } else {
                        /* topic not found local so ask the kernel */
                        uTopic = jniFindTopic(uParticipant, topic_name, timeout);
                        if (uTopic != 0) {
                            TopicImpl topicImpl = new TopicImpl();
                            result = topicImpl.clone(uTopic, this, type_support);
                            if (result == DDS.RETCODE_OK.value) {
                                topics.add(topicImpl);
                                topic = topicImpl;
                            }
                        } else {
                            result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                            ReportStack.report(result,
                                    "Failed to resolve Topic '" + topic_name + "'.");
                        }
                    }
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return topic;
    }

    @Override
    public DDS.TopicDescription lookup_topicdescription (
            String name)
    {
        int result = DDS.RETCODE_OK.value;
        DDS.TopicDescription found = null;
        ReportStack.start();

        if (name == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "name 'null' is invalid.");
        } else {
            synchronized (this)
            {
                for(TopicImpl topic : topics) {
                    if (topic.get_name().equals(name)) {
                        found = topic;
                        break;
                    }
                }

                if (found == null && isBuiltinTopic(name)) {
                    result = this.create_builtin_subscriber();

                    if (result == DDS.RETCODE_OK.value) {
                        for(TopicImpl topic : topics) {
                            if(topic.get_name().equals(name)){
                                found = topic;
                                break;
                            }
                        }
                    }
                }

                if(found == null) {
                    for (ContentFilteredTopicImpl cft : contentFilteredTopics) {
                        if(cft.get_name().equals(name)) {
                            found = cft;
                            break;
                        }
                    }
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return found;
    }

    @Override
    public DDS.ContentFilteredTopic create_contentfilteredtopic (
            String name,
            DDS.Topic related_topic,
            String filter_expression,
            String[] expression_parameters)
    {
        int result = DDS.RETCODE_OK.value;
        long uParticipant;
        ContentFilteredTopicImpl topic = null;
        ReportStack.start();

        if (name == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "name 'null' is invalid.");
        } else if (related_topic == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "related_topic 'null' is invalid.");
        } else if (filter_expression == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "filter_expression 'null' is invalid.");
        } else {
            synchronized (this)
            {
                uParticipant = this.get_user_object();
                if (uParticipant != 0) {
                    topic = new ContentFilteredTopicImpl();
                    result = topic.init(this, name, (TopicImpl) related_topic,
                            filter_expression, expression_parameters);
                    if (result == DDS.RETCODE_OK.value) {
                        this.contentFilteredTopics.add(topic);
                    } else {
                        topic = null;
                    }
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return topic;
    }

    @Override
    public int delete_contentfilteredtopic (
            DDS.ContentFilteredTopic topic)
    {
        int result = DDS.RETCODE_OK.value;
        long uParticipant;
        ReportStack.start();

        if (topic == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "topic 'null' is invalid.");
        } else {
            synchronized (this)
            {
                ContentFilteredTopicImpl cft = (ContentFilteredTopicImpl)topic;
                uParticipant = this.get_user_object();
                if (uParticipant != 0) {
                    if (this.contentFilteredTopics.remove(cft)) {
                        result = cft.deinit();
                        if (result != DDS.RETCODE_OK.value) {
                            this.contentFilteredTopics.add(cft);
                        }
                    } else {
                        result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                        ReportStack.report(result, "ContentFilteredTopic not registered to DomainParticipant.");
                    }
                } else {
                    result = DDS.RETCODE_ALREADY_DELETED.value;
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public DDS.MultiTopic create_multitopic (
            String name,
            String type_name,
            String subscription_expression,
            String[] expression_parameters)
    {
        ReportStack.start();
        ReportStack.report(DDS.RETCODE_UNSUPPORTED.value,
                "create_multitopic is not yet supported.");
        ReportStack.flush(this, true);
        return null;
    }

    @Override
    public int delete_multitopic (
            DDS.MultiTopic a_multitopic)
    {
        int result = DDS.RETCODE_UNSUPPORTED.value;
        ReportStack.start();
        ReportStack.report(result, "delete_multitopic is not yet supported.");
        ReportStack.flush(this, true);
        return result;
    }

    @Override
    public int delete_contained_entities ()
    {
        int result, endResult = DDS.RETCODE_OK.value;
        ReportStack.start();

        synchronized (this)
        {
            Iterator<PublisherImpl> ip = this.publishers.iterator();
            while (ip.hasNext()) {
                PublisherImpl p = ip.next();
                result = p.delete_contained_entities();
                if (result == DDS.RETCODE_OK.value) {
                    result = p.deinit();
                    if (result == DDS.RETCODE_OK.value || result == DDS.RETCODE_ALREADY_DELETED.value) {
                        /* Use iterator.remove() to remove from the set */
                        ip.remove();
                    }
                    if (result != DDS.RETCODE_OK.value) {
                        ReportStack.report (result, "Deletion of Publisher contained in DomainParticipant failed.");
                    }
                } else {
                    if (result == DDS.RETCODE_ALREADY_DELETED.value) {
                        /* Use iterator.remove() to remove from the set */
                        ip.remove();
                    }
                    ReportStack.report (result, "delete_contained_entities failed on Publisher contained in DomainParticipant.");
                }
                if (endResult == DDS.RETCODE_OK.value ) {
                    /* Store first encountered error. */
                    endResult = result;
                }
            }

            Iterator<SubscriberImpl> is = this.subscribers.iterator();
            while (is.hasNext()) {
                SubscriberImpl s = is.next();
                result = s.delete_contained_entities();
                if (result == DDS.RETCODE_OK.value) {
                    result = s.deinit();
                    if (result == DDS.RETCODE_OK.value || result == DDS.RETCODE_ALREADY_DELETED.value) {
                        /* Use iterator.remove() to remove from the set */
                        is.remove();
                    }
                    if (result != DDS.RETCODE_OK.value ) {
                        ReportStack.report (result, "Deletion of Subscriber contained in DomainParticipant failed.");
                    }
                } else {
                    if (result == DDS.RETCODE_ALREADY_DELETED.value) {
                       /* Use iterator.remove() to remove from the set */
                        is.remove();
                    }
                    ReportStack.report (result, "delete_contained_entities failed on Subscriber contained in DomainParticipant.");
                }

                if (endResult == DDS.RETCODE_OK.value ) {
                    /* Store first encountered error. */
                    endResult = result;
                }
            }

            result = this.delete_builtin_subscriber();
            if (endResult == DDS.RETCODE_OK.value) {
                endResult = result;
            }

            Iterator<ContentFilteredTopicImpl> ict = this.contentFilteredTopics.iterator();
            while (ict.hasNext()) {
                result = ict.next().deinit();
                if (result == DDS.RETCODE_OK.value || result == DDS.RETCODE_ALREADY_DELETED.value) {
                    /* Use iterator.remove() to remove from the set */
                    ict.remove();
                }
                if (result != DDS.RETCODE_OK.value ) {
                    ReportStack.report (result, "Deletion of ContentFilteredTopic contained in DomainParticipant failed.");

                    if (endResult == DDS.RETCODE_OK.value ) {
                        /* Store first encountered error. */
                        endResult = result;
                    }
                }
            }

            Iterator<TopicImpl> it = this.topics.iterator();
            while (it.hasNext()) {
                result = it.next().deinit();
                if (result == DDS.RETCODE_OK.value || result == DDS.RETCODE_ALREADY_DELETED.value) {
                    /* Use iterator.remove() to remove from the set */
                    it.remove();
                }
                if (result != DDS.RETCODE_OK.value ) {
                    ReportStack.report (result, "Deletion of Topic contained in DomainParticipant failed.");

                    if (endResult == DDS.RETCODE_OK.value ) {
                        /* Store first encountered error. */
                        endResult = result;
                    }
                }
            }
        }

        ReportStack.flush(this, endResult != DDS.RETCODE_OK.value);
        return endResult;
    }

    @Override
    public int set_qos (
            DDS.DomainParticipantQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        ListenerDispatcher dispatcher = null;
        DDS.SchedulingQosPolicyHolder scheduling_policy =
            new DDS.SchedulingQosPolicyHolder();

        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos == DDS.PARTICIPANT_QOS_DEFAULT.value) {
            DDS.DomainParticipantFactory factory;
            DDS.DomainParticipantQosHolder holder;

            factory = DDS.DomainParticipantFactory.get_instance();
            holder = new DDS.DomainParticipantQosHolder();

            factory.get_default_participant_qos(holder);
            qos = holder.value;
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            long uParticipant = this.get_user_object();
            if (uParticipant != 0) {
                dispatcher = get_dispatcher();
                result = dispatcher.get_scheduling(scheduling_policy);
                if (result == DDS.RETCODE_OK.value) {
                    result = dispatcher.set_scheduling(qos.listener_scheduling);
                    if (result == DDS.RETCODE_OK.value) {
                        result = jniSetQos(uParticipant, qos);
                        if (result != DDS.RETCODE_OK.value) {
                            dispatcher.set_scheduling(scheduling_policy.value);
                        }
                    }
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int get_qos (
            DDS.DomainParticipantQosHolder qos)
    {
        int result = DDS.RETCODE_OK.value;
        long uParticipant = 0;
        ListenerDispatcher dispatcher = null;
        DDS.SchedulingQosPolicyHolder scheduling_policy =
            new DDS.SchedulingQosPolicyHolder();

        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else {
            uParticipant = this.get_user_object();
            if (uParticipant != 0) {
                dispatcher = get_dispatcher();
                result = dispatcher.get_scheduling(scheduling_policy);
                if (result == DDS.RETCODE_OK.value) {
                    result = jniGetQos(uParticipant, qos);
                    if (result == DDS.RETCODE_OK.value) {
                        qos.value.listener_scheduling = scheduling_policy.value;
                    }
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int set_listener (
            DDS.DomainParticipantListener a_listener, int mask)
    {
        long uParticipant = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        synchronized (this)
        {
            uParticipant = this.get_user_object();
            if (uParticipant != 0) {
                this.listener = a_listener;
                result = this.set_listener_interest(mask);
                if (result == DDS.RETCODE_OK.value) {
                    this.listenerInterest = mask & (DDS.INCONSISTENT_TOPIC_STATUS.value |
                            DDS.ALL_DATA_DISPOSED_TOPIC_STATUS.value);
                    for(TopicImpl topic : topics) {
                        topic.set_participant_listener_mask(this.listenerInterest);
                    }
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public DDS.DomainParticipantListener get_listener ()
    {
        return this.listener;
    }

    @Override
    public int ignore_participant (
            long handle)
    {
        long uParticipant = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uParticipant = this.get_user_object();
        if (uParticipant != 0) {
            result = jniIgnoreParticipant(uParticipant, handle);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }
        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int ignore_topic (
            long handle)
    {
        long uParticipant = 0;
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        ReportStack.start();

        uParticipant = this.get_user_object();
        if (uParticipant != 0) {
            result = jniIgnoreTopic(uParticipant, handle);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int ignore_publication (
            long handle)
    {
        long uParticipant = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uParticipant = this.get_user_object();
        if (uParticipant != 0) {
            result = jniIgnorePublication(uParticipant, handle);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int ignore_subscription (
            long handle)
    {
        long uParticipant = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uParticipant = this.get_user_object();
        if (uParticipant != 0) {
            result = jniIgnoreSubscription(uParticipant, handle);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int get_domain_id ()
    {
        long uParticipant = 0;
        int result = DDS.DOMAIN_ID_INVALID.value;
        ReportStack.start();

        uParticipant = this.get_user_object();
        if (uParticipant != 0) {
            result = jniGetDomainId(uParticipant);
        }

        ReportStack.flush(this, result == DDS.DOMAIN_ID_INVALID.value);
        return result;
    }

    @Override
    public int assert_liveliness ()
    {
        long uParticipant = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uParticipant = this.get_user_object();
        if (uParticipant != 0) {
            result = jniAssertLiveliness(uParticipant);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int set_default_publisher_qos (
            DDS.PublisherQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos == DDS.PUBLISHER_QOS_DEFAULT.value) {
            qos = Utilities.defaultPublisherQos;
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            this.defaultPublisherQos = Utilities.deepCopy(qos);
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int get_default_publisher_qos (
            DDS.PublisherQosHolder qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else {
            qos.value = Utilities.deepCopy(this.defaultPublisherQos);
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int set_default_subscriber_qos (
            DDS.SubscriberQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos == DDS.SUBSCRIBER_QOS_DEFAULT.value) {
            qos = Utilities.defaultSubscriberQos;
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            this.defaultSubscriberQos = Utilities.deepCopy(qos);
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int get_default_subscriber_qos (
            DDS.SubscriberQosHolder qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else {
            qos.value = Utilities.deepCopy(this.defaultSubscriberQos);
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int set_default_topic_qos (
            DDS.TopicQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
        } else if (qos == DDS.TOPIC_QOS_DEFAULT.value) {
            qos = Utilities.defaultTopicQos;
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            this.defaultTopicQos = Utilities.deepCopy(qos);
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int get_default_topic_qos (
            DDS.TopicQosHolder qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else {
            qos.value = Utilities.deepCopy(this.defaultTopicQos);
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int get_discovered_participants (
            DDS.InstanceHandleSeqHolder participant_handles)
    {
        DDS.Subscriber sub;
        DataReaderImpl dr;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        sub = get_builtin_subscriber();
        if (sub != null) {
            dr = (DataReaderImpl)sub.lookup_datareader("DCPSParticipant");
            if (dr != null) {
                result = dr.read_instance_handles(participant_handles);
            } else {
                result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                ReportStack.report(result,
                        "Could not resolve builtin DataReader for Topic 'DCPSParticipant'.");
            }
        } else {
            result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int get_discovered_participant_data (
            DDS.ParticipantBuiltinTopicDataHolder participant_data, long handle)
    {
        DDS.Subscriber sub;
        DDS.ParticipantBuiltinTopicDataDataReaderImpl dr;
        DDS.ParticipantBuiltinTopicDataSeqHolder dataList = null;
        DDS.SampleInfoSeqHolder infoList = null;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        sub = get_builtin_subscriber();
        if (sub != null) {
            dr = (DDS.ParticipantBuiltinTopicDataDataReaderImpl)sub.lookup_datareader("DCPSParticipant");
            if (dr != null) {
                dataList = new DDS.ParticipantBuiltinTopicDataSeqHolder();
                infoList = new DDS.SampleInfoSeqHolder();
                result = dr.read_instance(dataList, infoList, 1, handle,
                        DDS.ANY_SAMPLE_STATE.value,
                        DDS.ANY_VIEW_STATE.value,
                        DDS.ANY_INSTANCE_STATE.value);
                if (result == DDS.RETCODE_OK.value ) {
                    if ( dataList.value.length == 1 ) {
                        participant_data.value = dataList.value[0];
                    }
                }
            } else {
                result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                ReportStack.report(result,
                        "Could not resolve builtin DataReader for Topic 'DCPSParticipant'.");
            }
        } else {
            result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int get_discovered_topics (
            DDS.InstanceHandleSeqHolder topic_handles)
    {
        DDS.Subscriber sub;
        DataReaderImpl dr;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        sub = get_builtin_subscriber();
        if (sub != null) {
            dr = (DataReaderImpl)sub.lookup_datareader("DCPSTopic");
            if (dr != null) {
                result = dr.read_instance_handles(topic_handles);
            } else {
                result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                ReportStack.report(result,
                        "Could not resolve builtin DataReader for Topic 'DCPSTopic'.");
            }
        } else {
            result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int get_discovered_topic_data (
            DDS.TopicBuiltinTopicDataHolder topic_data, long handle)
    {
        DDS.Subscriber sub;
        DDS.TopicBuiltinTopicDataDataReaderImpl dr;
        DDS.TopicBuiltinTopicDataSeqHolder dataList = null;
        DDS.SampleInfoSeqHolder infoList = null;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        sub = get_builtin_subscriber();
        if (sub != null) {
            dr = (DDS.TopicBuiltinTopicDataDataReaderImpl)sub.lookup_datareader("DCPSTopic");
            if (dr != null) {
                dataList = new DDS.TopicBuiltinTopicDataSeqHolder();
                infoList = new DDS.SampleInfoSeqHolder();
                result = dr.read_instance(dataList, infoList, 1, handle,
                        DDS.ANY_SAMPLE_STATE.value,
                        DDS.ANY_VIEW_STATE.value,
                        DDS.ANY_INSTANCE_STATE.value);
                if (result == DDS.RETCODE_OK.value ) {
                    if ( dataList.value.length == 1 ) {
                        topic_data.value = dataList.value[0];
                    }
                }
            } else {
                result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                ReportStack.report(result,
                        "Could not resolve builtin DataReader for Topic 'DCPSTopic'.");
            }
        } else {
            result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public boolean contains_entity (
            long a_handle)
    {
        boolean found = false;
        long handle;

        synchronized (this)
        {
            Iterator<TopicImpl> it = this.topics.iterator();
            while (it.hasNext() && !found) {
                handle = it.next().get_instance_handle();
                found = (handle == a_handle);
            }
            Iterator<PublisherImpl> ip = this.publishers.iterator();
            while (ip.hasNext() && !found) {
                PublisherImpl p = ip.next();
                handle = p.get_instance_handle();
                if (handle == a_handle) {
                    found = true;
                } else {
                    found = p.contains_entity(a_handle);
                }
            }
            Iterator<SubscriberImpl> is = this.subscribers.iterator();
            while (is.hasNext() && !found) {
                SubscriberImpl s = is.next();
                handle = s.get_instance_handle();
                if (handle == a_handle) {
                    found = true;
                } else {
                    found = s.contains_entity(a_handle);
                }
            }
            if (!found && this.builtinSubscriber != null) {
                if(this.builtinSubscriber.get_instance_handle() == a_handle){
                    found = true;
                } else if(this.builtinSubscriber.contains_entity(a_handle)){
                    found = true;
                }
            }
        }
        return found;
    }

    @Override
    public int get_current_time (
            DDS.Time_tHolder current_time)
    {
        long uParticipant = 0;
        int result = DDS.RETCODE_BAD_PARAMETER.value;
        ReportStack.start();

        if (current_time == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "current_time 'null' is invalid.");
        } else {
            uParticipant = this.get_user_object();
            if (uParticipant != 0) {
                result = jniGetCurrentTime(uParticipant, current_time);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    void setParticipantDataCopyCache(
            long copyCache)
    {
        this.participantDataCopyCache = copyCache;
    }

    void setTopicBuiltinTopicDataCopyCache(
            long copyCache)
    {
        this.topicBuiltinTopicDataCopyCache = copyCache;
    }

    @Override
    protected int notify(Event e)
    {
        int result = DDS.RETCODE_OK.value;

        DDS.DomainParticipantListener dpl = this.listener;
        DDS.ExtDomainParticipantListener edpl = null;
        if (dpl == null)
            return result;

        switch(e.kind) {
            case DDS.DATA_ON_READERS_STATUS.value:
                dpl.on_data_on_readers((DDS.Subscriber) e.observable);
                break;
            case DDS.DATA_AVAILABLE_STATUS.value:
                dpl.on_data_available((DDS.DataReader) e.observable);
                break;
            case DDS.REQUESTED_DEADLINE_MISSED_STATUS.value:
                dpl.on_requested_deadline_missed((DDS.DataReader) e.observable, (DDS.RequestedDeadlineMissedStatus) e.status);
                break;
            case DDS.REQUESTED_INCOMPATIBLE_QOS_STATUS.value:
                dpl.on_requested_incompatible_qos((DDS.DataReader) e.observable, (DDS.RequestedIncompatibleQosStatus) e.status);
                break;
            case DDS.SAMPLE_REJECTED_STATUS.value:
                dpl.on_sample_rejected((DDS.DataReader) e.observable, (DDS.SampleRejectedStatus) e.status);
                break;
            case DDS.LIVELINESS_CHANGED_STATUS.value:
                dpl.on_liveliness_changed((DDS.DataReader) e.observable, (DDS.LivelinessChangedStatus) e.status);
                break;
            case DDS.SUBSCRIPTION_MATCHED_STATUS.value:
                dpl.on_subscription_matched((DDS.DataReader) e.observable, (DDS.SubscriptionMatchedStatus) e.status);
                break;
            case DDS.SAMPLE_LOST_STATUS.value:
                dpl.on_sample_lost((DDS.DataReader) e.observable, (DDS.SampleLostStatus) e.status);
                break;
            case DDS.ALL_DATA_DISPOSED_TOPIC_STATUS.value:
                edpl = (ExtDomainParticipantListener) this.listener;
                edpl.on_all_data_disposed((DDS.Topic) e.observable);
                break;
            case DDS.INCONSISTENT_TOPIC_STATUS.value:
                edpl = (ExtDomainParticipantListener) this.listener;
                edpl.on_inconsistent_topic((DDS.Topic) e.observable, (DDS.InconsistentTopicStatus) e.status);
            break;
            case DDS.OFFERED_DEADLINE_MISSED_STATUS.value:
                dpl.on_offered_deadline_missed((DDS.DataWriter) e.observable, (DDS.OfferedDeadlineMissedStatus) e.status);
                break;
            case DDS.OFFERED_INCOMPATIBLE_QOS_STATUS.value:
                dpl.on_offered_incompatible_qos((DDS.DataWriter) e.observable, (DDS.OfferedIncompatibleQosStatus) e.status);
                break;
            case DDS.LIVELINESS_LOST_STATUS.value:
                dpl.on_liveliness_lost((DDS.DataWriter) e.observable, (DDS.LivelinessLostStatus) e.status);
                break;
            case DDS.PUBLICATION_MATCHED_STATUS.value:
                dpl.on_publication_matched((DDS.DataWriter) e.observable, (DDS.PublicationMatchedStatus) e.status);
                break;

            default:
                // TODO [jeroenk]: Should result be updated?
                ReportStack.report(DDS.RETCODE_UNSUPPORTED.value, "Received unsupported event kind '" + e.kind + "'.");
                break;
        }
        return result;
    }

    protected Map<String, TypeSupportImpl> get_typesupports() {
        return this.typeSupports;
    }

    protected int register_type (
            TypeSupportImpl typeSupport,
            java.lang.String type_alias)
    {
        int result = DDS.RETCODE_BAD_PARAMETER.value;

        synchronized (this) {
            long uParticipant = get_user_object();
            if (uParticipant != 0) {
                boolean res = add(typeSupport, type_alias);
                if (!res) {
                    TypeSupportImpl ts = lookup_typeSupport(type_alias);
                    if (ts != typeSupport) {
                        /* registry type name is used before */
                        /*
                         * OSPL-103: PRECONDITION_NOT_MET must be returned if
                         * type_keys does not match.
                         */
                        if (ts.get_type_name() != null && ts.get_key_list() != null) {
                            if (!ts.get_type_name().equals(typeSupport.get_type_name()) ||
                                    !ts.get_key_list().equals(typeSupport.get_key_list()))
                            {
                                result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                            } else {
                                result = DDS.RETCODE_OK.value;
                            }
                        } else {
                            result = DDS.RETCODE_ERROR.value;
                        }
                    } else {
                        /* register an already known type support again */
                        result = DDS.RETCODE_OK.value;
                    }
                } else {
                    result = jniRegisterType(uParticipant,
                                             type_alias,
                                             typeSupport.get_descriptor(),
                                             typeSupport.get_data_representation_id(),
                                             typeSupport.get_type_hash(),
                                             typeSupport.get_meta_data(),
                                             typeSupport.get_extentions());

                    long jniCopyCache = jniCopyCacheNew(uParticipant,
                                            typeSupport.get_type_name(),
                                            type_alias,
                                            typeSupport.get_package_redirects());
                    typeSupport.set_copyCache(jniCopyCache);
                    if (result != DDS.RETCODE_OK.value) {
                        remove(type_alias);
                    }
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        return result;
    }

    @Override
    public int delete_historical_data (
            String partition_expression,
            String topic_expression)
    {
        int result = DDS.RETCODE_OK.value;
        long uParticipant = 0;
        ReportStack.start();

        if (partition_expression == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "partition_expression 'null' is invalid.");
        } else if (topic_expression == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "topic_expression 'null' is invalid.");
        }

        if (result == DDS.RETCODE_OK.value) {
            synchronized (this)
            {
                uParticipant = this.get_user_object();
                if (uParticipant != 0) {
                    result = jniDeleteHistoricalData(uParticipant, partition_expression, topic_expression);
                    if (result != DDS.RETCODE_OK.value) {
                        ReportStack.report(result, "Could not delete historical data.");
                    }
                } else {
                    result = DDS.RETCODE_ERROR.value;
                    ReportStack.report(result, "Could not delete historical data invalid participant.");
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }


    private native long jniDomainParticipantNew(String name, int domainId, DDS.DomainParticipantQos qos);
    private native int jniDomainParticipantFree(long uParticipant);
    private native long jniFindTopic(long uParticipant, String topic_name, DDS.Duration_t timeout);
    private native int jniSetQos(long uParticipant, DDS.DomainParticipantQos qos);
    private native int jniGetQos(long uParticipant, DDS.DomainParticipantQosHolder qos);
    private native int jniIgnoreParticipant(long uParticipant, long handle);
    private native int jniIgnoreTopic(long uParticipant, long handle);
    private native int jniIgnorePublication(long uParticipant, long handle);
    private native int jniIgnoreSubscription(long uParticipant, long handle);
    private native int jniGetDomainId(long uParticipant);
    private native int jniAssertLiveliness(long uParticipant);
    private native int jniGetCurrentTime(long uParticipant, DDS.Time_tHolder current_time);
    private native int jniRegisterType(long uParticipant, String type_alias, String descriptor, short data_representation_id, byte[] type_hash, byte[] meta_data, byte[] extentions);
    private native int jniDeleteHistoricalData(long uParticipant, String partition_expression, String topic_expression);

    private native long jniCopyCacheNew(long uParticipant, String idlTypeName, String type_alias, String package_redirects);

}
