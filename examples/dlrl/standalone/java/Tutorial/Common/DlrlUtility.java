package Common;

/**
 * Contains various convienant utility operations used by the various
 * applications within this example.
 */
public class DlrlUtility
{
    private static DDS.DomainParticipantFactory dpFactory = DDS.DomainParticipantFactory.get_instance();
    private static DDS.CacheFactory cFactory = DDS.CacheFactory.get_instance();

    /**
     * This operation initializes the Cache for the specified CacheUsage
     * and attaches the relevant ObjectHomes to it. Finally, it also registers
     * all the required DCPS entities to the Cache, sets the appropriate QoS
     * settings and enables all these entities.
     *
     * @param name the name under which this Cache will be known within the
     * CacheFactory
     * @param usage the usage mode of the Cache (READ_ONLY, READ_WRITE or
     * WRITE_ONLY).
     * @param homes the array containing all homes that will be registered to
     * the Cache.
     * @throws Exception when something during initialization goes wrong.
     * @return the created Cache
     */
    public static DDS.Cache initializeDlrl(
        String cacheName,
        DDS.CacheUsage cacheUsage,
        DDS.ObjectHome[] homes) throws Exception
    {
        DDS.DomainParticipantQosHolder dpQos;
        DDS.SubscriberQosHolder subQos;
        DDS.PublisherQosHolder pubQos;
        DDS.DataWriterQosHolder dwQos;
        DDS.TopicQosHolder tQos;
        String[] topicNames;
        DDS.Cache cache = null;
        DDS.DomainParticipant participant = null;

        try
        {
            /* Create a DCPS DomainParticipant
             */
            dpQos = new DDS.DomainParticipantQosHolder();
            dpFactory.get_default_participant_qos(dpQos);
            participant = dpFactory.create_participant(DDS.DOMAIN_ID_DEFAULT.value, dpQos.value, null, 0);
            if (participant == null)
            {
                throw new DDS.DCPSError("Could not create DCPS DomainParticipant.");
            }

            /* Since most of the topics are transient, this makes for a good
             * default. Because the DLRL will always take the default topic qos
             * into account when creating a topic. And the topic Qos is taken
             * into account when creating the datareader and datawriter
             * entities. So we will eagerly take advantage of this feature.
             * Alternately one would adjust the QoS settings between the
             * register_all_for_pubsub and enable_all_for_pubsub for each
             * created DCPS entity (topic, reader, writer).
             */
            tQos = new DDS.TopicQosHolder();
            participant.get_default_topic_qos(tQos);
            tQos.value.reliability.kind = DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
            tQos.value.durability.kind = DDS.DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;
            participant.set_default_topic_qos(tQos.value);

            /* Use the CacheFactory to create a Cache. Each Cache is uniquely
             * identified by it's name and can always be found in the
             * CacheFactory by calling the 'find_cache_by_name' operation on the
             * CacheFactory. The usage of the cache will determine if
             * application is able to read (READ_ONLY), write (WRITE_ONLY) or
             * both (READ_WRITE). Caches that are readable have a DDS.Subscriber
             * entity created for that Cache, Caches that are writeable have a
             * DDS.Publisher entity created for that Cache. These entities are
             * directly accessible.
             */
            cache = cFactory.create_cache(
                cacheName,
                cacheUsage,
                participant);

            /* After creation of the cache the DCPS publisher and/or DCPS
             * subscriber entities are directly available as explained. We can
             * change any QoS settings we need. In our example we need to change
             * the partition the subscriber and publisher are operating on to
             * match that of the already existing DCPS chatter application to
             * ensure they will communicate with eachother. We will change the
             * partition to the 'ChatRoom' partition.
             *
             * For the publisher, we will also change the default datawriter
             * qos to ensure the datawriter will not auto dispose entities that
             * are unregistered. The reasoning behind this is that we do not
             * want our objects we wrote to become disposed at the reader sides
             * when the writer (i.e., CacheAccess) is destroyed (e.g., the
             * application terminates)
             */
            if(cache.the_subscriber() != null)
            {
                subQos = new DDS.SubscriberQosHolder();
                cache.the_subscriber().get_qos(subQos);
                subQos.value.partition.name = new String[1];
                subQos.value.partition.name[0] = "ChatRoom";
                cache.the_subscriber().set_qos(subQos.value);
            }
            if(cache.the_publisher() != null)
            {
                pubQos = new DDS.PublisherQosHolder();
                dwQos = new DDS.DataWriterQosHolder();

                cache.the_publisher().get_default_datawriter_qos(dwQos);
                dwQos.value.writer_data_lifecycle.autodispose_unregistered_instances = false;
                cache.the_publisher().set_default_datawriter_qos(dwQos.value);

                cache.the_publisher().get_qos(pubQos);
                pubQos.value.partition.name = new String[1];
                pubQos.value.partition.name[0] = "ChatRoom";
                cache.the_publisher().set_qos(pubQos.value);
            }

            /* Instantiate and register all relevant ObjectHomes to the Cache.
             * Each application model can define it's own view on the global
             * system state by tailoring it's local object model accordingly.
             * Not only does this increase performance (no need to manage
             * relations or objects that are of no interest), it also provides
             * a high level of flexibility when developing applications as each
             * application can look at the 'world' differently, while DLRL will
             * manage how that view relates to the global system state.
             */
            for(int i = 0; i < homes.length; i++)
            {
                cache.register_home(homes[i]);
            }

            /* Now that all homes are registered, we can instruct the cache to
             * create all DCPS entities needed for publication/subscribtion.
             * After this call no more ObjectHomes can be registered and all
             * DCPS entities such as readers, writers and topics are available.
             * It is important to note that the DCPS entities will be created in
             * a disabled state, this allows the application to change certain
             * immutable QoS settings by simply navigating to the desired DCPS
             * entity through the home and changing it's QoS. In our example
             * however we do not have to change any QoS settings here, as each
             * reader, writer and topic will have the correct QoS setting
             * through the copy_from_topic_qos usage of DLRL when creating
             * readers and writers and the usage of the default topic qos
             * or the qos settings if the topic already exists within the system
             * when creating the topics.
             */
            cache.register_all_for_pubsub();

            /* After all DCPS entities have been created and the QoS settings
             * have been adapted, we can enable the cache for pub/sub. After
             * this call the cache will be fully participating in the
             * system.
             */
            cache.enable_all_for_pubsub();

        }
        catch (Exception e)
        {
            /* Clean up our resources and then propagate the exception */
            deleteCache(cache);
            deleteParticipant(participant);
            throw e;
        }
        return cache;
    }

