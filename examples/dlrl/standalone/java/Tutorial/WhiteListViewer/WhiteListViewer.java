package WhiteListViewer;

import java.io.*;

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
public class WhiteListViewer
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
    private DDS.Cache cache = null;

    /**
     * The main operation of this application, invoked when this class is
     * executed. It contains the main flow of the application.
     *
     * @param args The arguments provided by the user on the command line
     */
    public static void main(
        String[] args) throws Exception
    {
        WhiteListViewer viewer = null;

        /* Instantiate the WhiteListEditor class */
        viewer = new WhiteListViewer();
        /* A common use case for DLRL is to map the DLRL objects onto already
         * existing DCPS topics. In most of these situations the DCPS topics
         * that already exist will have very specific system wide QoS settings
         * in place.
         * When the DLRL application enters the playing field and creates it's
         * topics it will always first do a lookup of the topic and re-use the
         * already set topic qos. If no topic can be found, then it will create
         * the topic itself.
         * The following call will simulate this behavior by creating the topics
         * and setting their Qos settings before any DLRL related action are
         * performed within this application. We will see that the Qos settings
         * are automatically copied over by the DLRL.
         */
        Common.PreconfigureTopics.simulateExternalTopicCreation();
        try
        {
            /* First initialize the viewer, this operation will ensure
             * the Cache is created and made ready for pub/sub.
             */
            viewer.initializeViewer();
            /* Once the application is set up for participation is DDS, we can
             * execute the viewer and show which WhiteLists exist and which
             * users are white listed in each list.
             */
            viewer.executeViewer();
        }
        catch (Exception e)
        {
            System.out.println(e.getMessage());
        }
        /* We must always clean up our resources, in DLRL terms it means
         * we must always delete the Cache object at the end of our
         * application. When we delete the Cache object it will ensure any
         * contained entities are deleted as well, such as ObjectHomes,
         * CacheAccesses, created objects within those CacheAccesses and
         * ofcourse any DCPS entity.
         * It is also important to realize that the DLRL will not clean up
         * the DCPS DomainParticipant object that was provided to it at
         * creation, as the Cache does not manage this object, it only
         * uses it. So it is not responsible for it's deletion. But any
         * DCPS entity the DLRL created with that DomainParticipant will be
         * cleaned.
         */
        if(viewer != null)
        {
            viewer.deleteViewer();
        }
    }

    /**
     * This operation initializes the required DLRL entities for this
     * application and ensures everything is setup so the application can
     * participate within the system after this operation.
     */
    private void initializeViewer(
        ) throws Exception
    {
        DDS.ObjectHome[] homes;

        /* First fill the array with the homes used within this application. */
        homes = new DDS.ObjectHome[2];
        homes[0] = new DLRLChat.WhiteListHome();
        homes[1] = new DLRLChat.UserHome();

        /* Call the initializeDlrl function in the utlity library. The steps
         * to initialize the DLRL are relatively the same each time for the
         * various applications within our example.
         * We will simply provide a unique name for our cache, indicate how we
         * will use it (READ_ONLY) and provide the homes array we filled above
         */
        cache = Common.DlrlUtility.initializeDlrl(
            "WhiteListViewerCache",
            DDS.CacheUsage.READ_ONLY,
            homes);
    }

    /**
     * This operation ensures all DDS entities used by the WhiteListViewer
     * are correctly deleted.
     */
    public void deleteViewer(
        ) throws Exception
    {
        Common.DlrlUtility.deleteCacheAndParticipant(cache);
    }

    /**
     * This operation performs a refresh of the Cache and then displays all the
     * WhiteList objects and their contained users
     */
    private void executeViewer(
        ) throws Exception
    {
        DLRLChat.WhiteListHome whiteListHome;
        DLRLChat.WhiteList[] whitelists;
        DLRLChat.WhiteList aWhitelist;
        DLRLChat.UserSet userSet;
        DLRLChat.User[] users;
        DLRLChat.User aUser;

        /* Perform a refresh of the Cache, this will result in explicitly
         * reading the current system state and creating, modifying or deleting
         * objects based upon that system state. In our simple viewer example
         * we have implemented a simple 'run once' application, so we will
         * refresh once and just look at the available objects in the cache by
         * using the 'get_created_objects' operation, in this case it will yield
         * the same results as calling the 'get_objects'.
         */
        cache.refresh();
        /* If at any time an ObjectHome is needed, it can be located by
         * performing a 'find' on the Cache. A home can be located in two
         * ways, by the index assigned during registration, or by the
         * fully qualified IDL type name of the object. We will use the later
         * way of locating the needed object home.
         */
        whiteListHome = (DLRLChat.WhiteListHome)cache.find_home_by_name("DLRLChat::WhiteList");
        /* Retrieve all newly created WhiteList objects within the WhiteList
         * home in the last refresh run.
         * Then iterate over the whitelists and print out the requested data.
         */
        whitelists = whiteListHome.get_created_objects(cache);
        if(whitelists.length > 0)
        {
            for(int i = 0; i < whitelists.length; i++)
            {
                /* Get the next whitelist */
                aWhitelist = whitelists[i];
                /* Each whitelist has a set of users, which is a collection type
                 * known within the DLRL. The set has various useful operations
                 * for examining it's state.
                 */
                userSet = aWhitelist.get_friends();
                /* Get all added users known within the set. During each refresh
                 * run a lot of useful information is maintained for collections
                 * such as which values were added and which were removed.
                 * In this application we are only interested in the added
                 * values as this is a 'run once' application. The 'added_values'
                 * operation will yield the same results as the 'values'
                 * operation in this specific example.
                 */
                users = userSet.added_elements();

                /* Finally just print out the status of the WhiteList and the
                 * users contained within the WhiteList
                 */
                if(users.length > 0)
                {
                    System.out.println("Whitelist '"+aWhitelist.get_name()+"' contains the following users:");
                    for(int j = 0; j < users.length; j++)
                    {
                        aUser = users[j];
                        System.out.println("* "+aUser.get_id()+","+aUser.get_name());
                    }
                } else
                {
                    System.out.println("Whitelist '"+aWhitelist.get_name()+"' contains no users.");
                }
            }
        } else
        {
            System.out.println("No white lists were found");
        }
    }
}
