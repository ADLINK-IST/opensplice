#ifndef WLMB_MYWHITELISTEDMESSAGEBOARD
#define WLMB_MYWHITELISTEDMESSAGEBOARD

#include "dlrl_tutorial_if.h"
#include "ccpp_dds_dlrl.h"
#include <string>
#include "ccpp_WhiteListObjects.h"

namespace WLMB //WhiteListedMessageBoard
{
    /**
     * The WhiteListedMessageBoard application only displays chat messages from
     * users that are listed in the whitelist indicated at startup. Any other
     * messages are blocked. Using relations managed by the DLRL this task becomes
     * a simple and easy task to accomplish, as is demonstrated within this
     * application.
     *
     * This application is an event driven application, where the DLRL will
     * automatically manage system updates to Objects and trigger application
     * listeners to indicate the changes that have taken place. The DLRL offers
     * a fine grained control when it comes to implementing event driven
     * applications. Three levels of listeners are available:
     * - CacheListener: triggered at the start of an update round (before any
     * modifications are made) and triggered at the end of an update round to signal
     * the end of the update round. The CacheListener is also triggered whenever
     * the Cache changes it's update mode (from pull to push mode and vice versa)
     * The CacheListener is not demonstrated in this application.
     * - ObjectListener: triggered after the 'start of an update round' trigger of
     * the CacheListener and after updates have been applied to the objects in the
     * Cache. Each created, modified or deleted object will result in a seperate
     * trigger to allow the application to take the appropriate action.
     * - SelectionListener: It is possible to define dynamic subsets of DLRL objects
     * based upon a specific filter or query. These selection may be automatically
     * managed by DLRL in which case the SelectionListener may be triggered each
     * time something within the selection changes. If an object enters or exits the
     * selection a trigger will be given, but also if an object, which was already
     * contained within the selection, is modified.
     * SelectionListeners are triggered after all ObjectListener triggers have
     * occured, but before the 'end of the update round' trigger of the
     * CacheListener.
     *
     * Another aspect briefly mentioned is the DLRLs ability to provide subsets of
     * Objects, where the subset is filtered according to a Query or an application
     * defined filter. Such Selections are demonstrated within this application and
     * specifically using an application defined filter. This type of filtering
     * allows application to use a very application specific algorithm for filtering
     * data. To keep in line with the event based nature of this application the
     * selection will use a listener to notify the application whenever something
     * changes within the selection.
     *
     * The object model used by this application defines a custom operation on the
     * ChatMessage object which can then be implemented with application specific
     * code. As can be observed an implementation class custom to this application
     * exists and will be used for the ChatMessage, see the ChatMessageCustomImpl
     * class for more details. This feature demonstrates the ease in integrating
     * custom operations within the DLRL objects.
     */

    /**
     * The main operation of this application, invoked when this class is
     * executed. It contains the main flow of the application.
     *
     * @param args The arguments provided by the user on the command line
     */
        int
        main(
            int argc,
            char *argv[]);

	class WhiteListedMessageBoard
    {
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
        private:
            DDS::Cache_var cache;

        /* Is set to true if a termination event was received */
        private:
            bool terminate;

        /* Holds the name of the whitelist provided by the user at startup */
        private:
            const char* name;

        /* If set to a valid object by the SelectionListener if an WhiteList is
         * found that meets the criteria (i.e., has the same name as the name
         * provided at start up. It is set to null if the WhiteList no longer
         * meets the criteria (e.g., it is deleted)
         */
        private:
            DLRLChat::WhiteList_var whiteList;


        public:
            WhiteListedMessageBoard();
            ~WhiteListedMessageBoard();

        /**
         * This operation initializes the required DLRL entities for this
         * application and ensures everything is setup so the application can
         * participate within the system after this operation.
         */
        public:
            void
            initializeMessageBoard();

        /**
         * This operation deletes the main Cache and the related DomainParticipant.
         *
         * @throws Exception when something goes wrong during deletion of the Cache
         * or the DomainParticipant.
         */
        public:
            void
            deleteMessageBoard();

        /**
         * This operation will simply enable the Cache for updates and then perform
         * a sleep and wake up every 100 ms to check if the application should
         * terminate or not.
         */
        public:
            void
            runMessageBoard();

        /**
         * This operation is invoked by the 'MyWhiteListSelectionListener' class to
         * set the appropriate WhiteList object once it is detected and filter out
         * by the selection.
         */
        public:
            void
            setWhiteList(
                DLRLChat::WhiteList* whiteList);

        /**
         * This operation is invoked by the 'MyChatMessageListener' class whenever
         * a ChatMessage change is detected and the message needs to be evaluated
         * if the message may be displayed or not.
         */
        public:
            bool
            isWhiteListed(
                DLRLChat::ChatMessage* msg);

        /**
         * This operation is invoked by the 'MyChatMessageListener' class when
         * a termination event is detected. It will simply set the terminate flag
         * to true, which will trigger the application to delete the Cache. It is
         * important to realize that the Cache will always completely finish it's
         * current update round before the deletion of the Cache is executed.
         */
        public:
            void
            setTerminate();

    /******************************************************************************
     ******************************************************************************
     * The following functions are merely utility functions for parsing arguments *
     * and showing the help instructions for this application. No DDS related     *
     * functionality is contained within these following operations               *
     ******************************************************************************
     ******************************************************************************/

        /**
         * This operation parses the input parameters to search for the name of the
         * WhiteList.
         *
         * @param args the command line arguments.
         * @return the name of the WhiteList.
         */
        public:
            void
            parseWhiteListName(
                int argc,
                char *argv[]);

        /**
         * This operation prints a help message on the screen explaining how to use
         * the command line parameters.
         */
        public:
            static void
            printHelp();
    };
};

#endif
