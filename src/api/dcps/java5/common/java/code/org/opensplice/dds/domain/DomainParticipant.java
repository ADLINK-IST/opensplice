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

}