    /**
     * This operation deletes a cache and the attached participant
     *
     * @param cache the cache that is to be deleted, may be null.
     * @throws DDS.DCPSError if the deletion of the participant linked to the
     * cache fails.
     */
    public static void deleteCacheAndParticipant(
        DDS.Cache cache) throws DDS.DCPSError
    {
        DDS.DomainParticipant participant;
        if(cache != null)
        {
            participant = cache.the_participant();
            cFactory.delete_cache(cache);
            deleteParticipant(participant);
        }
    }

    /**
     * This operation deletes a cache, but not the attached participant
     *
     * @param cache the cache that is to be deleted, may be null.
     */
    public static void deleteCache(
        DDS.Cache cache)
    {
        if(cache != null)
        {
            cFactory.delete_cache(cache);
        }
    }

    /**
     * This operation deletes a participant
     *
     * @param participant the participant that is to be deleted, may be null.
     * @throws DDS.DCPSError if the deletion of the participant fails.
     */
    public static void deleteParticipant(
        DDS.DomainParticipant participant) throws DDS.DCPSError
    {
        int result;

        if(participant != null)
        {
            result = participant.delete_contained_entities();
            if (result == DDS.RETCODE_OK.value)
            {
                result = dpFactory.delete_participant(participant);
            }
            if (result != DDS.RETCODE_OK.value)
            {
                throw new DDS.DCPSError("Unable to delete the DomainParticipant correctly: error code = " + result + ".");
            }
        }
    }
}