#include "WhiteListEditor.h"
#include "DlrlUtility.h"
#include "PreconfigureTopics.h"

#include <iostream>
#include <sstream>
#include <string>

#include "example_main.h"

using namespace WLE;
using namespace std;

int
OSPL_MAIN (
    int argc,
    char *argv[])
{
    WhiteListEditor* editor = 0;

    try
    {
        /* Instantiate the WhiteListEditor class */
        editor = new WhiteListEditor();
        /* Instruct the editor to parse the arguments provided at command line.
         * this operation will ensure that, if successfull, the resulting values
         * will be stored within the editor so they can be used later on.
         */
        editor->parseArguments(argc, argv);
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
        Common::PreconfigureTopics::simulateExternalTopicCreation();
        /* If the showHelp boolean is not true, then no request for the help
         * menu was detected and the application will continue, otherwise
         * the help menu is printed to the screen.
         */
        if(!editor->showHelp)
        {
            /* First initialize the editor, this operation will ensure
             * the Cache is created and made ready for pub/sub, and ensure
             * that a CacheAccess is also available to write data with.
             */
            editor->initializeEditor();
            /* This operation will create the requested whitelist and
             * create, then add, the requested users to the whitelist and
             * finally write these changes into the system.
             */
            editor->addUsersToWhiteList();
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
            editor->deleteEditor();
        } else
        {
            /* prints a help menu describing the commands that may be used for
             * this application.
             */
            editor->printHelp();
        }
    } catch(const DDS::PreconditionNotMet& e){
        cout << "Exception received: message = " << e.message << endl;
    } catch(const DDS::BadParameter& e){
        cout << "Exception received: message = " << e.message << endl;
    } catch(const DDS::InvalidObjects& e){
        cout << "Exception received: message = " << e.message << endl;
    } catch(const DDS::NotFound& e){
        cout << "Exception received: message = " << e.message << endl;
    } catch(const DDS::AlreadyExisting& e){
        cout << "Exception received: message = " << e.message << endl;
    } catch(const DDS::AlreadyDeleted& e){
        cout << "Exception received: message = " << e.message << endl;
    } catch(const DDS::NoSuchElement& e){
        cout << "Exception received: message = " << e.message << endl;
    } catch(const DDS::SQLError& e){
        cout << "Exception received: message = " << e.message << endl;
    } catch(const DDS::TimeOut& e){
        cout << "Exception received: message = " << e.message << endl;
    } catch(const DDS::OutOfMemory& e){
        cout << "Exception received: message = " << e.message << endl;
    } catch(const DDS::DCPSError& e){
        cout << "Exception received: message = " << e.message << endl;
    } catch(const DDS::DLRLError& e){
        cout << "Exception received: message = " << e.message << endl;
    } catch(const DDS::BadHomeDefinition& e){
        cout << "Exception received: message = " << e.message << endl;
    } catch(const DDS::UserException&){
        cout << "Unknown exception received. No message available." << endl;
    }
    if(editor)
    {
        delete editor;
    }
}

WhiteListEditor::WhiteListEditor()
{
    cache = NULL;
    access = NULL;
    whiteListHome = NULL;
    userHome = NULL;
    name = NULL;
    userList = new UserInfoList();
    showHelp = false;
}

WhiteListEditor::~WhiteListEditor()
{
   // cache = NULL;
    //access = NULL;
    //whiteListHome = NULL;
    //userHome = NULL;
    //name = NULL;
    //delete userList;
    //showHelp = false;
}

void
WhiteListEditor::initializeEditor(
    )
{
    DDS::ObjectHomeSeq homes(2);

    /* First fill the array with the homes used within this application,
     * also store the homes within the instance variables within the
     * WhiteListEditor class, so we can use these homes later on without
     * having to search for them within the Cache.
     */
    whiteListHome = new DLRLChat::WhiteListHome();
    userHome = new DLRLChat::UserHome();
    homes.length(2);
    homes[0] = whiteListHome;
    homes[1] = userHome;

    /* Call the initializeDlrl function in the utlity library. The steps
     * to initialize the DLRL are relatively the same each time for the
     * various applications within our example.
     * We will simply provide a unique name for our cache, indicate how we
     * will use it (WRITE_ONLY) and provide the homes array we filled above
     */
    cache = Common::DlrlUtility::initializeDlrl(
        "WhiteListEditorCache",
        DDS::WRITE_ONLY,
        homes);

    /* Create a writeable CacheAccess to hold our writeable DLRL objects. */
    access = cache->create_access(DDS::WRITE_ONLY);
}

void
WhiteListEditor::deleteEditor(
    )
{
    //Common::DlrlUtility::deleteCacheAndParticipant(cache);
    cache = NULL;
    access = NULL;
    whiteListHome = NULL;
    userHome = NULL;
    name = NULL;
    showHelp = false;
    delete userList;
}

