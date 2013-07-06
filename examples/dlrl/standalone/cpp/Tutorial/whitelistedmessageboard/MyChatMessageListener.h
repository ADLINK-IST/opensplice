#ifndef WLMB_MYCHATMESSAGELISTENER
#define WLMB_MYCHATMESSAGELISTENER

#include "ccpp_WhiteListObjects.h"
#include "WhiteListedMessageBoard.h"

namespace WLMB //WhiteListedMessageBoard
{
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
    class MyChatMessageListener :
        public DLRLChat::ChatMessageListener
    {

        /* Contains the message board object relevant to this specific listener
         * so it can invoke operations on the messageboard to determine if a
         * user sending the received message is whitelisted or not.
         */
        private:
            WhiteListedMessageBoard* msgBoard;

        /* Constructor for the listener
         *
         * @param msgBoard the WhiteListedMessageBoard this listener is linked too.
         */
        public:
            MyChatMessageListener(
                WhiteListedMessageBoard* msgBoard);

            ~MyChatMessageListener(
                );
        /**
         * This operation is defined by the ChatMessageListener interface that this
         * class is implementing. It will be invoked by DLRL each time a ChatMessage
         * object is created.
         */
        public:
            DDS::Boolean
            on_object_created(
                DLRLChat::ChatMessage* msg);

        public:
            DDS::Boolean
            on_object_modified(
                DLRLChat::ChatMessage* msg);

        public:
            DDS::Boolean
            on_object_deleted(
                DLRLChat::ChatMessage* msg);
    };
};

#endif
