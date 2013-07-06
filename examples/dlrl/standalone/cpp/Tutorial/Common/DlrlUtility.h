#include "ccpp_dds_dlrl.h"
#include "dlrl_tutorial_if.h"
/**
 * Contains various convienant utility operations used by the various
 * applications within this example.
 */
namespace Common
{
    class DLRLTUT_API DlrlUtility
    {

        public:
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
            static DDS::Cache_ptr
            initializeDlrl(
                const char *  cacheName,
                DDS::CacheUsage cacheUsage,
                DDS::ObjectHomeSeq& homes);

        public:
            /**
             * This operation deletes a cache and the attached participant
             *
             * @param cache the cache that is to be deleted, may be null.
             * @throws DDS::DCPSError if the deletion of the participant linked to the
             * cache fails.
             */
            static void
            deleteCacheAndParticipant(
                DDS::Cache_ptr cache);


        public:
            /**
             * This operation deletes a cache, but not the attached participant
             *
             * @param cache the cache that is to be deleted, may be null.
             */
            static void
            deleteCache(
                DDS::Cache_ptr cache);

        public:
            /**
             * This operation deletes a participant
             *
             * @param participant the participant that is to be deleted, may be null.
             * @throws DDS::DCPSError if the deletion of the participant fails.
             */
            static void
            deleteParticipant(
                DDS::DomainParticipant_ptr participant);
    };
};
