Partition QoS              {#DCPS_QoS_Partition}
================
This QosPolicy specifies the logical partitions in which the Subscribers
and Publishers are active.

Attributes
----------
<table>
    <tr>
        <th>Value</th>
        <th>Meaning</th>
        <th>Concerns</th>
        <th>RxO</th>
        <th>Changeable</th>
    </tr>
    <tr>
        <td>
            A string sequence:<br/>
            name
        </td>
        <td>
            A sequence of strings that introduces a
            logical partition among the topics
            visible by the \ref DCPS_Modules_Publication "Publisher" and
            \ref DCPS_Modules_Subscription "Subscriber". A \ref DCPS_Modules_Publication_DataWriter "DataWriter" within a
            \ref DCPS_Modules_Publication "Publisher" only communicates with
            a \ref DCPS_Modules_Subscription_DataReader "DataReader" in a \ref DCPS_Modules_Subscription "Subscriber" if (in
            addition to matching the \ref DCPS_Modules_TopicDefinition "Topic" and
            having compatible QoS) the
            \ref DCPS_Modules_Publication "Publisher" and \ref DCPS_Modules_Subscription "Subscriber" have a
            common partition name string. The
            empty string ("") is considered a
            valid partition that is matched with
            other partition names using the same
            rules of string matching and regular-
            expression matching used for any
            other partition name. The default value for the
            PARTITION QoS is a zero-length
            sequence. The zero-length sequence
            is treated as a special value
            equivalent to a sequence containing
            a single element consisting of the
            empty string.
        </td>
        <td>\ref DCPS_Modules_Publication "Publisher", \ref DCPS_Modules_Subscription "Subscriber"</td>
        <td>No</td>
        <td>Yes</td>
    </tr>
</table>

![Partition seperation](@ref Terminology_Layout_A4.png)

This policy allows the introduction of a logical partition concept inside the ‘physical’ partition induced by a domain.
For a \ref DCPS_Modules_Subscription_DataReader "DataReader" to see the changes made to an instance by a \ref DCPS_Modules_Publication_DataWriter "DataWriter", not only the \ref DCPS_Modules_TopicDefinition "Topic" must match, but also they must share a common partition. Each string in the list that defines this QoS policy defines a partition name.
A partition name may contain wildcards. Sharing a common partition means that one of the partition names matches.

Failure to match partitions is not considered an “incompatible” QoS and does not trigger any listeners nor conditions.

This policy is changeable. A change of this policy can potentially modify the “match” of existing \ref DCPS_Modules_Subscription_DataReader "DataReader" and
\ref DCPS_Modules_Publication_DataWriter "DataWriter" entities. It may establish new “matches” that did not exist before, or break existing matches.

PARTITION names can be regular expressions and include wildcards as defined by the POSIX fnmatch API (1003.2-1992
section B.6). Either \ref DCPS_Modules_Publication "Publisher" or \ref DCPS_Modules_Subscription "Subscriber" may include regular expressions in partition names, but no two names that
both contain wildcards will ever be considered to match. This means that although regular expressions may be used both
at \ref DCPS_Modules_Publication "Publisher" as well as \ref DCPS_Modules_Subscription "Subscriber" side, the service will not try to match two regular expressions (between publishers and
subscribers).

Partitions are different from creating Entity objects in different domains in several ways. First, entities belonging to
different domains are completely isolated from each other; there is no traffic, meta-traffic or any other way for an
application or the Service itself to see entities in a domain it does not belong to. Second, an Entity can only belong to one
domain whereas an Entity can be in multiple partitions. Finally, as far as the DDS Service is concerned, each unique data
instance is identified by the tuple (domainId, \ref DCPS_Modules_TopicDefinition "Topic", key). Therefore two Entity objects in different domains cannot refer
to the same data instance. On the other hand, the same data-instance can be made available (published) or requested
(subscribed) on one or more partitions.
