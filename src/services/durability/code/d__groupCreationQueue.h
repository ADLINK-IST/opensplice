#ifndef D__GROUPCREATIONQUEUE_H
#define D__GROUPCREATIONQUEUE_H

#include "d__types.h"
#include "d_lock.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_groupCreationQueue) {
    C_EXTENDS(d_lock);
    os_threadId actionThread;
    c_bool terminate;
    c_iter groups;
    d_admin admin;
    c_long groupsToCreateVolatile;
    c_long groupsToCreateTransient;
    c_long groupsToCreatePersistent;
};

void    d_groupCreationQueueDeinit  (d_object object);

#if defined (__cplusplus)
}
#endif

#endif /*D__GROUPCREATIONQUEUE_H*/
