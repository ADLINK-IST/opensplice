#ifndef WhiteListObjects_editor_CUSTOM_IMPL_H
#define WhiteListObjects_editor_CUSTOM_IMPL_H

#include "ccpp_WhiteListObjects_editor.h"
#include "ccpp_WhiteListObjects_editor_abstract.h"
#include "dlrl_tutorial_if.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace WhiteListEditor
{
    class WhiteListCustomImpl :
        public DLRLChat::WhiteList_abstract
    {
        public:
            void addUser (DLRLChat::User* a_user);
            void displayAllUsers();
    };

};
#endif /* WhiteListObjects_editor_CUSTOM_IMPL_H */

