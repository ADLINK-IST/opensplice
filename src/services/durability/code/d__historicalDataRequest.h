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

#ifndef D__HISTORICAL_DATA_REQUEST_H
#define D__HISTORICAL_DATA_REQUEST_H

#include "d__types.h"
#include "d__admin.h"
#include "os_time.h"
#include "client_durabilitySplType.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define HISTORICAL_DATA_KIND_BEAD 0
#define HISTORICAL_DATA_KIND_LINK 1


#define HISTORICAL_DATA_COMPLETENESS_UNKNOWN         0
#define HISTORICAL_DATA_COMPLETENESS_INCOMPLETE      1
#define HISTORICAL_DATA_COMPLETENESS_COMPLETE        2

#define PAYLOAD_SERIALIZATION_FORMAT_CDR_ANY         0
#define PAYLOAD_SERIALIZATION_FORMAT_CDR_BE          1
#define PAYLOAD_SERIALIZATION_FORMAT_CDR_LE          2

/**
 * Macro that checks the d_historicalDataRequest validity.
 * Because d_historicalDataRequest is a concrete class typechecking is required.
 */
#define             d_historicalDataRequestIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_HISTORICAL_DATA_REQ)


/**
 * \brief The d_historicalDataRequest cast macro.
 *
 * This macro casts an object to a d_historicalDataRequest object.
 */
#define d_historicalDataRequest(s) ((d_historicalDataRequest)(s))

C_STRUCT(d_historicalDataRequest) {
    C_EXTENDS(d_object);
    struct _DDS_DurabilityVersion_t version;                  /* version of the request */
    c_iter requestIds;                                        /* list of (combined) request ids */
    char * topic;                                             /* requested topic */
    c_iter partitions;                                        /* list of partitions expressions */
    _DDS_PayloadSerializationFormat_t serializationFormat;    /* serialization format */
    struct _DDS_DataReaderQos dataReaderQos;                  /* qos of datareader for the topic */
    os_timeW startTime;                                       /* start time for matching historical data */
    os_timeW endTime;                                         /* end time for matching historical data; this will never be higher than the time when the historicalDataRequest is received */
    char *sqlFilter;                                          /* filter expression, if "" no filter is applied */
    char **sqlFilterParams;                                   /* list of ptr to parameters of filter expressions */
    c_ulong sqlFilterParamsSize;                              /* nr of parameters */
    c_long maxSamples;                                        /* max samples to request, -1 for unlimited */
    c_long maxInstances;                                      /* max instances to request, -1 for unlimited */
    c_long maxSamplesPerInstance;                             /* max samples per instance, -1 for unlimited */
    c_iter alignmentPartition;                                /* candidate partitions to receive the response */
    c_iter serverIds;                                         /* serverIds that should respond */
    os_duration timeout;                                      /* timeout that indicates how long the server has to respond */
    c_iter extensions;                                        /* future extensions */
    os_timeE receptionTime;                                   /* elapsed time when request was received */
    os_timeE expirationTime;                                  /* elapsed time before which the request needs to be answered */
    os_timeE answerTime;                                      /* elapsed time when it is decide to answer the request */
    c_bool forMe;                                             /* indicates if the request is addressed to me */
    c_bool forEverybody;                                      /* indicates if request is addressed to everybody */
    c_ulong errorCode;                                        /* result of sanity check for this request */
    /* various response control settings */
    c_bool include_payload_unregistrations;  /* Indicates whether or not to include the payload in the unregistration response to this request */
};


#define D_CLIENT_DURABILITY_MAX_ERROR_LENGTH    255

d_historicalDataRequest                 d_historicalDataRequestNew                        (d_admin admin,
                                                                                           struct _DDS_HistoricalDataRequest *request);

void                                    d_historicalDataRequestDeinit                     (d_historicalDataRequest historicalDataRequest);

void                                    d_historicalDataRequestFree                       (d_historicalDataRequest historicalDataRequest);

c_ulong                                 d_historicalDataRequestSanityCheck                (d_historicalDataRequest historicalDataRequest);

int                                     d_historicalDataRequestCompareByRequestId         (d_historicalDataRequest historicalDataRequest1,
                                                                                           d_historicalDataRequest historicalDataRequest2);

#if defined (__cplusplus)
}
#endif

#endif /* D__HISTORICAL_DATA_REQUEST_H */
