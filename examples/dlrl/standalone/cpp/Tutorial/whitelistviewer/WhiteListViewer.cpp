#include "WhiteListViewer.h"
#include "DlrlUtility.h"
#include "PreconfigureTopics.h"
#include "ccpp_WhiteListObjects_editor.h"

#include <iostream>

#include "example_main.h"

using namespace WLV;
using namespace std;

int
OSPL_MAIN (
    int argc,
    char *argv[])
{
    WhiteListViewer* viewer = NULL;

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
    Common::PreconfigureTopics::simulateExternalTopicCreation();
    try
    {
        /* First initialize the viewer, this operation will ensure
         * the Cache is created and made ready for pub/sub.
         */
        viewer->initializeViewer();
        /* Once the application is set up for participation is DDS, we can
         * execute the viewer and show which WhiteLists exist and which
         * users are white listed in each list.
         */
        viewer->executeViewer();
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
    if(viewer != NULL)
    {
        viewer->deleteViewer();
        delete(viewer);
    }
}

void
WhiteListViewer::initializeViewer(
    )
{
    DDS::ObjectHomeSeq homes(2);

    /* First fill the array with the homes used within this application. */
    homes.length(2);
    homes[0] = new DLRLChat::WhiteListHome();
    homes[1] = new DLRLChat::UserHome();

    /* Call the initializeDlrl function in the utlity library. The steps
     * to initialize the DLRL are relatively the same each time for the
     * various applications within our example.
     * We will simply provide a unique name for our cache, indicate how we
     * will use it (READ_ONLY) and provide the homes array we filled above
     */
    cache = Common::DlrlUtility::initializeDlrl(
        "WhiteListViewerCache",
        DDS::READ_ONLY,
        homes);
}

void
WhiteListViewer::deleteViewer(
    )
{
    Common::DlrlUtility::deleteCacheAndParticipant(cache);
}

void
WhiteListViewer::executeViewer(
    )
{
    DLRLChat::WhiteListHome_var whiteListHome;
    DLRLChat::WhiteListSeq_var whitelists;
    DLRLChat::WhiteList_var aWhitelist;
    DLRLChat::UserSet_var userSet;
    DLRLChat::UserSeq_var users;
    DLRLChat::User_var aUser;

    /* Perform a refresh of the Cache, this will result in explicitly
     * reading the current system state and creating, modifying or deleting
     * objects based upon that system state. In our simple viewer example
     * we have implemented a simple 'run once' application, so we will
     * refresh once and just look at the available objects in the cache by
     * using the 'get_created_objects' operation, in this case it will yield
     * the same results as calling the 'get_objects'.
     */
    cache->refresh();
    /* If at any time an ObjectHome is needed, it can be located by
     * performing a 'find' on the Cache. A home can be located in two
     * ways, by the index assigned during registration, or by the
     * fully qualified IDL type name of the object. We will use the later
     * way of locating the needed object home.
     */
    whiteListHome = dynamic_cast<DLRLChat::WhiteListHome_ptr>(cache->find_home_by_name("DLRLChat::WhiteList"));
    /* Retrieve all newly created WhiteList objects within the WhiteList
     * home in the last refresh run.
     * Then iterate over the whitelists and print out the requested data.
     */
    whitelists = whiteListHome->get_created_objects(cache);
    if(whitelists->length() > 0)
    {
        for(int i = 0; i < (int)whitelists->length(); i++)
        {
            /* Get the next whitelist */
            aWhitelist = whitelists[i];
            /* Each whitelist has a set of users, which is a collection type
             * known within the DLRL. The set has various useful operations
             * for examining it's state.
             */
            userSet = aWhitelist->get_friends();
            /* Get all added users known within the set. During each refresh
             * run a lot of useful information is maintained for collections
             * such as which values were added and which were removed.
             * In this application we are only interested in the added
             * values as this is a 'run once' application. The 'added_values'
             * operation will yield the same results as the 'values'
             * operation in this specific example.
             */
            users = userSet->added_elements();

            /* Finally just print out the status of the WhiteList and the
             * users contained within the WhiteList
             */
            if(users->length() > 0)
            {
                cout << "Whitelist '" << aWhitelist->get_name() << "' contains the following users:" << endl;
                for(int j = 0; j < (int)users->length(); j++)
                {
                    aUser = users[j];
                    cout << "* " << aUser->get_id() << "," << aUser->get_name() << endl;
                }
            } else
            {
                cout << "Whitelist '" << aWhitelist->get_name() << "' contains no users." << endl;
            }
        }
    } else
    {
        cout << "No white lists were found" << endl;
    }
}
