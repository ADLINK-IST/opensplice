#include "WhiteListObjectsCustomImpl.h"

#include <iostream>

using namespace std;

/**
 * This operation prints the contents of the ChatMessage on screen, preceeded
 * by the name of its sender. Since this name is not part of the ChatMessage
 * object itself, it will navigate to its sender object to obtain it.
 */
void
WhiteListedMessageBoard::ChatMessageCustomImpl::display(
    )
{
    DLRLChat::User_var sender;
    DDS::String_var name;
    DDS::String_var message;
    try
    {
        /* Each DLRL object offers a finegrained control to see what exactly
         * is modified within the object. Operations are available to
         * easily determine which objects have changes, what attributes
         * have changes and if relations (1-1 or 1-n) are changed.
         * In our example we will only print a message if the message was
         * actually modified.
         */
        if(is_message_modified())
        {
            sender = get_sender();
            name = sender->get_name();
            message = get_message();
            cout << "<" << get_id() << ", " << name << "> " << message << endl;
        }
    } catch (const DDS::NotFound&)
    {
        message = get_message();
        cout << "<Unidentified> " << message << endl;
    }
}
