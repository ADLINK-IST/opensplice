#include "DlrlUtility.h"

using namespace Common;

static DDS::DomainParticipantFactory_var dpFactory = DDS::DomainParticipantFactory::get_instance();

static DDS::CacheFactory_var cFactory = DDS::CacheFactory::get_instance();

DDS::Cache_ptr
DlrlUtility::initializeDlrl(
    const char* cacheName,
    DDS::CacheUsage cacheUsage,
    DDS::ObjectHomeSeq& homes)
{
    DDS::DomainParticipantQos dpQos;
    DDS::SubscriberQos subQos;
    DDS::PublisherQos pubQos;
    DDS::DataWriterQos dwQos;
    DDS::TopicQos tQos;
    DDS::StringSeq_var topicNames;
    DDS::Cache_ptr cache = NULL;
    DDS::DomainParticipant_var participant = NULL;
    DDS::Subscriber_var subscriber = NULL;
    DDS::Publisher_var publisher = NULL;

    try
    {
        /* Create a DCPS DomainParticipant
         */
        dpFactory->get_default_participant_qos(dpQos);
        participant = dpFactory->create_participant(DDS::DOMAIN_ID_DEFAULT, dpQos, NULL, 0);
        if (participant.in() == NULL)
        {
            throw DDS::DCPSError("Could not create DCPS DomainParticipant.");
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
        participant->get_default_topic_qos(tQos);
        tQos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
        tQos.durability.kind = DDS::TRANSIENT_DURABILITY_QOS;
        participant->set_default_topic_qos(tQos);

        /* Use the CacheFactory to create a Cache. Each Cache is uniquely
         * identified by it's name and can always be found in the
         * CacheFactory by calling the 'find_cache_by_name' operation on the
         * CacheFactory. The usage of the cache will determine if
         * application is able to read (READ_ONLY), write (WRITE_ONLY) or
         * both (READ_WRITE). Caches that are readable have a DDS::Subscriber
         * entity created for that Cache, Caches that are writeable have a
         * DDS::Publisher entity created for that Cache. These entities are
         * directly accessible.
         */
        cache = cFactory->create_cache(
            cacheName,
            cacheUsage,
            participant.in());

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
        subscriber = cache->the_subscriber();
        if(subscriber.in() != NULL)
        {
            subscriber->get_qos(subQos);
            subQos.partition.name.length(1);
            subQos.partition.name[0] = "ChatRoom";
            subscriber->set_qos(subQos);
        }
        publisher = cache->the_publisher();
        if(publisher.in() != NULL)
        {
            publisher->get_default_datawriter_qos(dwQos);
            dwQos.writer_data_lifecycle.autodispose_unregistered_instances = false;
            publisher->set_default_datawriter_qos(dwQos);

            publisher->get_qos(pubQos);
            pubQos.partition.name.length(1);
            pubQos.partition.name[0] = "ChatRoom";
            publisher->set_qos(pubQos);
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
        for(unsigned int i = 0; i < homes.length(); i++)
        {
            DDS::ObjectHome_var ahome;

            ahome = homes[i];
            cache->register_home(ahome.in());
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
        cache->register_all_for_pubsub();

        /* After all DCPS entities have been created and the QoS settings
         * have been adapted, we can enable the cache for pub/sub. After
         * this call the cache will be fully participating in the
         * system.
         */
        cache->enable_all_for_pubsub();

    }
    catch (DDS::DCPSError e)
    {
        /* Clean up our resources and then propagate the exception */
        deleteCache(cache);
        deleteParticipant(participant.in());
        throw e;
    }
    catch (DDS::PreconditionNotMet e)
    {
        /* Clean up our resources and then propagate the exception */
        deleteCache(cache);
        deleteParticipant(participant.in());
        throw e;
    }
    catch (DDS::AlreadyDeleted e)
    {
        /* Clean up our resources and then propagate the exception */
        deleteCache(cache);
        deleteParticipant(participant.in());
        throw e;
    }
    return cache;
}

void
DlrlUtility::deleteCacheAndParticipant(
    DDS::Cache_ptr cache)
{
    DDS::DomainParticipant_var participant;
    if(cache != NULL)
    {
        participant = cache->the_participant();
        cFactory->delete_cache(cache);
        deleteParticipant(participant.in());
    }
}

void
DlrlUtility::deleteCache(
    DDS::Cache_ptr cache)
{
    if(cache != NULL)
    {
        cFactory->delete_cache(cache);
    }
}

void
DlrlUtility::deleteParticipant(
    DDS::DomainParticipant_ptr participant)
{
    int result;

    if(participant != NULL)
    {
        result = participant->delete_contained_entities();
        if (result == DDS::RETCODE_OK)
        {
            result = dpFactory->delete_participant(participant);
        }
        if (result != DDS::RETCODE_OK)
        {
            throw DDS::DCPSError("Unable to delete the DomainParticipant correctly.");
        }
    }
}
