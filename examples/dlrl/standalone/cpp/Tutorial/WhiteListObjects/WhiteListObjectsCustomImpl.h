#ifndef WhiteListObjects_CUSTOM_IMPL_H
#define WhiteListObjects_CUSTOM_IMPL_H

#include "ccpp_WhiteListObjects.h"
#include "ccpp_WhiteListObjects_abstract.h"
#include "dlrl_tutorial_if.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace WhiteListedMessageBoard
{
    class ChatMessageCustomImpl :
        public DLRLChat::ChatMessage_abstract
    {
        public:
            void display();
    };
};


#endif /* WhiteListObjects_CUSTOM_IMPL_H */

