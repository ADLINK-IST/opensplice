package WhiteListedMessageBoard;

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
public class WhiteListedMessageBoard
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
    /* Is set to true if a termination event was received */
    private boolean terminate = false;
    /* Holds the name of the whitelist provided by the user at startup */
    private String name;
    /* If set to a valid object by the SelectionListener if an WhiteList is
     * found that meets the criteria (i.e., has the same name as the name
     * provided at start up. It is set to null if the WhiteList no longer
     * meets the criteria (e.g., it is deleted)
     */
    private DLRLChat.WhiteList whiteList = null;

    /**
     * The main operation of this application, invoked when this class is
     * executed. It contains the main flow of the application.
     *
     * @param args The arguments provided by the user on the command line
     */
    public static void main(
        String[] args) throws Exception
    {
        WhiteListedMessageBoard msgBoard = null;

        /* Create the message board for the white list name provided */
        msgBoard = new WhiteListedMessageBoard();
        /* Parse input parameter */
        msgBoard.parseWhiteListName(args);
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
            /* First initialize the WhiteListedMessageBoard, this operation will
             * ensure the Cache is created and made ready for pub/sub.
             */
            msgBoard.initializeMessageBoard();
            /* Once the application is set up for participation is DDS, we can
             * run the messageboard.
             */
            msgBoard.runMessageBoard();
        }
        catch (Exception e)
        {
            System.out.println(e.getMessage());
            printHelp();
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
        if(msgBoard != null)
        {
            msgBoard.deleteMessageBoard();
        }
    }

    /**
     * This operation initializes the required DLRL entities for this
     * application and ensures everything is setup so the application can
     * participate within the system after this operation.
     */
    private void initializeMessageBoard(
        ) throws Exception
    {
        DDS.ObjectHome[] homes;
        DLRLChat.WhiteListSelection wlSelection;
        DLRLChat.WhiteListHome whiteListHome;
        DLRLChat.UserHome userHome;
        DLRLChat.ChatMessageHome chatMessageHome;

        /* First fill the array with the homes used within this application. */
        whiteListHome = new DLRLChat.WhiteListHome();
        userHome = new DLRLChat.UserHome();
        chatMessageHome = new DLRLChat.ChatMessageHome();
        homes = new DDS.ObjectHome[3];
        homes[0] = whiteListHome;
        homes[1] = userHome;
        homes[2] = chatMessageHome;

        /* Call the initializeDlrl function in the utlity library. The steps
         * to initialize the DLRL are relatively the same each time for the
         * various applications within our example.
         * We will simply provide a unique name for our cache, indicate how we
         * will use it (WRITE_ONLY) and provide the homes array we filled above
         */
        cache = Common.DlrlUtility.initializeDlrl(
            "WhiteListedMessageBoardCache",
            DDS.CacheUsage.READ_ONLY,
            homes);

        /* Create a selection with a application specific filter. The selection
         * is set to be automatically managed by the DLRL, which means that
         * each time the Cache processes updates the selection will
         * automatically be updated to take the changes to objects into account.
         * A listener is set for the selection, this listener will be invoked
         * each time something within the selection changed, allowing for
         * a convienant event driven approach to using the selection.
         * Alternately it is possible to use a selection and refresh it by
         * invoking the refresh operation on the selection. In such scenarios
         * the selection will update the entire selection based upon all
         * objects known within the cache (oppossed to only the changed objects
         * each update round). The selection listener is not available in such
         * cases.
         * A benefit of the 'manual' refresh of the selection is that this
         * refresh is completely decoupled from the refreshes within the
         * Cache, and a refresh on a selection will not change anything within
         * the Cache. This allows for intermittent refreshing of a selection,
         * which may be convienant in some usecases.
         * In our example we will use the convienant event driven approach.
         */
        wlSelection = whiteListHome.create_selection(
            new MyWhiteListFilter(name),
            true,
            false);
        wlSelection.set_listener(new MyWhiteListSelectionListener(this));

        /* Attach an ObjectListener for the ChatMessage type. This listener
         * will be triggered each time a ChatMessage object is created, modified
         * or deleted.
         */
        chatMessageHome.attach_listener(new MyChatMessageListener(this), false);
    }

    /**
     * This operation deletes the main Cache and the related DomainParticipant.
     *
     * @throws Exception when something goes wrong during deletion of the Cache
     * or the DomainParticipant.
     */
    private void deleteMessageBoard(
        ) throws Exception
    {
        Common.DlrlUtility.deleteCacheAndParticipant(cache);
    }

    /**
     * This operation will simply enable the Cache for updates and then perform
     * a sleep and wake up every 100 ms to check if the application should
     * terminate or not.
     */
    public void runMessageBoard(
        ) throws Exception
    {
        System.out.println("WhiteListedMessageBoard has opened: send a ChatMessage with userID = -1 to close it....\n");
        /* The invokation of the enable_updates operation will ensure that from
         * that moment forward the Cache will automatically manage any
         * incoming updates to objects, and it will also apply any already
         * present, but not yet applied, updates to objects.
         * A Cache in this state will invoke listeners to indicate which events
         * are taking place within the Cache and the application can then
         * execute the appropiate actions based upon the information presented.
         * If the application uses CacheListeners (not used in our example),
         * then each CacheListener will be triggered to indicate the change in
         * update mode within the cache.
         */
        cache.enable_updates();

        /* Now just sleep until the termination flag is set */
        while(!terminate)
        {
            /* Sleep for 100 ms. as not to consume too much CPU power. */
            Thread.sleep(100);
        }
    }

    /**
     * This operation is invoked by the 'MyWhiteListSelectionListener' class to
     * set the appropriate WhiteList object once it is detected and filter out
     * by the selection.
     */
    public void setWhiteList(
        DLRLChat.WhiteList whiteList)
    {
        this.whiteList = whiteList;
    }

    /**
     * This operation is invoked by the 'MyChatMessageListener' class whenever
     * a ChatMessage change is detected and the message needs to be evaluated
     * if the message may be displayed or not.
     */
    public boolean isWhiteListed(
        DLRLChat.ChatMessage msg) throws DDS.NotFound
    {
        DLRLChat.User user;
        boolean isWhiteListed = false;

        /* If the WhiteList with the specified name was not yet found (i.e.,
         * not created, name does not match, etc), then always return false
         * (i.e., block everything)
         */
        if(whiteList != null)
        {
            /* A message may be displayed if the user that sent the message is
             * contained within the WhiteList. We will simply navigate from
             * the ChatMessage object to the User object as the DLRL will always
             * ensure this relation is up to date and ready to be navigated.
             */
            user = msg.get_sender();
            /* Now that we have our User object, we can use the Set collection
             * of the WhiteList to check if the User is actually contained
             * within the 'friends' listing of the WhiteList. The DLRL also
             * automatically keeps this listing up to date and ensures the
             * contains the the 'friends' set matches what is known within the
             * system. It could not be simpler.
             */
            if (whiteList.get_friends().contains(user))
            {
                isWhiteListed = true;
            }
        }
        return isWhiteListed;

    }

    /**
     * This operation is invoked by the 'MyChatMessageListener' class when
     * a termination event is detected. It will simply set the terminate flag
     * to true, which will trigger the application to delete the Cache. It is
     * important to realize that the Cache will always completely finish it's
     * current update round before the deletion of the Cache is executed.
     */
    public void setTerminate(
        )
    {
        terminate = true;
    }

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
    private void parseWhiteListName(
        String[] args) throws Exception
    {
        /* We only accept the '-name' flag with the accompying name, so if there
         * are not exactly 2 arguements, something is wrong!
         */
        if(args.length != 2)
        {
            throw new Exception("Expected 2 arguments, but detected '"+args.length+"'");
        }
        /* Verify the first argument is -name */
        if(!args[0].equals("-name"))
        {
            throw new Exception("Expected the first argument to equals '-name', but found "+args[0]+".");
        }
        /* Verify the second argument is not another flag, i.e., does not start
         * with a '-'
         */
        if(args[1].startsWith("-"))
        {
            throw new Exception("Detected an invalid name. A name may not start with '-'.");
        }
        name = args[1];
    }

    /**
     * This operation prints a help message on the screen explaining how to use
     * the command line parameters.
     */
    private static void printHelp(
        )
    {
        System.out.println("Usage:");
        System.out.println("\tWhiteListedMessageBoard -name <whiteListName>");
        System.out.println("\n");
        System.out.println("Example:");
        System.out.println("\tWhiteListedMessageBoard -name family\n");
        System.out.println("\tDisplays only messages coming from senders mentioned in the WhiteList named 'family'.");
        System.out.println();
    }
}