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

package org.omg.dds.core.policy;

import java.util.Collection;
import java.util.Set;



/**
 * This policy allows the introduction of a logical partition concept inside
 * the "physical" partition induced by a domain. It consists of a set of
 * strings that introduces a logical partition among the topics
 * visible by the {@link org.omg.dds.pub.Publisher} and {@link org.omg.dds.sub.Subscriber}.
 *<p>
 * <b>Concerns:</b> {@link org.omg.dds.pub.Publisher}, {@link org.omg.dds.sub.Subscriber}
 *<p>
 * <b>RxO:</b> No
 *<p>
 * <b>Changeable:</b> Yes
 *<p>
 * A {@link org.omg.dds.pub.DataWriter} within a Publisher only communicates with a
 * {@link org.omg.dds.sub.DataReader} in a Subscriber if (in addition to matching the Topic
 * and having compatible QoS) the Publisher and Subscriber have a common
 * partition name string. Each string in the collection that defines this QoS
 * policy defines a partition name. A partition name may contain wild cards.
 * Sharing a common partition means that one of the partition names matches.
 * <p>
 * Failure to match partitions is not considered an "incompatible" QoS and
 * does not trigger any listeners nor conditions.
 * <p>
 * This policy is changeable. A change of this policy can potentially modify
 * the "match" of existing DataReader and DataWriter entities. It may
 * establish new "matches" that did not exist before, or break existing
 * matches.
 * <p>
 * The empty string ("") is considered a valid partition that is matched with
 * other partition names using the same rules of string matching and
 * regular expression matching used for any other partition name.
 * <p>
 * The default value for is a zero-size collection. The
 * zero-size collection is treated as a special value equivalent to a
 * collection containing a single element consisting of the empty string.
 * <p>
 * This policy is changeable. A change of this policy can potentially modify
 * the "match" of existing DataReader and DataWriter entities. It may
 * establish new "matches" that did not exist before, or break existing
 * matches.
 * <p>
 * PARTITION names can be regular expressions and include wild cards as
 * defined by the POSIX fnmatch API (1003.2-1992 section B.6). Either
 * {@link org.omg.dds.pub.Publisher} or {@link org.omg.dds.sub.Subscriber} may include regular expressions in
 * partition names, but no two names that both contain wild cards will ever
 * be considered to match. This means that although regular expressions may
 * be used both at publisher as well as subscriber side, the service will not
 * try to match two regular expressions (between publishers and subscribers).
 * <p>
 * Partitions are different from creating {@link org.omg.dds.core.Entity} objects in different
 * domains in several ways. First, entities belonging to different domains
 * are completely isolated from each other; there is no traffic, meta-traffic
 * or any other way for an application or the Service itself to see entities
 * in a domain it does not belong to. Second, an Entity can only belong to
 * one domain whereas an Entity can be in multiple partitions. Finally, as
 * far as the DDS Service is concerned, each unique data instance is
 * identified by the tuple (domainId, Topic, key). Therefore two Entity
 * objects in different domains cannot refer to the same data instance. On
 * the other hand, the same data instance can be made available (published)
 * or requested (subscribed) on one or more partitions.
 */
public interface Partition
extends QosPolicy.ForPublisher, QosPolicy.ForSubscriber
{
    /**
     * @return  an unmodifiable collection of partition names.
     */
    public Set<String> getName();


    // --- Modification: -----------------------------------------------------

    /**
     * Copy this policy and override the value of the property.
     * @param name              A Collection of strings, which specifies the partitions.
     *
     * @return  a new Partition policy
     */
    public Partition withName(Collection<String> name);

    /**
     * Copy this policy and override the value of the property.
     * @param name              A name string , which specifies the partition.
     *
     * @return  a new Partition policy
     */
    public Partition withName(String name);

    /**
     * Copy this policy and override the value of the property.
     * @param names              An arbitrary number of name strings , which specifies the partitions.
     *
     * @return  a new Partition policy
     */
    public Partition withName(String... names);
}
