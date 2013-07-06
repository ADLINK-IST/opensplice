/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef D_PUBLISHER_H
#define D_PUBLISHER_H

#include "d__types.h"
#include "u_user.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_publisher(w) ((d_publisher)(w))

d_publisher     d_publisherNew                      (d_admin admin);

void            d_publisherFree                     (d_publisher publisher);

c_bool          d_publisherStatusWrite              (d_publisher publisher,
                                                     d_status message,
                                                     d_networkAddress addressee);

c_bool          d_publisherNewGroupWrite            (d_publisher publisher,
                                                     d_newGroup message,
                                                     d_networkAddress addressee);

c_bool          d_publisherGroupsRequestWrite       (d_publisher publisher,
                                                     d_groupsRequest message,
                                                     d_networkAddress addressee);
                                                     
c_bool          d_publisherStatusRequestWrite       (d_publisher publisher,
                                                     d_statusRequest message,
                                                     d_networkAddress addressee);

c_bool          d_publisherSampleRequestWrite       (d_publisher publisher,
                                                     d_sampleRequest message,
                                                     d_networkAddress addressee);

c_bool          d_publisherSampleChainWrite         (d_publisher publisher,
                                                     d_sampleChain message,
                                                     d_networkAddress addressee);

c_bool          d_publisherNameSpacesRequestWrite   (d_publisher publisher,
                                                     d_nameSpacesRequest message,
                                                     d_networkAddress addressee);

c_bool          d_publisherNameSpacesWrite          (d_publisher publisher,
                                                     d_nameSpaces message,
                                                     d_networkAddress addressee);

c_bool          d_publisherDeleteDataWrite          (d_publisher publisher,
                                                     d_deleteData message,
                                                     d_networkAddress addressee);

#if defined (__cplusplus)
}
#endif

#endif /* D__PUBLISHER_H */
