package WhiteListEditor;

import java.io.*;
import java.util.Vector;

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
public class WhiteListEditor
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

    /* The CacheAccess object used by this application to write objects into
     * the system. Whenever an application wants to write objects into the
     * system, this must occur within the scope of a CacheAccess. It is
     * allowed to create as many CacheAccess objects as required and if
     * wanted the same objects (i.e., objects that have the same key fields, but
     * which are different instances) may be edited within each CacheAccess.
     * In our example however, we only require one CacheAccess which we to
     * write and commit our objects into the system.
     */
    private DDS.CacheAccess access = null;

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
    private DLRLChat.WhiteListHome whiteListHome;
    private DLRLChat.UserHome userHome;
    /* Holds the name of the whitelist provided by the user at startup */
    private String name = null;
    /* A list of all users that are to be added to the whitelist, these users
     * where provided at startup by the user.
     */
    private Vector<UserInfo> userList = new Vector<UserInfo>();
    /* A boolean indicating if the user requested to see the help menu */
    public boolean showHelp;

    /**
     * A small private class to contain the parsed user information read from
     * the command line at startup
     */
    private class UserInfo
    {
        public int userID;
        public String name;
    }

    /**
     * The main operation of this application, invoked when this class is
     * executed. It contains the main flow of the application.
     *
     * @param args The arguments provided by the user on the command line
     */
    public static void main(
        String[] args) throws Exception
    {
        WhiteListEditor editor;

        /* Instantiate the WhiteListEditor class */
        editor = new WhiteListEditor();
        /* Instruct the editor to parse the arguments provided at command line.
         * this operation will ensure that, if successfull, the resulting values
         * will be stored within the editor so they can be used later on.
         */
        editor.parseArguments(args);
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
        /* If the showHelp boolean is not true, then no request for the help
         * menu was detected and the application will continue, otherwise
         * the help menu is printed to the screen.
         */
        if(!editor.showHelp)
        {
            try
            {
                /* First initialize the editor, this operation will ensure
                 * the Cache is created and made ready for pub/sub, and ensure
                 * that a CacheAccess is also available to write data with.
                 */
                editor.initializeEditor();
                /* This operation will create the requested whitelist and
                 * create, then add, the requested users to the whitelist and
                 * finally write these changes into the system.
                 */
                editor.addUsersToWhiteList();
            }
            catch (Exception e)
            {
                System.out.println("Exception: "+e.getMessage());
            }
            /* We must always clean up our resources, in DLRL terms it means
             * we must always delete the Cache object at the end of our
             * application. When we delete the Cache object it will ensure any
             * contained entities are deleted as well, such as ObjectHomes,
             * CacheAccesses, created objects within those CacheAccesses and
             * ofcourse any DCPS entities.
             * It is also important to realize that the DLRL will not clean up
             * the DCPS DomainParticipant object that was provided to it at
             * creation, as the Cache does not manage this object, it only
             * uses it. So it is not responsible for it's deletion. But any
             * DCPS entity the DLRL created with that DomainParticipant will be
             * cleaned.
             */
            editor.deleteEditor();
        } else
        {
            /* prints a help menu describing the commands that may be used for
             * this application.
             */
            editor.printHelp();
        }
    }

    /**
     * This operation initializes the required DLRL entities for this
     * application and ensures everything is setup so the application can
     * participate within the system after this operation.
     */
    private void initializeEditor(
        ) throws Exception
    {
        DDS.ObjectHome[] homes;

        /* First fill the array with the homes used within this application,
         * also store the homes within the instance variables within the
         * WhiteListEditor class, so we can use these homes later on without
         * having to search for them within the Cache.
         */
        whiteListHome = new DLRLChat.WhiteListHome();
        userHome = new DLRLChat.UserHome();
        homes = new DDS.ObjectHome[2];
        homes[0] = whiteListHome;
        homes[1] = userHome;

        /* Call the initializeDlrl function in the utlity library. The steps
         * to initialize the DLRL are relatively the same each time for the
         * various applications within our example.
         * We will simply provide a unique name for our cache, indicate how we
         * will use it (WRITE_ONLY) and provide the homes array we filled above
         */
        cache = Common.DlrlUtility.initializeDlrl(
            "WhiteListEditorCache",
            DDS.CacheUsage.WRITE_ONLY,
            homes);

        /* Create a writeable CacheAccess to hold our writeable DLRL objects. */
        access = cache.create_access(DDS.CacheUsage.WRITE_ONLY);
    }

    /**
     * This operation ensures all DDS entities used by the WhiteListEditor
     * are correctly deleted.
     */
    public void deleteEditor(
        ) throws Exception
    {
        Common.DlrlUtility.deleteCacheAndParticipant(cache);
    }

    /**
     * This operation creates a writeable CacheAccess with which it creates
     * and writes the WhiteList into the system.
     *
     * @throws Exception when something goes wrong during creation/modification/
     * writing of the objects in the WhiteList.
     */
    private void addUsersToWhiteList(
        ) throws Exception
    {
        DLRLChat.WhiteList whiteList;
        UserInfo userInfo;

        /* The first step is to create a WhiteList object with the name
         * specified at command line. The WhiteList object used within this
         * application uses custom topic keys instead of the default DLRL
         * topic key values. This implies that the DLRL can not determine the
         * identity of the object, as it is application logic to determine
         * the values of the custom key values.
         * Due to this choice the creation of the WhiteList object happens in
         * three small and simple steps, instead of one step if the default
         * key approach of DLRL was used.
         * First an unregistered object is created by calling the
         * 'create_unregistered_object' operation on the WhiteListHome.
         * This operation takes a CacheAccess object as parameter because
         * each object must be created in a (writeable) CacheAccess.
         * The ObjectHome acts like a factory for the creation of an object
         * and ensures all neccesary DDS related actions are taken whenever
         * an object is created. Behind the scenes the ObjectHome factory will
         * ensure the WhiteList object is actually instantiated as the custom
         * implementation class 'WhiteListCustomImpl' as indicated in our
         * mapping file by the following line:
         *
         * <classMapping name="DLRLChat::WhiteList" implClass="WhiteListEditor::WhiteListCustomImpl">
         *
         * The result of the operation is an unregistered object. This is an
         * object that has no identity and will not yet participate in any DDS
         * related activities. It can not be used as a relation end point,
         * it will not show up when getting the objects contained within the
         * CacheAccess, it will be ignored when writing changes into the
         * system, etc.
         * An unregistered object can be destroyed by calling the 'destroy'
         * operation on the object, in which case it is immediately
         * destroyed and invalidated. Alternately when the CacheAccess is
         * destroyed, the unregistered objects will also be cleaned.
         */
        whiteList = whiteListHome.create_unregistered_object(access);

        /* As explained our unregistered object has no identity, as the DLRL
         * can not determine it's identity as custom key fields are used.
         * So it is up to the application to determine the identity of the
         * object, in the case of the WhiteList the only key field is the name
         * so if we set the name, we have given the WhiteList object an
         * identity and we can finalize the object by calling the
         * 'register_object' operation.
         * Only the key fields should be edited between the
         * 'create_unregistered_object' and 'register_object' calls.
         */
        whiteList.set_name(name);

        /* Now that the WhiteList object has an identity we can finalize the
         * object by calling the 'register_object' operation. If this operation
         * is successful (it can fail if an object with the specified identity
         * already exists within the scope of the CacheAccess), then the object
         * from that point forward will participate as normal within the actions
         * of the CacheAccess. Meaning that if the 'write' operation is called
         * that the object will be written into the system and that the object
         * will appear in the objects listing of the CacheAccess, etc.
         */
        whiteListHome.register_object(whiteList);
        /* Now that we successfully created our WhiteList object we can iterate
         * over the list of users that need to be added to the WhiteList
         */
        for (int i = 0; i < userList.size(); i++)
        {
            DLRLChat.User a_user;

            userInfo = (UserInfo)userList.get(i);
            System.out.println("For whitelist '"+name+"' adding user '"+
                userInfo.name+"' with id '"+userInfo.userID+"'.");

            /* In the same manner as the creation of the WhiteList object we
             * will create the user object. It also uses custom key fields
             * and thus needs to perform the same three simple steps.
             */
            a_user = userHome.create_unregistered_object(access);

            /* Set its identity. The only key field of the user is it's id
             * so that is all we need to edit here before registering the object
             */
            a_user.set_id(userInfo.userID);

            /* Register the object with it's identity within the CacheAccess. */
            userHome.register_object(a_user);

            /* After the object has been registered, we can set the remaining
             * fields with the appropiate values
             */
            a_user.set_name(userInfo.name);

            /* Finally the user can be added to the WhiteList, both the user and
             * the WhiteList objects are registered objects, so relationships
             * may be formed between these objects.
             * In our example we  invoke a special custom operation defined
             * on the WhiteList object, this demonstrates how this feature
             * of DLRL works and shows that each DLRL object can easily be
             * extended with a few custom operations and can then be used
             * without any special requirements.
             * The implementation of this custom operation can be found in the
             * WhiteListCustomImpl class.
             */
            whiteList.addUser(a_user);
        }

        /* The WhiteList object has been created and the users have been related
         * to the WhiteList. All changes within the CacheAccess now need to
         * be committed into the system, as long as the 'write' operation is
         * not called, all changes are deemed to be local and will not be
         * distributed throughout the system.
         * After the write call, all changes will be available system wide.
         */
        access.write();
    }

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
    public void parseArguments(
        String[] args) throws Exception
    {
        UserInfo userInfo;
        /* very simple state machine */
        final int SCAN = 0;
        final int ADD = 1;
        final int NAME = 2;
        int state = SCAN;

        for(int i = 0; i < args.length; i++)
        {
            switch (state)
            {
            case SCAN:
                if(args[i].equalsIgnoreCase("-add"))
                {
                    state = ADD;
                } else if(args[i].equalsIgnoreCase("-name"))
                {
                    state = NAME;
                } else if(args[i].equalsIgnoreCase("help") ||
                          args[i].equalsIgnoreCase("-help") ||
                          args[i].equalsIgnoreCase("-h"))
                {
                    showHelp = true;
                } else
                {
                    throw new Exception("Invalid token, encountered '"+args[i]+"', but expected '-add' or '-name'.");
                }
                break;
            case ADD:
                if(args[i].equalsIgnoreCase("-add"))
                {
                    state = ADD;//i.e. ignore
                } else if(args[i].equalsIgnoreCase("-name"))
                {
                    state = NAME;
                } else
                {
                    String[] elements = args[i].split(",");
                    if(elements.length != 2)
                    {
                        throw new Exception("Invalid token, encountered '"+args[i]+"', but expected a userId,name combination like '1,dad'.");
                    } else
                    {
                        userInfo = new UserInfo();
                        userInfo.userID = Integer.parseInt(elements[0]);
                        userInfo.name = elements[1];
                        userList.add(userInfo);
                    }
                }
                break;
            case NAME:
                if(name == null)
                {
                    name = args[i];
                    state = SCAN;
                } else
                {
                    throw new Exception("Invalid token, encountered a second name '"+args[i]+"', only one name may be specified!");
                }
                break;
            }
        }
        if(!showHelp)
        {
            if(name == null)
            {
                throw new Exception("No whitelist name was specified!");
            }
            if(userList.size() == 0)
            {
                throw new Exception("No users to be whitelisted were specified!");
            }
        }
    }

    /**
     * This operation prints a help message on the screen explaining how to use
     * the editor.
     */
    private void printHelp(
        )
    {
        System.out.println("Available commands:");
        System.out.println("-name <whiteListName> [-add <userID1,userName1> <userID2,userName2> <...>]");
        System.out.println("\tExample:");
        System.out.println("\t-name family -add 1,mom 2,dad 3,John");
        System.out.println("\tAdds mom, dad and John to the WhiteList named family.");
        System.out.println("\tThe '-add' command may be repeated between each user listing if so desired.");
        System.out.println("help");
        System.out.println("\tDisplays this help menu.");
    }
}
