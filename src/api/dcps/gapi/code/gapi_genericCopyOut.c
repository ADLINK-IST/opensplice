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
#include "gapi_genericCopyBuffer.h"
#include "gapi_genericCopyOut.h"
#include "gapi_common.h"

#include "c_base.h"

#include "os_abstract.h"
#include "os_report.h"
#include "os_stdlib.h"

#include "gapi.h"

#define TRACE(function) /* function */
#define STATIC static

#define SEQ_ALLOC_PRIM_BUFFER(seq,len,size) \
    if (seq->_maximum > 0UL) {\
        assert(seq->_buffer);\
        if (len > seq->_maximum) {\
            if (seq->_release) {\
                gapi_free(seq->_buffer);\
            }\
            seq->_maximum = 0UL;\
            seq->_length  = 0UL;\
            seq->_buffer  = NULL;\
        }\
    }\
    \
    if ((len > 0UL) && (seq->_maximum == 0UL)) {\
        seq->_buffer = gapi_sequence_allocbuf(NULL, size, len);\
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
                gapi_free(seq->_buffer);\
            }\
            seq->_maximum = 0UL;\
            seq->_length  = 0UL;\
            seq->_buffer  = NULL;\
        }\
    }\
    \
    if ((len > 0UL) && (seq->_maximum == 0UL)) {\
        seq->_buffer = gapi_genericCopyBufferAllocSeqBuffer(cc, ch, size, len);\
        if ( seq->_buffer ) {\
            seq->_maximum = len;\
            seq->_release = TRUE;\
        }\
    }



typedef struct {
    gapi_copyCache copyCache;
    void *dst;
    c_long src_offset;
    c_long dst_correction;
} gapi_co_context;

