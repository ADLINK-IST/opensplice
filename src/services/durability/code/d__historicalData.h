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

#ifndef D__HISTORICAL_DATA_H
#define D__HISTORICAL_DATA_H

#include "d__types.h"
#include "d__admin.h"
#include "d__historicalDataRequestListener.h"
#include "d__historicalDataRequest.h"
#include "client_durabilitySplType.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_historicalData validity.
 * Because d_historicalData is a concrete class typechecking is required.
 */
#define             d_historicalDataIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_HISTORICAL_DATA)


/**
 * \brief The d_historicalData cast macro.
 *
 * This macro casts an object to a d_historicalData object.
 */
#define d_historicalData(s) ((d_historicalData)(s))

C_STRUCT(d_historicalData) {
    C_EXTENDS(d_object);
    d_durability durability;                                  /* the durability service */
    d_historicalDataRequest historicalDataRequest;            /* (combined) historicalDataRequest to answer */
    struct _DDS_DurabilityVersion_t version;                  /* durability version of the server */
    struct _DDS_Gid_t serverId;                               /* Id of the server */
    char *alignmentPartition;                                 /* The partition used to publish the data */
    struct _DDS_HistoricalDataContent_t content;              /* data content */
    c_iter requestIds;                                        /* list of (combined) request ids */
    c_iter extensions;
    c_iter partitions;
    c_ulong errorCode;
    /* Data collection */
    c_iter list;                                              /* the group data (vmessages) to send */
    c_iter instances;                                         /* group instance data */
    d_group group;                                            /* durability group to write */
    v_group vgroup;                                           /* associated vgroup to write */
    struct sd_cdrInfo *cdrInfo;                               /* cdr serializer info */
    v_message vmessage;                                       /* the vmessage associated with the bead */
    c_ulong beadSize;                                         /* size of the current payload */
    c_ulong totalSize;                                        /* cumulative size of all payloads per group */
    c_bool checkTimeRange;                                    /* time range checking required */
    unsigned short messageKind;                               /* kind of message */
    c_voidp *blob;                                            /* serialized data blob */
    /* Various counters*/
    c_ulong count;
    c_ulong writeCount;
    c_ulong disposeCount;
    c_ulong writeDisposeCount;
    c_ulong registerCount;
    c_ulong unregisterCount;
    c_ulong skipCount;
    /* completeness */
    unsigned short completeness;
    /* publisher struct containing the publisher and writer */
    struct pubInfo *pubinfo;
};


d_historicalData                        d_historicalDataNew                               (d_admin admin, d_historicalDataRequest request);

void                                    d_historicalDataDeinit                            (d_historicalData historicalData);

void                                    d_historicalDataFree                              (d_historicalData historicalData);

c_bool                                  d_historicalDataWrite                             (c_voidp *data);

c_bool delete_pubinfo(void *listener, void *arg);

#if defined (__cplusplus)
}
#endif

#endif /* D__HISTORICAL_DATA_H */
