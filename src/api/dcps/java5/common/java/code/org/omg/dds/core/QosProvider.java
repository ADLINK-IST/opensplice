/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.omg.dds.core;

import org.omg.dds.domain.DomainParticipantFactoryQos;
import org.omg.dds.domain.DomainParticipantQos;
import org.omg.dds.pub.DataWriterQos;
import org.omg.dds.pub.PublisherQos;
import org.omg.dds.sub.DataReaderQos;
import org.omg.dds.sub.SubscriberQos;
import org.omg.dds.topic.TopicQos;

/**
 * The QoS provider API allows users to specify the QoS settings of their DCPS entities outside of application code in XML.
 * This can be seen as a useful feature where code recompilation is restricted during the later stages of
 * application development / during application support.
 * <p>
 * Example:
 * <pre>
 * <code>
 * // Create a named QosProvider from a URL
 * QosProvider qosProvider = QosProvider.newQosProvider("file://qos.xml", "foo-profile", env);
 *
 * //Create a topic named "FooTopic" using the QoS Provider and no Listener and no status.
 * TopicListener&lt;Foo&gt; listener = null;
 * Collection&lt;Class&lt;? extends Status&gt;&gt; status = new HashSet&lt;Class&lt;? extends Status&gt;&gt;();
 * Topic&lt;Foo&gt; fooTopic = participant.createTopic("FooTopic", Foo.class,qosProvider.getTopicQos("foo-topic-qos"),listener, status);
 * </code>
 * </pre>
 */
public abstract class QosProvider implements DDSObject {

    /**
     * Constructs a new QosProvider based on the provided uri and profile.
     *
     * @param uri
     *            A Uniform Resource Identifier (URI that points to the location
     *            where the QoS profile needs to be loaded from. Currently only
     *            URI's with a "file" scheme that point to an XML file are
     *            supported. If profiles and/or QoS settings are not uniquely
     *            identifiable by name within the resource pointed to by the
     *            uri, a random one of them will be stored.
     * @param profile
     *            The name of the QoS profile that serves as the default QoS
     *            profile for the get*Qos operations.
     * @return A QosProvider instance that is instantiated with all profiles
     *         and/or QoS-ses loaded from the location specified by the provided
     *         uri.
     *
     * @throws IllegalArgumentException
     *             If no uri is provided or the resource pointed to by uri
     *             cannot be found or the content of the resource pointed to by
     *             uri is malformed (e.g. malformed XML).
     *
     */
    public static QosProvider newQosProvider(String uri,
            String profile,
            ServiceEnvironment env)
    {
        if (env == null) {
            throw new IllegalArgumentException("Invalid environment provided");
        }
        return env.getSPI().newQosProvider(uri, profile);
    }

    /**
     * Resolves the DomainParticipantFactoryQos associated with QosProvider.
     *
     * @return A valid DomainParticipantFactoryQos instance, null if no
     *         DomainParticipantFactoryQos can be found in the associated
     *         QosProvider.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the QosProvider instance is not properly initialized.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if not enough memory is available to perform the operation.
     * @throws org.omg.dds.core.DDSException
     *             if an internal error occurred.
     * @throws java.lang.UnsupportedOperationException
     *             if the supplied operation is not implemented.
     *
     */
    public abstract DomainParticipantFactoryQos getDomainParticipantFactoryQos();

    /**
     * Resolves the DomainParticipantFactoryQos identified by the id from the
     * uri the QosProvider is associated with.
     *
     * @param id
     *            The fully-qualified name that identifies a QoS within the uri
     *            associated with the QosProvider or a name that identifies a
     *            QoS within the uri associated with the QosProvider instance
     *            relative to its default QoS profile. Id's starting with "::"
     *            are interpreted as fully-qualified names and all others are
     *            interpreted as names relative to the default QoS profile of
     *            the QosProvider instance. A nil id is interpreted as a
     *            non-named QoS within the default QoS profile associated with
     *            the QosProvider.
     *
     * @return A valid DomainParticipantFactoryQos instance, null if no
     *         DomainParticipantFactoryQos that matches the provided id can be
     *         found within the uri associated with the QosProvider.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the QosProvider instance is not properly initialized.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if not enough memory is available to perform the operation.
     * @throws org.omg.dds.core.DDSException
     *             if an internal error occurred.
     * @throws java.lang.UnsupportedOperationException
     *             if the supplied operation is not implemented.
     */
    public abstract DomainParticipantFactoryQos getDomainParticipantFactoryQos(String id);

