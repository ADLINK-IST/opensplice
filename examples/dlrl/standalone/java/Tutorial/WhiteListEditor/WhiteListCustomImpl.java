package WhiteListEditor;

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
 * instantiate (which happens through a JNI call, so the only reference to the
 * specified class is a string given to a JNI call if one were to search the
 * generated code outputted by the DCG...)
 */
public final class WhiteListCustomImpl extends DLRLChat.WhiteList
{

    /**
     * This operation adds the specified user to the WhiteList.
     *
     * @param a_user the User object that will be added to the WhiteList.
     */
    public void addUser(
        DLRLChat.User a_user)
    {
        try
        {
            this.get_friends().add(a_user);
        } catch (DDS.PreconditionNotMet e)
        {
            System.out.println("Unable to add user to white list '"+
				this.get_name()+"'. The white list is not writeable");
        }
    }
}