typedef void (*copyInFromStruct)(gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
typedef void (*copyInFromUnion)(gapiCopyHeader *ch, void * src, gapi_co_context *ctx);
typedef void (*copyInFromArray)(gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_co_context *ctx);

    /* blackBox, copy as one block of data */
STATIC void gapi_cfsoBlackBox    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* Primitive types */
STATIC void gapi_cfsoBoolean    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoByte       (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoChar       (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoShort      (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoInt        (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoLong       (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoFloat      (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoDouble     (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* Array of primitive type */
STATIC void gapi_cfsoArrBoolean (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoArrByte    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoArrChar    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoArrShort   (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoArrInt     (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoArrLong    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoArrFloat   (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoArrDouble  (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* Sequence of primitive type */
STATIC void gapi_cfsoSeqBoolean (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoSeqByte    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoSeqChar    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoSeqShort   (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoSeqInt     (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoSeqLong    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoSeqFloat   (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoSeqDouble  (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* Enumeration type */
STATIC void gapi_cfsoEnum       (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* Structured types */
STATIC void gapi_cfsoStruct     (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoUnion      (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* String types */
STATIC void gapi_cfsoString     (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfsoBString    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* Array of object type */
STATIC void gapi_cfsoArray      (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* Sequence of object type */
STATIC void gapi_cfsoSequence   (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* reference to previous defined type */
STATIC void gapi_cfsoReference  (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);

    /* blackBox, copy as one block of data */
STATIC void gapi_cfuoBlackBox    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* Primitive types */
STATIC void gapi_cfuoBoolean    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoByte       (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoChar       (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoShort      (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoInt        (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoLong       (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoFloat      (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoDouble     (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* Array of primitive type */
STATIC void gapi_cfuoArrBoolean (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoArrByte    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoArrChar    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoArrShort   (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoArrInt     (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoArrLong    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoArrFloat   (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoArrDouble  (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* Sequence of primitive type */
STATIC void gapi_cfuoSeqBoolean (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoSeqByte    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoSeqChar    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoSeqShort   (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoSeqInt     (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoSeqLong    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoSeqFloat   (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoSeqDouble  (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* Enumeration type */
STATIC void gapi_cfuoEnum       (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* Structured types */
STATIC void gapi_cfuoStruct     (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoUnion      (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* String types */
STATIC void gapi_cfuoString     (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
STATIC void gapi_cfuoBString    (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* Array of object type */
STATIC void gapi_cfuoArray      (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* Sequence of object type */
STATIC void gapi_cfuoSequence   (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);
    /* reference to previous defined type */
STATIC void gapi_cfuoReference  (gapiCopyHeader *ch, void * src,  gapi_co_context *ctx);

    /* blackBox, copy as one block of data */
STATIC void gapi_cfooBlackBox       (gapiCopyHeader *ch, void * srcBlock, void *dstBlock, gapi_co_context *ctx);
    /* Array of primitive type */
STATIC void gapi_cfooArrBoolean (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_co_context *ctx);
STATIC void gapi_cfooArrByte    (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_co_context *ctx);
STATIC void gapi_cfooArrChar    (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_co_context *ctx);
STATIC void gapi_cfooArrShort   (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_co_context *ctx);
STATIC void gapi_cfooArrInt     (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_co_context *ctx);
STATIC void gapi_cfooArrLong    (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_co_context *ctx);
STATIC void gapi_cfooArrFloat   (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_co_context *ctx);
STATIC void gapi_cfooArrDouble  (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_co_context *ctx);
    /* Sequence of primitive type */
STATIC void gapi_cfooSeqBoolean (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_co_context *ctx);
STATIC void gapi_cfooSeqByte    (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_co_context *ctx);
STATIC void gapi_cfooSeqChar    (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_co_context *ctx);
STATIC void gapi_cfooSeqShort   (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_co_context *ctx);
STATIC void gapi_cfooSeqInt     (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_co_context *ctx);
STATIC void gapi_cfooSeqLong    (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_co_context *ctx);
STATIC void gapi_cfooSeqFloat   (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_co_context *ctx);
STATIC void gapi_cfooSeqDouble  (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_co_context *ctx);
    /* Enumeration type */
STATIC void gapi_cfooEnum       (gapiCopyHeader *ch, void * srcEnum, void *dstEnum, gapi_co_context *ctx);
    /* Structured types */
STATIC void gapi_cfooStruct     (gapiCopyHeader *ch, void * srcStruct, void *dstStruct, gapi_co_context *ctx);
STATIC void gapi_cfooUnion      (gapiCopyHeader *ch, void * srcUnion,  void *dstUnion,  gapi_co_context *ctx);
    /* String types */
STATIC void gapi_cfooString     (gapiCopyHeader *ch, void * srcString, void *dstString, gapi_co_context *ctx);
STATIC void gapi_cfooBString    (gapiCopyHeader *ch, void * srcString, void *dstString, gapi_co_context *ctx);
    /* Array of object type */
STATIC void gapi_cfooArray      (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_co_context *ctx);
    /* Sequence of object type */
STATIC void gapi_cfooSequence   (gapiCopyHeader *ch, void * srcSeq, void *dstSeq, gapi_co_context *ctx);
    /* reference to previous defined type */
STATIC void gapi_cfooReference  (gapiCopyHeader *ch, void * src, void *dst, gapi_co_context *ctx);

STATIC copyInFromStruct coFromStruct[] = {
    gapi_cfsoBlackBox,
    gapi_cfsoBoolean,
    gapi_cfsoByte,
    gapi_cfsoChar,
    gapi_cfsoShort,
    gapi_cfsoInt,
    gapi_cfsoLong,
    gapi_cfsoFloat,
    gapi_cfsoDouble,
    gapi_cfsoArrBoolean,
    gapi_cfsoArrByte,
    gapi_cfsoArrChar,
    gapi_cfsoArrShort,
    gapi_cfsoArrInt,
    gapi_cfsoArrLong,
    gapi_cfsoArrFloat,
    gapi_cfsoArrDouble,
    gapi_cfsoSeqBoolean,
    gapi_cfsoSeqByte,
    gapi_cfsoSeqChar,
    gapi_cfsoSeqShort,
    gapi_cfsoSeqInt,
    gapi_cfsoSeqLong,
    gapi_cfsoSeqFloat,
    gapi_cfsoSeqDouble,
    gapi_cfsoEnum,
    gapi_cfsoStruct,
    gapi_cfsoUnion,
    gapi_cfsoString,
    gapi_cfsoBString,
    gapi_cfsoArray,
    gapi_cfsoSequence,
    gapi_cfsoReference
    };

STATIC copyInFromUnion coFromUnion[] = {
    gapi_cfuoBlackBox,
    gapi_cfuoBoolean,
    gapi_cfuoByte,
    gapi_cfuoChar,
    gapi_cfuoShort,
    gapi_cfuoInt,
    gapi_cfuoLong,
    gapi_cfuoFloat,
    gapi_cfuoDouble,
    gapi_cfuoArrBoolean,
    gapi_cfuoArrByte,
    gapi_cfuoArrChar,
    gapi_cfuoArrShort,
    gapi_cfuoArrInt,
    gapi_cfuoArrLong,
    gapi_cfuoArrFloat,
    gapi_cfuoArrDouble,
    gapi_cfuoSeqBoolean,
    gapi_cfuoSeqByte,
    gapi_cfuoSeqChar,
    gapi_cfuoSeqShort,
    gapi_cfuoSeqInt,
    gapi_cfuoSeqLong,
    gapi_cfuoSeqFloat,
    gapi_cfuoSeqDouble,
    gapi_cfuoEnum,
    gapi_cfuoStruct,
    gapi_cfuoUnion,
    gapi_cfuoString,
    gapi_cfuoBString,
    gapi_cfuoArray,
    gapi_cfuoSequence,
    gapi_cfuoReference
    };

STATIC copyInFromUnion coUnionDiscr[] = {
    NULL, /*gapi_cfuoBlackBox*/
    gapi_cfuoBoolean,
    NULL, /*gapi_cfuoByte*/
    gapi_cfuoChar,
    gapi_cfuoShort,
    gapi_cfuoInt,
    gapi_cfuoLong,
    NULL, /*gapi_cfuoFloat*/
    NULL, /*gapi_cfuoDouble*/
    NULL, /*gapi_cfuoArrBoolean*/
    NULL, /*gapi_cfuoArrByte*/
    NULL, /*gapi_cfuoArrChar*/
    NULL, /*gapi_cfuoArrShort*/
    NULL, /*gapi_cfuoArrInt*/
    NULL, /*gapi_cfuoArrLong*/
    NULL, /*gapi_cfuoArrFloat*/
    NULL, /*gapi_cfuoArrDouble*/
    NULL, /*gapi_cfuoSeqBoolean*/
    NULL, /*gapi_cfuoSeqByte*/
    NULL, /*gapi_cfuoSeqChar*/
    NULL, /*gapi_cfuoSeqShort*/
    NULL, /*gapi_cfuoSeqInt*/
    NULL, /*gapi_cfuoSeqLong*/
    NULL, /*gapi_cfuoSeqFloat*/
    NULL, /*gapi_cfuoSeqDouble*/
    gapi_cfuoEnum,
    NULL, /*gapi_cfuoStruct*/
    NULL, /*gapi_cfuoUnion*/
    NULL, /*gapi_cfuoString*/
    NULL, /*gapi_cfuoBString*/
    NULL, /*gapi_cfuoArray*/
    NULL, /*gapi_cfuoSequence*/
    NULL /*gapi_cfuoReference*/
    };


STATIC copyInFromArray coFromArray[] = {
    gapi_cfooBlackBox,
    NULL, /* gapi_cfooBoolean */
    NULL, /* gapi_cfooByte */
    NULL, /* gapi_cfooChar */
    NULL, /* gapi_cfooShort */
    NULL, /* gapi_cfooInt */
    NULL, /* gapi_cfooLong */
    NULL, /* gapi_cfooFloat */
    NULL, /* gapi_cfooDouble */
    gapi_cfooArrBoolean,
    gapi_cfooArrByte,
    gapi_cfooArrChar,
    gapi_cfooArrShort,
    gapi_cfooArrInt,
    gapi_cfooArrLong,
    gapi_cfooArrFloat,
    gapi_cfooArrDouble,
    gapi_cfooSeqBoolean,
    gapi_cfooSeqByte,
    gapi_cfooSeqChar,
    gapi_cfooSeqShort,
    gapi_cfooSeqInt,
    gapi_cfooSeqLong,
    gapi_cfooSeqFloat,
    gapi_cfooSeqDouble,
    gapi_cfooEnum,
    gapi_cfooStruct,
    gapi_cfooUnion,
    gapi_cfooString,
    gapi_cfooBString,
    gapi_cfooArray,
    gapi_cfooSequence,
    gapi_cfooReference
    };

STATIC  gapiCopyType
to_copyType(c_type t)
{
    gapiCopyType ct = gapiBlackBox;
    switch(c_baseObject(t)->kind ) {
    case M_ENUMERATION:
        ct = gapiEnum;
    break;
    case M_PRIMITIVE:
        switch (c_primitive(t)->kind) {
        case P_BOOLEAN:
            ct = gapiBoolean;
        break;
        case P_CHAR:
            ct = gapiChar;
        break;
        case P_SHORT:
        case P_USHORT:
            ct = gapiShort;
        break;
        case P_LONG:
        case P_ULONG:
            ct = gapiInt;
        break;
        case P_LONGLONG:
        case P_ULONGLONG:
            ct = gapiLong;
        break;
        default:
            OS_REPORT_1(OS_ERROR,"to_copyType",0,
                        "Illegal primitive type (%d) detected.",
                        c_primitive(t)->kind);
            assert (0);
        }
    break;
    default:
        OS_REPORT_1(OS_ERROR,"to_copyType",0,
                    "Illegal type (%d) detected.",
                    c_baseObject(t)->kind);
        assert (0);
    }
    return ct;
}

    /* Primitive types */
STATIC void
gapi_cfsoBoolean (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    c_bool *dst = (c_bool *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_bool *)((PA_ADDRCAST)src + ctx->src_offset);

    TRACE(printf ("Copied out Boolean = %d @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
gapi_cfuoBoolean (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    c_bool *dst = (c_bool *)ctx->dst;
    *dst = *(c_bool *)src;

    TRACE(printf ("Copied out Boolean = %d\n", *dst));
}

STATIC void
gapi_cfsoByte (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    c_octet *dst = (c_octet *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_octet *)((PA_ADDRCAST)src + ctx->src_offset);

    TRACE(printf ("Copied out BYTE = %hd @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
gapi_cfuoByte (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    c_octet *dst = (c_octet *)ctx->dst;
    *dst = *(c_octet *)src;

    TRACE(printf ("Copied out Byte = %hd\n", *dst));
}

STATIC void
gapi_cfsoChar (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    c_char *dst = (c_char *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_char *)((PA_ADDRCAST)src + ctx->src_offset);

    TRACE(printf ("Copied out Char = %hd @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
gapi_cfuoChar (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    c_char *dst = (c_char *)ctx->dst;
    *dst = *(c_char *)src;

    TRACE(printf ("Copied out Char = %hd\n", *dst));
}

STATIC void
gapi_cfsoShort (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    c_short *dst = (c_short *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_short *)((PA_ADDRCAST)src + ctx->src_offset);

    TRACE(printf ("Copied out Chort = %hd @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
gapi_cfuoShort (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    c_short *dst = (c_short *)ctx->dst;
    *dst = *(c_short *)src;

    TRACE(printf ("Copied out Short = %hd\n", *dst));
}

STATIC void
gapi_cfsoInt (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    c_long *dst = (c_long *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_long *)((PA_ADDRCAST)src + ctx->src_offset);

    TRACE(printf ("Copied out Int = %d @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
gapi_cfuoInt (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    c_long *dst = (c_long *)ctx->dst;
    *dst = *(c_long *)src;

    TRACE(printf ("Copied out Int = %d\n", *dst));
}

STATIC void
gapi_cfsoLong (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    TRACE (char llstr[36];)

    c_longlong *dst = (c_longlong *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_longlong *)((PA_ADDRCAST)src + ctx->src_offset);

    TRACE(llstr[35] = '\0'; printf ("Copied out Long = %s @ offset = %d\n", os_lltostr(*dst, &llstr[35]), ctx->src_offset));
}

STATIC void
gapi_cfuoLong (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    TRACE (char llstr[36];)

    c_longlong *dst = (c_longlong *)ctx->dst;
    *dst = *(c_longlong *)src;

    TRACE(llstr[35] = '\0'; printf ("Copied out Long = %s\n", os_lltostr(*dst, &llstr[35])));
}

STATIC void
gapi_cfsoFloat (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    c_float *dst = (c_float *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_float *)((PA_ADDRCAST)src + ctx->src_offset);

    TRACE(printf ("Copied out Float = %f @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
gapi_cfuoFloat (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    c_float *dst = (c_float *)ctx->dst;
    *dst = *(c_float *)src;

    TRACE(printf ("Copied out Float = %f\n", *dst));
}

STATIC void
gapi_cfsoDouble (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    c_double *dst = (c_double *)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction);
    *dst = *(c_double *)((PA_ADDRCAST)src + ctx->src_offset);

    TRACE(printf ("Copied out Double = %f @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
gapi_cfuoDouble (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    c_double *dst = (c_double *)ctx->dst;
    *dst = *(c_double *)src;

    TRACE(printf ("Copied out Double = %f\n", *dst));
}

    /* Enumeration type */
STATIC void
gapi_cfooEnum (
    gapiCopyHeader *ch,
    void *srcEnum,
    void *dstEnum,
    gapi_co_context *ctx)
{
    c_long *dst;

    dst = (c_long *)dstEnum;
    *dst = *(c_long *)srcEnum;
    TRACE(printf ("Copied out in Enum = %d @ offset = %d\n", *dst, ctx->src_offset));
}

STATIC void
gapi_cfsoEnum (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooEnum (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                       (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoEnum (
    gapiCopyHeader *ch,
    void * src,

    gapi_co_context *ctx)
{
    gapi_cfooEnum (ch, src, ctx->dst, ctx);
}

    /* BlackBox data type */
STATIC void
gapi_cfooBlackBox (
    gapiCopyHeader *ch,
    void *srcBlock,
    void *dstBlock,
    gapi_co_context *ctx)
{
    gapiCopyBlackBox *bbh;

    bbh=(gapiCopyBlackBox *)ch;
    memcpy(dstBlock,srcBlock,bbh->size);
    TRACE(printf ("Copied out BlackBox size = %d @ offset = %d\n", bbh->size, ctx->src_offset));
}

STATIC void
gapi_cfsoBlackBox (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooBlackBox (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                           (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoBlackBox (
    gapiCopyHeader *ch,
    void * src,

    gapi_co_context *ctx)
{
    gapi_cfooBlackBox (ch, src, ctx->dst, ctx);
}

    /* Array of primitive type */
STATIC void
gapi_cfooArrBoolean (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_co_context *ctx)
{
    gapiCopyArray *ah;
    c_bool *dst = dstArray;
    c_bool *src = srcArray;
    unsigned int i;

    ah = (gapiCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(printf ("%d @ [%d];", dst[i], i));
    }
    TRACE(printf ("Copied out Boolean array size %d @ offset = %d\n",
                               ah->size, ctx->src_offset));
}

STATIC void
gapi_cfsoArrBoolean (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrBoolean (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                             (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoArrBoolean (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrBoolean (ch, src, ctx->dst, ctx);
}



STATIC void
gapi_cfooArrByte (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_co_context *ctx)
{
    gapiCopyArray *ah;
    c_octet *dst = dstArray;
    c_octet *src = srcArray;
    unsigned int i;

    ah = (gapiCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(printf ("%d @ [%d];", dst[i], i));
    }
    TRACE(printf ("Copied out Byte array size %d @ offset = %d\n",
                            ah->size, ctx->src_offset));
}

STATIC void
gapi_cfsoArrByte (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrByte (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoArrByte (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrByte (ch, src, ctx->dst, ctx);
}

STATIC void
gapi_cfooArrChar (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_co_context *ctx)
{
    gapiCopyArray *ah;
    c_char *dst = dstArray;
    c_char *src = srcArray;
    unsigned int i;

    ah = (gapiCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(printf ("%d @ [%d];", dst[i], i));
    }
    TRACE(printf ("Copied out Char array size %d @ offset = %d\n",
                            ah->size, ctx->src_offset));
}

STATIC void
gapi_cfsoArrChar (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrChar (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoArrChar (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrChar (ch, src, ctx->dst, ctx);
}

STATIC void
gapi_cfooArrShort (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_co_context *ctx)
{
    gapiCopyArray *ah;
    c_short *dst = dstArray;
    c_short *src = srcArray;
    unsigned int i;

    ah = (gapiCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(printf ("%d @ [%d];", dst[i], i));
    }
    TRACE(printf ("Copied out Short array size %d @ offset = %d\n",
                            ah->size, ctx->src_offset));
}

STATIC void
gapi_cfsoArrShort (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrShort (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                           (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoArrShort (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrShort (ch, src, ctx->dst, ctx);
}

STATIC void
gapi_cfooArrInt (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_co_context *ctx)
{
    gapiCopyArray *ah;
    c_long *dst = dstArray;
    c_long *src = srcArray;
    unsigned int i;

    ah = (gapiCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(printf ("%d @ [%d];", dst[i], i));
    }
    TRACE(printf ("Copied out Int array size %d @ offset = %d\n",
                            ah->size, ctx->src_offset));

}

STATIC void
gapi_cfsoArrInt (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrInt (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                         (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoArrInt (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrInt (ch, src, ctx->dst, ctx);
}

STATIC void
gapi_cfooArrLong (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_co_context *ctx)
{
    gapiCopyArray *ah;
    c_longlong *dst = dstArray;
    c_longlong *src = srcArray;
    unsigned int i;

    TRACE (char llstr[36];)

    ah = (gapiCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(llstr[35] = '\0'; printf ("%s @ [%d];", os_lltostr(dst[i], &llstr[35]), i));
    }
    TRACE(printf ("Copied out Long array size %d @ offset = %d\n",
                            ah->size, ctx->src_offset));
}

STATIC void
gapi_cfsoArrLong (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrLong (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoArrLong (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrLong (ch, src, ctx->dst, ctx);
}

STATIC void
gapi_cfooArrFloat (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_co_context *ctx)
{
    gapiCopyArray *ah;
    c_float *dst = dstArray;
    c_float *src = srcArray;
    unsigned int i;

    ah = (gapiCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(printf ("%f @ [%d];", dst[i], i));
    }
    TRACE(printf ("Copied out Float array size %d @ offset = %d\n",
                            ah->size, ctx->src_offset));
}

STATIC void
gapi_cfsoArrFloat (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrFloat (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                           (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoArrFloat (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrFloat (ch, src, ctx->dst, ctx);
}

STATIC void
gapi_cfooArrDouble (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_co_context *ctx)
{
    gapiCopyArray *ah;
    c_double *dst = dstArray;
    c_double *src = srcArray;
    unsigned int i;

    ah = (gapiCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(printf ("%f @ [%d];", dst[i], i));
    }
    TRACE(printf ("Copied out Double array size %d @ offset = %d\n",
                            ah->size, ctx->src_offset));
}

STATIC void
gapi_cfsoArrDouble (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrDouble (ch, (void*)((PA_ADDRCAST)src + ctx->src_offset),
                            (void*)((PA_ADDRCAST)ctx->dst + ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoArrDouble (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArrDouble (ch, src, ctx->dst, ctx);
}

    /* Sequence of primitive type */
STATIC void
gapi_cfooSeqBoolean (
    gapiCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    gapi_co_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    c_bool **src;
    gapiSequenceType *dst;
    c_bool * buffer;
    unsigned int i;

    sh = (gapiCopySequence *)ch;
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

    ctx->dst_correction += GAPI_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Boolean sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
gapi_cfsoSeqBoolean (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqBoolean (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                             (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoSeqBoolean (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqBoolean (ch, src,  ctx->dst, ctx);
}



STATIC void
gapi_cfooSeqByte (
    gapiCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    gapi_co_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    c_octet **src;
    gapiSequenceType *dst;
    c_octet * buffer;
    unsigned int i;

    sh = (gapiCopySequence *)ch;
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

    ctx->dst_correction += GAPI_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Byte sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
gapi_cfsoSeqByte (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqByte (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoSeqByte (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqByte (ch, src,  ctx->dst, ctx);
}

STATIC void
gapi_cfooSeqChar (
    gapiCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    gapi_co_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    c_char **src;
    gapiSequenceType *dst;
    c_char * buffer;
    unsigned int i;

    sh = (gapiCopySequence *)ch;
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

    ctx->dst_correction += GAPI_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Char sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
gapi_cfsoSeqChar (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqChar (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoSeqChar (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqChar (ch, src,  ctx->dst, ctx);
}

STATIC void
gapi_cfooSeqShort (
    gapiCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    gapi_co_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    c_short **src;
    gapiSequenceType *dst;
    c_short *buffer;
    unsigned int i;

    sh = (gapiCopySequence *)ch;
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

    ctx->dst_correction += GAPI_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Short sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
gapi_cfsoSeqShort (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqShort (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                           (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoSeqShort (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqShort (ch, src,  ctx->dst, ctx);
}

STATIC void
gapi_cfooSeqInt (
    gapiCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    gapi_co_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    c_long **src;
    gapiSequenceType *dst;
    c_long * buffer;
    unsigned int i;

    sh = (gapiCopySequence *)ch;
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

    ctx->dst_correction += GAPI_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Int sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
gapi_cfsoSeqInt (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqInt (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                         (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoSeqInt (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqInt (ch, src,  ctx->dst, ctx);
}

STATIC void
gapi_cfooSeqLong (
    gapiCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    gapi_co_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    c_longlong **src;
    gapiSequenceType *dst;
    c_longlong * buffer;
    unsigned int i;

    sh = (gapiCopySequence *)ch;
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

    ctx->dst_correction += GAPI_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Long sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
gapi_cfsoSeqLong (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqLong (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoSeqLong (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqLong (ch, src,  ctx->dst, ctx);
}

STATIC void
gapi_cfooSeqFloat (
    gapiCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    gapi_co_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    c_float **src;
    gapiSequenceType *dst;
    c_float *buffer;
    unsigned int i;

    sh = (gapiCopySequence *)ch;
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

    ctx->dst_correction += GAPI_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Float sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
gapi_cfsoSeqFloat (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqFloat (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                           (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoSeqFloat (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqFloat (ch, src,  ctx->dst, ctx);
}

STATIC void
gapi_cfooSeqDouble (
    gapiCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    gapi_co_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    c_double **src;
    gapiSequenceType *dst;
    c_double *buffer;
    unsigned int i;

    sh = (gapiCopySequence *)ch;
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

    ctx->dst_correction += GAPI_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Double sequence size %d @ offset = %d\n",
                            arrLen, ctx->src_offset));
}

STATIC void
gapi_cfsoSeqDouble (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqDouble (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                            (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoSeqDouble (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSeqDouble (ch, src,  ctx->dst, ctx);
}

    /* Structured types */
STATIC void
gapi_cfooStruct (
    gapiCopyHeader *ch,
    void *srcStruct,
    void *dstStruct,
    gapi_co_context *ctx)
{
    gapi_co_context context;
    unsigned long mi;
    gapiCopyStruct *csh;
    gapiCopyStructMember *csm;

    context.copyCache = ctx->copyCache;
    context.dst = dstStruct;
    context.dst_correction = ctx->dst_correction;

    csh = (gapiCopyStruct *)ch;

    csm = gapiCopyStructMemberObject (csh);

    for (mi = 0; mi < csh->nrOfMembers; mi++) {
        context.src_offset = csm->memberOffset;
        ch = gapiCopyStructMemberDescription (csm);
        coFromStruct[ch->copyType] (ch, srcStruct, &context);
        csm = (gapiCopyStructMember *)gapiCopyHeaderNextObject (ch);
    }
    ctx->dst_correction += csh->userSize - csh->size;
    TRACE(printf ("Copied out Struct @ offset = %d\n", ctx->src_offset));

}

STATIC void
gapi_cfsoStruct (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooStruct (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                         (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoStruct (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooStruct (ch, src,  ctx->dst, ctx);
}

STATIC void
gapi_cfooUnion (
    gapiCopyHeader *ch,
    void *srcUnion,
    void *dstUnion,
    gapi_co_context *ctx)
{
    gapi_co_context context;
    unsigned long co;
    gapiCopyUnion *cuh;
    gapiCopyUnionLabels *csl;
    unsigned long long discrVal;
    unsigned long long oldDiscr;
    gapiCopyUnionLabels *defaultLabel = NULL;
    int active_case = 0;
    void * src;

    cuh = (gapiCopyUnion *)ch;

    assert(coUnionDiscr[to_copyType(cuh->discrType)]);

    context.dst = dstUnion;
    context.copyCache = ctx->copyCache;

    discrVal = gapi_getUnionDescriptor(to_copyType(cuh->discrType), srcUnion);
    oldDiscr = gapi_getUnionDescriptor(to_copyType(cuh->discrType), dstUnion);

    if (discrVal != oldDiscr) {
        gapi_genericCopyBufferFreeType(ch, dstUnion);
    }

    coUnionDiscr[to_copyType(cuh->discrType)] (ch, srcUnion, &context);

    src = (void *)((PA_ADDRCAST)srcUnion + cuh->casesOffset);
    context.dst = (void *)((PA_ADDRCAST)dstUnion + cuh->casesOffset);
    context.src_offset = 0;
    context.dst_correction = 0;

    csl = gapiCopyUnionLabelsObject (cuh);
    co = 0;
    while (co < cuh->nrOfCases) {
        unsigned int label;
        gapiCopyUnionLabel *lab;

        lab = gapiCopyUnionLabelObject (csl);
        if (csl->labelCount) {
            for (label = 0; label < csl->labelCount; label++) {
                if (lab->labelVal == discrVal) {
                    active_case = 1;
                }
                lab++;
            }
        } else {
            defaultLabel = (gapiCopyUnionLabels *)gapiCopyUnionCaseObject(csl);
        }
        ch = gapiCopyUnionCaseObject(csl);
        if (active_case) {
            coFromUnion[ch->copyType] (ch, src, &context);
            co = cuh->nrOfCases;
        } else {
            co++;
        }
        csl = (gapiCopyUnionLabels *)gapiCopyHeaderNextObject (ch);
    }
    if (!active_case && defaultLabel) {
        ch = (gapiCopyHeader *)defaultLabel;
        coFromUnion[ch->copyType] (ch, src, &context);
    }
    ctx->dst_correction += cuh->userSize - cuh->size;
    TRACE(printf ("Copied out Union\n"));
}

STATIC void
gapi_cfsoUnion (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooUnion (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                        (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoUnion (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooUnion (ch, src,  ctx->dst, ctx);
}

    /* String types */
STATIC void
gapi_cfooString (
    gapiCopyHeader *ch,
    void *srcString,
    void *dstString,
    gapi_co_context *ctx)
{
    gapi_string *dst;

    dst = (gapi_string *)(dstString);
    if ( *dst ) {
        gapi_free(*dst);
    }
    *dst = gapi_string_dup(*(gapi_string *)srcString);

    TRACE(printf ("Copied out string = %s @ offset = %d\n",
                               *dst, ctx->src_offset));
}

STATIC void
gapi_cfsoString (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooString (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                         (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoString (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooString (ch, src, ctx->dst, ctx);
}

STATIC void
gapi_cfooBString (
    gapiCopyHeader *ch,
    void *srcString,
    void *dstString,
    gapi_co_context *ctx)
{
    gapi_string *dst;

    dst = (gapi_string *)(dstString);
    if ( *dst ) {
        gapi_free(*dst);
    }
    *dst = gapi_string_dup(*(gapi_string*)srcString);

    TRACE(printf ("Copied out string = %s @ offset = %d\n",
                               *dst, ctx->src_offset));
}

STATIC void
gapi_cfsoBString (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooBString (ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoBString (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooBString (ch, src, ctx->dst, ctx);
}

    /* Array of object type */
STATIC void
gapi_cfooArray (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_co_context *ctx)
{
    gapiCopyObjectArray *ah;
    gapiCopyHeader *aech;
    void *dst;
    void *src;
    unsigned int i;
    c_long old_dst_correction;

    src = srcArray;
    dst = dstArray;
    ah = (gapiCopyObjectArray *)ch;
    aech = gapiCopyObjectArrayDescription (ah);
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
gapi_cfsoArray (
    gapiCopyHeader *ch,
    void *src,
    gapi_co_context *ctx)
{
    gapi_cfooArray(ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                       (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoArray (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooArray (ch, src, ctx->dst, ctx);
}

    /* Sequence of object type */
#if 0
STATIC void
gapi_cfooSequence (
    gapiCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    gapi_co_context *ctx)
{
    gapiCopyObjectSequence *sh;
    gapiCopyHeader *sech;
    void *src;
    c_long i;
    c_long seqLen;
    gapiSequenceType *dst;
    void *buffer;
    c_long dst_correction;

    sh = (gapiCopyObjectSequence *)ch;
    sech = gapiCopyObjectSequenceDescription (sh);
    src = *(void**)srcSeq;
    dst = dstSeq;
    dst_correction = ctx->dst_correction;

    seqLen = c_arraySize((c_array)src);

    ALLOC_BUFFER(dst, seqLen, sh->userTypeSize, NULL);

    buffer = dst->_buffer;

    if ( buffer ) {
        for (i = 0; i < seqLen; i++) {
            ctx->dst_correction = 0;
            coFromArray[sech->copyType] (sech, src, buffer, ctx);
            buffer = (void *)((PA_ADDRCAST)buffer + sh->userTypeSize);
            src = (void *)((PA_ADDRCAST)src + sh->baseTypeSize);
        }
        dst->_length = seqLen;
    }

    ctx->dst_correction = dst_correction + GAPI_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Object sequence size %d @ offset = %d\n",
                            sh->baseTypeSize, ctx->src_offset));
}
#endif
STATIC void
gapi_cfooSequence (
    gapiCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    gapi_co_context *ctx)
{
    gapi_co_context context;
    gapiCopyObjectSequence *sh;
    gapiCopyHeader *sech;
    void *src;
    c_ulong i;
    c_ulong seqLen;
    gapiSequenceType *dst;
    void *buffer;

    sh = (gapiCopyObjectSequence *)ch;
    sech = gapiCopyObjectSequenceDescription (sh);
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

    ctx->dst_correction += GAPI_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied out Object sequence size %d @ offset = %d\n",
                            seqLen, ctx->src_offset));
}


STATIC void
gapi_cfsoSequence (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapi_cfooSequence(ch, (void*)((PA_ADDRCAST)src+ ctx->src_offset),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->src_offset + ctx->dst_correction), ctx);
}

STATIC void
gapi_cfuoSequence (
    gapiCopyHeader *ch,
    void * src,

    gapi_co_context *ctx)
{
    gapi_cfooSequence (ch, src, ctx->dst, ctx);
}

    /* backward referenced type */
STATIC void
gapi_cfooReference (
    gapiCopyHeader *ch,
    void * src,
    void *dst,
    gapi_co_context *ctx)
{
    gapiCopyReference *ref;
    gapiCopyHeader *nch;

    ref = (gapiCopyReference *)ch;
    nch = gapiCopyReferencedObject (ref);
    coFromArray[nch->copyType] (nch, src, dst, ctx);
}

STATIC void
gapi_cfsoReference (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapiCopyReference *ref;
    gapiCopyHeader *nch;

    ref = (gapiCopyReference *)ch;
    nch = gapiCopyReferencedObject (ref);
    coFromStruct[nch->copyType] (nch, src, ctx);
}

STATIC void
gapi_cfuoReference (
    gapiCopyHeader *ch,
    void * src,
    gapi_co_context *ctx)
{
    gapiCopyReference *ref;
    gapiCopyHeader *nch;

    ref = (gapiCopyReference *)ch;
    nch = gapiCopyReferencedObject (ref);
    coFromUnion[nch->copyType] (nch, src, ctx);
}

void
gapi_copyOutStruct (
    void *src,
    void *dst)
{
    gapi_co_context context;
    gapi_dstInfo dstInfo = (gapi_dstInfo)dst;
    gapiCopyHeader *ch;

    ch = gapi_copyCacheCache(dstInfo->copyProgram);
    context.copyCache = dstInfo->copyProgram;
    context.dst = dst;
    context.src_offset = 0;
    context.dst_correction = 0;
    coFromArray[ch->copyType] (ch, src, dstInfo->dst, &context);
}

void *
gapi_copyOutAllocBuffer (
    gapi_copyCache copyCache,
    gapi_unsigned_long len)
{
    gapi_unsigned_long size;
    gapiCopyHeader *ch;

    ch = gapi_copyCacheCache(copyCache);

    size = gapi_copyCacheGetUserSize(copyCache);

    assert(size > 0);

    return gapi_genericCopyBufferAllocSeqBuffer(copyCache, ch, size, len);
}

