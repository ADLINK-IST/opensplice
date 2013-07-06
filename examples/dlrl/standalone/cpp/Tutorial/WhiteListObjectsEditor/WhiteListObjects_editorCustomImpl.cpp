#include "WhiteListObjects_editorCustomImpl.h"

#include <iostream>

using namespace std;

/**
 * In our example we defined a custom operation within our Object model for the
 * WhiteList object. We must implement this custom operation in a subclass. By
 * default the DLRL generates a subclass with a dummy implementation.
 * we can overwrite this default implementation easily by introducing a new
 * subclass and indicating in our mapping file that the DLRL should not use the
 * default subclass with it's dummy implementation, but an application custom
 * subclass with an application specific implementation of the subclass.
 * For the whitelist the mapping XML file containing the following line:
 *
 * <classMapping name="DLRLChat::WhiteList" implClass="WhiteListEditor::WhiteListCustomImpl">
 *
 * This is enough information for the DLRL to know which class it should
 * instantiate.
 *
 * Currently in the C++ API of the DLRL this functionality is not complete. So
 * we copy a custom implementation file over the generated file.
 */

/**
 * This operation adds the specified user to the WhiteList.
 *
 * @param a_user the User object that will be added to the WhiteList.
 */
void
WhiteListEditor::WhiteListCustomImpl::addUser(
    DLRLChat::User* a_user)
{
    DLRLChat::UserSet_var friends;

    try
    {
        friends = this->get_friends();
        friends->add(a_user);
    } catch (const DDS::PreconditionNotMet&)
    {
        DDS::String_var name;

        name = this->get_name();
        cout << "Unable to add user to white list '" << name.in() << "'. ";
        cout << "The white list is not writeable"  << endl;
    }
}
