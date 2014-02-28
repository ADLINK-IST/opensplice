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

#include "gapi_common.h"
#include "gapi_genericCopyBuffer.h"
#include "gapi_genericCopyIn.h"

#include "c_base.h"

#include "os_abstract.h"
#include "os_report.h"
#include "os_stdlib.h"

#include "gapi.h"

#define TRACE(function) /* function */
#define STATIC static

typedef struct {
    void *dst;
    c_base base;
    c_long dst_offset;
    c_long src_correction;
} gapi_ci_context;

typedef gapi_boolean (*copyInFromStruct)(gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
typedef gapi_boolean (*copyInFromUnion)(gapiCopyHeader *ch, void * src, gapi_ci_context *ctx);
typedef gapi_boolean (*copyInFromArray)(gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_ci_context *ctx);

    /* blackBox, copy as one block of data */
STATIC gapi_boolean gapi_cfsiBlackBox    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* Primitive types */
STATIC gapi_boolean gapi_cfsiBoolean    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiByte       (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiChar       (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiShort      (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiInt        (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiLong       (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiFloat      (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiDouble     (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* Array of primitive type */
STATIC gapi_boolean gapi_cfsiArrBoolean (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiArrByte    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiArrChar    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiArrShort   (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiArrInt     (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiArrLong    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiArrFloat   (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiArrDouble  (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* Sequence of primitive type */
STATIC gapi_boolean gapi_cfsiSeqBoolean (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiSeqByte    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiSeqChar    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiSeqShort   (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiSeqInt     (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiSeqLong    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiSeqFloat   (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiSeqDouble  (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* Enumeration type */
STATIC gapi_boolean gapi_cfsiEnum       (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* Structured types */
STATIC gapi_boolean gapi_cfsiStruct     (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiUnion      (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* String types */
STATIC gapi_boolean gapi_cfsiString     (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfsiBString    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* Array of object type */
STATIC gapi_boolean gapi_cfsiArray      (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* Sequence of object type */
STATIC gapi_boolean gapi_cfsiSequence   (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* reference to previous defined type */
STATIC gapi_boolean gapi_cfsiReference  (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);

    /* blackBox, copy as one block of data */
STATIC gapi_boolean gapi_cfuiBlackBox    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* Primitive types */
STATIC gapi_boolean gapi_cfuiBoolean    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiByte       (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiChar       (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiShort      (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiInt        (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiLong       (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiFloat      (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiDouble     (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* Array of primitive type */
STATIC gapi_boolean gapi_cfuiArrBoolean (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiArrByte    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiArrChar    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiArrShort   (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiArrInt     (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiArrLong    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiArrFloat   (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiArrDouble  (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* Sequence of primitive type */
STATIC gapi_boolean gapi_cfuiSeqBoolean (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiSeqByte    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiSeqChar    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiSeqShort   (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiSeqInt     (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiSeqLong    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiSeqFloat   (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiSeqDouble  (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* Enumeration type */
STATIC gapi_boolean gapi_cfuiEnum       (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* Structured types */
STATIC gapi_boolean gapi_cfuiStruct     (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiUnion      (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* String types */
STATIC gapi_boolean gapi_cfuiString     (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfuiBString    (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* Array of object type */
STATIC gapi_boolean gapi_cfuiArray      (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* Sequence of object type */
STATIC gapi_boolean gapi_cfuiSequence   (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);
    /* reference to previous defined type */
STATIC gapi_boolean gapi_cfuiReference  (gapiCopyHeader *ch, void * src,  gapi_ci_context *ctx);

    /* blackBox, copy as one block of data */
STATIC gapi_boolean gapi_cfoiBlackBox       (gapiCopyHeader *ch, void * srcBlock, void *dstBlock, gapi_ci_context *ctx);
    /* Array of primitive type */
STATIC gapi_boolean gapi_cfoiArrBoolean (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiArrByte    (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiArrChar    (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiArrShort   (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiArrInt     (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiArrLong    (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiArrFloat   (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiArrDouble  (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_ci_context *ctx);
    /* Sequence of primitive type */
STATIC gapi_boolean gapi_cfoiSeqBoolean (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiSeqByte    (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiSeqChar    (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiSeqShort   (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiSeqInt     (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiSeqLong    (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiSeqFloat   (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiSeqDouble  (gapiCopyHeader *ch, void * srcArray, void *dstSeq, gapi_ci_context *ctx);
    /* Enumeration type */
STATIC gapi_boolean gapi_cfoiEnum       (gapiCopyHeader *ch, void * srcEnum, void *dstEnum, gapi_ci_context *ctx);
    /* Structured types */
STATIC gapi_boolean gapi_cfoiStruct     (gapiCopyHeader *ch, void * srcStruct, void *dstStruct, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiUnion      (gapiCopyHeader *ch, void * srcUnion,  void *dstUnion,  gapi_ci_context *ctx);
    /* String types */
STATIC gapi_boolean gapi_cfoiString     (gapiCopyHeader *ch, void * srcString, void *dstString, gapi_ci_context *ctx);
STATIC gapi_boolean gapi_cfoiBString    (gapiCopyHeader *ch, void * srcString, void *dstString, gapi_ci_context *ctx);
    /* Array of object type */
STATIC gapi_boolean gapi_cfoiArray      (gapiCopyHeader *ch, void * srcArray, void *dstArray, gapi_ci_context *ctx);
    /* Sequence of object type */
STATIC gapi_boolean gapi_cfoiSequence   (gapiCopyHeader *ch, void * srcSeq, void *dstSeq, gapi_ci_context *ctx);
    /* reference to previous defined type */
STATIC gapi_boolean gapi_cfoiReference  (gapiCopyHeader *ch, void * src, void *dst, gapi_ci_context *ctx);

STATIC copyInFromStruct ciFromStruct[] = {
    gapi_cfsiBlackBox,
    gapi_cfsiBoolean,
    gapi_cfsiByte,
    gapi_cfsiChar,
    gapi_cfsiShort,
    gapi_cfsiInt,
    gapi_cfsiLong,
    gapi_cfsiFloat,
    gapi_cfsiDouble,
    gapi_cfsiArrBoolean,
    gapi_cfsiArrByte,
    gapi_cfsiArrChar,
    gapi_cfsiArrShort,
    gapi_cfsiArrInt,
    gapi_cfsiArrLong,
    gapi_cfsiArrFloat,
    gapi_cfsiArrDouble,
    gapi_cfsiSeqBoolean,
    gapi_cfsiSeqByte,
    gapi_cfsiSeqChar,
    gapi_cfsiSeqShort,
    gapi_cfsiSeqInt,
    gapi_cfsiSeqLong,
    gapi_cfsiSeqFloat,
    gapi_cfsiSeqDouble,
    gapi_cfsiEnum,
    gapi_cfsiStruct,
    gapi_cfsiUnion,
    gapi_cfsiString,
    gapi_cfsiBString,
    gapi_cfsiArray,
    gapi_cfsiSequence,
    gapi_cfsiReference
    };

STATIC copyInFromUnion ciFromUnion[] = {
    gapi_cfuiBlackBox,
    gapi_cfuiBoolean,
    gapi_cfuiByte,
    gapi_cfuiChar,
    gapi_cfuiShort,
    gapi_cfuiInt,
    gapi_cfuiLong,
    gapi_cfuiFloat,
    gapi_cfuiDouble,
    gapi_cfuiArrBoolean,
    gapi_cfuiArrByte,
    gapi_cfuiArrChar,
    gapi_cfuiArrShort,
    gapi_cfuiArrInt,
    gapi_cfuiArrLong,
    gapi_cfuiArrFloat,
    gapi_cfuiArrDouble,
    gapi_cfuiSeqBoolean,
    gapi_cfuiSeqByte,
    gapi_cfuiSeqChar,
    gapi_cfuiSeqShort,
    gapi_cfuiSeqInt,
    gapi_cfuiSeqLong,
    gapi_cfuiSeqFloat,
    gapi_cfuiSeqDouble,
    gapi_cfuiEnum,
    gapi_cfuiStruct,
    gapi_cfuiUnion,
    gapi_cfuiString,
    gapi_cfuiBString,
    gapi_cfuiArray,
    gapi_cfuiSequence,
    gapi_cfuiReference
    };

STATIC copyInFromUnion ciUnionDiscr[] = {
    NULL, /*gapi_cfuiBlackBox*/
    gapi_cfuiBoolean,
    NULL, /*gapi_cfuiByte*/
    gapi_cfuiChar,
    gapi_cfuiShort,
    gapi_cfuiInt,
    gapi_cfuiLong,
    NULL, /*gapi_cfuiFloat*/
    NULL, /*gapi_cfuiDouble*/
    NULL, /*gapi_cfuiArrBoolean*/
    NULL, /*gapi_cfuiArrByte*/
    NULL, /*gapi_cfuiArrChar*/
    NULL, /*gapi_cfuiArrShort*/
    NULL, /*gapi_cfuiArrInt*/
    NULL, /*gapi_cfuiArrLong*/
    NULL, /*gapi_cfuiArrFloat*/
    NULL, /*gapi_cfuiArrDouble*/
    NULL, /*gapi_cfuiSeqBoolean*/
    NULL, /*gapi_cfuiSeqByte*/
    NULL, /*gapi_cfuiSeqChar*/
    NULL, /*gapi_cfuiSeqShort*/
    NULL, /*gapi_cfuiSeqInt*/
    NULL, /*gapi_cfuiSeqLong*/
    NULL, /*gapi_cfuiSeqFloat*/
    NULL, /*gapi_cfuiSeqDouble*/
    gapi_cfuiEnum,
    NULL, /*gapi_cfuiStruct*/
    NULL, /*gapi_cfuiUnion*/
    NULL, /*gapi_cfuiString*/
    NULL, /*gapi_cfuiBString*/
    NULL, /*gapi_cfuiArray*/
    NULL, /*gapi_cfuiSequence*/
    NULL /*gapi_cfuiReference*/
    };


STATIC copyInFromArray ciFromArray[] = {
    gapi_cfoiBlackBox,
    NULL, /* gapi_cfoiBoolean */
    NULL, /* gapi_cfoiByte */
    NULL, /* gapi_cfoiChar */
    NULL, /* gapi_cfoiShort */
    NULL, /* gapi_cfoiInt */
    NULL, /* gapi_cfoiLong */
    NULL, /* gapi_cfoiFloat */
    NULL, /* gapi_cfoiDouble */
    gapi_cfoiArrBoolean,
    gapi_cfoiArrByte,
    gapi_cfoiArrChar,
    gapi_cfoiArrShort,
    gapi_cfoiArrInt,
    gapi_cfoiArrLong,
    gapi_cfoiArrFloat,
    gapi_cfoiArrDouble,
    gapi_cfoiSeqBoolean,
    gapi_cfoiSeqByte,
    gapi_cfoiSeqChar,
    gapi_cfoiSeqShort,
    gapi_cfoiSeqInt,
    gapi_cfoiSeqLong,
    gapi_cfoiSeqFloat,
    gapi_cfoiSeqDouble,
    gapi_cfoiEnum,
    gapi_cfoiStruct,
    gapi_cfoiUnion,
    gapi_cfoiString,
    gapi_cfoiBString,
    gapi_cfoiArray,
    gapi_cfoiSequence,
    gapi_cfoiReference
    };

STATIC gapiCopyType
to_copyType(c_type t)
{
    gapiCopyType ct = gapiBlackBox;
    switch(c_baseObject(t)->kind ) {
    case M_ENUMERATION:
        ct = gapiEnum;
    break;
    case M_PRIMITIVE:
        switch (c_primitive (t)->kind) {
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
            assert (0);
        }
    break;
    default:
        assert (0);
    }
    return ct;
}

    /* Primitive types */
STATIC gapi_boolean
gapi_cfsiBoolean (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    c_bool *dst = (c_bool *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_bool *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    TRACE(printf ("Copied in Boolean = %d @ offset = %d\n", *dst, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfuiBoolean (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    c_bool *dst = (c_bool *)ctx->dst;
    *dst = *(c_bool *)src;

    TRACE(printf ("Copied in Boolean = %d\n", *dst));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiByte (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    c_octet *dst = (c_octet *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_octet *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    TRACE(printf ("Copied in BYTE = %hd @ offset = %d\n", *dst, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfuiByte (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    c_octet *dst = (c_octet *)ctx->dst;
    *dst = *(c_octet *)src;

    TRACE(printf ("Copied in Byte = %hd\n", *dst));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiChar (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    c_char *dst = (c_char *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_char *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    TRACE(printf ("Copied in Char = %hd @ offset = %d\n", *dst, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfuiChar (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    c_char *dst = (c_char *)ctx->dst;
    *dst = *(c_char *)src;

    TRACE(printf ("Copied in Char = %hd\n", *dst));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiShort (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    c_short *dst = (c_short *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_short *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    TRACE(printf ("Copied in Chort = %hd @ offset = %d\n", *dst, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfuiShort (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    c_short *dst = (c_short *)ctx->dst;
    *dst = *(c_short *)src;

    TRACE(printf ("Copied in Short = %hd\n", *dst));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiInt (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    c_long *dst = (c_long *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_long *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    TRACE(printf ("Copied in Int = %d @ offset = %d\n", *dst, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfuiInt (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    c_long *dst = (c_long *)ctx->dst;
    *dst = *(c_long *)src;

    TRACE(printf ("Copied in Int = %d\n", *dst));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiLong (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    TRACE (char llstr[36];)

    c_longlong *dst = (c_longlong *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_longlong *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    TRACE(llstr[35] = '\0'; printf ("Copied in Long = %s @ offset = %d\n", os_lltostr(*dst, &llstr[35]), ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfuiLong (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    TRACE (char llstr[36];)

    c_longlong *dst = (c_longlong *)ctx->dst;
    *dst = *(c_longlong *)src;

    TRACE(llstr[35] = '\0'; printf ("Copied in Long = %s\n", os_lltostr(*dst, &llstr[35])));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiFloat (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    c_float *dst = (c_float *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_float *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    TRACE(printf ("Copied in Float = %f @ offset = %d\n", *dst, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfuiFloat (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    c_float *dst = (c_float *)ctx->dst;
    *dst = *(c_float *)src;

    TRACE(printf ("Copied in Float = %f\n", *dst));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiDouble (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    c_double *dst = (c_double *)((PA_ADDRCAST)ctx->dst + ctx->dst_offset);
    *dst = *(c_double *)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction);

    TRACE(printf ("Copied in Double = %f @ offset = %d\n", *dst, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfuiDouble (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    c_double *dst = (c_double *)ctx->dst;
    *dst = *(c_double *)src;

    TRACE(printf ("Copied in Double = %f\n", *dst));
    return TRUE;
}

    /* Enumeration type */
STATIC gapi_boolean
gapi_cfoiEnum (
    gapiCopyHeader *ch,
    void *srcEnum,
    void *dstEnum,
    gapi_ci_context *ctx)
{
    c_long *dst;
    gapiCopyEnum *ce;
    c_bool result;

    ce = (gapiCopyEnum *)ch;
    dst = (c_long *)dstEnum;

    if(((*(c_long *)srcEnum) < 0) || ((*(c_long *)srcEnum) >= (c_long)ce->nrOfElements)){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "Invalid enumeration label.");
        result = FALSE;
    } else {
        *dst = *(c_long *)srcEnum;
        TRACE(printf ("Copied in in Enum = %d @ offset = %d\n", *dst, ctx->dst_offset));
        result = TRUE;
    }
    return result;
}

STATIC gapi_boolean
gapi_cfsiEnum (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiEnum (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                       (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiEnum (
    gapiCopyHeader *ch,
    void * src,

    gapi_ci_context *ctx)
{
    return gapi_cfoiEnum (ch, src, ctx->dst, ctx);
}

    /* BlackBox data type */
STATIC c_bool
gapi_cfoiBlackBox (
    gapiCopyHeader *ch,
    void *srcBlock,
    void *dstBlock,
    gapi_ci_context *ctx)
{
    gapiCopyBlackBox *bbh;

    bbh=(gapiCopyBlackBox *)ch;
    memcpy(dstBlock,srcBlock,bbh->size);
    TRACE(printf ("Copied in BlackBox size = %d @ offset = %d\n", bbh->size, ctx->dst_offset));

    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiBlackBox (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiBlackBox (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                           (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiBlackBox (
    gapiCopyHeader *ch,
    void * src,

    gapi_ci_context *ctx)
{
    return gapi_cfoiBlackBox (ch, src, ctx->dst, ctx);
}

    /* Array of primitive type */
STATIC gapi_boolean
gapi_cfoiArrBoolean (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_ci_context *ctx)
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
    TRACE(printf ("Copied in Boolean array size %d @ offset = %d\n",
                               ah->size, ctx->dst_offset));

    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiArrBoolean (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrBoolean (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                             (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiArrBoolean (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrBoolean (ch, src, ctx->dst, ctx);
}



STATIC gapi_boolean
gapi_cfoiArrByte (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_ci_context *ctx)
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
    TRACE(printf ("Copied in Byte array size %d @ offset = %d\n",
                            ah->size, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiArrByte (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrByte (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiArrByte (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrByte (ch, src, ctx->dst, ctx);
}

STATIC gapi_boolean
gapi_cfoiArrChar (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_ci_context *ctx)
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
    TRACE(printf ("Copied in Char array size %d @ offset = %d\n",
                            ah->size, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiArrChar (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrChar (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiArrChar (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrChar (ch, src, ctx->dst, ctx);
}

STATIC gapi_boolean
gapi_cfoiArrShort (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_ci_context *ctx)
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
    TRACE(printf ("Copied in Short array size %d @ offset = %d\n",
                            ah->size, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiArrShort (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrShort (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                           (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiArrShort (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrShort (ch, src, ctx->dst, ctx);
}

STATIC gapi_boolean
gapi_cfoiArrInt (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_ci_context *ctx)
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
    TRACE(printf ("Copied in Int array size %d @ offset = %d\n",
                            ah->size, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiArrInt (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrInt (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                         (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiArrInt (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrInt (ch, src, ctx->dst, ctx);
}

STATIC gapi_boolean
gapi_cfoiArrLong (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_ci_context *ctx)
{
    gapiCopyArray *ah;
    c_longlong *dst = dstArray;
    c_longlong *src = srcArray;
    unsigned int i;

    TRACE (char llstr[36];)

    ah = (gapiCopyArray *)ch;
    for (i = 0; i < ah->size; i++) {
        dst[i] = src[i];
        TRACE(llstr[35] = '\0'; printf ("%s @ [%d];", os_lltostr(*dst, dst[i]), i));
    }
    TRACE(printf ("Copied in Long array size %d @ offset = %d\n",
                            ah->size, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiArrLong (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrLong (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiArrLong (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrLong (ch, src, ctx->dst, ctx);
}

STATIC gapi_boolean
gapi_cfoiArrFloat (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_ci_context *ctx)
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
    TRACE(printf ("Copied in Float array size %d @ offset = %d\n",
                            ah->size, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiArrFloat (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrFloat (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                           (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiArrFloat (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrFloat (ch, src, ctx->dst, ctx);
}

STATIC gapi_boolean
gapi_cfoiArrDouble (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_ci_context *ctx)
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
    TRACE(printf ("Copied in Double array size %d @ offset = %d\n",
                            ah->size, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiArrDouble (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrDouble (ch, (void*)((PA_ADDRCAST)src + ctx->dst_offset + ctx->src_correction),
                            (void*)((PA_ADDRCAST)ctx->dst + ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiArrDouble (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArrDouble (ch, src, ctx->dst, ctx);
}

    /* Sequence of primitive type */
STATIC gapi_boolean
gapi_cfoiSeqBoolean (
    gapiCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    gapi_ci_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    unsigned int seqLen;
    c_bool **dst;
    gapiSequenceType * src;
    c_bool * buffer;
    unsigned int i;
    gapi_boolean result;

    sh = (gapiCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;


    if((sh->size) && (src->_maximum > ((gapi_unsigned_long)sh->size))){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Boolean Sequence).");
        result = FALSE;
    } else if(src->_maximum < src->_length){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Boolean Sequence).");
        result = FALSE;
    } else {
        arrLen = src->_length;
        seqLen = GAPI_MAX(sh->size, arrLen);

        *dst = (c_bool *)c_arrayNew (sh->type, arrLen);
        buffer  = (c_bool *)src->_buffer;

        for (i = 0; i < arrLen; i++) {
            (*dst)[i] = buffer[i];
            TRACE(printf ("%d;", (*dst)[i]));
        }
        ctx->src_correction += GAPI_SEQUENCE_CORRECTION;

        TRACE(printf ("Copied in Boolean sequence size %d @ offset = %d\n",
                                arrLen, ctx->dst_offset));
        result = TRUE;
    }
    return result;
}

STATIC gapi_boolean
gapi_cfsiSeqBoolean (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqBoolean (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                             (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiSeqBoolean (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqBoolean (ch, src,  ctx->dst, ctx);
}



STATIC gapi_boolean
gapi_cfoiSeqByte (
    gapiCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    gapi_ci_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    unsigned int seqLen;
    c_octet **dst;
    gapiSequenceType * src;
    c_octet * buffer;
    unsigned int i;
    gapi_boolean result;

    sh = (gapiCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((gapi_unsigned_long)sh->size))){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Byte Sequence).");
        result = FALSE;
    } else if(src->_maximum < src->_length){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Byte Sequence).");
        result = FALSE;
    } else {
        arrLen = src->_length;
        seqLen = GAPI_MAX(sh->size, arrLen);

        *dst = (c_octet *)c_arrayNew (sh->type, arrLen);
        buffer  = (c_octet *)src->_buffer;

        for (i = 0; i < arrLen; i++) {
            (*dst)[i] = buffer[i];
            TRACE(printf ("%d;", (*dst)[i]));
        }
        ctx->src_correction += GAPI_SEQUENCE_CORRECTION;

        TRACE(printf ("Copied in Byte sequence size %d @ offset = %d\n",
                                arrLen, ctx->dst_offset));
        result = TRUE;
    }
    return result;
}

STATIC gapi_boolean
gapi_cfsiSeqByte (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqByte (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiSeqByte (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqByte (ch, src,  ctx->dst, ctx);
}

STATIC gapi_boolean
gapi_cfoiSeqChar (
    gapiCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    gapi_ci_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    unsigned int seqLen;
    c_char **dst;
    gapiSequenceType * src;
    c_char * buffer;
    unsigned int i;
    gapi_boolean result;

    sh = (gapiCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((gapi_unsigned_long)sh->size))){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Char Sequence).");
        result = FALSE;
    } else if(src->_maximum < src->_length){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Char Sequence).");
        result = FALSE;
    } else {
        arrLen = src->_length;
        seqLen = GAPI_MAX(sh->size, arrLen);

        *dst = (c_char *)c_arrayNew (sh->type, arrLen);
        buffer  = (c_char *)src->_buffer;

        for (i = 0; i < arrLen; i++) {
            (*dst)[i] = buffer[i];
            TRACE(printf ("%d;", (*dst)[i]));
        }

        ctx->src_correction += GAPI_SEQUENCE_CORRECTION;

        TRACE(printf ("Copied in Char sequence size %d @ offset = %d\n",
                                arrLen, ctx->dst_offset));
        result = TRUE;
    }
    return result;
}

STATIC gapi_boolean
gapi_cfsiSeqChar (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqChar (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiSeqChar (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqChar (ch, src,  ctx->dst, ctx);
}

STATIC gapi_boolean
gapi_cfoiSeqShort (
    gapiCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    gapi_ci_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    unsigned int seqLen;
    c_short **dst;
    gapiSequenceType * src;
    c_short * buffer;
    unsigned int i;
    gapi_boolean result;

    sh = (gapiCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((gapi_unsigned_long)sh->size))){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Short Sequence).");
        result = FALSE;
    } else if(src->_maximum < src->_length){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Short Sequence).");
        result = FALSE;
    } else {
        arrLen = src->_length;
        seqLen = GAPI_MAX(sh->size, arrLen);

        *dst = (c_short *)c_arrayNew (sh->type, arrLen);
        buffer  = (c_short *)src->_buffer;

        for (i = 0; i < arrLen; i++) {
            (*dst)[i] = buffer[i];
            TRACE(printf ("%d;", (*dst)[i]));
        }

        ctx->src_correction += GAPI_SEQUENCE_CORRECTION;

        TRACE(printf ("Copied in Short sequence size %d @ offset = %d\n",
                                arrLen, ctx->dst_offset));
        result = TRUE;
    }
    return result;
}

STATIC gapi_boolean
gapi_cfsiSeqShort (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqShort (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                           (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiSeqShort (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqShort (ch, src,  ctx->dst, ctx);
}

STATIC gapi_boolean
gapi_cfoiSeqInt (
    gapiCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    gapi_ci_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    unsigned int seqLen;
    c_long **dst;
    gapiSequenceType * src;
    c_long * buffer;
    unsigned int i;
    gapi_boolean result;

    sh = (gapiCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((gapi_unsigned_long)sh->size))){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Int Sequence).");
        result = FALSE;
    } else if(src->_maximum < src->_length){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Int Sequence).");
        result = FALSE;
    } else {
        arrLen = src->_length;
        seqLen = GAPI_MAX(sh->size, arrLen);

        *dst = (c_long *)c_arrayNew (sh->type, arrLen);
        buffer  = (c_long *)src->_buffer;
        arrLen = src->_length;

        for (i = 0; i < arrLen; i++) {
            (*dst)[i] = buffer[i];
            TRACE(printf ("%d;", (*dst)[i]));
        }

        ctx->src_correction += GAPI_SEQUENCE_CORRECTION;

        TRACE(printf ("Copied in Int sequence size %d @ offset = %d\n",
                                arrLen, ctx->dst_offset));
        result = TRUE;
    }
    return result;
}

STATIC gapi_boolean
gapi_cfsiSeqInt (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqInt (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                         (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiSeqInt (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqInt (ch, src,  ctx->dst, ctx);
}

STATIC gapi_boolean
gapi_cfoiSeqLong (
    gapiCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    gapi_ci_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    unsigned int seqLen;
    c_longlong **dst;
    gapiSequenceType * src;
    c_longlong * buffer;
    unsigned int i;
    gapi_boolean result;

    sh = (gapiCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((gapi_unsigned_long)sh->size))){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Long Sequence).");
        result = FALSE;
    } else if(src->_maximum < src->_length){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Long Sequence).");
        result = FALSE;
    } else {
        arrLen = src->_length;
        seqLen = GAPI_MAX(sh->size, arrLen);

        *dst = (c_longlong *)c_arrayNew (sh->type, arrLen);
        buffer  = (c_longlong *)src->_buffer;

        for (i = 0; i < arrLen; i++) {
            (*dst)[i] = buffer[i];
            TRACE(printf ("%d;", (*dst)[i]));
        }

        ctx->src_correction += GAPI_SEQUENCE_CORRECTION;

        TRACE(printf ("Copied in Long sequence size %d @ offset = %d\n",
                                arrLen, ctx->dst_offset));
        result = TRUE;
    }
    return result;
}

STATIC gapi_boolean
gapi_cfsiSeqLong (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqLong (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiSeqLong (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqLong (ch, src,  ctx->dst, ctx);
}

STATIC gapi_boolean
gapi_cfoiSeqFloat (
    gapiCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    gapi_ci_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    unsigned int seqLen;
    c_float **dst;
    gapiSequenceType * src;
    c_float * buffer;
    unsigned int i;
    gapi_boolean result;

    sh = (gapiCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((gapi_unsigned_long)sh->size))){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Float Sequence).");
        result = FALSE;
    } else if(src->_maximum < src->_length){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Float Sequence).");
        result = FALSE;
    } else {
        arrLen = src->_length;
        seqLen = GAPI_MAX(sh->size, arrLen);

        *dst = (c_float *)c_arrayNew (sh->type, arrLen);
        buffer  = (c_float *)src->_buffer;

        for (i = 0; i < arrLen; i++) {
            (*dst)[i] = buffer[i];
            TRACE(printf ("%d;", (*dst)[i]));
        }

        ctx->src_correction += GAPI_SEQUENCE_CORRECTION;

        TRACE(printf ("Copied in Float sequence size %d @ offset = %d\n",
                                arrLen, ctx->dst_offset));
        result = TRUE;
    }
    return result;
}

STATIC gapi_boolean
gapi_cfsiSeqFloat (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqFloat (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                           (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiSeqFloat (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqFloat (ch, src,  ctx->dst, ctx);
}

STATIC gapi_boolean
gapi_cfoiSeqDouble (
    gapiCopyHeader *ch,
    void * srcSeq,
    void *dstSeq,
    gapi_ci_context *ctx)
{
    gapiCopySequence *sh;
    unsigned int arrLen;
    unsigned int seqLen;
    c_double **dst;
    gapiSequenceType * src;
    c_double * buffer;
    unsigned int i;
    gapi_boolean result;

    sh = (gapiCopySequence *)ch;
    dst = dstSeq;
    src = srcSeq;

    if((sh->size) && (src->_maximum > ((gapi_unsigned_long)sh->size))){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Double Sequence).");
        result = FALSE;
    } else if(src->_maximum < src->_length){
        OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (Double Sequence).");
        result = FALSE;
    } else {
        arrLen = src->_length;
        seqLen = GAPI_MAX(sh->size, arrLen);

        *dst = (c_double *)c_arrayNew (sh->type, arrLen);
        buffer  = (c_double *)src->_buffer;

        for (i = 0; i < arrLen; i++) {
            (*dst)[i] = buffer[i];
            TRACE(printf ("%d;", (*dst)[i]));
        }

        ctx->src_correction += GAPI_SEQUENCE_CORRECTION;

        TRACE(printf ("Copied in Double sequence size %d @ offset = %d\n",
                                arrLen, ctx->dst_offset));
        result = TRUE;
    }
    return result;
}

STATIC gapi_boolean
gapi_cfsiSeqDouble (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqDouble (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                            (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiSeqDouble (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSeqDouble (ch, src,  ctx->dst, ctx);
}

    /* Structured types */
STATIC gapi_boolean
gapi_cfoiStruct (
    gapiCopyHeader *ch,
    void *srcStruct,
    void *dstStruct,
    gapi_ci_context *ctx)
{
    gapi_ci_context context;
    unsigned long mi;
    gapiCopyStruct *csh;
    gapiCopyStructMember *csm;

    context.dst = dstStruct;
    context.base = ctx->base;
    context.src_correction = ctx->src_correction;
    csh = (gapiCopyStruct *)ch;

    csm = gapiCopyStructMemberObject (csh);

    for (mi = 0; mi < csh->nrOfMembers; mi++) {
        context.dst_offset = csm->memberOffset;
        ch = gapiCopyStructMemberDescription (csm);
        ciFromStruct[ch->copyType] (ch, srcStruct, &context);
        csm = (gapiCopyStructMember *)gapiCopyHeaderNextObject (ch);
    }
    ctx->src_correction += csh->userSize - csh->size;
    TRACE(printf ("Copied in Struct @ offset = %d\n", ctx->dst_offset));
    return TRUE;

}

STATIC gapi_boolean
gapi_cfsiStruct (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiStruct (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                         (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiStruct (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiStruct (ch, src,  ctx->dst, ctx);
}


STATIC gapi_boolean
gapi_cfoiUnion (
    gapiCopyHeader *ch,
    void *srcUnion,
    void *dstUnion,
    gapi_ci_context *ctx)
{
    gapi_ci_context context;
    unsigned long ci;
    gapiCopyUnion *cuh;
    gapiCopyUnionLabels *csl;
    unsigned long long discrVal;
    gapiCopyUnionLabel *defaultLabel = NULL;
    int active_case = 0;
    void * src;

    cuh = (gapiCopyUnion *)ch;

    assert(ciUnionDiscr[to_copyType(cuh->discrType)]);

    context.dst = dstUnion;
    ciUnionDiscr[to_copyType(cuh->discrType)] (ch, srcUnion, &context);

    discrVal = gapi_getUnionDescriptor(to_copyType(cuh->discrType), srcUnion);

    src = (void *)((PA_ADDRCAST)srcUnion + cuh->casesOffset);
    context.dst = (void *)((PA_ADDRCAST)dstUnion + cuh->casesOffset);
    context.base = ctx->base;
    context.dst_offset = 0;
    context.src_correction = 0;
    csl = gapiCopyUnionLabelsObject (cuh);
    ci = 0;
    while (ci < cuh->nrOfCases) {
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
            defaultLabel = (gapiCopyUnionLabel *)gapiCopyUnionCaseObject(csl);
        }
        ch = gapiCopyUnionCaseObject(csl);
        if (active_case) {
            ciFromUnion[ch->copyType] (ch, src, &context);
            ci = cuh->nrOfCases;
        } else {
            ci++;
        }
        csl = (gapiCopyUnionLabels *)gapiCopyHeaderNextObject (ch);
    }
    if (!active_case && defaultLabel) {
        ch = (gapiCopyHeader *)defaultLabel;
        ciFromUnion[ch->copyType] (ch, src, &context);
    }
    ctx->src_correction += cuh->userSize - cuh->size;
    TRACE(printf ("Copied in Union\n"));

    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiUnion (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiUnion (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                        (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiUnion (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiUnion (ch, src,  ctx->dst, ctx);
}

    /* String types */
STATIC gapi_boolean
gapi_cfoiString (
    gapiCopyHeader *ch,
    void *srcString,
    void *dstString,
    gapi_ci_context *ctx)
{
    c_string *dst;
    gapi_boolean result;

    if((*(c_char **)srcString) != NULL){
        dst = (c_string *)(dstString);
        *dst = c_stringNew (ctx->base, *(c_char **)srcString);
        TRACE(printf ("Copied in string = %s @ offset = %d\n",
                                   *dst, ctx->dst_offset));
        result = TRUE;
    } else {
        dst = (c_string *)(dstString);
        *dst = c_stringNew (ctx->base, "");
        result = TRUE;
    }
    return result;
}

STATIC gapi_boolean
gapi_cfsiString (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiString (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                         (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiString (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiString (ch, src, ctx->dst, ctx);
}

STATIC gapi_boolean
gapi_cfoiBString (
    gapiCopyHeader *ch,
    void *srcString,
    void *dstString,
    gapi_ci_context *ctx)
{
    c_string *dst;
    gapiCopyBoundedString* cb;
    unsigned int length;
    gapi_boolean result;

    if((*(c_char **)srcString) != NULL){
        cb = (gapiCopyBoundedString*)ch;
        length = (unsigned int)(strlen(*(c_char **)srcString));

        if(length <= cb->max){
            dst = (c_string *)(dstString);
            *dst = c_stringNew (ctx->base, *(c_char **)srcString);
            TRACE(printf ("Copied in string = %s @ offset = %d\n",
                                    *dst, ctx->dst_offset));
            result = TRUE;
        } else {
            OS_REPORT(OS_ERROR, "dcpsgapi", 0, "CopyIn: Array bounds write (bounded string).");
            result = FALSE;
        }
    } else {
        dst = (c_string *)(dstString);
        *dst = c_stringNew (ctx->base, "");
        result = TRUE;
    }
    return result;
}

STATIC gapi_boolean
gapi_cfsiBString (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiBString (ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiBString (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiBString (ch, src, ctx->dst, ctx);
}

    /* Array of object type */
STATIC gapi_boolean
gapi_cfoiArray (
    gapiCopyHeader *ch,
    void * srcArray,
    void *dstArray,
    gapi_ci_context *ctx)
{
    gapiCopyObjectArray *ah;
    gapiCopyHeader *aech;
    void *dst;
    void *src;
    unsigned int i;
    c_long old_src_correction;

    src = srcArray;
    dst = dstArray;
    ah = (gapiCopyObjectArray *)ch;
    aech = gapiCopyObjectArrayDescription (ah);
    old_src_correction = ctx->src_correction;
    for (i = 0; i < ah->arraySize; i++) {
        ciFromArray[aech->copyType] (aech, src, dst, ctx);
        dst = (void *)((PA_ADDRCAST)dst + ah->typeSize);
        src = (void *)((PA_ADDRCAST)src + ah->typeSize + ctx->src_correction - old_src_correction);
        old_src_correction = ctx->src_correction;
    }
    TRACE(printf ("Copied in Object array size %d @ offset = %d\n",
                                ah->typeSize, ctx->dst_offset));
    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiArray (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArray(ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                       (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiArray (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiArray (ch, src, ctx->dst, ctx);
}

    /* Sequence of object type */
STATIC gapi_boolean
gapi_cfoiSequence (
    gapiCopyHeader *ch,
    void *srcSeq,
    void *dstSeq,
    gapi_ci_context *ctx)
{
    gapiCopyObjectSequence *sh;
    gapiCopyHeader *sech;
    void *dst;
    c_long i;
    c_long seqLen;
    gapiSequenceType *src;
    void *buffer;
    c_long src_correction;

    sh = (gapiCopyObjectSequence *)ch;
    sech = gapiCopyObjectSequenceDescription (sh);
    src = srcSeq;

    seqLen = src->_length;
    *(c_array *)dstSeq = c_arrayNew (sh->type, seqLen);

    src_correction = ctx->src_correction;

    dst = (void *)*(c_array *)dstSeq;
    buffer = src->_buffer;

    for (i = 0; i < seqLen; i++) {
        ctx->src_correction = 0;
        ciFromArray[sech->copyType] (sech, buffer, dst, ctx);
        dst = (void *)((PA_ADDRCAST)dst + sh->baseTypeSize);
        buffer = (void *)((PA_ADDRCAST)buffer + sh->userTypeSize);
    }

    ctx->src_correction = src_correction + GAPI_SEQUENCE_CORRECTION;

    TRACE(printf ("Copied in Object sequence size %d @ offset = %d\n",
                            sh->baseTypeSize, ctx->dst_offset));

    return TRUE;
}

STATIC gapi_boolean
gapi_cfsiSequence (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    return gapi_cfoiSequence(ch, (void*)((PA_ADDRCAST)src+ ctx->dst_offset + ctx->src_correction),
                          (void*)((PA_ADDRCAST)ctx->dst+ ctx->dst_offset), ctx);
}

STATIC gapi_boolean
gapi_cfuiSequence (
    gapiCopyHeader *ch,
    void * src,

    gapi_ci_context *ctx)
{
    return gapi_cfoiSequence (ch, src, ctx->dst, ctx);
}

    /* backward referenced type */
STATIC gapi_boolean
gapi_cfoiReference (
    gapiCopyHeader *ch,
    void * src,
    void *dst,
    gapi_ci_context *ctx)
{
    gapiCopyReference *ref;
    gapiCopyHeader *nch;

    ref = (gapiCopyReference *)ch;
    nch = gapiCopyReferencedObject (ref);
    return ciFromArray[nch->copyType] (nch, src, dst, ctx);
}

STATIC gapi_boolean
gapi_cfsiReference (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    gapiCopyReference *ref;
    gapiCopyHeader *nch;

    ref = (gapiCopyReference *)ch;
    nch = gapiCopyReferencedObject (ref);
    return ciFromStruct[nch->copyType] (nch, src, ctx);
}

STATIC gapi_boolean
gapi_cfuiReference (
    gapiCopyHeader *ch,
    void * src,
    gapi_ci_context *ctx)
{
    gapiCopyReference *ref;
    gapiCopyHeader *nch;

    ref = (gapiCopyReference *)ch;
    nch = gapiCopyReferencedObject (ref);
    return ciFromUnion[nch->copyType] (nch, src, ctx);
}

gapi_boolean
gapi_copyInStruct (
    c_base base,
    void *src,
    void *dst)
{
    gapi_ci_context context;
    gapi_srcInfo srcInfo = (gapi_srcInfo)src;
    gapiCopyHeader *ch;

    ch = gapi_copyCacheCache(srcInfo->copyProgram);
    context.dst = dst;
    context.dst_offset = 0;
    context.src_correction = 0;
    context.base = base;
    return ciFromArray[ch->copyType] (ch, srcInfo->src, dst, &context);
}
