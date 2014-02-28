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

#ifndef qp_qosProvider_H_
#define qp_qosProvider_H_

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
} qp_result;

C_CLASS(qp_qosProviderInputAttr);
C_CLASS(qp_qosProvider);

/* copyOut: copy from database to language binding object */
typedef void(*qp_copyOut)(void *from, void *to);

/* qp_qosInputAttr is a descriptor for the language-specific copyOut for the
 * QoS. */
C_STRUCT(qp_qosInputAttr) {
    qp_copyOut                  copyOut;
};

/* qp_qosProviderAttr is used to provide the qp_qosProvider with the copy-
 * routines for the QoS policies. */
C_STRUCT(qp_qosProviderInputAttr) {
    C_STRUCT(qp_qosInputAttr)   participantQos;
    C_STRUCT(qp_qosInputAttr)   topicQos;
    C_STRUCT(qp_qosInputAttr)   subscriberQos;
    C_STRUCT(qp_qosInputAttr)   dataReaderQos;
    C_STRUCT(qp_qosInputAttr)   publisherQos;
    C_STRUCT(qp_qosInputAttr)   dataWriterQos;
};

OS_API qp_qosProvider
qp_qosProviderNew (
        const c_char *uri,
        const c_char *profile,
        const C_STRUCT(qp_qosProviderInputAttr) *attr);

OS_API void
qp_qosProviderFree(
        qp_qosProvider _this);

OS_API qp_result
qp_qosProviderGetParticipantQos(
        qp_qosProvider _this,
        const c_char *id,
        c_voidp qos)
    __attribute__((nonnull(1,3)));

OS_API qp_result
qp_qosProviderGetParticipantQosType(
        qp_qosProvider _this,
        c_type *type)
    __attribute__((nonnull));

OS_API qp_result
qp_qosProviderGetTopicQos(
        qp_qosProvider _this,
        const c_char *id,
        c_voidp qos)
    __attribute__((nonnull(1,3)));

OS_API qp_result
qp_qosProviderGetTopicQosType(
        qp_qosProvider _this,
        c_type *type)
    __attribute__((nonnull));

OS_API qp_result
qp_qosProviderGetPublisherQos(
        qp_qosProvider _this,
        const c_char *id,
        c_voidp qos)
    __attribute__((nonnull(1,3)));

OS_API qp_result
qp_qosProviderGetPublisherQosType(
        qp_qosProvider _this,
        c_type *type)
    __attribute__((nonnull));

OS_API qp_result
qp_qosProviderGetDataWriterQos(
        qp_qosProvider _this,
        const c_char *id,
        c_voidp qos)
    __attribute__((nonnull(1,3)));

OS_API qp_result
qp_qosProviderGetDataWriterQosType(
        qp_qosProvider _this,
        c_type *type)
    __attribute__((nonnull));

OS_API qp_result
qp_qosProviderGetSubscriberQos(
        qp_qosProvider _this,
        const c_char *id,
        c_voidp qos)
    __attribute__((nonnull(1,3)));

OS_API qp_result
qp_qosProviderGetSubscriberQosType(
        qp_qosProvider _this,
        c_type *type)
    __attribute__((nonnull));

OS_API qp_result
qp_qosProviderGetDataReaderQos(
        qp_qosProvider _this,
        const c_char *id,
        c_voidp qos)
    __attribute__((nonnull(1,3)));

OS_API qp_result
qp_qosProviderGetDataReaderQosType(
        qp_qosProvider _this,
        c_type *type)
    __attribute__((nonnull));

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* qp_qosProvider_H_ */
