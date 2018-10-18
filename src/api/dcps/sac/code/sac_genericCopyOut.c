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

#include "dds_dcps.h"
#include "sac_common.h"
#include "sac_objManag.h"
#include "sac_genericCopyBuffer.h"
#include "sac_genericCopyOut.h"

#include "c_base.h"

#include "os_abstract.h"
#include "sac_report.h"
#include "os_stdlib.h"

#define TRACE(function) /* function */
#define STATIC static

#define SEQ_ALLOC_PRIM_BUFFER(seq,len,size) \
    if (seq->_maximum > 0UL) {\
        assert(seq->_buffer);\
        if (len > seq->_maximum) {\
            if (seq->_release) {\
                DDS_free(seq->_buffer);\
            }\
            seq->_maximum = 0UL;\
            seq->_length  = 0UL;\
            seq->_buffer  = NULL;\
        }\
    }\
    \
    if ((len > 0UL) && (seq->_maximum == 0UL)) {\
        seq->_buffer = DDS_sequence_allocbuf(NULL, size, len);\
        if ( seq->_buffer ) {\
            seq->_maximum = len;\
            seq->_release = TRUE;\
        }\
    } else {\
        seq->_length = 0UL;\
    }

#define SEQ_ALLOC_OBJECT_BUFFER(seq,len,size,cc,ch) \
    if (seq->_maximum > 0UL) {\
        assert(seq->_buffer);\
        if (len != seq->_maximum) {\
            if (seq->_release) {\
                DDS_free(seq->_buffer);\
            }\
            seq->_maximum = 0UL;\
            seq->_length  = 0UL;\
            seq->_buffer  = NULL;\
        }\
    }\
    \
    if ((len > 0UL) && (seq->_maximum == 0UL)) {\
        seq->_buffer = DDS_genericCopyBufferAllocSeqBuffer(cc, ch, size, len);\
        if ( seq->_buffer ) {\
            seq->_maximum = len;\
            seq->_release = TRUE;\
        }\
    }



typedef struct {
    DDS_copyCache copyCache;
    void *dst;
    c_long src_offset;
    c_long dst_correction;
} DDS_co_context;

