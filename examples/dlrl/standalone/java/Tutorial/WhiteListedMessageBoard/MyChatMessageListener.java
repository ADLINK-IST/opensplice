package WhiteListedMessageBoard;

/**
 * This class implements the DLRL specific ObjectListener interface, this
 * listener is used within the WhiteListedMessageBoard application to get
 * fine grained notification every time a ChatMessage object is created,
 * modified or deleted.
 * This listener can be attached to a ChatMessageHome and will then be
 * triggered each time updates arrives for a ChatMessage home. Assuming the
 * cache has updates enabled (meaning that a call to the cache 'updates_enabled'
 * operation returns true).
 */
public class MyChatMessageListener implements DLRLChat.ChatMessageListener
{

    /* Contains the message board object relevant to this specific listener
     * so it can invoke operations on the messageboard to determine if a
     * user sending the received message is whitelisted or not.
     */
    private WhiteListedMessageBoard msgBoard;

    /* Constructor for the listener
     *
     * @param msgBoard the WhiteListedMessageBoard this listener is linked too.
     */
    public MyChatMessageListener(
        WhiteListedMessageBoard msgBoard)
    {
        this.msgBoard = msgBoard;
    }


    /**
     * This operation is defined by the ChatMessageListener interface that this
     * class is implementing. It will be invoked by DLRL each time a ChatMessage
     * object is created.
     */
    public boolean on_object_created(
        DLRLChat.ChatMessage msg)
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
            if(msg.get_id() == -1)
            {
                msgBoard.setTerminate();
            } else
            {
                /* The creation of the ChatMessage indicates the user has begun
                 * it's chat session in our example. So a message is printed
                 * indicating that the user has entered the chatroom.
                 */
                System.out.println("* User "+msg.get_sender().get_name()+" has joined the chatroom!");
                /* If the ChatMessage is whitelisted, then display it, otherwise
                 * ignore it.
                 */
                if(msgBoard.isWhiteListed(msg))
                {
                    /* In our example we  invoke a special custom operation
                     * defined on the ChatMessage object, this demonstrates
                     * how this feature of DLRL works and shows that each DLRL
                     * object can easily be extended with a few custom
                     * operations and can then be used without any special
                     * requirements. The implementation of this custom operation
                     * can be found in the ChatMessageCustomImpl class.
                     */
                    msg.display();
                } else
                {
                    System.out.println("* Message from user '"+
                        msg.get_sender().get_name()+"' with id '"+
                        msg.get_sender().get_id()+"' was blocked....");
                }
            }
        }
        catch (Exception e)
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

    public boolean on_object_modified(
        DLRLChat.ChatMessage msg)
    {
        try
        {
            /* If the ChatMessage is whitelisted, then display it, otherwise
             * ignore it.
             */
            if(msgBoard.isWhiteListed(msg))
            {
                /* Invoke the custom operation to display the message */
                msg.display();
            } else
            {
                System.out.println("* Message from user '"+
                    msg.get_sender().get_name()+"' with id '"+
                    msg.get_sender().get_id()+"' was blocked....");
            }
        }
        catch (Exception e)
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

    public boolean on_object_deleted(
        DLRLChat.ChatMessage msg)
    {
        try
        {
            /* If the ChatMessage is whitelisted, then display it, otherwise
             * ignore it.
             */
            if(msgBoard.isWhiteListed(msg))
            {
                /* Invoke the custom operation to display the message */
                msg.display();
            } else
            {
                System.out.println("* Message from user '"+
                    msg.get_sender().get_name()+"' with id '"+
                    msg.get_sender().get_id()+"' was blocked....");
            }
            /* The deletion of the ChatMessage indicates the user has terminated
             * it's chat session in our example. So a message is printed
             * indicating that the user has left the chatroom.
             */
            System.out.println("* User "+msg.get_sender().get_name()+" has left the chatroom!");
        }
        catch (Exception e)
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
}