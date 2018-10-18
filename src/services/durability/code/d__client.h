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
#ifndef D__CLIENT_H
#define D__CLIENT_H

#include "d__types.h"
#include "d__lock.h"
#include "d__table.h"
#include "os_mutex.h"
#include "os_time.h"

#include "client_durabilitySplType.h"


#if defined (__cplusplus)
extern "C" {
#endif

/* client durability reader flags.
 * These flags can be used to determine what functionality is
 * offered by a fellow
 */
#define D_HISTORICALDATAREQUEST_READER_FLAG  (0x0001U << 9) /*    1 */
#define D_DURABILITYSTATEREQUEST_READER_FLAG (0x0001U << 10) /*   2 */
#define D_HISTORICALDATA_READER_FLAG         (0x0001U << 11) /*   4 */
#define D_DURABILITYSTATE_READER_FLAG        (0x0001U << 12) /*   8 */

#define D_BASIC_CLIENT_DURABILITY_READER_FLAGS  \
    D_HISTORICALDATA_READER_FLAG

#define D_FULL_CLIENT_DURABILITY_READER_FLAGS  \
    (D_HISTORICALDATA_READER_FLAG | D_DURABILITYSTATE_READER_FLAG)

/**
 * Macro that checks the d_client validity.
 * Because d_client is a concrete class typechecking is required.
 */
#define             d_clientIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_CLIENT)

/**
 * \brief The d_client cast macro.
 *
 * This macro casts an object to a d_client object.
 */
#define d_client(_this) ((d_client)(_this))


C_STRUCT(d_client){
    C_EXTENDS(d_lock);
    d_networkAddress address;   /* Federation of the client */
    struct _DDS_Gid_t clientId; /* Client id */
    c_ulong readers;            /* Bit pattern to indicate which client durability readers are discovered */
    c_ulong requiredReaders;    /* Bit pattern to indicate which client durability readers MUST BE discovered before the client is considered responsive */
    c_bool isConfirmed;         /* Indicates if client is confirmed or not */
};

d_client                d_clientNew                     (d_networkAddress address);

void                    d_clientDeinit                  (d_client client);

void                    d_clientFree                    (d_client client);

int                     d_clientCompareByAddress        (d_client client1,
                                                         d_client client2);

d_networkAddress        d_clientGetAddress              (d_client client);

void                    d_clientAddReader               (d_client client,
                                                         c_ulong reader);

void                    d_clientRemoveReader            (d_client client,
                                                         c_ulong reader);

void                    d_clientSetClientId             (d_client client,
                                                         struct _DDS_Gid_t clientId);

c_bool                  d_clientIsConfirmed             (d_client client);

c_bool                  d_clientIsResponsive            (d_client client,
                                                         c_ulong requiredReaders,
                                                         c_bool waitForRemoteReaders);

#if defined (__cplusplus)
}
#endif

#endif /* D__CLIENT_H */
