#ifndef D_GROUPCREATIONQUEUE_H
#define D_GROUPCREATIONQUEUE_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_groupCreationQueue(g) ((d_groupCreationQueue)(g))

d_groupCreationQueue    d_groupCreationQueueNew     (d_admin admin);

void                    d_groupCreationQueueFree    (d_groupCreationQueue queue);

c_bool                  d_groupCreationQueueAdd     (d_groupCreationQueue queue,
                                                     d_group group);

c_bool                  d_groupCreationQueueIsEmpty (d_groupCreationQueue queue);

#if defined (__cplusplus)
}
#endif

#endif /*D_GROUPCREATIONQUEUE_H*/
