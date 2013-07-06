package WhiteListedMessageBoard;

/**
 * In our example we defined a custom operation within our Object model for the
 * ChatMessage object. We must implement this custom operation in a subclass. By
 * default the DLRL generates a subclass with a dummy implementation.
 * we can overwrite this default implementation easily by introducing a new
 * subclass and indicating in our mapping file that the DLRL should not use the
 * default subclass with it's dummy implementation, but an application custom
 * subclass with an application specific implementation of the subclass.
 * For the ChatMessage the mapping XML file containing the following line:
 *
 * <classMapping name="DLRLChat::ChatMessage" implClass="WhiteListedMessageBoard::ChatMessageCustomImpl">
 *
 * This is enough information for the DLRL to know which class it should
 * instantiate (which happens through a JNI call, so the only reference to the
 * specified class is a string given to a JNI call if one were to search the
 * generated code outputted by the DCG...)
 */
public final class ChatMessageCustomImpl extends DLRLChat.ChatMessage
{
    /**
     * This operation prints the contents of the ChatMessage on screen,
     * preceeded by the name of its sender. Since this name is not part of the
     * ChatMessage object itself, it will navigate to its sender object
     * to obtain it.
     */
    public void display()
    {
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
                System.out.println("<"+get_id()+", "+get_sender().get_name()+ "> " + get_message());
            }
        } catch (DDS.NotFound e)
        {
            System.out.println("<Unidentified> " + get_message());
        }
    }

}
