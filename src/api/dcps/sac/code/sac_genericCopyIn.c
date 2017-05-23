/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#include "dds_dcps.h"
#include "sac_common.h"
#include "sac_genericCopyBuffer.h"
#include "sac_genericCopyIn.h"

#include "c_base.h"

#include "os_abstract.h"
#include "sac_report.h"
#include "os_stdlib.h"

#define TRACE(function) /* function */
#define STATIC static
#define DDS_MAX(a,b) ((a >= b)?a:b)

typedef struct {
    void *dst;
    c_base base;
    c_long dst_offset;
    c_long src_correction;
} DDS_ci_context;

typedef v_copyin_result (*copyInFromStruct)(DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
typedef v_copyin_result (*copyInFromUnion)(DDSCopyHeader *ch, void * src, DDS_ci_context *ctx);
typedef v_copyin_result (*copyInFromArray)(DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_ci_context *ctx);

    /* blackBox, copy as one block of data */
STATIC v_copyin_result DDS_cfsiBlackBox    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* Primitive types */
STATIC v_copyin_result DDS_cfsiBoolean    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiByte       (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiChar       (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiShort      (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiInt        (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiLong       (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiFloat      (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiDouble     (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* Array of primitive type */
STATIC v_copyin_result DDS_cfsiArrBoolean (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiArrByte    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiArrChar    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiArrShort   (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiArrInt     (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiArrLong    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiArrFloat   (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiArrDouble  (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* Sequence of primitive type */
STATIC v_copyin_result DDS_cfsiSeqBoolean (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiSeqByte    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiSeqChar    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiSeqShort   (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiSeqInt     (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiSeqLong    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiSeqFloat   (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiSeqDouble  (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* Enumeration type */
STATIC v_copyin_result DDS_cfsiEnum       (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* Structured types */
STATIC v_copyin_result DDS_cfsiStruct     (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiUnion      (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* String types */
STATIC v_copyin_result DDS_cfsiString     (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfsiBString    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* Array of object type */
STATIC v_copyin_result DDS_cfsiArray      (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* Sequence of object type */
STATIC v_copyin_result DDS_cfsiSequence   (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* reference to previous defined type */
STATIC v_copyin_result DDS_cfsiReference  (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);

    /* blackBox, copy as one block of data */
STATIC v_copyin_result DDS_cfuiBlackBox    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* Primitive types */
STATIC v_copyin_result DDS_cfuiBoolean    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiByte       (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiChar       (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiShort      (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiInt        (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiLong       (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiFloat      (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiDouble     (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* Array of primitive type */
STATIC v_copyin_result DDS_cfuiArrBoolean (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiArrByte    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiArrChar    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiArrShort   (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiArrInt     (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiArrLong    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiArrFloat   (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiArrDouble  (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* Sequence of primitive type */
STATIC v_copyin_result DDS_cfuiSeqBoolean (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiSeqByte    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiSeqChar    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiSeqShort   (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiSeqInt     (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiSeqLong    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiSeqFloat   (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiSeqDouble  (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* Enumeration type */
STATIC v_copyin_result DDS_cfuiEnum       (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* Structured types */
STATIC v_copyin_result DDS_cfuiStruct     (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiUnion      (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* String types */
STATIC v_copyin_result DDS_cfuiString     (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfuiBString    (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* Array of object type */
STATIC v_copyin_result DDS_cfuiArray      (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* Sequence of object type */
STATIC v_copyin_result DDS_cfuiSequence   (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);
    /* reference to previous defined type */
STATIC v_copyin_result DDS_cfuiReference  (DDSCopyHeader *ch, void * src,  DDS_ci_context *ctx);

    /* blackBox, copy as one block of data */
STATIC v_copyin_result DDS_cfoiBlackBox       (DDSCopyHeader *ch, void * srcBlock, void *dstBlock, DDS_ci_context *ctx);
    /* Array of primitive type */
STATIC v_copyin_result DDS_cfoiArrBoolean (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiArrByte    (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiArrChar    (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiArrShort   (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiArrInt     (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiArrLong    (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiArrFloat   (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiArrDouble  (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_ci_context *ctx);
    /* Sequence of primitive type */
STATIC v_copyin_result DDS_cfoiSeqBoolean (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiSeqByte    (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiSeqChar    (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiSeqShort   (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiSeqInt     (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiSeqLong    (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiSeqFloat   (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiSeqDouble  (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_ci_context *ctx);
    /* Enumeration type */
STATIC v_copyin_result DDS_cfoiEnum       (DDSCopyHeader *ch, void * srcEnum, void *dstEnum, DDS_ci_context *ctx);
    /* Structured types */
STATIC v_copyin_result DDS_cfoiStruct     (DDSCopyHeader *ch, void * srcStruct, void *dstStruct, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiUnion      (DDSCopyHeader *ch, void * srcUnion,  void *dstUnion,  DDS_ci_context *ctx);
    /* String types */
STATIC v_copyin_result DDS_cfoiString     (DDSCopyHeader *ch, void * srcString, void *dstString, DDS_ci_context *ctx);
STATIC v_copyin_result DDS_cfoiBString    (DDSCopyHeader *ch, void * srcString, void *dstString, DDS_ci_context *ctx);
    /* Array of object type */
STATIC v_copyin_result DDS_cfoiArray      (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_ci_context *ctx);
    /* Sequence of object type */
STATIC v_copyin_result DDS_cfoiSequence   (DDSCopyHeader *ch, void * srcSeq, void *dstSeq, DDS_ci_context *ctx);
    /* reference to previous defined type */
STATIC v_copyin_result DDS_cfoiReference  (DDSCopyHeader *ch, void * src, void *dst, DDS_ci_context *ctx);

STATIC copyInFromStruct ciFromStruct[] = {
    DDS_cfsiBlackBox,
    DDS_cfsiBoolean,
    DDS_cfsiByte,
    DDS_cfsiChar,
    DDS_cfsiShort,
    DDS_cfsiInt,
    DDS_cfsiLong,
    DDS_cfsiFloat,
    DDS_cfsiDouble,
    DDS_cfsiArrBoolean,
    DDS_cfsiArrByte,
    DDS_cfsiArrChar,
    DDS_cfsiArrShort,
    DDS_cfsiArrInt,
    DDS_cfsiArrLong,
    DDS_cfsiArrFloat,
    DDS_cfsiArrDouble,
    DDS_cfsiSeqBoolean,
    DDS_cfsiSeqByte,
    DDS_cfsiSeqChar,
    DDS_cfsiSeqShort,
    DDS_cfsiSeqInt,
    DDS_cfsiSeqLong,
    DDS_cfsiSeqFloat,
    DDS_cfsiSeqDouble,
    DDS_cfsiEnum,
    DDS_cfsiStruct,
    DDS_cfsiUnion,
    DDS_cfsiString,
    DDS_cfsiBString,
    DDS_cfsiArray,
    DDS_cfsiSequence,
    DDS_cfsiReference
    };

STATIC copyInFromUnion ciFromUnion[] = {
    DDS_cfuiBlackBox,
    DDS_cfuiBoolean,
    DDS_cfuiByte,
    DDS_cfuiChar,
    DDS_cfuiShort,
    DDS_cfuiInt,
    DDS_cfuiLong,
    DDS_cfuiFloat,
    DDS_cfuiDouble,
    DDS_cfuiArrBoolean,
    DDS_cfuiArrByte,
    DDS_cfuiArrChar,
    DDS_cfuiArrShort,
    DDS_cfuiArrInt,
    DDS_cfuiArrLong,
    DDS_cfuiArrFloat,
    DDS_cfuiArrDouble,
    DDS_cfuiSeqBoolean,
    DDS_cfuiSeqByte,
    DDS_cfuiSeqChar,
    DDS_cfuiSeqShort,
    DDS_cfuiSeqInt,
    DDS_cfuiSeqLong,
    DDS_cfuiSeqFloat,
    DDS_cfuiSeqDouble,
    DDS_cfuiEnum,
    DDS_cfuiStruct,
    DDS_cfuiUnion,
    DDS_cfuiString,
    DDS_cfuiBString,
    DDS_cfuiArray,
    DDS_cfuiSequence,
    DDS_cfuiReference
    };

STATIC copyInFromArray ciFromArray[] = {
    DDS_cfoiBlackBox,
    NULL, /* DDS_cfoiBoolean */
    NULL, /* DDS_cfoiByte */
    NULL, /* DDS_cfoiChar */
    NULL, /* DDS_cfoiShort */
    NULL, /* DDS_cfoiInt */
    NULL, /* DDS_cfoiLong */
    NULL, /* DDS_cfoiFloat */
    NULL, /* DDS_cfoiDouble */
    DDS_cfoiArrBoolean,
    DDS_cfoiArrByte,
    DDS_cfoiArrChar,
    DDS_cfoiArrShort,
    DDS_cfoiArrInt,
    DDS_cfoiArrLong,
    DDS_cfoiArrFloat,
    DDS_cfoiArrDouble,
    DDS_cfoiSeqBoolean,
    DDS_cfoiSeqByte,
    DDS_cfoiSeqChar,
    DDS_cfoiSeqShort,
    DDS_cfoiSeqInt,
    DDS_cfoiSeqLong,
    DDS_cfoiSeqFloat,
    DDS_cfoiSeqDouble,
    DDS_cfoiEnum,
    DDS_cfoiStruct,
    DDS_cfoiUnion,
    DDS_cfoiString,
    DDS_cfoiBString,
    DDS_cfoiArray,
    DDS_cfoiSequence,
    DDS_cfoiReference
    };

STATIC DDSCopyType
to_copyType(c_type t)
{
    DDSCopyType ct;
    switch(c_baseObject(t)->kind ) {
    case M_ENUMERATION:
        ct = DDSEnum;
    break;
    case M_PRIMITIVE:
        switch (c_primitive (t)->kind) {
        case P_BOOLEAN:
            ct = DDSBoolean;
        break;
        case P_CHAR:
            ct = DDSChar;
        break;
        case P_SHORT:
        case P_USHORT:
            ct = DDSShort;
        break;
        case P_LONG:
        case P_ULONG:
            ct = DDSInt;
        break;
        case P_LONGLONG:
        case P_ULONGLONG:
            ct = DDSLong;
        break;
        default:
            ct = DDSBlackBox;
            assert (0);
        }
    break;
    default:
        ct = DDSBlackBox;
        assert (0);
    }
    return ct;
}

    /* Primitive types */
STATIC v_copyin_result
DDS_cfsiBoolean (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    c_bool *dst = (c_bool *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_bool *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied in Boolean = %d @ offset = %d\n", *dst, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfuiBoolean (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    c_bool *dst = (c_bool *)ctx->dst;
    *dst = *(c_bool *)src;

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied in Boolean = %d\n", *dst));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiByte (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    c_octet *dst = (c_octet *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_octet *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied in BYTE = %hd @ offset = %d\n", *dst, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfuiByte (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    c_octet *dst = (c_octet *)ctx->dst;
    *dst = *(c_octet *)src;

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied in Byte = %hd\n", *dst));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiChar (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    c_char *dst = (c_char *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_char *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied in Char = %hd @ offset = %d\n", *dst, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfuiChar (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    c_char *dst = (c_char *)ctx->dst;
    *dst = *(c_char *)src;

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied in Char = %hd\n", *dst));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiShort (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    c_short *dst = (c_short *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_short *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied in Chort = %hd @ offset = %d\n", *dst, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfuiShort (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    c_short *dst = (c_short *)ctx->dst;
    *dst = *(c_short *)src;

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied in Short = %hd\n", *dst));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiInt (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    c_long *dst = (c_long *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_long *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied in Int = %d @ offset = %d\n", *dst, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfuiInt (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    c_long *dst = (c_long *)ctx->dst;
    *dst = *(c_long *)src;

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied in Int = %d\n", *dst));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiLong (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    TRACE (char llstr[36];)

    c_longlong *dst = (c_longlong *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_longlong *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    OS_UNUSED_ARG(ch);

    TRACE(llstr[35] = '\0'; printf ("Copied in Long = %s @ offset = %d\n", os_lltostr(*dst, &llstr[35]), ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfuiLong (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    TRACE (char llstr[36];)

    c_longlong *dst = (c_longlong *)ctx->dst;
    *dst = *(c_longlong *)src;

    OS_UNUSED_ARG(ch);

    TRACE(llstr[35] = '\0'; printf ("Copied in Long = %s\n", os_lltostr(*dst, &llstr[35])));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiFloat (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    c_float *dst = (c_float *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_float *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied in Float = %f @ offset = %d\n", *dst, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfuiFloat (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    c_float *dst = (c_float *)ctx->dst;
    *dst = *(c_float *)src;

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied in Float = %f\n", *dst));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiDouble (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    c_double *dst = (c_double *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_double *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied in Double = %f @ offset = %d\n", *dst, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfuiDouble (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    c_double *dst = (c_double *)ctx->dst;
    *dst = *(c_double *)src;

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied in Double = %f\n", *dst));
    return V_COPYIN_RESULT_OK;
}

    /* Enumeration type */
STATIC v_copyin_result
DDS_cfoiEnum (
    DDSCopyHeader *ch,
    void *srcEnum,
    void *dstEnum,
    DDS_ci_context *ctx)
{
    c_long *dst;
    DDSCopyEnum *ce;
    v_copyin_result result;

    OS_UNUSED_ARG(ctx);

    ce = (DDSCopyEnum *)ch;
    dst = (c_long *)dstEnum;

    if(((*(c_long *)srcEnum) < 0) || ((*(c_long *)srcEnum) >= (c_long)ce->nrOfElements)){
        SAC_REPORT(DDS_RETCODE_ERROR, "Invalid enumeration label.");
        result = V_COPYIN_RESULT_INVALID;
    } else {
        *dst = *(c_long *)srcEnum;
        TRACE(printf ("Copied in in Enum = %d @ offset = %d\n", *dst, ctx->dst_offset));
        result = V_COPYIN_RESULT_OK;
    }
    return result;
}

STATIC v_copyin_result
DDS_cfsiEnum (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiEnum (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                       (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiEnum (
    DDSCopyHeader *ch,
    void * src,

    DDS_ci_context *ctx)
{
    return DDS_cfoiEnum (ch, src, ctx->dst, ctx);
}

    /* BlackBox data type */
STATIC v_copyin_result
DDS_cfoiBlackBox (
    DDSCopyHeader *ch,
    void *srcBlock,
    void *dstBlock,
    DDS_ci_context *ctx)
{
    DDSCopyBlackBox *bbh;

    OS_UNUSED_ARG(ctx);

    bbh=(DDSCopyBlackBox *)ch;
    memcpy(dstBlock,srcBlock,bbh->size);
    TRACE(printf ("Copied in BlackBox size = %d @ offset = %d\n", bbh->size, ctx->dst_offset));

    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiBlackBox (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiBlackBox (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                           (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiBlackBox (
    DDSCopyHeader *ch,
    void * src,

    DDS_ci_context *ctx)
{
    return DDS_cfoiBlackBox (ch, src, ctx->dst, ctx);
}

    /* Array of primitive type */
STATIC v_copyin_result
DDS_cfoiArrBoolean (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_ci_context *ctx)
{
    DDSCopyArray *ah;
    c_bool *dst = dstArray;
    c_bool *src = srcArray;
    unsigned int i;

    OS_UNUSED_ARG(ctx);

    ah = (DDSCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(printf ("%d @ [%d];", dst[i], i));
    }
    TRACE(printf ("Copied in Boolean array size %d @ offset = %d\n",
                               ah->size, ctx->dst_offset));

    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiArrBoolean (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrBoolean (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                             (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiArrBoolean (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrBoolean (ch, src, ctx->dst, ctx);
}



STATIC v_copyin_result
DDS_cfoiArrByte (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_ci_context *ctx)
{
    DDSCopyArray *ah;
    c_octet *dst = dstArray;
    c_octet *src = srcArray;
    unsigned int i;

    OS_UNUSED_ARG(ctx);

    ah = (DDSCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(printf ("%d @ [%d];", dst[i], i));
    }
    TRACE(printf ("Copied in Byte array size %d @ offset = %d\n",
                            ah->size, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiArrByte (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrByte (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiArrByte (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrByte (ch, src, ctx->dst, ctx);
}

STATIC v_copyin_result
DDS_cfoiArrChar (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_ci_context *ctx)
{
    DDSCopyArray *ah;
    c_char *dst = dstArray;
    c_char *src = srcArray;
    unsigned int i;

    OS_UNUSED_ARG(ctx);

    ah = (DDSCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(printf ("%d @ [%d];", dst[i], i));
    }
    TRACE(printf ("Copied in Char array size %d @ offset = %d\n",
                            ah->size, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiArrChar (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrChar (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiArrChar (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrChar (ch, src, ctx->dst, ctx);
}

STATIC v_copyin_result
DDS_cfoiArrShort (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_ci_context *ctx)
{
    DDSCopyArray *ah;
    c_short *dst = dstArray;
    c_short *src = srcArray;
    unsigned int i;

    OS_UNUSED_ARG(ctx);

    ah = (DDSCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(printf ("%d @ [%d];", dst[i], i));
    }
    TRACE(printf ("Copied in Short array size %d @ offset = %d\n",
                            ah->size, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiArrShort (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrShort (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                           (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiArrShort (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrShort (ch, src, ctx->dst, ctx);
}

STATIC v_copyin_result
DDS_cfoiArrInt (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_ci_context *ctx)
{
    DDSCopyArray *ah;
    c_long *dst = dstArray;
    c_long *src = srcArray;
    unsigned int i;

    OS_UNUSED_ARG(ctx);

    ah = (DDSCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(printf ("%d @ [%d];", dst[i], i));
    }
    TRACE(printf ("Copied in Int array size %d @ offset = %d\n",
                            ah->size, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiArrInt (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrInt (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                         (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiArrInt (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrInt (ch, src, ctx->dst, ctx);
}

STATIC v_copyin_result
DDS_cfoiArrLong (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_ci_context *ctx)
{
    DDSCopyArray *ah;
    c_longlong *dst = dstArray;
    c_longlong *src = srcArray;
    unsigned int i;

    OS_UNUSED_ARG(ctx);

    TRACE (char llstr[36];)

    ah = (DDSCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(llstr[35] = '\0'; printf ("%s @ [%d];", os_lltostr(*dst, dst[i]), i));
    }
    TRACE(printf ("Copied in Long array size %d @ offset = %d\n",
                            ah->size, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiArrLong (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrLong (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiArrLong (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrLong (ch, src, ctx->dst, ctx);
}

STATIC v_copyin_result
DDS_cfoiArrFloat (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_ci_context *ctx)
{
    DDSCopyArray *ah;
    c_float *dst = dstArray;
    c_float *src = srcArray;
    unsigned int i;

    OS_UNUSED_ARG(ctx);

    ah = (DDSCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(printf ("%f @ [%d];", dst[i], i));
    }
    TRACE(printf ("Copied in Float array size %d @ offset = %d\n",
                            ah->size, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiArrFloat (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrFloat (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                           (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiArrFloat (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrFloat (ch, src, ctx->dst, ctx);
}

STATIC v_copyin_result
DDS_cfoiArrDouble (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_ci_context *ctx)
{
    DDSCopyArray *ah;
    c_double *dst = dstArray;
    c_double *src = srcArray;
    unsigned int i;

    OS_UNUSED_ARG(ctx);

    ah = (DDSCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(printf ("%f @ [%d];", dst[i], i));
    }
    TRACE(printf ("Copied in Double array size %d @ offset = %d\n",
                            ah->size, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiArrDouble (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrDouble (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                            (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiArrDouble (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArrDouble (ch, src, ctx->dst, ctx);
}

    /* Sequence of primitive type */
STATIC v_copyin_result
DDS_cfoiSeqBoolean (
    DDSCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    DDS_ci_context *ctx)
{
    DDSCopySequence *sh;
    unsigned int arrLen;
    c_bool **dst;
    DDSSequenceType * src;
    c_bool * buffer;
    unsigned int i;
    v_copyin_result result;

    sh = (DDSCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((DDS_unsigned_long)sh->size))){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Boolean Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else if(src->_maximum < src->_length){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Boolean Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else {
        arrLen = src->_length;

        *dst = (c_bool *)c_arrayNew_s (sh->type, arrLen);
        if (*dst != NULL || arrLen == 0) {
            buffer  = (c_bool *)src->_buffer;

            for (i = 0; i < arrLen; i++) {
                (*dst)[i] = buffer[i];
                TRACE(printf ("%d;", (*dst)[i]));
            }
            ctx->src_correction += DDS_SEQUENCE_CORRECTION;

            TRACE(printf ("Copied in Boolean sequence size %d @ offset = %d\n",
                    arrLen, ctx->dst_offset));
            result = V_COPYIN_RESULT_OK;
        } else {
            SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "CopyIn: Memory allocation failed (Boolean Sequence).");
            result = V_COPYIN_RESULT_OUT_OF_MEMORY;
        }
    }
    return result;
}

STATIC v_copyin_result
DDS_cfsiSeqBoolean (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqBoolean (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                             (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiSeqBoolean (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqBoolean (ch, src,  ctx->dst, ctx);
}



STATIC v_copyin_result
DDS_cfoiSeqByte (
    DDSCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    DDS_ci_context *ctx)
{
    DDSCopySequence *sh;
    unsigned int arrLen;
    c_octet **dst;
    DDSSequenceType * src;
    c_octet * buffer;
    unsigned int i;
    v_copyin_result result;

    sh = (DDSCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((DDS_unsigned_long)sh->size))){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Byte Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else if(src->_maximum < src->_length){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Byte Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else {
        arrLen = src->_length;

        *dst = (c_octet *)c_arrayNew_s (sh->type, arrLen);
        if (*dst != NULL || arrLen == 0) {
            buffer  = (c_octet *)src->_buffer;

            for (i = 0; i < arrLen; i++) {
                (*dst)[i] = buffer[i];
                TRACE(printf ("%d;", (*dst)[i]));
            }
            ctx->src_correction += DDS_SEQUENCE_CORRECTION;

            TRACE(printf ("Copied in Byte sequence size %d @ offset = %d\n",
                    arrLen, ctx->dst_offset));
            result = V_COPYIN_RESULT_OK;
        } else {
            SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "CopyIn: Memory allocation failed (Byte Sequence).");
            result = V_COPYIN_RESULT_OUT_OF_MEMORY;
        }
    }
    return result;
}

#define GET_SRC_REF(src, ctx) \
    (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction)

#define GET_DST_REF()
STATIC v_copyin_result
DDS_cfsiSeqByte (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqByte (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiSeqByte (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqByte (ch, src,  ctx->dst, ctx);
}

STATIC v_copyin_result
DDS_cfoiSeqChar (
    DDSCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    DDS_ci_context *ctx)
{
    DDSCopySequence *sh;
    unsigned int arrLen;
    c_char **dst;
    DDSSequenceType * src;
    c_char * buffer;
    unsigned int i;
    v_copyin_result result;

    sh = (DDSCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((DDS_unsigned_long)sh->size))){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Char Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else if(src->_maximum < src->_length){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Char Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else {
        arrLen = src->_length;

        *dst = (c_char *)c_arrayNew_s (sh->type, arrLen);
        if (*dst != NULL || arrLen == 0) {
            buffer  = (c_char *)src->_buffer;

            for (i = 0; i < arrLen; i++) {
                (*dst)[i] = buffer[i];
                TRACE(printf ("%d;", (*dst)[i]));
            }

            ctx->src_correction += DDS_SEQUENCE_CORRECTION;

            TRACE(printf ("Copied in Char sequence size %d @ offset = %d\n",
                    arrLen, ctx->dst_offset));
            result = V_COPYIN_RESULT_OK;
        } else {
            SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "CopyIn: Memory allocation failed (Char Sequence).");
            result = V_COPYIN_RESULT_OUT_OF_MEMORY;
        }
    }
    return result;
}

STATIC v_copyin_result
DDS_cfsiSeqChar (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqChar (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiSeqChar (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqChar (ch, src,  ctx->dst, ctx);
}

STATIC v_copyin_result
DDS_cfoiSeqShort (
    DDSCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    DDS_ci_context *ctx)
{
    DDSCopySequence *sh;
    unsigned int arrLen;
    c_short **dst;
    DDSSequenceType * src;
    c_short * buffer;
    unsigned int i;
    v_copyin_result result;

    sh = (DDSCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((DDS_unsigned_long)sh->size))){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Short Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else if(src->_maximum < src->_length){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Short Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else {
        arrLen = src->_length;

        *dst = (c_short *)c_arrayNew_s (sh->type, arrLen);
        if (*dst != NULL || arrLen == 0) {
            buffer  = (c_short *)src->_buffer;

            for (i = 0; i < arrLen; i++) {
                (*dst)[i] = buffer[i];
                TRACE(printf ("%d;", (*dst)[i]));
            }

            ctx->src_correction += DDS_SEQUENCE_CORRECTION;

            TRACE(printf ("Copied in Short sequence size %d @ offset = %d\n",
                    arrLen, ctx->dst_offset));
            result = V_COPYIN_RESULT_OK;
        } else {
            SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "CopyIn: Memory allocation failed (Short Sequence).");
            result = V_COPYIN_RESULT_OUT_OF_MEMORY;
        }
    }
    return result;
}

STATIC v_copyin_result
DDS_cfsiSeqShort (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqShort (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                           (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiSeqShort (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqShort (ch, src,  ctx->dst, ctx);
}

STATIC v_copyin_result
DDS_cfoiSeqInt (
    DDSCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    DDS_ci_context *ctx)
{
    DDSCopySequence *sh;
    unsigned int arrLen;
    c_long **dst;
    DDSSequenceType * src;
    c_long * buffer;
    unsigned int i;
    v_copyin_result result;

    sh = (DDSCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((DDS_unsigned_long)sh->size))){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Int Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else if(src->_maximum < src->_length){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Int Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else {
        arrLen = src->_length;

        *dst = (c_long *)c_arrayNew_s (sh->type, arrLen);
        if (*dst != NULL || arrLen == 0) {
            buffer  = (c_long *)src->_buffer;
            arrLen = src->_length;

            for (i = 0; i < arrLen; i++) {
                (*dst)[i] = buffer[i];
                TRACE(printf ("%d;", (*dst)[i]));
            }

            ctx->src_correction += DDS_SEQUENCE_CORRECTION;

            TRACE(printf ("Copied in Int sequence size %d @ offset = %d\n",
                    arrLen, ctx->dst_offset));
            result = V_COPYIN_RESULT_OK;
        } else {
            SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "CopyIn: Memory allocation failed (Int Sequence).");
            result = V_COPYIN_RESULT_OUT_OF_MEMORY;
        }
    }
    return result;
}

STATIC v_copyin_result
DDS_cfsiSeqInt (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqInt (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                         (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiSeqInt (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqInt (ch, src,  ctx->dst, ctx);
}

STATIC v_copyin_result
DDS_cfoiSeqLong (
    DDSCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    DDS_ci_context *ctx)
{
    DDSCopySequence *sh;
    unsigned int arrLen;
    c_longlong **dst;
    DDSSequenceType * src;
    c_longlong * buffer;
    unsigned int i;
    v_copyin_result result;

    sh = (DDSCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((DDS_unsigned_long)sh->size))){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Long Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else if(src->_maximum < src->_length){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Long Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else {
        arrLen = src->_length;

        *dst = (c_longlong *)c_arrayNew_s (sh->type, arrLen);
        if (*dst != NULL || arrLen == 0) {
            buffer  = (c_longlong *)src->_buffer;

            for (i = 0; i < arrLen; i++) {
                (*dst)[i] = buffer[i];
                TRACE(printf ("%d;", (*dst)[i]));
            }

            ctx->src_correction += DDS_SEQUENCE_CORRECTION;

            TRACE(printf ("Copied in Long sequence size %d @ offset = %d\n",
                    arrLen, ctx->dst_offset));
            result = V_COPYIN_RESULT_OK;
        } else {
            SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "CopyIn: Memory allocation failed (Long Sequence).");
            result = V_COPYIN_RESULT_OUT_OF_MEMORY;
        }
    }
    return result;
}

STATIC v_copyin_result
DDS_cfsiSeqLong (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqLong (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiSeqLong (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqLong (ch, src,  ctx->dst, ctx);
}

STATIC v_copyin_result
DDS_cfoiSeqFloat (
    DDSCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    DDS_ci_context *ctx)
{
    DDSCopySequence *sh;
    unsigned int arrLen;
    c_float **dst;
    DDSSequenceType * src;
    c_float * buffer;
    unsigned int i;
    v_copyin_result result;

    sh = (DDSCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((DDS_unsigned_long)sh->size))){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Float Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else if(src->_maximum < src->_length){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Float Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else {
        arrLen = src->_length;

        *dst = (c_float *)c_arrayNew_s (sh->type, arrLen);
        if (*dst != NULL || arrLen == 0) {
            buffer  = (c_float *)src->_buffer;

            for (i = 0; i < arrLen; i++) {
                (*dst)[i] = buffer[i];
                TRACE(printf ("%d;", (*dst)[i]));
            }

            ctx->src_correction += DDS_SEQUENCE_CORRECTION;

            TRACE(printf ("Copied in Float sequence size %d @ offset = %d\n",
                    arrLen, ctx->dst_offset));
            result = V_COPYIN_RESULT_OK;
        } else {
            SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "CopyIn: Memory allocation failed (Float Sequence).");
            result = V_COPYIN_RESULT_OUT_OF_MEMORY;
        }
    }
    return result;
}

STATIC v_copyin_result
DDS_cfsiSeqFloat (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqFloat (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                           (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiSeqFloat (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqFloat (ch, src,  ctx->dst, ctx);
}

STATIC v_copyin_result
DDS_cfoiSeqDouble (
    DDSCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    DDS_ci_context *ctx)
{
    DDSCopySequence *sh;
    unsigned int arrLen;
    c_double **dst;
    DDSSequenceType * src;
    c_double * buffer;
    unsigned int i;
    v_copyin_result result;

    sh = (DDSCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((DDS_unsigned_long)sh->size))){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Double Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else if(src->_maximum < src->_length){
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (Double Sequence).");
        result = V_COPYIN_RESULT_INVALID;
    } else {
        arrLen = src->_length;

        *dst = (c_double *)c_arrayNew_s (sh->type, arrLen);
        if (*dst != NULL || arrLen == 0) {
            buffer  = (c_double *)src->_buffer;

            for (i = 0; i < arrLen; i++) {
                (*dst)[i] = buffer[i];
                TRACE(printf ("%d;", (*dst)[i]));
            }

            ctx->src_correction += DDS_SEQUENCE_CORRECTION;

            TRACE(printf ("Copied in Double sequence size %d @ offset = %d\n",
                    arrLen, ctx->dst_offset));
            result = V_COPYIN_RESULT_OK;
        } else {
            SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "CopyIn: Memory allocation failed (Array Sequence).");
            result = V_COPYIN_RESULT_OUT_OF_MEMORY;
        }
    }
    return result;
}

STATIC v_copyin_result
DDS_cfsiSeqDouble (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqDouble (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                            (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiSeqDouble (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSeqDouble (ch, src,  ctx->dst, ctx);
}

    /* Structured types */
STATIC v_copyin_result
DDS_cfoiStruct (
    DDSCopyHeader *ch,
    void *srcStruct,
    void *dstStruct,
    DDS_ci_context *ctx)
{
    DDS_ci_context context;
    unsigned long mi;
    DDSCopyStruct *csh;
    DDSCopyStructMember *csm;

    context.dst = dstStruct;
    context.base = ctx->base;
    context.src_correction = 0;
    csh = (DDSCopyStruct *)ch;

    csm = DDSCopyStructMemberObject (csh);

    for (mi = 0; mi < csh->nrOfMembers; mi++) {
        context.dst_offset = csm->memberOffset;
        ch = DDSCopyStructMemberDescription (csm);
        ciFromStruct[ch->copyType] (ch, srcStruct, &context);
        csm = (DDSCopyStructMember *)DDSCopyHeaderNextObject (ch);
    }
    ctx->src_correction += csh->userSize - csh->size;
    TRACE(printf ("Copied in Struct @ offset = %d\n", ctx->dst_offset));
    return V_COPYIN_RESULT_OK;

}

STATIC v_copyin_result
DDS_cfsiStruct (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiStruct (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                         (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiStruct (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiStruct (ch, src,  ctx->dst, ctx);
}


STATIC v_copyin_result
DDS_cfoiUnion (
    DDSCopyHeader *ch,
    void *srcUnion,
    void *dstUnion,
    DDS_ci_context *ctx)
{
    DDS_ci_context context;
    unsigned long ci;
    DDSCopyUnion *cuh;
    DDSCopyHeader *cdh;
    DDSCopyUnionLabels *csl;
    unsigned long long discrVal;
    DDSCopyUnionLabel *defaultLabel = NULL;
    int active_case = 0;
    void * src;

    cuh = (DDSCopyUnion *)ch;

    context.dst = dstUnion;
    cdh = DDSCopyUnionDiscriminantObject (cuh);
    ciFromUnion[cdh->copyType] (cdh, srcUnion, &context);

    discrVal = DDS_getUnionDescriptor(to_copyType(cuh->discrType), srcUnion);

    src = (void *)((PA_ADDRCAST)srcUnion + cuh->casesOffset);
    context.dst = (void *)((PA_ADDRCAST)dstUnion + cuh->casesOffset);
    context.base = ctx->base;
    context.dst_offset = 0;
    context.src_correction = 0;
    csl = DDSCopyUnionLabelsObject (cdh);
    ci = 0;
    while (ci < cuh->nrOfCases) {
        unsigned int label;
        DDSCopyUnionLabel *lab;

        lab = DDSCopyUnionLabelObject (csl);
        if (csl->labelCount) {
            for (label = 0; label < csl->labelCount; label++) {
                if (lab->labelVal == discrVal) {
                    active_case = 1;
                }
                lab++;
            }
        } else {
            defaultLabel = (DDSCopyUnionLabel *)DDSCopyUnionCaseObject(csl);
        }
        ch = DDSCopyUnionCaseObject(csl);
        if (active_case) {
            ciFromUnion[ch->copyType] (ch, src, &context);
            ci = cuh->nrOfCases;
        } else {
            ci++;
        }
        csl = (DDSCopyUnionLabels *)DDSCopyHeaderNextObject (ch);
    }
    if (!active_case && defaultLabel) {
        ch = (DDSCopyHeader *)defaultLabel;
        ciFromUnion[ch->copyType] (ch, src, &context);
    }
    ctx->src_correction += cuh->userSize - cuh->size;
    TRACE(printf ("Copied in Union\n"));

    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiUnion (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiUnion (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                        (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiUnion (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiUnion (ch, src,  ctx->dst, ctx);
}

    /* String types */
STATIC v_copyin_result
DDS_cfoiString (
    DDSCopyHeader *ch,
    void *srcString,
    void *dstString,
    DDS_ci_context *ctx)
{
    c_string *dst;
    v_copyin_result result = V_COPYIN_RESULT_OK;

    OS_UNUSED_ARG(ch);

    if((*(c_char **)srcString) != NULL){
        dst = (c_string *)(dstString);
        *dst = c_stringNew_s (ctx->base, *(c_char **)srcString);
        TRACE(printf ("Copied in string = %s @ offset = %d\n",
                 (*dst ? *dst : "NULL"), ctx->dst_offset));
        if (*dst == NULL) {
            SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "CopyIn: Memory allocation for string failed.");
            result = V_COPYIN_RESULT_OUT_OF_MEMORY;
        }
    } else {
        dst = (c_string *)(dstString);
        *dst = c_stringNew_s (ctx->base, "");
        if (*dst == NULL) {
            SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "CopyIn: Memory allocation for string failed.");
            result = V_COPYIN_RESULT_OUT_OF_MEMORY;
        }
    }
    return result;
}

STATIC v_copyin_result
DDS_cfsiString (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiString (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                         (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiString (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiString (ch, src, ctx->dst, ctx);
}

STATIC v_copyin_result
DDS_cfoiBString (
    DDSCopyHeader *ch,
    void *srcString,
    void *dstString,
    DDS_ci_context *ctx)
{
    c_string *dst;
    DDSCopyBoundedString* cb;
    unsigned int length;
    v_copyin_result result = V_COPYIN_RESULT_OK;

    if((*(c_char **)srcString) != NULL){
        cb = (DDSCopyBoundedString*)ch;
        length = (unsigned int)(strlen(*(c_char **)srcString));

        if(length <= cb->max){
            dst = (c_string *)(dstString);
            *dst = c_stringNew_s (ctx->base, *(c_char **)srcString);
            TRACE(printf ("Copied in string = %s @ offset = %d\n",
                    (*dst ? *dst : "NULL"), ctx->dst_offset));
            if (*dst == NULL) {
                SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "CopyIn: Memory allocation for string failed.");
                result = V_COPYIN_RESULT_OUT_OF_MEMORY;
            }
        } else {
            SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "CopyIn: Array bounds write (bounded string).");
            result = V_COPYIN_RESULT_INVALID;
        }
    } else {
        dst = (c_string *)(dstString);
        *dst = c_stringNew_s (ctx->base, "");
        if (*dst == NULL) {
            SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "CopyIn: Memory allocation for string failed.");
            result = V_COPYIN_RESULT_OUT_OF_MEMORY;
        }
    }
    return result;
}

STATIC v_copyin_result
DDS_cfsiBString (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiBString (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiBString (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiBString (ch, src, ctx->dst, ctx);
}

    /* Array of object type */
STATIC v_copyin_result
DDS_cfoiArray (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_ci_context *ctx)
{
    DDSCopyObjectArray *ah;
    DDSCopyHeader *aech;
    void *dst;
    void *src;
    unsigned int i;
    c_long old_src_correction;

    src = srcArray;
    dst = dstArray;
    ah = (DDSCopyObjectArray *)ch;
    aech = DDSCopyObjectArrayDescription (ah);
    old_src_correction = ctx->src_correction;
    for (i = 0; i < ah->arraySize; i++) {
        ciFromArray[aech->copyType] (aech, src, dst, ctx);
        dst = (void *)((PA_ADDRCAST)dst + ah->typeSize);
        src = (void *)((PA_ADDRCAST)src + ah->typeSize + ctx->src_correction - old_src_correction);
        old_src_correction = ctx->src_correction;
    }
    TRACE(printf ("Copied in Object array size %d @ offset = %d\n",
                                ah->typeSize, ctx->dst_offset));
    return V_COPYIN_RESULT_OK;
}

STATIC v_copyin_result
DDS_cfsiArray (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArray(ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                       (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiArray (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiArray (ch, src, ctx->dst, ctx);
}

    /* Sequence of object type */
STATIC v_copyin_result
DDS_cfoiSequence (
    DDSCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    DDS_ci_context *ctx)
{
    DDSCopyObjectSequence *sh;
    DDSCopyHeader *sech;
    void *dst;
    c_long i;
    c_long seqLen;
    DDSSequenceType *src;
    void *buffer;
    c_long src_correction;
    v_copyin_result result = V_COPYIN_RESULT_OK;

    sh = (DDSCopyObjectSequence *)ch;
    sech = DDSCopyObjectSequenceDescription (sh);
    src = srcSeq;

    seqLen = src->_length;
    *(c_array *)dstSeq = c_arrayNew_s (sh->type, seqLen);

    src_correction = ctx->src_correction;

    dst = (void *)*(c_array *)dstSeq;
    if (dst != NULL || seqLen == 0) {
        buffer = src->_buffer;

        for (i = 0; i < seqLen && V_COPYIN_RESULT_IS_OK(result); i++) {
            ctx->src_correction = 0;
            result = ciFromArray[sech->copyType] (sech, buffer, dst, ctx);
            if (V_COPYIN_RESULT_IS_OK(result)) {
                dst = (void *)((PA_ADDRCAST)dst + sh->baseTypeSize);
                buffer = (void *)((PA_ADDRCAST)buffer + sh->userTypeSize);
            } else {
                SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "CopyIn: Memory allocation failed (Object Sequence).");
            }
        }

        ctx->src_correction = src_correction + DDS_SEQUENCE_CORRECTION;

        TRACE(printf ("Copied in Object sequence size %d @ offset = %d\n",
                sh->baseTypeSize, ctx->dst_offset));
    } else {
        SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "CopyIn: Memory allocation failed (Object Sequence).");
        result = V_COPYIN_RESULT_OUT_OF_MEMORY;
    }

    return result;
}

STATIC v_copyin_result
DDS_cfsiSequence (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    return DDS_cfoiSequence(ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC v_copyin_result
DDS_cfuiSequence (
    DDSCopyHeader *ch,
    void * src,

    DDS_ci_context *ctx)
{
    return DDS_cfoiSequence (ch, src, ctx->dst, ctx);
}

    /* backward referenced type */
STATIC v_copyin_result
DDS_cfoiReference (
    DDSCopyHeader *ch,
    void * src,
    void *dst,
    DDS_ci_context *ctx)
{
    DDSCopyReference *ref;
    DDSCopyHeader *nch;

    ref = (DDSCopyReference *)ch;
    nch = DDSCopyReferencedObject (ref);
    return ciFromArray[nch->copyType] (nch, src, dst, ctx);
}

STATIC v_copyin_result
DDS_cfsiReference (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    DDSCopyReference *ref;
    DDSCopyHeader *nch;

    ref = (DDSCopyReference *)ch;
    nch = DDSCopyReferencedObject (ref);
    return ciFromStruct[nch->copyType] (nch, src, ctx);
}

STATIC v_copyin_result
DDS_cfuiReference (
    DDSCopyHeader *ch,
    void * src,
    DDS_ci_context *ctx)
{
    DDSCopyReference *ref;
    DDSCopyHeader *nch;

    ref = (DDSCopyReference *)ch;
    nch = DDSCopyReferencedObject (ref);
    return ciFromUnion[nch->copyType] (nch, src, ctx);
}

v_copyin_result
DDS_copyInStruct (
    c_base base,
    void *src,
    void *dst)
{
    DDS_ci_context context;
    DDS_srcInfo srcInfo = (DDS_srcInfo)src;
    DDSCopyHeader *ch;

    ch = DDS_copyCacheCache(srcInfo->copyProgram);
    context.dst = dst;
    context.dst_offset = 0;
    context.src_correction = 0;
    context.base = base;
    return ciFromArray[ch->copyType] (ch, srcInfo->src, dst, &context);
}