    /**
     * Resolves the DomainParticipantQos associated with QosProvider.
     *
     * @return A valid DomainParticipantQos instance, null if no
     *         DomainParticipantQos can be found in the associated QosProvider.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the QosProvider instance is not properly initialized.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if not enough memory is available to perform the operation.
     * @throws org.omg.dds.core.DDSException
     *             if an internal error occurred.
     */
    public abstract DomainParticipantQos getDomainParticipantQos();

    /**
     * Resolves the DomainParticipantQos identified by the id from the uri the
     * QosProvider is associated with.
     *
     * @param id
     *            The fully-qualified name that identifies a QoS within the uri
     *            associated with the QosProvider or a name that identifies a
     *            QoS within the uri associated with the QosProvider instance
     *            relative to its default QoS profile. Id's starting with "::"
     *            are interpreted as fully-qualified names and all others are
     *            interpreted as names relative to the default QoS profile of
     *            the QosProvider instance. A nil id is interpreted as a
     *            non-named QoS within the default QoS profile associated with
     *            the QosProvider.
     *
     * @return A valid DomainParticipantQos instance, null if no
     *         DomainParticipantQos that matches the provided id can be found
     *         within the uri associated with the QosProvider.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the QosProvider instance is not properly initialized.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if not enough memory is available to perform the operation.
     * @throws org.omg.dds.core.DDSException
     *             if an internal error occurred.
     */
    public abstract DomainParticipantQos getDomainParticipantQos(String id);

    /**
     * Resolves the TopicQos associated with QosProvider.
     *
     * @return A valid TopicQos instance, null if no TopicQos can be found in
     *         the associated QosProvider.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the QosProvider instance is not properly initialized.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if not enough memory is available to perform the operation.
     * @throws org.omg.dds.core.DDSException
     *             if an internal error occurred.
     */
    public abstract TopicQos getTopicQos();

    /**
     * Resolves the TopicQos identified by the id from the uri the QosProvider
     * is associated with.
     *
     * @param id
     *            The fully-qualified name that identifies a QoS within the uri
     *            associated with the QosProvider or a name that identifies a
     *            QoS within the uri associated with the QosProvider instance
     *            relative to its default QoS profile. Id's starting with "::"
     *            are interpreted as fully-qualified names and all others are
     *            interpreted as names relative to the default QoS profile of
     *            the QosProvider instance. A nil id is interpreted as a
     *            non-named QoS within the default QoS profile associated with
     *            the QosProvider.
     *
     * @return A valid TopicQos instance, null if no TopicQos that matches the
     *         provided id can be found within the uri associated with the
     *         QosProvider.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the QosProvider instance is not properly initialized.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if not enough memory is available to perform the operation.
     * @throws org.omg.dds.core.DDSException
     *             if an internal error occurred.
     */
    public abstract TopicQos getTopicQos(String id);

    /**
     * Resolves the SubscriberQos associated with QosProvider.
     *
     * @return A valid SubscriberQos instance, null if no SubscriberQos can be
     *         found in the associated QosProvider.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the QosProvider instance is not properly initialized.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if not enough memory is available to perform the operation.
     * @throws org.omg.dds.core.DDSException
     *             if an internal error occurred.
     */
    public abstract SubscriberQos getSubscriberQos();

    /**
     * Resolves the SubscriberQos identified by the id from the uri the
     * QosProvider is associated with.
     *
     * @param id
     *            The fully-qualified name that identifies a QoS within the uri
     *            associated with the QosProvider or a name that identifies a
     *            QoS within the uri associated with the QosProvider instance
     *            relative to its default QoS profile. Id's starting with "::"
     *            are interpreted as fully-qualified names and all others are
     *            interpreted as names relative to the default QoS profile of
     *            the QosProvider instance. A nil id is interpreted as a
     *            non-named QoS within the default QoS profile associated with
     *            the QosProvider.
     *
     * @return A valid SubscriberQos instance, null if no SubscriberQos that
     *         matches the provided id can be found within the uri associated
     *         with the QosProvider.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the QosProvider instance is not properly initialized.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if not enough memory is available to perform the operation.
     * @throws org.omg.dds.core.DDSException
     *             if an internal error occurred.
     */
    public abstract SubscriberQos getSubscriberQos(String id);

