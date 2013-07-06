#include "WhiteListedMessageBoard.h"
#include "DlrlUtility.h"
#include "PreconfigureTopics.h"
#include "MyWhiteListFilter.h"
#include "MyWhiteListSelectionListener.h"
#include "MyChatMessageListener.h"

#include <iostream>

#include "example_main.h"

using namespace WLMB;
using namespace std;

int
OSPL_MAIN (
    int argc,
    char *argv[])
{
    WhiteListedMessageBoard* msgBoard = 0;

    try
    {
        /* Create the message board for the white list name provided */
        msgBoard = new WhiteListedMessageBoard();
        /* Parse input parameter */
        msgBoard->parseWhiteListName(argc, argv);
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
        /* First initialize the WhiteListedMessageBoard, this operation will
         * ensure the Cache is created and made ready for pub/sub.
         */
        msgBoard->initializeMessageBoard();
        /* Once the application is set up for participation is DDS, we can
         * run the messageboard.
         */
        msgBoard->runMessageBoard();

    } catch(const DDS::PreconditionNotMet& e){
        cout << "Exception received: message = " << e.message << endl;
        msgBoard->printHelp();
    } catch(const DDS::BadParameter& e){
        cout << "Exception received: message = " << e.message << endl;
        msgBoard->printHelp();
    } catch(const DDS::InvalidObjects& e){
        cout << "Exception received: message = " << e.message << endl;
        msgBoard->printHelp();
    } catch(const DDS::NotFound& e){
        cout << "Exception received: message = " << e.message << endl;
        msgBoard->printHelp();
    } catch(const DDS::AlreadyExisting& e){
        cout << "Exception received: message = " << e.message << endl;
        msgBoard->printHelp();
    } catch(const DDS::AlreadyDeleted& e){
        cout << "Exception received: message = " << e.message << endl;
        msgBoard->printHelp();
    } catch(const DDS::NoSuchElement& e){
        cout << "Exception received: message = " << e.message << endl;
        msgBoard->printHelp();
    } catch(const DDS::SQLError& e){
        cout << "Exception received: message = " << e.message << endl;
        msgBoard->printHelp();
    } catch(const DDS::TimeOut& e){
        cout << "Exception received: message = " << e.message << endl;
        msgBoard->printHelp();
    } catch(const DDS::OutOfMemory& e){
        cout << "Exception received: message = " << e.message << endl;
        msgBoard->printHelp();
    } catch(const DDS::DCPSError& e){
        cout << "Exception received: message = " << e.message << endl;
        msgBoard->printHelp();
    } catch(const DDS::DLRLError& e){
        cout << "Exception received: message = " << e.message << endl;
        msgBoard->printHelp();
    } catch(const DDS::BadHomeDefinition& e){
        cout << "Exception received: message = " << e.message << endl;
        msgBoard->printHelp();
    } catch(const DDS::UserException&){
        cout << "Unknown exception received. No message available." << endl;
        msgBoard->printHelp();
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
    if(msgBoard != NULL)
    {
        msgBoard->deleteMessageBoard();
        delete(msgBoard);
    }
}

WhiteListedMessageBoard::WhiteListedMessageBoard(
    )
{
    cache = NULL;
    whiteList = NULL;
    terminate = false;
}
WhiteListedMessageBoard::~WhiteListedMessageBoard(
    )
{
    cache = NULL;
    whiteList = NULL;
}

void
WhiteListedMessageBoard::initializeMessageBoard(
    )
{
    DDS::ObjectHomeSeq homes(3);
    DLRLChat::WhiteListSelection_var wlSelection;
    DLRLChat::WhiteListHome_var whiteListHome;
    DLRLChat::UserHome_var userHome;
    DLRLChat::ChatMessageHome_var chatMessageHome;
    DLRLChat::WhiteListFilter_var wlFilter;
    DLRLChat::WhiteListSelectionListener_var wlSelectionListener;
    DLRLChat::ChatMessageListener_var chatMessageListener;

    /* First fill the array with the homes used within this application. */
    whiteListHome = new DLRLChat::WhiteListHome();
    userHome = new DLRLChat::UserHome();
    chatMessageHome = new DLRLChat::ChatMessageHome();
    homes.length(3);
    homes[0] = whiteListHome;
    homes[1] = userHome;
    homes[2] = chatMessageHome;

    /* Call the initializeDlrl function in the utlity library. The steps
     * to initialize the DLRL are relatively the same each time for the
     * various applications within our example.
     * We will simply provide a unique name for our cache, indicate how we
     * will use it (WRITE_ONLY) and provide the homes array we filled above
     */
    cache = Common::DlrlUtility::initializeDlrl(
        "WhiteListedMessageBoardCache",
        DDS::READ_ONLY,
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
    wlFilter = new MyWhiteListFilter(name);
    wlSelection = whiteListHome->create_selection(
        wlFilter,
        true,
        false);
    wlSelectionListener = new MyWhiteListSelectionListener(this);
    wlSelection->set_listener(wlSelectionListener);

    /* Attach an ObjectListener for the ChatMessage type. This listener
     * will be triggered each time a ChatMessage object is created, modified
     * or deleted.
     */
    chatMessageListener = new MyChatMessageListener(this);
    chatMessageHome->attach_listener(chatMessageListener, false);
}

void
WhiteListedMessageBoard::deleteMessageBoard(
    )
{
   //Common::DlrlUtility::deleteCacheAndParticipant(cache);
}

void
WhiteListedMessageBoard::runMessageBoard(
    )
{
    cout << "WhiteListedMessageBoard has opened: send a ChatMessage with userID = -1 to close it....\n" << endl;
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
    cache->enable_updates();

    /* Now just sleep until the termination flag is set */
    while(!terminate)
    {
        /* Sleep for 100 ms. as not to consume too much CPU power. */
#ifdef USE_NANOSLEEP
        struct timespec sleeptime;
        struct timespec remtime;
        sleeptime.tv_sec = 0;
        sleeptime.tv_nsec = 100000000;
        nanosleep(&sleeptime, &remtime);
#else
#ifdef WIN32
		Sleep (100);
#else
        usleep(100000);
#endif
#endif
    }
}

void
WhiteListedMessageBoard::setWhiteList(
    DLRLChat::WhiteList* whiteList)
{
    this->whiteList = whiteList;
    whiteList->_add_ref();
}

bool
WhiteListedMessageBoard::isWhiteListed(
    DLRLChat::ChatMessage* msg)
{
    DLRLChat::User_var user;
    bool isWhiteListed = false;

    /* If the WhiteList with the specified name was not yet found (i.e.,
     * not created, name does not match, etc), then always return false
     * (i.e., block everything)
     */
    if(whiteList != NULL)
    {
        DLRLChat::UserSet_var friends;
        /* A message may be displayed if the user that sent the message is
         * contained within the WhiteList. We will simply navigate from
         * the ChatMessage object to the User object as the DLRL will always
         * ensure this relation is up to date and ready to be navigated.
         */
        user = msg->get_sender();
        /* Now that we have our User object, we can use the Set collection
         * of the WhiteList to check if the User is actually contained
         * within the 'friends' listing of the WhiteList. The DLRL also
         * automatically keeps this listing up to date and ensures the
         * contains the the 'friends' set matches what is known within the
         * system. It could not be simpler.
         */
        friends = whiteList->get_friends();
        if (friends->contains(user))
        {
            isWhiteListed = true;
        }
    }
    return isWhiteListed;

}

void
WhiteListedMessageBoard::setTerminate(
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

void
WhiteListedMessageBoard::parseWhiteListName(
    int argc,
    char *argv[])
{
    /* We only accept the '-name' flag with the accompying name, so if there
     * are not exactly 2 arguements, something is wrong!
     */
    if(argc != 3)/*2 arguements plus program name */
    {
        throw DDS::BadParameter("Expected (only) 2 arguments.");
    }
    /* Verify the first argument is -name */
    if(0 != strcmp(argv[1], "-name"))
    {
        throw DDS::BadParameter("Expected the first argument to equals '-name'.");
    }
    /* Verify the second argument is not another flag, i.e., does not start
     * with a '-'
     */
    if(argv[2][0] == '-')
    {
        throw DDS::BadParameter("Detected an invalid name. A name may not start with '-'.");
    }
    name = argv[2];
}

void
WhiteListedMessageBoard::printHelp(
    )
{
    cout << "Usage:" << endl;
    cout << "\tWhiteListedMessageBoard -name <whiteListName>" << endl;
    cout << "\n" << endl;
    cout << "Example:" << endl;
    cout << "\tWhiteListedMessageBoard -name family\n" << endl;
    cout << "\tDisplays only messages coming from senders mentioned in the WhiteList named 'family'." << endl;
    cout << endl;
}
