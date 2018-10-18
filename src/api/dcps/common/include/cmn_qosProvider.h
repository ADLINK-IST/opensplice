/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef cmn_qosProvider_H_
#define cmn_qosProvider_H_

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_if.h"
#include "c_typebase.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* If we're not using GNU C, elide __attribute__ */
#ifndef __GNUC__
#  define  __attribute__(x)  /*NOTHING*/
#endif

typedef enum {
    QP_RESULT_OK,
    QP_RESULT_NO_DATA,
    QP_RESULT_OUT_OF_MEMORY,
    QP_RESULT_PARSE_ERROR,
    QP_RESULT_ILL_PARAM,
    QP_RESULT_UNKNOWN_ELEMENT,
    QP_RESULT_UNEXPECTED_ELEMENT,
    QP_RESULT_UNKNOWN_ARGUMENT,
    QP_RESULT_ILLEGAL_VALUE,
    QP_RESULT_NOT_IMPLEMENTED
} cmn_qpResult;

C_CLASS(cmn_qosProviderInputAttr);
C_CLASS(cmn_qosProvider);

/* copyOut: copy from database to language binding object */
typedef void(*cmn_qpCopyOut)(void *from, void *to);

/* cmn_qosInputAttr is a descriptor for the language-specific copyOut for the
 * QoS. */
C_STRUCT(cmn_qosInputAttr) {
    cmn_qpCopyOut               copyOut;
};

/* cmn_qosProviderAttr is used to provide the cmn_qosProvider with the copy-
 * routines for the QoS policies. */
C_STRUCT(cmn_qosProviderInputAttr) {
    C_STRUCT(cmn_qosInputAttr)   participantQos;
    C_STRUCT(cmn_qosInputAttr)   topicQos;
    C_STRUCT(cmn_qosInputAttr)   subscriberQos;
    C_STRUCT(cmn_qosInputAttr)   dataReaderQos;
    C_STRUCT(cmn_qosInputAttr)   publisherQos;
    C_STRUCT(cmn_qosInputAttr)   dataWriterQos;
};

OS_API cmn_qosProvider
cmn_qosProviderNew (
        const c_char *uri,
        const c_char *profile,
        const C_STRUCT(cmn_qosProviderInputAttr) *attr);

OS_API void
cmn_qosProviderFree(
        cmn_qosProvider _this);

OS_API cmn_qpResult
cmn_qosProviderGetParticipantQos(
        cmn_qosProvider _this,
        const c_char *id,
        c_voidp qos)
    __attribute__((nonnull(1,3)));

OS_API cmn_qpResult
cmn_qosProviderGetParticipantQosType(
        cmn_qosProvider _this,
        c_type *type)
    __attribute__((nonnull));

OS_API cmn_qpResult
cmn_qosProviderGetTopicQos(
        cmn_qosProvider _this,
        const c_char *id,
        c_voidp qos)
    __attribute__((nonnull(1,3)));

OS_API cmn_qpResult
cmn_qosProviderGetTopicQosType(
        cmn_qosProvider _this,
        c_type *type)
    __attribute__((nonnull));

OS_API cmn_qpResult
cmn_qosProviderGetPublisherQos(
        cmn_qosProvider _this,
        const c_char *id,
        c_voidp qos)
    __attribute__((nonnull(1,3)));

OS_API cmn_qpResult
cmn_qosProviderGetPublisherQosType(
        cmn_qosProvider _this,
        c_type *type)
    __attribute__((nonnull));

OS_API cmn_qpResult
cmn_qosProviderGetDataWriterQos(
        cmn_qosProvider _this,
        const c_char *id,
        c_voidp qos)
    __attribute__((nonnull(1,3)));

OS_API cmn_qpResult
cmn_qosProviderGetDataWriterQosType(
        cmn_qosProvider _this,
        c_type *type)
    __attribute__((nonnull));

OS_API cmn_qpResult
cmn_qosProviderGetSubscriberQos(
        cmn_qosProvider _this,
        const c_char *id,
        c_voidp qos)
    __attribute__((nonnull(1,3)));

OS_API cmn_qpResult
cmn_qosProviderGetSubscriberQosType(
        cmn_qosProvider _this,
        c_type *type)
    __attribute__((nonnull));

OS_API cmn_qpResult
cmn_qosProviderGetDataReaderQos(
        cmn_qosProvider _this,
        const c_char *id,
        c_voidp qos)
    __attribute__((nonnull(1,3)));

OS_API cmn_qpResult
cmn_qosProviderGetDataReaderQosType(
        cmn_qosProvider _this,
        c_type *type)
    __attribute__((nonnull));

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* cmn_qosProvider_H_ */
