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
#ifndef GAPI_OBJMANAG_H
#define GAPI_OBJMANAG_H

#include "gapi.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DCPSGAPI
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

OS_API void *
gapi__header (
    void *object);

OS_API gapi_boolean
gapi_domainParticipantQos_free (
    void *object);

OS_API gapi_boolean
gapi_topicQos_free (
    void *object);

OS_API gapi_boolean
gapi_dataWriterQos_free (
    void *object);

OS_API gapi_boolean
gapi_publisherQos_free (
    void *object);

OS_API gapi_boolean
gapi_dataReaderQos_free (
    void *object);

OS_API gapi_boolean
gapi_dataReaderViewQos_free (
    void *object);

OS_API gapi_boolean
gapi_subscriberQos_free (
    void *object);

gapi_dataSampleSeq *
gapi_dataSampleSeq__alloc (
    void);

gapi_dataSample *
gapi_dataSampleSeq_allocbuf (
    gapi_unsigned_long len);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
