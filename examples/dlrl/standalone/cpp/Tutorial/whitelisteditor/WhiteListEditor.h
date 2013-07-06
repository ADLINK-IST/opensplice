#include "ccpp_dds_dlrl.h"
#include <vector>
#include <string>
#include "ccpp_WhiteListObjects_editor.h"
#include "dlrl_tutorial_if.h"

/**
 * The WhiteListEditor application is used to create whitelists and add users
 * to a whitelist. These created whitelists and the users contained within them
 * are used by the WhiteListedMessageBoard to determine if a message sent by a
 * user will be displayed or blocked.
 *
 * The WhiteListEditor application uses a different slightly customized object
 * model compared to the  WhiteListedMessageBoard application. The difference
 * for the WhiteListEditor application is that it's object model does not
 * contain the ChatMessage object, and the user has no relation to
 * this object. The benefit of customizing the object model in this regard is
 * that we now do not have to think about the relation from the user
 * to the chatmessage object when we want to add a user to a whitelist. So we
 * have to do less work, we lower our memory footprint (less objects and meta
 * data managed by the DLRL) and if we were a reading application we increase
 * performance because the DLRL does not need to read the chatmessage object or
 * maintain the relation to the chatmessage. So all in all, customizing the
 * object model to suit the needs of an application holds many benefits.
 */

 namespace WLE //WhiteListEditior
{
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

    /**
     * A small private class to contain the parsed user information read from
     * the command line at startup
     */
    struct UserInfo {
        int userID;
        std::string name;
    };

    typedef std::vector<UserInfo*> UserInfoList;

    class WhiteListEditor
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

        /* The CacheAccess object used by this application to write objects into
         * the system. Whenever an application wants to write objects into the
         * system, this must occur within the scope of a CacheAccess. It is
         * allowed to create as many CacheAccess objects as required and if
         * wanted the same objects (i.e., objects that have the same key fields, but
         * which are different instances) may be edited within each CacheAccess.
         * In our example however, we only require one CacheAccess which we to
         * write and commit our objects into the system.
         */
        private:
            DDS::CacheAccess_var access;

        /* Each object type specified in the IDL file containing the object model
         * will have a typed home object to represent that type. An instance of the
         * object home class may only be associated to one Cache, but multiple
         * instances of an object home may be created, each instance may then be
         * associated with a different Cache object.
         * In this application we have two types, the WhiteList class and the User
         * class, each of these types has a different typed ObjectHome.
         * The ObjectHome is used to manage the specific type, it provides access
         * to the underlying datawriters, datareaders and topics for a specific
         * object type as well as access to operations to attach or detach listeners
         * retrieve all objects known within the cache for a specific type, or
         * only the created/modified/deleted objects during a refresh run.
         * The ObjectHome can be viewed as the gateway for performing all sorts of
         * operations and actions for a specific object type.
         */
        private:
            DLRLChat::WhiteListHome_var whiteListHome;
        private:
            DLRLChat::UserHome_var userHome;

        /* Holds the name of the whitelist provided by the user at startup */
        private:
            const char* name;

        /* A list of all users that are to be added to the whitelist, these users
         * where provided at startup by the user.
         */
        private:
            UserInfoList* userList;

        /* A boolean indicating if the user requested to see the help menu */
        public:
            bool showHelp;

        /**
         * This operation initializes the required DLRL entities for this
         * application and ensures everything is setup so the application can
         * participate within the system after this operation.
         */
        public:
            void
            initializeEditor(
                );

        /**
         * This operation ensures all DDS entities used by the WhiteListEditor
         * are correctly deleted.
         */
        public:
            void
            deleteEditor(
                );

        /**
         * This operation creates a writeable CacheAccess with which it creates
         * and writes the WhiteList into the system.
         *
         * @throws Exception when something goes wrong during creation/modification/
         * writing of the objects in the WhiteList.
         */
        public:
            void
            addUsersToWhiteList(
                );
        public:
            WhiteListEditor();
            ~WhiteListEditor();

    /******************************************************************************
     ******************************************************************************
     * The following functions are merely utility functions for parsing arguments *
     * and showing the help instructions for this application. No DDS related     *
     * functionality is contained within these following operations               *
     ******************************************************************************
     ******************************************************************************/

        /* Parses all arguments, allows random order, allows multiple uses of the
         * '-add' command, requires at least one user to be added though. Also
         * requires a whitelist name (only 1)
         */
        public:
            void
            parseArguments(
                int argc,
                char *argv[]);

        /**
         * This operation prints a help message on the screen explaining how to use
         * the editor.
         */
        public:
            void
            printHelp(
                );
    };
};
