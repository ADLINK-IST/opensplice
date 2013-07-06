#include "ccpp_dds_dlrl.h"
#include "dlrl_tutorial_if.h"

/**
 * The WhiteListViewer application is a simple application which displays all
 * whitelists known within the system and shows which users are contained within
 * each whitelist.
 * This application demonstrates the DLRL in it's so called 'manual' update
 * mode, where the application triggers the refresh action of the Cache. So that
 * we have a pull mechanism instead of a push mechanism as we do in the
 * event driven WhiteListedMessageBoard application.
 * In the 'manual' update mode we do not have the availability of listeners, and
 * therefor have to fetch the objects located within the cache by making
 * explicit calls. Fine grained control is available which allows an application
 * to only fetch newly created, modified or deleted objects and take the
 * appropiate action accordingly. Furthermore as demonstrated in this
 * application, it is also possible to fetch all objects within the cache.
 * It is noteworthy to mention that the same functionality is also available
 * when using the event driven approach with listeners.
 *
 * Implementing applications like this simple viewer application is a good
 * choice when dealing with a use case where the application is not interested
 * in the latest information as soon as it arrives, but wants to only update
 * when at specific moments in time, for example only once per time interval or
 * upon the request of a user.
 */
namespace WLV //WLV = WhiteListViewer
{

    /**
     * The main operation of this application, invoked when this class is
     * executed. It contains the main flow of the application.
     *
     * @param argc The number of arguments provided by the user on the command line
     * @param argv The arguments provided by the user on the command line
     */
    int
    main(
        int argc,
        char *argv[]);

    class WhiteListViewer
    {
        private:
            /* The Cache object used by this application. The Cache is the starting
             * point of any application wanting to use the DLRL. It manages the various
             * DLRL and DCPS entities needed by the application and is the gateway to
             * accessing these entities. The Cache can also be used to configure how
             * certain aspects of the DLRL behave.
             * It is perfectly legal to have multiple Cache objects within an
             * application. Each Cache is 100% seperated from other Cache objects, an
             * application may chose to have two Cache objects with the same object
             * model registered to it, or different object models. Anything goes as far
             * as using multiple Cache objects is concerned. But in our example we will
             * always only use one Cache.
             */
            DDS::Cache_ptr cache;

        public:
            /**
             * This operation initializes the required DLRL entities for this
             * application and ensures everything is setup so the application can
             * participate within the system after this operation.
             */
            void
            initializeViewer(
                );
        public:
            /**
             * This operation ensures all DDS entities used by the WhiteListViewer
             * are correctly deleted.
             */
            void
            deleteViewer(
                );

        public:
            /**
             * This operation performs a refresh of the Cache and then displays all the
             * WhiteList objects and their contained users
             */
            void
            executeViewer(
            );
    };
};
