/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
package org.opensplice.dds.domain;


/**
 * OpenSplice-specific extension of {@link org.omg.dds.domain.DomainParticipant} with
 * support for deleting historical data from the durability service and creating a snapshot of current
 * persistent data
 *
 */
public interface DomainParticipant extends org.omg.dds.domain.DomainParticipant {
    /**
     * This operation deletes all historical TRANSIENT and PERSISTENT data that is
     * stored by the durability service that is configured to support this DomainParticipant.
     * It only deletes the samples stored in the transient and persistent store, samples stored
     * in individual application DataReaders is spared and remains available to these
     * readers. However, late-joiners will no longer be able to obtain the deleted samples.
     * <p>
     * The partition_expression and topic_expression strings can be used to
     * specify selection criteria for the topic and/or partition in which the data will be
     * deleted. Wildcards are supported. Note that these parameters are mandatory and
     * cannot be empty. The "*" expression can be used to match all partitions and/or topics.
     * Only data that exists prior to this method invocation is deleted. Data that is still being
     * inserted during this method invocation will not be removed.
     *
     * @param partitionExpression   An expression to define a filter on partitions.
     * @param topicExpression       An expression to define a filter on topic names.
     * @throws IllegalArgumentException        if partitionExpression or topicExpression is null.
     * @throws org.omg.dds.core.DDSException    An internal error has occurred.
     */
    public void deleteHistoricalData(String partitionExpression,String topicExpression);

    /**
     * This operation will create a snapshot of all persistent data matching the provided
     * partition and topic expressions and store the snapshot at the location indicated by
     * the URI. Only persistent data available on the local node is considered. This
     * operation will fire an event to trigger the snapshot creation by the durability service
     * and then return while the durability service fulfills the snapshot request; if no
     * durability service is available then there is no persistent data available and the
     * operation will return OK as a snapshot of an empty store is an empty store.
     * <p>
     * The created snapshot can then be used as the persistent store for the durability
     * service next time it starts up by configuring the location of the snapshot as the
     * persistent store in the configuration file. The durability service will then use the
     * snapshot as the regular store (and can thus also alter its contents).
     *
     * @param partitionExpression   The expression of all partitions involved in the snapshot;
     *                              this may contain wildcards
     * @param topicExpression       The expression of all topics involved in the snapshot;
     *                              this may contain wildcards.
     * @param uri                   The location where to store the snapshot.
     *                              Currently only directories are supported.
     * @throws IllegalArgumentException        if partitionExpression, topicExpression or uri is null.
     * @throws org.omg.dds.core.DDSException    An internal error has occurred.
     */
    public void createPersistentSnapshot(String partitionExpression,String topicExpression,String uri);

    /**
     * This operation sets the property specified by a key value pair.
     * Currently, the following property is defined:
     *
     * isolateNode
     * The isolateNode property allows applications to isolate the federation from the rest of
     * the Domain, i.e. at network level disconnect the node from the rest of the system.
     * Additionally, they also need to be able to issue a request to reconnect their federation
     * to the domain again after which the durability merge-policy that is configured needs to be
     * applied.
     *
     * To isolate a federation, the application needs to set the isolateNode property value
     * to 'true' and to (de)isolate the federation the same property needs to set to 'false'.
     * The default value of the isolateNode property is 'false'.
     *
     * All data that is published after isolateNode is set to true will not be sent to the network
     * and any data received from the network will be ignored. Be aware that data being processed
     * by the network service at time of isolating a node may still be sent to the network due
     * to asynchronous nature of network service internals.
     *
     * The value is interpreted as a boolean (i.e., it must be either 'true' or 'false').
     * false (default): The federation is connected to the domain.
     * true: The federation is disconnected from the domain meaning that data is not published
     * on the network and data from the network is ignored.
     *
     * @param   key      The name of the property
     * @param   value    The value of the property
     *
     * @throws  IllegalArgumentException
     *                  if an invalid value has been specified
     * @throws  UnsupportedOperationException
     *                  if the key specifies an undefined property or the operation is not
     *                  supported in this version.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DomainParticipant has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     *
     */
    public void setProperty(String key, String value);

    /**
     * This operation looks up the property for a given key
     * in the DomainParticipant, returning the value belonging to this key
     * If the property has not been set using setProperty,
     * the default value of the property is returned.
     *
     * @param   key      The name of the property to request the value from
     * @return the value of the requested property
     *
     * @throws  IllegalArgumentException
     *                  if an invalid key has been specified
     * @throws  UnsupportedOperationException
     *                  if the key specifies an undefined property or the operation is not
     *                  supported in this version.
     * @throws org.omg.dds.core.DDSException
     *                  An internal error has occurred.
     * @throws org.omg.dds.core.AlreadyClosedException
     *                  The corresponding DomainParticipant has been closed.
     * @throws org.omg.dds.core.OutOfResourcesException
     *                  The Data Distribution Service ran out of resources to
     *                  complete this operation.
     *
     */
    public String getProperty(String key);

}
