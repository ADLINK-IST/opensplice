#include "MyWhiteListFilter.h"

using namespace WLMB;

MyWhiteListFilter::MyWhiteListFilter(
    const char* name) : nameFilter(name)
{
}

DDS::Boolean
MyWhiteListFilter::check_object(
    DLRLChat::WhiteList* an_object,
    DDS::MembershipState membership_state)
{
    bool passesFilter = false;
    /* It is very important to catch ALL exceptions within the scope of the
     * filter operation. If this is not done the exception will be
     * propagated into the DLRL kernel, where the DLRL will report the
     * error to an error log and shutdown the thread handling updates
     * due to the exception being a fatal error, as any exception thrown
     * by this operation is by definition an application induced exception
     * for which there is no generic way of recovery.
     */
    try
    {
        DDS::String_var name = an_object->get_name();
        if (0 == strcmp(nameFilter, name))
        {
            passesFilter = true;
        }
    }
    catch (...)
    {
        /* Any sort of exception handling may occur here, this is
         * application driven. In our example the exception will be
         * simply caught and ignored.
         */
    }
    return passesFilter;
}
