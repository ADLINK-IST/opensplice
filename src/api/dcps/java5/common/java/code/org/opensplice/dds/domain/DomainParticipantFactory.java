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
 * OpenSplice-specific extension of {@link org.omg.dds.domain.DomainParticipantFactory} with
 * support for detaching from all domains
 *
 */
public interface DomainParticipantFactory {
    /**
     * This operation safely detaches the application from all domains it is currently
     * participating in.
     *
     * <p><b>Note:</b> This is a proprietary OpenSplice extension.</p>
     *
     * <p>
     * This operation safely detaches the application from all domains it is currently
     * participating in. When this operation has been performed successfully,
     * the application is no longer connected to any Domain.
     * For Federated domains finishing this operation successfully means that all shared
     * memory segments have been safely un-mapped from the application process.
     * For SingleProcess mode domains this means all services for all domains have been
     * stopped. This allows graceful termination of the OSPL services that run as threads
     * within the application. Graceful termination of services in this mode would for
     * instance allow durability flushing of persistent data and networking termination
     * announcement over the network.
     * When this call returns further access to all domains will be denied and it will
     * not be possible for the application to open or re-open any DDS domain.
     * </p>
     * The behavior of the detach_all_domains operation is determined by the block_operations
     * and delete_entities parameters:<br>
     * <dl>
     * <dt>block_operations:</dt>
     * <dd>This parameter specifies if the application wants any DDS operation to be blocked
     *     or not while detaching. When true, any DDS operation called during this operation
     *     will be blocked and remain blocked forever (so also after the detach operation has
     *     completed and returns to the caller). When false, any DDS operation called during
     *     this operation may return RETCODE_ALREADY_DELETED. Please note that a listener
     *     callback is not considered an operation in progress. Of course, if a DDS operation
     *     is called from within the listener callback, that operation will be blocked
     *     during the detaching if this attribute is set to TRUE.
     * </dd>
     * <dt>delete_entities:</dt>
     * <dd>This parameter specifies if the application wants the DDS entities created by
     *     the application to be deleted (synchronously) while detaching from the domain or
     *     not. If true, all application entities are guaranteed to be deleted when the call
     *     returns. If false, application entities will not explicitly be deleted by this
     *     operation. In case of federated mode, the splice-daemon will delete them
     *     asynchronously after this operation has returned. In case of SingleProcess mode
     *     this attribute is ignored and clean up will always be performed, as this cannot
     *     be delegated to a different process.
     * </dd>
     * </dl>
     * <p>
     * <b>Note:</b> In federated mode when the detach_all_domain operation is called with
     * block_operations is false and delete_entities is false then the DDS operations
     * which are in progress and which are waiting for some condition to become true
     * or waiting for an event to occur while the detach operation is performed may be
     * blocked.
     * </p>
     *
     * @param blockOperations  Indicates whether the application wants any operations that
     *                         are called while detaching to be blocked or not.
     * @param deleteEntities   Indicates whether the application DDS entities in the 'connected'
     *                         domains must be deleted synchronously during detaching.
     */
    public void detachAllDomains(boolean blockOperations, boolean deleteEntities);
}
