#include "MyChatMessageListener.h"

#include <iostream>

using namespace WLMB;
using namespace std;

MyChatMessageListener::MyChatMessageListener(
    WhiteListedMessageBoard* msgBoard)
{
    this->msgBoard = msgBoard;
}

MyChatMessageListener::~MyChatMessageListener(
    )
{
}

DDS::Boolean
MyChatMessageListener::on_object_created(
    DLRLChat::ChatMessage* msg)
{
    /* It is very important to catch ALL exceptions within the scope of the
     * listener operation. If this is not done the exception will be
     * propagated into the DLRL kernel, where the DLRL will report the
     * error to an error log and shutdown the thread handling updates
     * due to the exception being a fatal error, as any exception thrown
     * by this operation is by definition an application induced exception
     * for which there is no generic way of recovery.
     */
    try
    {
        /* In our example a message with a user ID of '-1' indicates that
         * the messageboard must be terminated, so if such a message is
         * encountered indicate to the messageboard that is should
         * terminate.
         */
        if(msg->get_id() == -1)
        {
            msgBoard->setTerminate();
        } else
        {
            DLRLChat::User_var user;
            /* The creation of the ChatMessage indicates the user has begun
             * it's chat session in our example. So a message is printed
             * indicating that the user has entered the chatroom.
             */
            user = msg->get_sender();
            cout << "* User " << user->get_name() << " has joined the chatroom!" << endl;
            /* If the ChatMessage is whitelisted, then display it, otherwise
             * ignore it.
             */
            if(msgBoard->isWhiteListed(msg))
            {
                /* In our example we  invoke a special custom operation
                 * defined on the ChatMessage object, this demonstrates
                 * how this feature of DLRL works and shows that each DLRL
                 * object can easily be extended with a few custom
                 * operations and can then be used without any special
                 * requirements. The implementation of this custom operation
                 * can be found in the ChatMessageCustomImpl class.
                 */
                msg->display();
            } else
            {
                cout << "* Message from user '" <<
                    user->get_name()<<"' with id '" <<
                    user->get_id() << "' was blocked...." << endl;
            }
        }
    }
    catch (...)
    {
        /* Any sort of exception handling may occur here, this is
         * application driven. In our example the exception will be
         * simply caught and ignored.
         */
    }
    /* At the end of the callback a 'true' value is always returned, which
     * indicates to the DLRL that this message has been fully processed.
     * If 'false' were to be returned, then the DLRL would also trigger any
     * parent ObjectHome registered listeners if these were available.
     * This is not relevant for our example
     */
    return true;
}

DDS::Boolean
MyChatMessageListener::on_object_modified(
    DLRLChat::ChatMessage* msg)
{
    try
    {
        /* If the ChatMessage is whitelisted, then display it, otherwise
         * ignore it.
         */
        if(msgBoard->isWhiteListed(msg))
        {
            /* Invoke the custom operation to display the message */
            msg->display();
        } else
        {
            DLRLChat::User_var user;

            user = msg->get_sender();
            cout << "* Message from user '" <<
                user->get_name()<<"' with id '" <<
                user->get_id() <<"' was blocked...." << endl;
        }
    }
    catch (...)
    {
        /* Any sort of exception handling may occur here, this is
         * application driven. In our example the exception will be
         * simply caught and ignored.
         */
    }
    /* At the end of the callback a 'true' value is always returned, which
     * indicates to the DLRL that this message has been fully processed.
     * If 'false' were to be returned, then the DLRL would also trigger any
     * parent ObjectHome registered listeners if these were available.
     * This is not relevant for our example
     */
    return true;
}

DDS::Boolean
MyChatMessageListener::on_object_deleted(
    DLRLChat::ChatMessage* msg)
{
    try
    {
        DLRLChat::User_var user;

        user = msg->get_sender();
        /* If the ChatMessage is whitelisted, then display it, otherwise
         * ignore it.
         */
        if(msgBoard->isWhiteListed(msg))
        {
            /* Invoke the custom operation to display the message */
            msg->display();
        } else
        {
            cout << "* Message from user '" <<
                user->get_name()<<"' with id '" <<
                user->get_id() <<"' was blocked...." << endl;
        }
        /* The deletion of the ChatMessage indicates the user has terminated
         * it's chat session in our example. So a message is printed
         * indicating that the user has left the chatroom.
         */
        cout << "* User " << user->get_name() << " has left the chatroom!" << endl;
    }
    catch (...)
    {
        /* Any sort of exception handling may occur here, this is
         * application driven. In our example the exception will be
         * simply caught and ignored.
         */
    }
    /* At the end of the callback a 'true' value is always returned, which
     * indicates to the DLRL that this message has been fully processed.
     * If 'false' were to be returned, then the DLRL would also trigger any
     * parent ObjectHome registered listeners if these were available.
     * This is not relevant for our example
     */
    return true;
}