void
WhiteListEditor::addUsersToWhiteList(
    )
{
    DLRLChat::WhiteList_var whiteList;
    UserInfo* userInfo;

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
    whiteList = whiteListHome->create_unregistered_object(access);

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
    whiteList->set_name(name);

    /* Now that the WhiteList object has an identity we can finalize the
     * object by calling the 'register_object' operation. If this operation
     * is successful (it can fail if an object with the specified identity
     * already exists within the scope of the CacheAccess), then the object
     * from that point forward will participate as normal within the actions
     * of the CacheAccess. Meaning that if the 'write' operation is called
     * that the object will be written into the system and that the object
     * will appear in the objects listing of the CacheAccess, etc.
     */
    whiteListHome->register_object(whiteList);
    /* Now that we successfully created our WhiteList object we can iterate
     * over the list of users that need to be added to the WhiteList
     */
    for (unsigned int i = 0; i < userList->size(); i++)
    {
        DLRLChat::User_var a_user;

        userInfo = (*userList)[i];
        cout << "For whitelist '" << name <<"' adding user '" << userInfo->name << "' with id '" << userInfo->userID << "'." << endl;

        /* In the same manner as the creation of the WhiteList object we
         * will create the user object. It also uses custom key fields
         * and thus needs to perform the same three simple steps.
         */
        a_user = userHome->create_unregistered_object(access);

        /* Set its identity. The only key field of the user is it's id
         * so that is all we need to edit here before registering the object
         */
        a_user->set_id(userInfo->userID);

        /* Register the object with it's identity within the CacheAccess. */
        userHome->register_object(a_user);

        /* After the object has been registered, we can set the remaining
         * fields with the appropiate values
         */
        a_user->set_name(userInfo->name.c_str());

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
        whiteList->addUser(a_user);
    }

    /* The WhiteList object has been created and the users have been related
     * to the WhiteList. All changes within the CacheAccess now need to
     * be committed into the system, as long as the 'write' operation is
     * not called, all changes are deemed to be local and will not be
     * distributed throughout the system.
     * After the write call, all changes will be available system wide.
     */
    access->write();
}

/******************************************************************************
 ******************************************************************************
 * The following functions are merely utility functions for parsing arguments *
 * and showing the help instructions for this application. No DDS related     *
 * functionality is contained within these following operations               *
 ******************************************************************************
 ******************************************************************************/
void
WhiteListEditor::parseArguments(
    int argc,
    char *argv[])
{
    UserInfo* userInfo;
    /* very simple state machine */
    const int SCAN = 0;
    const int ADD = 1;
    const int NAME = 2;
    int state = SCAN;

    for(int i = 1; i < argc; i++)
    {
        switch (state)
        {
        case SCAN:
            if(0 == strcmp(argv[i], "-add"))
            {
                state = ADD;
            } else if(0 == strcmp(argv[i], "-name"))
            {
                state = NAME;
            } else if(0 == strcmp(argv[i], "help") ||
                      0 == strcmp(argv[i], "-help") ||
                      0 == strcmp(argv[i], "-h"))
            {
                showHelp = true;
            } else
            {
                throw DDS::BadParameter("Invalid token, expected '-add' or '-name'.");
            }
            break;
        case ADD:
            if(0 == strcmp(argv[i], "-add"))
            {
                state = ADD;//i.e. ignore
            } else if(0 == strcmp(argv[i], "-name"))
            {
                state = NAME;
            } else
            {
                userInfo = new UserInfo();
                std::string* token = new std::string (argv[i]);
                os_size_t index = token->find(',');
                if(index < 1)
                {
                    throw DDS::BadParameter("Invalid token, expected a userId,name combination like '1,dad'.");
                }
                delete token;
                istringstream userSpec(argv[i]);
                std::string id;
                getline(userSpec, id, ',');
                getline(userSpec, userInfo->name, ',');
                istringstream idSpec(id);
                idSpec >> userInfo->userID;
                userList->push_back(userInfo);
            }
            break;
        case NAME:
            if(name == NULL)
            {
                name = argv[i];
                state = SCAN;
            } else
            {
                throw DDS::BadParameter("Invalid token, encountered a second name, only one name may be specified!");
            }
            break;
        }
    }
    if(!showHelp)
    {
        if(name == NULL)
        {
            throw DDS::BadParameter("No whitelist name was specified!");
        }
        if(userList->size() == 0)
        {
            throw DDS::BadParameter("No users to be whitelisted were specified!");
        }
    }
}

/**
 * This operation prints a help message on the screen explaining how to use
 * the editor.
 */
void
WhiteListEditor::printHelp(
    )
{
    cout << "Available commands:" << endl;
    cout << "-name <whiteListName> [-add <userID1,userName1> <userID2,userName2> <...>]" << endl;
    cout << "\tExample:" << endl;
    cout << "\t-name family -add 1,mom 2,dad 3,John" << endl;
    cout << "\tAdds mom, dad and John to the WhiteList named family." << endl;
    cout << "\tThe '-add' command may be repeated between each user listing if so desired." << endl;
    cout << "help" << endl;
    cout << "\tDisplays this help menu." << endl;
}
