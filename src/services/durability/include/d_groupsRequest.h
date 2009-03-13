
#ifndef D_GROUPSREQUEST_H
#define D_GROUPSREQUEST_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_groupsRequest(s) ((d_groupsRequest)(s))

d_groupsRequest     d_groupsRequestNew     (d_admin admin, 
                                            d_partition partition,
                                            d_topic topic);

void                d_groupsRequestFree    (d_groupsRequest groupsRequest);

#if defined (__cplusplus)
}
#endif

#endif /* D_GROUPSREQUEST_H */
