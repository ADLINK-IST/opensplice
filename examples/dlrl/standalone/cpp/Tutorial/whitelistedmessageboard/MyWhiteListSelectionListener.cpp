#include "MyWhiteListSelectionListener.h"

using namespace WLMB;

MyWhiteListSelectionListener::MyWhiteListSelectionListener(
    WhiteListedMessageBoard* msgBoard)
{
    this->msgBoard = msgBoard;
}

MyWhiteListSelectionListener::~MyWhiteListSelectionListener(
    )
{
}

void
MyWhiteListSelectionListener::on_object_in (
    DLRLChat::WhiteList* the_object)
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
        /* When we find a white list that matches our filter, we will
         * notify this to the messageboard.
         */
        msgBoard->setWhiteList(the_object);
    }
    catch (...)
    {
        /* Any sort of exception handling may occur here, this is
         * application driven. In our example the exception will be
         * simply caught and ignored.
         */
    }
}

void
MyWhiteListSelectionListener::on_object_modified (
    DLRLChat::WhiteList* the_object)
{
    try
    {
        /* do nothing, we're not interested in this event */
    }
    catch (...)
    {
        /* Any sort of exception handling may occur here, this is
         * application driven. In our example the exception will be
         * simply caught and ignored.
         */
    }
}

void
MyWhiteListSelectionListener::on_object_out (
    DLRLChat::WhiteList* the_object)
{
    try
    {
        msgBoard->setWhiteList(NULL);
    }
    catch (...)
    {
        /* Any sort of exception handling may occur here, this is
         * application driven. In our example the exception will be
         * simply caught and ignored.
         */
    }
}
