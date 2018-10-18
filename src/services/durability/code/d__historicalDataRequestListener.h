/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef D__HISTORICAL_DATA_REQUEST_LISTENER_H
#define D__HISTORICAL_DATA_REQUEST_LISTENER_H

#include "d__types.h"
#include "d__listener.h"
#include "d__table.h"
#include "d__historicalDataRequest.h"
#include "ut_fibheap.h"
#include "ut_avl.h"
#include "u_dataReader.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_historicalDataRequestListener) {
    C_EXTENDS(d_listener);
    u_dataReader dataReader;
    d_waitsetEntity waitsetData;
    d_eventListener historicalDataRequestEventListener;
    d_subscriber subscriber;
    /* Internal structure to administrate pending historicalDataRquests */
    ut_avlTreedef_t request_avltreedef; /* tree definition for requests */
    ut_avlTreedef_t pubinfo_avltreedef; /* tree definition for writer/publisher */
    ut_avlTree_t request_id_tree;       /* tree structure for request lookup by id */
    ut_avlTree_t pubinfo_tree;          /* tree structure for pubinfo lookup by partition */
    ut_fibheap_t prioqueue;             /* priority queue structure ordered on expiration time */
    os_mutex queue_lock;                /* lock to protect modifications to the request queue */
    os_cond cond;                       /* condition variable to trigger the answering of requests */
    c_bool terminate;                   /* Indicates whether or not the listener should terminate */
    os_threadId handlerThread;          /* Thread to handle incoming requests */
    c_bool handlerThreadExists;         /* Indicates if handlerThread creation succeeded or not */
};


/* Internal structure to administrate pending historicalDataRquests */
struct request_admin_node {
    ut_avlNode_t request_avlnode;       /* node in request_avltreedef */
    ut_avlNode_t pubinfo_avlnode;       /* node in pubinfo_avltreedef */
    ut_fibheapNode_t fhnode;            /* ptr to node in fibheap priority queue */
    os_timeE insert_time;               /* time to insert */
    os_timeE expiration_time;           /* time to expire */
    os_timeE handle_time;               /* time to handle */
    c_bool (*callback)(void *listener, void *arg);       /* callback function to call when item is handled */
    void *listener;                     /* listener that provides the context for the callback */
    void *arg;                          /* argument of callback function */
};



/* Internal structure to administrate cached publishers */
struct pubInfo {
    u_publisher publisher;              /* publisher */
    u_writer writer;                    /* writer for historical data */
    char *partition;                    /* partition used to publish the data */
};

extern const ut_fibheapDef_t prioqueue_fhdef;

d_historicalDataRequestListener    d_historicalDataRequestListenerNew                  (d_subscriber subscriber);

void                               d_historicalDataRequestListenerFree                 (d_historicalDataRequestListener listener);

c_bool                             d_historicalDataRequestListenerStart                (d_historicalDataRequestListener listener);

c_bool                             d_historicalDataRequestListenerStop                 (d_historicalDataRequestListener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D__HISTORICAL_DATA_REQUEST_LISTENER_H */