    /**
     * Resolves the PublisherQos associated with QosProvider.
     *
     * @return A valid PublisherQos instance, null if no PublisherQos can be
     *         found in the associated QosProvider.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the QosProvider instance is not properly initialized.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if not enough memory is available to perform the operation.
     * @throws org.omg.dds.core.DDSException
     *             if an internal error occurred.
     */
    public abstract PublisherQos getPublisherQos();

    /**
     * Resolves the PublisherQos identified by the id from the uri the
     * QosProvider is associated with.
     *
     * @param id
     *            The fully-qualified name that identifies a QoS within the uri
     *            associated with the QosProvider or a name that identifies a
     *            QoS within the uri associated with the QosProvider instance
     *            relative to its default QoS profile. Id's starting with "::"
     *            are interpreted as fully-qualified names and all others are
     *            interpreted as names relative to the default QoS profile of
     *            the QosProvider instance. A nil id is interpreted as a
     *            non-named QoS within the default QoS profile associated with
     *            the QosProvider.
     *
     * @return A valid PublisherQos instance, null if no PublisherQos that
     *         matches the provided id can be found within the uri associated
     *         with the QosProvider.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the QosProvider instance is not properly initialized.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if not enough memory is available to perform the operation.
     * @throws org.omg.dds.core.DDSException
     *             if an internal error occurred.
     */
    public abstract PublisherQos getPublisherQos(String id);

    /**
     * Resolves the DataReaderQos associated with QosProvider.
     *
     * @return A valid DataReaderQos instance, null if no DataReaderQos can be
     *         found in the associated QosProvider.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the QosProvider instance is not properly initialized.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if not enough memory is available to perform the operation.
     * @throws org.omg.dds.core.DDSException
     *             if an internal error occurred.
     */
    public abstract DataReaderQos getDataReaderQos();

    /**
     * Resolves the DataReaderQos identified by the id from the uri the
     * QosProvider is associated with.
     *
     * @param id
     *            The fully-qualified name that identifies a QoS within the uri
     *            associated with the QosProvider or a name that identifies a
     *            QoS within the uri associated with the QosProvider instance
     *            relative to its default QoS profile. Id's starting with "::"
     *            are interpreted as fully-qualified names and all others are
     *            interpreted as names relative to the default QoS profile of
     *            the QosProvider instance. A nil id is interpreted as a
     *            non-named QoS within the default QoS profile associated with
     *            the QosProvider.
     *
     * @return A valid DataReaderQos instance, null if no DataReaderQos that
     *         matches the provided id can be found within the uri associated
     *         with the QosProvider.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the QosProvider instance is not properly initialized.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if not enough memory is available to perform the operation.
     * @throws org.omg.dds.core.DDSException
     *             if an internal error occurred.
     */
    public abstract DataReaderQos getDataReaderQos(String id);

    /**
     * Resolves the DataWriterQos associated with QosProvider.
     *
     * @return A valid DataWriterQos instance, null if no DataWriterQos can be
     *         found in the associated QosProvider.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the QosProvider instance is not properly initialized.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if not enough memory is available to perform the operation.
     * @throws org.omg.dds.core.DDSException
     *             if an internal error occurred.
     */
    public abstract DataWriterQos getDataWriterQos();

    /**
     * Resolves the DataWriterQos identified by the id from the uri the
     * QosProvider is associated with.
     *
     * @param id
     *            The fully-qualified name that identifies a QoS within the uri
     *            associated with the QosProvider or a name that identifies a
     *            QoS within the uri associated with the QosProvider instance
     *            relative to its default QoS profile. Id's starting with "::"
     *            are interpreted as fully-qualified names and all others are
     *            interpreted as names relative to the default QoS profile of
     *            the QosProvider instance. A nil id is interpreted as a
     *            non-named QoS within the default QoS profile associated with
     *            the QosProvider.
     *
     * @return A valid DataWriterQos instance, null if no DataWriterQos that
     *         matches the provided id can be found within the uri associated
     *         with the QosProvider.
     *
     * @throws org.omg.dds.core.PreconditionNotMetException
     *             if the QosProvider instance is not properly initialized.
     * @throws org.omg.dds.core.OutOfResourcesException
     *             if not enough memory is available to perform the operation.
     * @throws org.omg.dds.core.DDSException
     *             if an internal error occurred.
     */
    public abstract DataWriterQos getDataWriterQos(String id);
}