typedef void (*copyInFromStruct)(DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
typedef void (*copyInFromUnion)(DDSCopyHeader *ch, void * src, DDS_co_context *ctx);
typedef void (*copyInFromArray)(DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_co_context *ctx);

    /* blackBox, copy as one block of data */
STATIC void DDS_cfsoBlackBox    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* Primitive types */
STATIC void DDS_cfsoBoolean    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoByte       (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoChar       (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoShort      (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoInt        (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoLong       (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoFloat      (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoDouble     (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* Array of primitive type */
STATIC void DDS_cfsoArrBoolean (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoArrByte    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoArrChar    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoArrShort   (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoArrInt     (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoArrLong    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoArrFloat   (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoArrDouble  (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* Sequence of primitive type */
STATIC void DDS_cfsoSeqBoolean (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoSeqByte    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoSeqChar    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoSeqShort   (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoSeqInt     (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoSeqLong    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoSeqFloat   (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoSeqDouble  (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* Enumeration type */
STATIC void DDS_cfsoEnum       (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* Structured types */
STATIC void DDS_cfsoStruct     (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoUnion      (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* String types */
STATIC void DDS_cfsoString     (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfsoBString    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* Array of object type */
STATIC void DDS_cfsoArray      (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* Sequence of object type */
STATIC void DDS_cfsoSequence   (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* reference to previous defined type */
STATIC void DDS_cfsoReference  (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);

    /* blackBox, copy as one block of data */
STATIC void DDS_cfuoBlackBox    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* Primitive types */
STATIC void DDS_cfuoBoolean    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoByte       (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoChar       (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoShort      (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoInt        (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoLong       (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoFloat      (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoDouble     (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* Array of primitive type */
STATIC void DDS_cfuoArrBoolean (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoArrByte    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoArrChar    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoArrShort   (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoArrInt     (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoArrLong    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoArrFloat   (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoArrDouble  (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* Sequence of primitive type */
STATIC void DDS_cfuoSeqBoolean (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoSeqByte    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoSeqChar    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoSeqShort   (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoSeqInt     (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoSeqLong    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoSeqFloat   (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoSeqDouble  (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* Enumeration type */
STATIC void DDS_cfuoEnum       (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* Structured types */
STATIC void DDS_cfuoStruct     (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoUnion      (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* String types */
STATIC void DDS_cfuoString     (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
STATIC void DDS_cfuoBString    (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* Array of object type */
STATIC void DDS_cfuoArray      (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* Sequence of object type */
STATIC void DDS_cfuoSequence   (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);
    /* reference to previous defined type */
STATIC void DDS_cfuoReference  (DDSCopyHeader *ch, void * src,  DDS_co_context *ctx);

    /* blackBox, copy as one block of data */
STATIC void DDS_cfooBlackBox       (DDSCopyHeader *ch, void * srcBlock, void *dstBlock, DDS_co_context *ctx);
    /* Array of primitive type */
STATIC void DDS_cfooArrBoolean (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_co_context *ctx);
STATIC void DDS_cfooArrByte    (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_co_context *ctx);
STATIC void DDS_cfooArrChar    (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_co_context *ctx);
STATIC void DDS_cfooArrShort   (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_co_context *ctx);
STATIC void DDS_cfooArrInt     (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_co_context *ctx);
STATIC void DDS_cfooArrLong    (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_co_context *ctx);
STATIC void DDS_cfooArrFloat   (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_co_context *ctx);
STATIC void DDS_cfooArrDouble  (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_co_context *ctx);
    /* Sequence of primitive type */
STATIC void DDS_cfooSeqBoolean (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_co_context *ctx);
STATIC void DDS_cfooSeqByte    (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_co_context *ctx);
STATIC void DDS_cfooSeqChar    (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_co_context *ctx);
STATIC void DDS_cfooSeqShort   (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_co_context *ctx);
STATIC void DDS_cfooSeqInt     (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_co_context *ctx);
STATIC void DDS_cfooSeqLong    (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_co_context *ctx);
STATIC void DDS_cfooSeqFloat   (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_co_context *ctx);
STATIC void DDS_cfooSeqDouble  (DDSCopyHeader *ch, void * srcArray, void *dstSeq, DDS_co_context *ctx);
    /* Enumeration type */
STATIC void DDS_cfooEnum       (DDSCopyHeader *ch, void * srcEnum, void *dstEnum, DDS_co_context *ctx);
    /* Structured types */
STATIC void DDS_cfooStruct     (DDSCopyHeader *ch, void * srcStruct, void *dstStruct, DDS_co_context *ctx);
STATIC void DDS_cfooUnion      (DDSCopyHeader *ch, void * srcUnion,  void *dstUnion,  DDS_co_context *ctx);
    /* String types */
STATIC void DDS_cfooString     (DDSCopyHeader *ch, void * srcString, void *dstString, DDS_co_context *ctx);
STATIC void DDS_cfooBString    (DDSCopyHeader *ch, void * srcString, void *dstString, DDS_co_context *ctx);
    /* Array of object type */
STATIC void DDS_cfooArray      (DDSCopyHeader *ch, void * srcArray, void *dstArray, DDS_co_context *ctx);
    /* Sequence of object type */
STATIC void DDS_cfooSequence   (DDSCopyHeader *ch, void * srcSeq, void *dstSeq, DDS_co_context *ctx);
    /* reference to previous defined type */
STATIC void DDS_cfooReference  (DDSCopyHeader *ch, void * src, void *dst, DDS_co_context *ctx);

STATIC copyInFromStruct coFromStruct[] = {
    DDS_cfsoBlackBox,
    DDS_cfsoBoolean,
    DDS_cfsoByte,
    DDS_cfsoChar,
    DDS_cfsoShort,
    DDS_cfsoInt,
    DDS_cfsoLong,
    DDS_cfsoFloat,
    DDS_cfsoDouble,
    DDS_cfsoArrBoolean,
    DDS_cfsoArrByte,
    DDS_cfsoArrChar,
    DDS_cfsoArrShort,
    DDS_cfsoArrInt,
    DDS_cfsoArrLong,
    DDS_cfsoArrFloat,
    DDS_cfsoArrDouble,
    DDS_cfsoSeqBoolean,
    DDS_cfsoSeqByte,
    DDS_cfsoSeqChar,
    DDS_cfsoSeqShort,
    DDS_cfsoSeqInt,
    DDS_cfsoSeqLong,
    DDS_cfsoSeqFloat,
    DDS_cfsoSeqDouble,
    DDS_cfsoEnum,
    DDS_cfsoStruct,
    DDS_cfsoUnion,
    DDS_cfsoString,
    DDS_cfsoBString,
    DDS_cfsoArray,
    DDS_cfsoSequence,
    DDS_cfsoReference
    };

STATIC copyInFromUnion coFromUnion[] = {
    DDS_cfuoBlackBox,
    DDS_cfuoBoolean,
    DDS_cfuoByte,
    DDS_cfuoChar,
    DDS_cfuoShort,
    DDS_cfuoInt,
    DDS_cfuoLong,
    DDS_cfuoFloat,
    DDS_cfuoDouble,
    DDS_cfuoArrBoolean,
    DDS_cfuoArrByte,
    DDS_cfuoArrChar,
    DDS_cfuoArrShort,
    DDS_cfuoArrInt,
    DDS_cfuoArrLong,
    DDS_cfuoArrFloat,
    DDS_cfuoArrDouble,
    DDS_cfuoSeqBoolean,
    DDS_cfuoSeqByte,
    DDS_cfuoSeqChar,
    DDS_cfuoSeqShort,
    DDS_cfuoSeqInt,
    DDS_cfuoSeqLong,
    DDS_cfuoSeqFloat,
    DDS_cfuoSeqDouble,
    DDS_cfuoEnum,
    DDS_cfuoStruct,
    DDS_cfuoUnion,
    DDS_cfuoString,
    DDS_cfuoBString,
    DDS_cfuoArray,
    DDS_cfuoSequence,
    DDS_cfuoReference
    };

STATIC copyInFromArray coFromArray[] = {
    DDS_cfooBlackBox,
    NULL, /* DDS_cfooBoolean */
    NULL, /* DDS_cfooByte */
    NULL, /* DDS_cfooChar */
    NULL, /* DDS_cfooShort */
    NULL, /* DDS_cfooInt */
    NULL, /* DDS_cfooLong */
    NULL, /* DDS_cfooFloat */
    NULL, /* DDS_cfooDouble */
    DDS_cfooArrBoolean,
    DDS_cfooArrByte,
    DDS_cfooArrChar,
    DDS_cfooArrShort,
    DDS_cfooArrInt,
    DDS_cfooArrLong,
    DDS_cfooArrFloat,
    DDS_cfooArrDouble,
    DDS_cfooSeqBoolean,
    DDS_cfooSeqByte,
    DDS_cfooSeqChar,
    DDS_cfooSeqShort,
    DDS_cfooSeqInt,
    DDS_cfooSeqLong,
    DDS_cfooSeqFloat,
    DDS_cfooSeqDouble,
    DDS_cfooEnum,
    DDS_cfooStruct,
    DDS_cfooUnion,
    DDS_cfooString,
    DDS_cfooBString,
    DDS_cfooArray,
    DDS_cfooSequence,
    DDS_cfooReference
    };

STATIC  DDSCopyType
to_copyType(c_type t)
{
    DDSCopyType ct;
    switch(c_baseObject(t)->kind ) {
    case M_ENUMERATION:
        ct = DDSEnum;
    break;
    case M_PRIMITIVE:
        switch (c_primitive(t)->kind) {
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
            SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Illegal primitive type (%d) detected.",
                        c_primitive(t)->kind);
            ct = DDSBlackBox;
            assert (0);
        }
    break;
    default:
        SAC_REPORT(DDS_RETCODE_BAD_PARAMETER, "Illegal type (%d) detected.",
                    c_baseObject(t)->kind);
        ct = DDSBlackBox;
        assert (0);
    }
    return ct;
}

    /* Primitive types */
STATIC void
DDS_cfsoBoolean (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    c_bool *dst = (c_bool *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_bool *)((PA_ADDRCAST)src + ctx->src_offset);

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied out Boolean = %d @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
DDS_cfuoBoolean (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    c_bool *dst = (c_bool *)ctx->dst;
    *dst = *(c_bool *)src;

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied out Boolean = %d\n", *dst));
}

STATIC void
DDS_cfsoByte (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    c_octet *dst = (c_octet *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_octet *)((PA_ADDRCAST)src + ctx->src_offset);

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied out BYTE = %hd @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
DDS_cfuoByte (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    c_octet *dst = (c_octet *)ctx->dst;
    *dst = *(c_octet *)src;

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied out Byte = %hd\n", *dst));
}

STATIC void
DDS_cfsoChar (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    c_char *dst = (c_char *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_char *)((PA_ADDRCAST)src + ctx->src_offset);

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied out Char = %hd @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
DDS_cfuoChar (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    c_char *dst = (c_char *)ctx->dst;
    *dst = *(c_char *)src;

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied out Char = %hd\n", *dst));
}

STATIC void
DDS_cfsoShort (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    c_short *dst = (c_short *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_short *)((PA_ADDRCAST)src + ctx->src_offset);

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied out Chort = %hd @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
DDS_cfuoShort (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    c_short *dst = (c_short *)ctx->dst;
    *dst = *(c_short *)src;

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied out Short = %hd\n", *dst));
}

STATIC void
DDS_cfsoInt (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    c_long *dst = (c_long *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_long *)((PA_ADDRCAST)src + ctx->src_offset);

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied out Int = %d @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
DDS_cfuoInt (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    c_long *dst = (c_long *)ctx->dst;
    *dst = *(c_long *)src;

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied out Int = %d\n", *dst));
}

STATIC void
DDS_cfsoLong (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    TRACE (char llstr[36];)

    c_longlong *dst = (c_longlong *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_longlong *)((PA_ADDRCAST)src + ctx->src_offset);

    OS_UNUSED_ARG(ch);

    TRACE(llstr[35] = '\0'; printf ("Copied out Long = %s @ offset = %d\n", os_lltostr(*dst, &llstr[35]), ctx->src_offset));
}

STATIC void
DDS_cfuoLong (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    TRACE (char llstr[36];)

    c_longlong *dst = (c_longlong *)ctx->dst;
    *dst = *(c_longlong *)src;

    OS_UNUSED_ARG(ch);

    TRACE(llstr[35] = '\0'; printf ("Copied out Long = %s\n", os_lltostr(*dst, &llstr[35])));
}

STATIC void
DDS_cfsoFloat (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    c_float *dst = (c_float *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_float *)((PA_ADDRCAST)src + ctx->src_offset);

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied out Float = %f @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
DDS_cfuoFloat (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    c_float *dst = (c_float *)ctx->dst;
    *dst = *(c_float *)src;

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied out Float = %f\n", *dst));
}

STATIC void
DDS_cfsoDouble (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    c_double *dst = (c_double *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_double *)((PA_ADDRCAST)src + ctx->src_offset);

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied out Double = %f @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
DDS_cfuoDouble (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    c_double *dst = (c_double *)ctx->dst;
    *dst = *(c_double *)src;

    OS_UNUSED_ARG(ch);

    TRACE(printf ("Copied out Double = %f\n", *dst));
}

    /* Enumeration type */
STATIC void
DDS_cfooEnum (
    DDSCopyHeader *ch,
    void *srcEnum,
    void *dstEnum,
    DDS_co_context *ctx)
{
    c_long *dst;

    OS_UNUSED_ARG(ch);
    OS_UNUSED_ARG(ctx);

    dst = (c_long *)dstEnum;
    *dst = *(c_long *)srcEnum;
    TRACE(printf ("Copied out in Enum = %d @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
DDS_cfsoEnum (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooEnum (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                       (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoEnum (
    DDSCopyHeader *ch,
    void * src,

    DDS_co_context *ctx)
{
    DDS_cfooEnum (ch, src, ctx->dst, ctx);
}

    /* BlackBox data type */
STATIC void
DDS_cfooBlackBox (
    DDSCopyHeader *ch,
    void *srcBlock,
    void *dstBlock,
    DDS_co_context *ctx)
{
    DDSCopyBlackBox *bbh;

    OS_UNUSED_ARG(ctx);

    bbh=(DDSCopyBlackBox *)ch;
    memcpy(dstBlock,srcBlock,bbh->size);
    TRACE(printf ("Copied out BlackBox size = %d @ offset = %d\n", bbh->size, ctx->src_offset));
}

STATIC void
DDS_cfsoBlackBox (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooBlackBox (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                           (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoBlackBox (
    DDSCopyHeader *ch,
    void * src,

    DDS_co_context *ctx)
{
    DDS_cfooBlackBox (ch, src, ctx->dst, ctx);
}

    /* Array of primitive type */
STATIC void
DDS_cfooArrBoolean (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_co_context *ctx)
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
    TRACE(printf ("Copied out Boolean array size %d @ offset = %d\n",
                               ah->size, ctx->src_offset));
}

STATIC void
DDS_cfsoArrBoolean (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrBoolean (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                             (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoArrBoolean (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrBoolean (ch, src, ctx->dst, ctx);
}



STATIC void
DDS_cfooArrByte (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_co_context *ctx)
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
    TRACE(printf ("Copied out Byte array size %d @ offset = %d\n",
                            ah->size, ctx->src_offset));
}

STATIC void
DDS_cfsoArrByte (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrByte (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoArrByte (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrByte (ch, src, ctx->dst, ctx);
}

STATIC void
DDS_cfooArrChar (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_co_context *ctx)
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
    TRACE(printf ("Copied out Char array size %d @ offset = %d\n",
                            ah->size, ctx->src_offset));
}

STATIC void
DDS_cfsoArrChar (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrChar (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoArrChar (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrChar (ch, src, ctx->dst, ctx);
}

STATIC void
DDS_cfooArrShort (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_co_context *ctx)
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
    TRACE(printf ("Copied out Short array size %d @ offset = %d\n",
                            ah->size, ctx->src_offset));
}

STATIC void
DDS_cfsoArrShort (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrShort (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                           (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoArrShort (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrShort (ch, src, ctx->dst, ctx);
}

STATIC void
DDS_cfooArrInt (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_co_context *ctx)
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
    TRACE(printf ("Copied out Int array size %d @ offset = %d\n",
                            ah->size, ctx->src_offset));

}

STATIC void
DDS_cfsoArrInt (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrInt (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                         (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoArrInt (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrInt (ch, src, ctx->dst, ctx);
}

STATIC void
DDS_cfooArrLong (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_co_context *ctx)
{
    DDSCopyArray *ah;
    c_longlong *dst = dstArray;
    c_longlong *src = srcArray;
    unsigned int i;

    TRACE (char llstr[36];)

    OS_UNUSED_ARG(ctx);

    ah = (DDSCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(llstr[35] = '\0'; printf ("%s @ [%d];", os_lltostr(dst[i], &llstr[35]), i));
    }
    TRACE(printf ("Copied out Long array size %d @ offset = %d\n",
                            ah->size, ctx->src_offset));
}

STATIC void
DDS_cfsoArrLong (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrLong (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoArrLong (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrLong (ch, src, ctx->dst, ctx);
}

STATIC void
DDS_cfooArrFloat (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_co_context *ctx)
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
    TRACE(printf ("Copied out Float array size %d @ offset = %d\n",
                            ah->size, ctx->src_offset));
}

STATIC void
DDS_cfsoArrFloat (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrFloat (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                           (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoArrFloat (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrFloat (ch, src, ctx->dst, ctx);
}

STATIC void
DDS_cfooArrDouble (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_co_context *ctx)
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
    TRACE(printf ("Copied out Double array size %d @ offset = %d\n",
                            ah->size, ctx->src_offset));
}

STATIC void
DDS_cfsoArrDouble (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrDouble (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                            (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoArrDouble (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArrDouble (ch, src, ctx->dst, ctx);
}

    /* Sequence of primitive type */
STATIC void
DDS_cfooSeqBoolean (
    DDSCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    DDS_co_context *ctx)
{
    unsigned int arrLen;
    c_bool **src;
    DDSSequenceType *dst;
    c_bool * buffer;
    unsigned int i;

    OS_UNUSED_ARG(ch);

    dst = dstSeq;
    src = srcSeq;

    arrLen = c_arraySize((c_array)*src);

    SEQ_ALLOC_PRIM_BUFFER(dst, arrLen, sizeof(c_bool));

    buffer = dst->_buffer;
    if ( buffer ) {
        for (i = 0; i < arrLen; i++) {
            buffer[i] = (*src)[i];
            TRACE(printf ("%d;", buffer[i]));
        }
        dst->_length  = arrLen;
    }

    ctx->dst_correction += DDS_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Boolean sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
DDS_cfsoSeqBoolean (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqBoolean (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                             (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoSeqBoolean (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqBoolean (ch, src,  ctx->dst, ctx);
}



STATIC void
DDS_cfooSeqByte (
    DDSCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    DDS_co_context *ctx)
{
    unsigned int arrLen;
    c_octet **src;
    DDSSequenceType *dst;
    c_octet * buffer;
    unsigned int i;

    OS_UNUSED_ARG(ch);

    dst = dstSeq;
    src = srcSeq;

    arrLen = c_arraySize((c_array)*src);

    SEQ_ALLOC_PRIM_BUFFER(dst, arrLen, sizeof(c_octet));

    buffer = dst->_buffer;
    if ( buffer ) {
        for (i = 0; i < arrLen; i++) {
            buffer[i] = (*src)[i];
            TRACE(printf ("%d;", buffer[i]));
        }
        dst->_length  = arrLen;
    }

    ctx->dst_correction += DDS_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Byte sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
DDS_cfsoSeqByte (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqByte (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoSeqByte (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqByte (ch, src,  ctx->dst, ctx);
}

STATIC void
DDS_cfooSeqChar (
    DDSCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    DDS_co_context *ctx)
{
    unsigned int arrLen;
    c_char **src;
    DDSSequenceType *dst;
    c_char * buffer;
    unsigned int i;

    OS_UNUSED_ARG(ch);

    dst = dstSeq;
    src = srcSeq;

    arrLen = c_arraySize((c_array)*src);

    SEQ_ALLOC_PRIM_BUFFER(dst, arrLen, sizeof(c_char));

    buffer = dst->_buffer;
    if ( buffer ) {
        for (i = 0; i < arrLen; i++) {
            buffer[i] = (*src)[i];
            TRACE(printf ("%d;", buffer[i]));
        }
        dst->_length  = arrLen;
    }

    ctx->dst_correction += DDS_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Char sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
DDS_cfsoSeqChar (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqChar (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoSeqChar (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqChar (ch, src,  ctx->dst, ctx);
}

STATIC void
DDS_cfooSeqShort (
    DDSCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    DDS_co_context *ctx)
{
    unsigned int arrLen;
    c_short **src;
    DDSSequenceType *dst;
    c_short *buffer;
    unsigned int i;

    OS_UNUSED_ARG(ch);

    dst = dstSeq;
    src = srcSeq;

    arrLen = c_arraySize((c_array)*src);

    SEQ_ALLOC_PRIM_BUFFER(dst, arrLen, sizeof(c_short));

    buffer = dst->_buffer;
    if ( buffer ) {
        for (i = 0; i < arrLen; i++) {
            buffer[i] = (*src)[i];
            TRACE(printf ("%d;", buffer[i]));
        }
        dst->_length  = arrLen;
    }

    ctx->dst_correction += DDS_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Short sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
DDS_cfsoSeqShort (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqShort (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                           (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoSeqShort (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqShort (ch, src,  ctx->dst, ctx);
}

STATIC void
DDS_cfooSeqInt (
    DDSCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    DDS_co_context *ctx)
{
    unsigned int arrLen;
    c_long **src;
    DDSSequenceType *dst;
    c_long * buffer;
    unsigned int i;

    OS_UNUSED_ARG(ch);

    dst = dstSeq;
    src = srcSeq;

    arrLen = c_arraySize((c_array)*src);

    SEQ_ALLOC_PRIM_BUFFER(dst, arrLen, sizeof(c_long));

    buffer = dst->_buffer;
    if ( buffer ) {
        for (i = 0; i < arrLen; i++) {
            buffer[i] = (*src)[i];
            TRACE(printf ("%d;", buffer[i]));
        }
        dst->_length  = arrLen;
    }

    ctx->dst_correction += DDS_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Int sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
DDS_cfsoSeqInt (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqInt (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                         (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoSeqInt (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqInt (ch, src,  ctx->dst, ctx);
}

STATIC void
DDS_cfooSeqLong (
    DDSCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    DDS_co_context *ctx)
{
    unsigned int arrLen;
    c_longlong **src;
    DDSSequenceType *dst;
    c_longlong * buffer;
    unsigned int i;

    OS_UNUSED_ARG(ch);

    dst = dstSeq;
    src = srcSeq;

    arrLen = c_arraySize((c_array)*src);

    SEQ_ALLOC_PRIM_BUFFER(dst, arrLen, sizeof(c_longlong));

    buffer = dst->_buffer;
    if ( buffer ) {
        for (i = 0; i < arrLen; i++) {
            buffer[i] = (*src)[i];
            TRACE(printf ("%d;", buffer[i]));
        }
        dst->_length  = arrLen;
    }

    ctx->dst_correction += DDS_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Long sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
DDS_cfsoSeqLong (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqLong (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoSeqLong (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqLong (ch, src,  ctx->dst, ctx);
}

STATIC void
DDS_cfooSeqFloat (
    DDSCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    DDS_co_context *ctx)
{
    unsigned int arrLen;
    c_float **src;
    DDSSequenceType *dst;
    c_float *buffer;
    unsigned int i;

    OS_UNUSED_ARG(ch);

    dst = dstSeq;
    src = srcSeq;

    arrLen = c_arraySize((c_array)*src);

    SEQ_ALLOC_PRIM_BUFFER(dst, arrLen, sizeof(c_float));

    buffer = dst->_buffer;
    if ( buffer ) {
        for (i = 0; i < arrLen; i++) {
            buffer[i] = (*src)[i];
            TRACE(printf ("%d;", buffer[i]));
        }
        dst->_length  = arrLen;
    }

    ctx->dst_correction += DDS_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Float sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
DDS_cfsoSeqFloat (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqFloat (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                           (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoSeqFloat (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqFloat (ch, src,  ctx->dst, ctx);
}

STATIC void
DDS_cfooSeqDouble (
    DDSCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    DDS_co_context *ctx)
{
    unsigned int arrLen;
    c_double **src;
    DDSSequenceType *dst;
    c_double *buffer;
    unsigned int i;

    OS_UNUSED_ARG(ch);

    dst = dstSeq;
    src = srcSeq;

    arrLen = c_arraySize((c_array)*src);

    SEQ_ALLOC_PRIM_BUFFER(dst, arrLen, sizeof(c_double));

    buffer = dst->_buffer;
    if ( buffer ) {
        for (i = 0; i < arrLen; i++) {
            buffer[i] = (*src)[i];
            TRACE(printf ("%d;", buffer[i]));
        }
        dst->_length  = arrLen;
    }

    ctx->dst_correction += DDS_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Double sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
DDS_cfsoSeqDouble (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqDouble (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                            (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoSeqDouble (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSeqDouble (ch, src,  ctx->dst, ctx);
}

    /* Structured types */
STATIC void
DDS_cfooStruct (
    DDSCopyHeader *ch,
    void *srcStruct,
    void *dstStruct,
    DDS_co_context *ctx)
{
    DDS_co_context context;
    unsigned long mi;
    DDSCopyStruct *csh;
    DDSCopyStructMember *csm;

    context.copyCache = ctx->copyCache;
    context.dst = dstStruct;
    context.dst_correction = 0;

    csh = (DDSCopyStruct *)ch;

    csm = DDSCopyStructMemberObject (csh);

    for (mi = 0; mi < csh->nrOfMembers; mi++) {
        context.src_offset = csm->memberOffset;
        ch = DDSCopyStructMemberDescription (csm);
        coFromStruct[ch->copyType] (ch, srcStruct, &context);
        csm = (DDSCopyStructMember *)DDSCopyHeaderNextObject (ch);
    }
    ctx->dst_correction += csh->userSize - csh->size;
    TRACE(printf ("Copied out Struct @ offset = %d\n", ctx->src_offset));

}

STATIC void
DDS_cfsoStruct (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooStruct (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                         (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoStruct (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooStruct (ch, src,  ctx->dst, ctx);
}

STATIC void
DDS_cfooUnion (
    DDSCopyHeader *ch,
    void *srcUnion,
    void *dstUnion,
    DDS_co_context *ctx)
{
    DDS_co_context context;
    unsigned long co;
    DDSCopyUnion *cuh;
    DDSCopyHeader *cdh;
    DDSCopyUnionLabels *csl;
    unsigned long long discrVal;
    unsigned long long oldDiscr;
    DDSCopyUnionLabels *defaultLabel = NULL;
    int active_case = 0;
    void * src;

    cuh = (DDSCopyUnion *)ch;

    context.dst = dstUnion;
    context.copyCache = ctx->copyCache;

    discrVal = DDS_getUnionDescriptor(to_copyType(cuh->discrType), srcUnion);
    oldDiscr = DDS_getUnionDescriptor(to_copyType(cuh->discrType), dstUnion);

    if (discrVal != oldDiscr) {
        DDS_genericCopyBufferFreeType(ch, dstUnion);
    }

    cdh = DDSCopyUnionDiscriminantObject (cuh);
    coFromUnion[cdh->copyType] (cdh, srcUnion, &context);

    src = (void *)((PA_ADDRCAST)srcUnion + cuh->casesOffset);
    context.dst = (void *)((PA_ADDRCAST)dstUnion + cuh->casesOffset);
    context.src_offset = 0;
    context.dst_correction = 0;

    csl = DDSCopyUnionLabelsObject (cdh);
    co = 0;
    while (co < cuh->nrOfCases) {
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
            defaultLabel = (DDSCopyUnionLabels *)DDSCopyUnionCaseObject(csl);
        }
        ch = DDSCopyUnionCaseObject(csl);
        if (active_case) {
            coFromUnion[ch->copyType] (ch, src, &context);
            co = cuh->nrOfCases;
        } else {
            co++;
        }
        csl = (DDSCopyUnionLabels *)DDSCopyHeaderNextObject (ch);
    }
    if (!active_case && defaultLabel) {
        ch = (DDSCopyHeader *)defaultLabel;
        coFromUnion[ch->copyType] (ch, src, &context);
    }
    ctx->dst_correction += cuh->userSize - cuh->size;
    TRACE(printf ("Copied out Union\n"));
}

STATIC void
DDS_cfsoUnion (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooUnion (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                        (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoUnion (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooUnion (ch, src,  ctx->dst, ctx);
}

    /* String types */
STATIC void
DDS_cfooString (
    DDSCopyHeader *ch,
    void *srcString,
    void *dstString,
    DDS_co_context *ctx)
{
    DDS_string *dst;

    OS_UNUSED_ARG(ch);
    OS_UNUSED_ARG(ctx);

    dst = (DDS_string *)(dstString);
    if ( *dst ) {
        DDS_free(*dst);
    }
    *dst = DDS_string_dup(*(DDS_string *)srcString);

    TRACE(printf ("Copied out string = %s @ offset = %d\n",
                               *dst, ctx->src_offset));
}

STATIC void
DDS_cfsoString (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooString (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                         (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoString (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooString (ch, src, ctx->dst, ctx);
}

STATIC void
DDS_cfooBString (
    DDSCopyHeader *ch,
    void *srcString,
    void *dstString,
    DDS_co_context *ctx)
{
    DDS_string *dst;

    OS_UNUSED_ARG(ch);
    OS_UNUSED_ARG(ctx);

    dst = (DDS_string *)(dstString);
    if ( *dst ) {
        DDS_free(*dst);
    }
    *dst = DDS_string_dup(*(DDS_string*)srcString);

    TRACE(printf ("Copied out string = %s @ offset = %d\n",
                               *dst, ctx->src_offset));
}

STATIC void
DDS_cfsoBString (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooBString (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoBString (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooBString (ch, src, ctx->dst, ctx);
}

    /* Array of object type */
STATIC void
DDS_cfooArray (
    DDSCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    DDS_co_context *ctx)
{
    DDSCopyObjectArray *ah;
    DDSCopyHeader *aech;
    void *dst;
    void *src;
    unsigned int i;
    c_long old_dst_correction;

    src = srcArray;
    dst = dstArray;
    ah = (DDSCopyObjectArray *)ch;
    aech = DDSCopyObjectArrayDescription (ah);
    old_dst_correction = ctx->dst_correction;
    for (i = 0; i < ah->arraySize; i++) {
        coFromArray[aech->copyType] (aech, src, dst, ctx);
        dst = (void *)((PA_ADDRCAST)dst + ah->typeSize + ctx->dst_correction - old_dst_correction);
        src = (void *)((PA_ADDRCAST)src + ah->typeSize);
        old_dst_correction = ctx->dst_correction;
    }
    TRACE(printf ("Copied out Object array size %d @ offset = %d\n",
                                ah->typeSize, ctx->src_offset));
}

STATIC void
DDS_cfsoArray (
    DDSCopyHeader *ch,
    void *src,
    DDS_co_context *ctx)
{
    DDS_cfooArray(ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                       (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoArray (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooArray (ch, src, ctx->dst, ctx);
}

    /* Sequence of object type */
STATIC void
DDS_cfooSequence (
    DDSCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    DDS_co_context *ctx)
{
    DDS_co_context context;
    DDSCopyObjectSequence *sh;
    DDSCopyHeader *sech;
    void *src;
    c_ulong i;
    c_ulong seqLen;
    DDSSequenceType *dst;
    void *buffer;

    sh = (DDSCopyObjectSequence *)ch;
    sech = DDSCopyObjectSequenceDescription (sh);
    src = *(void**)srcSeq;
    dst = dstSeq;

    context.dst = dst;
    context.copyCache = ctx->copyCache;

    seqLen = c_arraySize((c_array)src);

    SEQ_ALLOC_OBJECT_BUFFER(dst, seqLen, sh->userTypeSize, ctx->copyCache, sech);

    buffer = dst->_buffer;

    if ( buffer ) {
        for (i = 0; i < seqLen; i++) {
            context.dst_correction = 0;
            context.src_offset = 0;
            coFromArray[sech->copyType] (sech, src, buffer, &context);
            buffer = (void *)((PA_ADDRCAST)buffer + sh->userTypeSize);
            src = (void *)((PA_ADDRCAST)src + sh->baseTypeSize);
        }
        dst->_length  = seqLen;
        dst->_maximum = seqLen;
    } else {
        dst->_length  = 0;
        dst->_maximum = 0;
    }

    ctx->dst_correction += DDS_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Object sequence size %d @ offset = %d\n",
                            seqLen, ctx->src_offset));
}


STATIC void
DDS_cfsoSequence (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDS_cfooSequence(ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
DDS_cfuoSequence (
    DDSCopyHeader *ch,
    void * src,

    DDS_co_context *ctx)
{
    DDS_cfooSequence (ch, src, ctx->dst, ctx);
}

    /* backward referenced type */
STATIC void
DDS_cfooReference (
    DDSCopyHeader *ch,
    void * src,
    void *dst,
    DDS_co_context *ctx)
{
    DDSCopyReference *ref;
    DDSCopyHeader *nch;

    ref = (DDSCopyReference *)ch;
    nch = DDSCopyReferencedObject (ref);
    coFromArray[nch->copyType] (nch, src, dst, ctx);
}

STATIC void
DDS_cfsoReference (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDSCopyReference *ref;
    DDSCopyHeader *nch;

    ref = (DDSCopyReference *)ch;
    nch = DDSCopyReferencedObject (ref);
    coFromStruct[nch->copyType] (nch, src, ctx);
}

STATIC void
DDS_cfuoReference (
    DDSCopyHeader *ch,
    void * src,
    DDS_co_context *ctx)
{
    DDSCopyReference *ref;
    DDSCopyHeader *nch;

    ref = (DDSCopyReference *)ch;
    nch = DDSCopyReferencedObject (ref);
    coFromUnion[nch->copyType] (nch, src, ctx);
}

void
DDS_copyOutStruct (
    void *src,
    void *dst)
{
    DDS_co_context context;
    DDS_dstInfo dstInfo = (DDS_dstInfo)dst;
    DDSCopyHeader *ch;

    ch = DDS_copyCacheCache(dstInfo->copyProgram);
    context.copyCache = dstInfo->copyProgram;
    context.dst = dst;
    context.src_offset = 0;
    context.dst_correction = 0;
    coFromArray[ch->copyType] (ch, src, dstInfo->dst, &context);
}

void *
DDS_copyOutAllocBuffer (
    DDS_copyCache copyCache,
    DDS_unsigned_long len)
{
    DDS_unsigned_long size;
    DDSCopyHeader *ch;

    ch = DDS_copyCacheCache(copyCache);

    size = DDS_copyCacheGetUserSize(copyCache);

    assert(size > 0);

    return DDS_genericCopyBufferAllocSeqBuffer(copyCache, ch, size, len);
}

