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
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_abstract.h"
#include "os_report.h"
#include "c_base.h"
#include "c_iterator.h"

#include "gapi_genericCopyBuffer.h"
#include "gapi_genericCopyCache.h"

#define TRACE(function)     /* function */
#define CACHE_BLOCKSIZE     (100)
#define STATIC

C_CLASS(gapi_context);
#define gapi_context(o)     ((gapi_context)(o))

C_STRUCT(gapi_context) {
    gapi_copyCache  copyCache;
    c_iter      typeStack;
};

C_STRUCT(gapi_copyCache) {
    void         *cache;   /* start of cache  */
    c_long       length;   /* length of cache */
    c_long       iWrite;   /* write index     */
    unsigned int refCount;
    unsigned int userSize;
};

C_CLASS(gapi_typeHistory);
#define gapi_typeHistory(o) ((gapi_typeHistory)(o))

C_STRUCT(gapi_typeHistory) {
    c_metaObject    metaObject;
    c_long      cacheIndex;
};

gapi_typeHistory
gapi_typeHistoryNew (
    const c_metaObject metaObject,
    c_long index)
{
    gapi_typeHistory history = os_malloc (C_SIZEOF(gapi_typeHistory));

    history->metaObject = metaObject;
    history->cacheIndex = index;

    return history;
}

void
gapi_typeHistoryFree (
    gapi_typeHistory history)
{
    os_free (history);
}

c_long
gapi_typeHistoryIndex (
    gapi_typeHistory history)
{
    return history->cacheIndex;
}

STATIC void gapi_copyCacheBuild (gapi_copyCache copyCache, c_metaObject object);
STATIC c_long gapi_copyCacheWrite (gapi_copyCache copyCache, void *data, c_long size);
STATIC void gapi_copyCacheWriteIndex (gapi_copyCache copyCache, void *data, c_long size, c_long index);
STATIC void gapi_copyCacheFinalize (gapi_copyCache copyCache);

STATIC void gapi_metaObjectBuild (c_type o, gapi_context context);
STATIC void gapi_cacheStructBuild (c_structure o, gapi_context context);
STATIC void gapi_cacheStructMember (c_member o, gapi_context context);
STATIC void gapi_cacheUnionLabel (c_literal lit, gapi_context ctx);
STATIC void gapi_cacheUnionBuild (c_union o, gapi_context context);
STATIC void gapi_cacheBlackBoxBuild (unsigned int size, gapi_context ctx);
STATIC void gapi_cacheBooleanBuild (c_primitive o, gapi_context context);
STATIC void gapi_cacheByteBuild (c_primitive o, gapi_context context);
STATIC void gapi_cacheCharBuild (c_primitive o, gapi_context context);
STATIC void gapi_cacheShortBuild (c_primitive o, gapi_context context);
STATIC void gapi_cacheIntBuild (c_primitive o, gapi_context context);
STATIC void gapi_cacheLongBuild (c_primitive o, gapi_context context);
STATIC void gapi_cacheFloatBuild (c_primitive o, gapi_context context);
STATIC void gapi_cacheDoubleBuild (c_primitive o, gapi_context context);
STATIC void gapi_cacheArrBooleanBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheArrByteBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheArrCharBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheArrShortBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheArrIntBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheArrLongBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheArrFloatBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheArrDoubleBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheSeqBooleanBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheSeqByteBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheSeqCharBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheSeqShortBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheSeqIntBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheSeqLongBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheSeqFloatBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheSeqDoubleBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheStringBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheBStringBuild (c_collectionType o, gapi_context context);
STATIC void gapi_cacheEnumBuild (c_enumeration o, gapi_context ctx);
STATIC void gapi_cacheArrObjectBuild (c_collectionType o, gapi_context ctx);
STATIC void gapi_cacheSeqObjectBuild (c_collectionType o, gapi_context ctx);
STATIC unsigned int gapi_cacheObjectUserSize (c_type o);

gapi_copyCache
gapi_copyCacheNew (
    c_metaObject object)
{
    gapi_copyCache copyCache = os_malloc (C_SIZEOF(gapi_copyCache));

    if (copyCache) {
        copyCache->cache = os_malloc (CACHE_BLOCKSIZE);
        if (copyCache->cache) {
            copyCache->length   = CACHE_BLOCKSIZE;
            copyCache->iWrite   = 0;
            copyCache->refCount = 1;
            gapi_copyCacheBuild (copyCache, object);
        } else {
            os_free (copyCache);
            copyCache = NULL;
        }
    }
    return copyCache;
}

gapi_copyCache
gapi_copyCacheDup (
    gapi_copyCache copyCache)
{
    gapi_copyCache new_cc = NULL;
    
    if (copyCache){
        new_cc = os_malloc (C_SIZEOF(gapi_copyCache));
        if (new_cc) {
            new_cc->cache = os_malloc(copyCache->length);
            if(new_cc->cache) {
                memcpy(new_cc->cache,copyCache->cache,copyCache->length);
                new_cc->length   = copyCache->length;
                new_cc->iWrite   = copyCache->iWrite;
                new_cc->refCount = 1;
            }else{
                os_free(new_cc);
                new_cc = NULL;
            }
        }
    }
    return new_cc;
}

void
gapi_copyCacheFree (
    gapi_copyCache copyCache)
{
    assert (copyCache);
    assert(copyCache->refCount > 0);

    copyCache->refCount--;
    if ( copyCache->refCount <= 0 ) {
        os_free (copyCache->cache);
        os_free (copyCache);
    }
}

void 
gapi_copyCacheIncRef (
    gapi_copyCache copyCache)
{
    assert (copyCache);
    assert(copyCache->refCount > 0);

    copyCache->refCount++;
}



c_long
gapi_copyCacheLength (
    gapi_copyCache copyCache)
{
    assert (copyCache);

    return copyCache->iWrite;
}

void *
gapi_copyCacheCache (
    gapi_copyCache copyCache)
{
    assert (copyCache);

    return copyCache->cache;
}

c_long
gapi_copyCacheWrite (
    gapi_copyCache copyCache,
    void *data,
    c_long size)
{
    c_long additionalLength;
    void *newCache;
    c_long writeIndex = copyCache->iWrite;

    assert (copyCache);
    assert (data);
    assert (size);

    if ((copyCache->iWrite + size) >= copyCache->length) {
        /* If new data does not fit, allocate new cache */
        additionalLength = ((copyCache->iWrite + size)/CACHE_BLOCKSIZE) *
                           CACHE_BLOCKSIZE;
        newCache = os_malloc (copyCache->iWrite + additionalLength);
        if (newCache) {
            memcpy (newCache, copyCache->cache, copyCache->iWrite);
            os_free (copyCache->cache);
            copyCache->cache = newCache;
            copyCache->length = copyCache->iWrite + additionalLength;
        } else {
            return -1;
        }
    }
    memcpy ((void *)((PA_ADDRCAST)copyCache->cache + copyCache->iWrite),
            data, size);
    copyCache->iWrite += size;
    return writeIndex;
}

void
gapi_copyCacheWriteIndex (
    gapi_copyCache copyCache,
    void *data,
    c_long size,
    c_long index)
{
    assert (copyCache);
    assert (data);
    assert (size);

    memcpy ((void *)((PA_ADDRCAST)copyCache->cache + index), data, size);
}

void
gapi_copyCacheUpdateSize (
    gapi_copyCache copyCache,
    c_long headerIndex)
{
    short length;
    gapiCopyHeader *hAddr;

    assert (copyCache);

    hAddr = (gapiCopyHeader *)((PA_ADDRCAST)copyCache->cache + headerIndex);
    length = copyCache->iWrite - headerIndex;
    hAddr->size = length;
}

void
gapi_copyCacheBackReference (
    gapi_copyCache copyCache,
    c_long headerIndex)
{
    gapiCopyReference ref;

    ref.header.copyType = gapiRecursive;
    ref.refIndex = (unsigned int)(copyCache->iWrite - headerIndex);
    headerIndex = gapi_copyCacheWrite (copyCache, &ref, sizeof(ref));
    gapi_copyCacheUpdateSize (copyCache, headerIndex);
}

void
gapi_copyCacheFinalize (
    gapi_copyCache copyCache)
{
    void *exactCache;

    assert (copyCache);

    if (copyCache->iWrite < copyCache->length) {
        exactCache = os_malloc (copyCache->iWrite);
        /* If out of resources, keep cache */
        if (exactCache) {
            memcpy (exactCache, copyCache->cache, copyCache->iWrite);
            os_free (copyCache->cache);
            copyCache->cache = exactCache;
            copyCache->length = copyCache->iWrite;
        }
    }
}

unsigned long long
gapi_getUnionDescriptor (
    gapiCopyType ct,
    void         *src)
{
    unsigned long long discrVal = 0ULL;
    
    switch ( ct ) {
        case gapiBoolean:
            discrVal = *(gapi_boolean *)src;
            break;
        case gapiByte:
            discrVal = *(gapi_octet *)src;
            break;
        case gapiChar:
            discrVal = *(gapi_char *)src;
            break;
        case gapiShort:
            discrVal = *(gapi_short *)src;
            break;
        case gapiInt:
            discrVal = *(gapi_long *)src;
            break;
        case gapiLong:
            discrVal = *(gapi_long_long *)src;
            break;
        case gapiEnum:
            discrVal = *(gapi_long *)src;
            break;            
        default:
            OS_REPORT_1(OS_ERROR,"gapi_getUnionDescriptor",0,
                        "Illegal gapiCopyType (%d) detected.",
                        ct);
            assert(0);
            break;
    }

    return discrVal;
}

 
unsigned long 
gapi_copyCacheGetUserSize (
    gapi_copyCache copyCache)
{
    return copyCache->userSize;
}

STATIC void
gapi_metaObjectBuild(
    c_type o,
    gapi_context context)
{
    assert (o);
    assert (context);

    /* catch all unsupported types */
    switch (c_baseObject(o)->kind) {
    case M_UNDEFINED:
    case M_ATTRIBUTE:
    case M_CLASS:
    case M_EXCEPTION:
    case M_INTERFACE:
    case M_EXPRESSION:
    case M_OPERATION:
    case M_RELATION:
    case M_PARAMETER:
    case M_BASE:
    case M_COUNT:
    case M_CONSTOPERAND:
    case M_LITERAL:
    case M_MEMBER:
    case M_UNIONCASE:
    case M_CONSTANT:
    case M_MODULE:
        OS_REPORT_1(OS_ERROR,"gapi_metaObject",0,
                    "Unsupported type (%d) detected.",
                    c_baseObject(o)->kind);
        assert (FALSE);
        break;
    default:
        break;
    }

    if( !c_typeHasRef(o) ) {
        gapi_cacheBlackBoxBuild(o->size, context);
        return;
    }

    switch (c_baseObject(o)->kind) {
    case M_PRIMITIVE:
        switch (c_primitive(o)->kind) {
        case P_BOOLEAN:
            /* boolean */
            gapi_cacheBooleanBuild (c_primitive(o), context);
            break;
        case P_SHORT:
            /* short */
            gapi_cacheShortBuild (c_primitive(o), context);
            break;
        case P_LONG:
            /* int */
            gapi_cacheIntBuild (c_primitive(o), context);
            break;
        case P_LONGLONG:
            /* long */
            gapi_cacheLongBuild (c_primitive(o), context);
            break;
        case P_USHORT:
            /* short */
            gapi_cacheShortBuild (c_primitive(o), context);
            break;
        case P_ULONG:
            /* int */
            gapi_cacheIntBuild (c_primitive(o), context);
            break;
        case P_ULONGLONG:
            /* long */
            gapi_cacheLongBuild (c_primitive(o), context);
            break;
        case P_CHAR:
            /* char */
            gapi_cacheCharBuild (c_primitive(o), context);
            break;
        case P_OCTET:
            /* byte */
            gapi_cacheByteBuild (c_primitive(o), context);
            break;
        case P_FLOAT:
            /* float */
            gapi_cacheFloatBuild (c_primitive(o), context);
            break;
        case P_DOUBLE:
            /* double */
            gapi_cacheDoubleBuild (c_primitive(o), context);
            break;
        default:
            OS_REPORT_1(OS_ERROR,"gapi_metaObject",0,
                        "Illegal primitive type (%d) detected.",
                        c_primitive(o)->kind);
            assert (FALSE);
        }
        break;
    case M_COLLECTION:
        if (c_collectionType(o)->kind == C_STRING) {
            if (c_collectionType(o)->maxSize > 0) {
                    /* bounded string */
                gapi_cacheBStringBuild (c_collectionType(o), context);
            } else {
                gapi_cacheStringBuild (c_collectionType(o), context);
            }
        } else if (c_collectionType(o)->kind == C_SEQUENCE) {
            /* sequence */
            if (c_baseObject(c_typeActualType(c_collectionType(o)->subType))->kind == M_PRIMITIVE) {
                switch (c_primitive(c_typeActualType(c_collectionType(o)->subType))->kind) {
                case P_BOOLEAN:
                    /* boolean */
                    gapi_cacheSeqBooleanBuild (c_collectionType(o), context);
                    break;
                case P_SHORT:
                    /* short */
                    gapi_cacheSeqShortBuild (c_collectionType(o), context);
                    break;
                case P_LONG:
                    /* int */
                    gapi_cacheSeqIntBuild (c_collectionType(o), context);
                    break;
                case P_LONGLONG:
                    /* long */
                    gapi_cacheSeqLongBuild (c_collectionType(o), context);
                    break;
                case P_USHORT:
                    /* short */
                    gapi_cacheSeqShortBuild (c_collectionType(o), context);
                    break;
                case P_ULONG:
                    /* int */
                    gapi_cacheSeqIntBuild (c_collectionType(o), context);
                    break;
                case P_ULONGLONG:
                    /* long */
                    gapi_cacheSeqLongBuild (c_collectionType(o), context);
                    break;
                case P_CHAR:
                    /* char */
                    gapi_cacheSeqCharBuild (c_collectionType(o), context);
                    break;
                case P_OCTET:
                    /* byte */
                    gapi_cacheSeqByteBuild (c_collectionType(o), context);
                    break;
                case P_FLOAT:
                    /* float */
                    gapi_cacheSeqFloatBuild (c_collectionType(o), context);
                    break;
                case P_DOUBLE:
                    /* double */
                    gapi_cacheSeqDoubleBuild (c_collectionType(o), context);
                    break;
                default:
                    OS_REPORT_1(OS_ERROR,"gapi_metaObject",0,
                                "Illegal collection type (%d) detected.",
                                c_collectionType(o)->kind);
                    assert (FALSE);
                }
            } else {
                /** sequence of object */
                gapi_cacheSeqObjectBuild (c_collectionType(o), context);
            }
        } else if (c_collectionType(o)->kind == C_ARRAY) {
            /* array */
            if (c_baseObject(c_typeActualType(c_collectionType(o)->subType))->kind == M_PRIMITIVE) {
                switch (c_primitive(c_typeActualType(c_collectionType(o)->subType))->kind) {
                case P_BOOLEAN:
                    /* boolean */
                    gapi_cacheArrBooleanBuild (c_collectionType(o), context);
                    break;
                case P_SHORT:
                    /* short */
                    gapi_cacheArrShortBuild (c_collectionType(o), context);
                    break;
                case P_LONG:
                    /* int */
                    gapi_cacheArrIntBuild (c_collectionType(o), context);
                    break;
                case P_LONGLONG:
                    /* long */
                    gapi_cacheArrLongBuild (c_collectionType(o), context);
                    break;
                case P_USHORT:
                    /* short */
                    gapi_cacheArrShortBuild (c_collectionType(o), context);
                    break;
                case P_ULONG:
                    /* int */
                    gapi_cacheArrIntBuild (c_collectionType(o), context);
                    break;
                case P_ULONGLONG:
                    /* long */
                    gapi_cacheArrLongBuild (c_collectionType(o), context);
                    break;
                case P_CHAR:
                    /* char */
                    gapi_cacheArrCharBuild (c_collectionType(o), context);
                    break;
                case P_OCTET:
                    /* byte */
                    gapi_cacheArrByteBuild (c_collectionType(o), context);
                    break;
                case P_FLOAT:
                    /* float */
                    gapi_cacheArrFloatBuild (c_collectionType(o), context);
                    break;
                case P_DOUBLE:
                    /* double */
                    gapi_cacheArrDoubleBuild (c_collectionType(o), context);
                    break;
                default:
                    OS_REPORT_1(OS_ERROR,"gapi_metaObject",0,
                                "Illegal collection type (%d) detected.",
                                c_collectionType(o)->kind);
                    assert (FALSE);
                }
            } else {
                /** array of object */
                gapi_cacheArrObjectBuild (c_collectionType(o), context);
            }
        }
        break;
    case M_ENUMERATION:
        gapi_cacheEnumBuild (c_enumeration(o), context);
        break;
    case M_STRUCTURE:
        gapi_cacheStructBuild (c_structure(o), context);
        break;
    case M_TYPEDEF:
            gapi_metaObjectBuild (c_typeDef(o)->alias, context);
        break;
    case M_UNION:
            gapi_cacheUnionBuild (c_union(o), context);
        break;
    default:
        OS_REPORT_1(OS_ERROR,"gapi_metaObject",0,
                    "Illegal object type (%d) detected.",
                    c_baseObject(o)->kind);
        assert (FALSE);
        break;
    }
}

#define gapiCopyHeader_init(_this,_type,_size) \
        ((gapiCopyHeader *)_this)->copyType = _type; \
        ((gapiCopyHeader *)_this)->size = _size

STATIC void
gapi_cacheStructBuild (
    c_structure o,
    gapi_context ctx)
{
    gapiCopyStruct copyStruct;
    c_long headerIndex;
    c_long mi;

    gapiCopyHeader_init(&copyStruct.header, gapiStruct, 0);
    copyStruct.nrOfMembers = c_arraySize(o->members);
    copyStruct.size     = c_typeActualType((c_type)o)->size;
    copyStruct.userSize = gapi_cacheObjectUserSize((c_type)o);
    
    TRACE (printf("Struct userSize = %u\n", copyStruct.userSize));
    
    headerIndex = gapi_copyCacheWrite (ctx->copyCache,
                                       &copyStruct,
                                       sizeof(gapiCopyStruct));
    c_iterInsert (ctx->typeStack,
                  gapi_typeHistoryNew (c_metaObject(o),
                                       headerIndex));

    TRACE (printf ("Struct\n"));

    for (mi = 0; mi < c_arraySize(o->members); mi++) {
        gapi_cacheStructMember (o->members[mi], ctx);
    }
    gapi_typeHistoryFree (c_iterTakeFirst (ctx->typeStack));
    gapi_copyCacheUpdateSize (ctx->copyCache, headerIndex);


}

STATIC void
gapi_cacheStructMember (
    c_member o,
    gapi_context ctx)
{
    gapiCopyStructMember member;

    member.memberOffset = o->offset;
    TRACE (printf ("    Struct Member @ %d\n", member.memberOffset));

    gapi_copyCacheWrite (ctx->copyCache, &member, sizeof(member));
    gapi_metaObjectBuild (c_specifier(o)->type, ctx);
}

STATIC void
gapi_cacheUnionLabel (
    c_literal lit,
    gapi_context ctx)
{
    gapiCopyUnionLabel labelVal;

    TRACE (char llstr[36];)

    switch (lit->value.kind) {
        case V_UNDEFINED:
        case V_OCTET:
        case V_FLOAT:
        case V_DOUBLE:
        case V_STRING:
        case V_WCHAR:
        case V_WSTRING:
        case V_FIXED:
        case V_OBJECT:
        case V_COUNT:
            /* Invalid type */
            break;
        case V_BOOLEAN:
            labelVal.labelVal = lit->value.is.Boolean;
            break;
        case V_SHORT:
            labelVal.labelVal = lit->value.is.Short;
            break;
        case V_LONG:
            labelVal.labelVal = lit->value.is.Long;
            break;
        case V_LONGLONG:
            labelVal.labelVal = lit->value.is.LongLong;
            break;
        case V_USHORT:
            labelVal.labelVal = lit->value.is.UShort;
            break;
        case V_ULONG:
            labelVal.labelVal = lit->value.is.ULong;
            break;
        case V_ULONGLONG:
            labelVal.labelVal = lit->value.is.ULongLong;
            break;
        case V_CHAR:
            labelVal.labelVal = lit->value.is.Char;
            break;
        case V_VOIDP:
        case V_ADDRESS:
            OS_REPORT_1(OS_ERROR," gapi_cacheUnionLabel",0,
                        "Illegal value kind (%d) detected.",
                        lit->value.kind);
            assert(FALSE);
            break;
    }

    TRACE (llstr[35] = '\0'; printf ("    labels value = %s\n", os_lltostr(labelVal.labelVal, &llstr[35])));
    gapi_copyCacheWrite (ctx->copyCache, &labelVal, sizeof(labelVal));
}

STATIC void
gapi_cacheUnionCaseField (
    c_unionCase o,
    gapi_context ctx)
{
    TRACE (printf ("    Union Case\n"));

    gapi_metaObjectBuild (c_specifier(o)->type, ctx);
}

STATIC void
gapi_cacheUnionCase (
    c_unionCase unionCase,
    gapi_context ctx)
{
    gapiCopyUnionLabels labels;
    c_ulong l;

    labels.labelCount = c_arraySize (unionCase->labels);
    gapi_copyCacheWrite (ctx->copyCache, &labels, sizeof(labels));

    TRACE (printf ("    labels count = %d\n", labels.labelCount));

    for (l = 0; l < labels.labelCount; l++) {
        gapi_cacheUnionLabel (c_literal(unionCase->labels[l]), ctx);
    }
    gapi_cacheUnionCaseField (unionCase, ctx);
}

STATIC void
gapi_cacheUnionBuild (
    c_union o,
    gapi_context ctx)
{
    gapiCopyUnion copyUnion;
    c_long headerIndex;
    c_long mi;

    gapiCopyHeader_init (&copyUnion.header, gapiUnion, 0);
    copyUnion.nrOfCases = c_arraySize(o->cases);
    copyUnion.discrType = c_typeActualType(o->switchType);
    if (copyUnion.discrType->size > c_type(o)->alignment) {
        copyUnion.casesOffset = copyUnion.discrType->size;
    } else {
        copyUnion.casesOffset = c_type(o)->alignment;
    }
    copyUnion.size     = c_typeActualType(c_type(o))->size;
    copyUnion.userSize = gapi_cacheObjectUserSize((c_type)o);

    TRACE (printf("Union userSize = %u\n", copyUnion.userSize));
    
    headerIndex = gapi_copyCacheWrite (ctx->copyCache, &copyUnion, sizeof(gapiCopyUnion));
    c_iterInsert (ctx->typeStack, gapi_typeHistoryNew (c_metaObject(o), headerIndex));
    TRACE (printf ("Union\n"));

    for (mi = 0; mi < c_arraySize(o->cases); mi++) {
    gapi_cacheUnionCase (o->cases[mi], ctx);
    }
    
    gapi_typeHistoryFree (c_iterTakeFirst (ctx->typeStack));
    gapi_copyCacheUpdateSize (ctx->copyCache, headerIndex);
}

STATIC void
gapi_cacheBlackBoxBuild (
    unsigned int size,
    gapi_context ctx)
{
    gapiCopyBlackBox blackBox;

    TRACE (printf ("BlackBox (%d bytes)\n",size));
    gapiCopyHeader_init ((gapiCopyHeader *)&blackBox,
                      gapiBlackBox,
                      sizeof(blackBox));
    blackBox.size = size;
    gapi_copyCacheWrite (ctx->copyCache, &blackBox, sizeof(blackBox));
}

STATIC void
gapi_cacheBooleanBuild (
    c_primitive o,
    gapi_context ctx)
{
    gapiCopyHeader booleanHeader;

    TRACE (printf ("Boolean\n"));
    gapiCopyHeader_init (&booleanHeader,
                      gapiBoolean,
                      sizeof(booleanHeader));
    gapi_copyCacheWrite (ctx->copyCache, &booleanHeader, sizeof(booleanHeader));
}

STATIC void
gapi_cacheByteBuild (
    c_primitive o,
    gapi_context ctx)
{
    gapiCopyHeader byteHeader;

    TRACE (printf ("Byte\n"));
    gapiCopyHeader_init (&byteHeader,
                      gapiByte,
                      sizeof(byteHeader));
    gapi_copyCacheWrite (ctx->copyCache, &byteHeader, sizeof(byteHeader));
}

STATIC void
gapi_cacheCharBuild (
    c_primitive o,
    gapi_context ctx)
{
    gapiCopyHeader charHeader;

    TRACE (printf ("Char\n"));
    gapiCopyHeader_init (&charHeader,
                      gapiChar,
                      sizeof(charHeader));
    gapi_copyCacheWrite (ctx->copyCache, &charHeader, sizeof(charHeader));
}

STATIC void
gapi_cacheShortBuild (
    c_primitive o,
    gapi_context ctx)
{
    gapiCopyHeader shortHeader;

    TRACE (printf ("Short\n"));
    gapiCopyHeader_init (&shortHeader, gapiShort, sizeof(shortHeader));
    gapi_copyCacheWrite (ctx->copyCache, &shortHeader, sizeof(shortHeader));
}

STATIC void
gapi_cacheIntBuild (
    c_primitive o,
    gapi_context ctx)
{
    gapiCopyHeader intHeader;

    TRACE (printf ("Int\n"));
    gapiCopyHeader_init (&intHeader,
                      gapiInt,
                      sizeof(intHeader));
    gapi_copyCacheWrite (ctx->copyCache, &intHeader, sizeof(intHeader));
}

STATIC void
gapi_cacheLongBuild (
    c_primitive o,
    gapi_context ctx)
{
    gapiCopyHeader longHeader;

    TRACE (printf ("Long\n"));
    gapiCopyHeader_init (&longHeader,
                      gapiLong,
                      sizeof(longHeader));
    gapi_copyCacheWrite (ctx->copyCache, &longHeader, sizeof(longHeader));
}

STATIC void
gapi_cacheFloatBuild (
    c_primitive o,
    gapi_context ctx)
{
    gapiCopyHeader floatHeader;

    TRACE (printf ("Float\n"));
    gapiCopyHeader_init (&floatHeader,
                      gapiFloat,
                      sizeof(floatHeader));
    gapi_copyCacheWrite (ctx->copyCache, &floatHeader, sizeof(floatHeader));
}

STATIC void
gapi_cacheDoubleBuild (
    c_primitive o,
    gapi_context ctx)
{
    gapiCopyHeader doubleHeader;

    TRACE (printf ("Double\n"));
    gapiCopyHeader_init (&doubleHeader,
                      gapiDouble,
                      sizeof(doubleHeader));
    gapi_copyCacheWrite (ctx->copyCache, &doubleHeader, sizeof(doubleHeader));
}

STATIC void
gapi_cacheArrBooleanBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopyArray booleanHeader;

    TRACE (printf ("Boolean Array\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&booleanHeader,
                      gapiArrBoolean,
                      sizeof(booleanHeader));
    booleanHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache,
                         &booleanHeader,
                         sizeof(booleanHeader));
}

STATIC void
gapi_cacheArrByteBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopyArray byteHeader;

    TRACE (printf ("Byte Array\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&byteHeader,
                      gapiArrByte,
                      sizeof(byteHeader));
    byteHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache, &byteHeader, sizeof(byteHeader));
}

STATIC void
gapi_cacheArrCharBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopyArray charHeader;

    TRACE (printf ("Char Array\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&charHeader,
                      gapiArrChar,
                      sizeof(charHeader));
    charHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache, &charHeader, sizeof(charHeader));
}

STATIC void
gapi_cacheArrShortBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopyArray shortHeader;

    TRACE (printf ("Short Array\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&shortHeader,
                      gapiArrShort,
                      sizeof(shortHeader));
    shortHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache, &shortHeader, sizeof(shortHeader));
}

STATIC void
gapi_cacheArrIntBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopyArray intHeader;

    TRACE (printf ("Int Array\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&intHeader,
                      gapiArrInt,
                      sizeof(intHeader));
    intHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache, &intHeader, sizeof(intHeader));
}

STATIC void
gapi_cacheArrLongBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopyArray longHeader;

    TRACE (printf ("Long Array\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&longHeader,
                      gapiArrLong,
                      sizeof(longHeader));
    longHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache, &longHeader, sizeof(longHeader));
}

STATIC void
gapi_cacheArrFloatBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopyArray floatHeader;

    TRACE (printf ("Float Array\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&floatHeader,
                      gapiArrFloat,
                      sizeof(floatHeader));
    floatHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache, &floatHeader, sizeof(floatHeader));
}

STATIC void
gapi_cacheArrDoubleBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopyArray doubleHeader;

    TRACE (printf ("Double Array\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&doubleHeader,
                      gapiArrDouble,
                      sizeof(doubleHeader));
    doubleHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache, &doubleHeader, sizeof(doubleHeader));
}

STATIC void
gapi_cacheArrObjectBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopyObjectArray objectArrHeader;
    c_long headerIndex;

    objectArrHeader.arraySize = o->maxSize;
    objectArrHeader.typeSize = c_type(c_typeActualType(o->subType))->size;

    TRACE (printf ("Array\n    size %d\n", o->maxSize));

    gapiCopyHeader_init ((gapiCopyHeader *)&objectArrHeader,
                      gapiArray,
                      sizeof(objectArrHeader));
    headerIndex = gapi_copyCacheWrite (ctx->copyCache, &objectArrHeader, sizeof(objectArrHeader));
    
    gapi_metaObjectBuild (c_typeActualType(o->subType), ctx);
    gapi_copyCacheUpdateSize (ctx->copyCache, headerIndex);
}

STATIC int
gapi_isDefinedInScope (
    c_collectionType type)
{
    c_metaObject scope;
    int inScope = 0;

    scope = c_metaObject(type)->definedIn;
    while (scope) {
        if (scope == c_metaObject(c_typeActualType(type->subType))) {
            inScope = 1;
            scope = NULL;
        } else {
            scope = scope->definedIn;
        }
    }
    return inScope;
}

STATIC int
gapi_headerIndex (
    c_metaObject type,
    gapi_context ctx)
{
    int i;
    gapi_typeHistory history;

    for (i = 0; i < c_iterLength (ctx->typeStack); i++) {
        history = c_iterObject (ctx->typeStack, i);
        if (history->metaObject == type) {
            return history->cacheIndex;
        }
    }
    return -1;
}

STATIC unsigned int
userSizeCorrection (
    c_type o);

STATIC unsigned int
gapi_cacheObjectRefCount (
    c_type o);

STATIC unsigned int
structureUserSizeCorrection (
    c_structure o)
{
    unsigned int size = 0;
    c_long mi;

    for (mi = 0; mi < c_arraySize(o->members); mi++) {
        size += userSizeCorrection(c_specifier(o->members[mi])->type);
    }
   
    return size;
}

STATIC unsigned int
unionUserSizeCorrection (
    c_union o)
{
    unsigned int size = 0;
    unsigned int cs = 0;
    c_long ci;

    for (ci = 0; ci < c_arraySize(o->cases); ci++) {
        cs = userSizeCorrection(c_specifier(o->cases[ci])->type);
        if ( cs > size ) {
            size = cs;
        }
    }
   
    return size;
}

STATIC unsigned int
collectionUserSizeCorrection (
    c_collectionType o)
{
    unsigned int size = 0;

    switch ( o->kind ) {
        case C_SEQUENCE:
            size = GAPI_SEQUENCE_CORRECTION;
            break;
        case C_ARRAY:
            size = o->maxSize * userSizeCorrection(c_type(c_typeActualType(o->subType)));
            break;
        default:
            break;
    }

    return size;
}

STATIC unsigned int
userSizeCorrection (
    c_type o)
{
    unsigned int size = 0;
    c_type actual;

    actual = c_typeActualType(o);

    switch ( c_baseObject(actual)->kind ) {
        case M_STRUCTURE:
            size = structureUserSizeCorrection(c_structure(actual));
            break;
        case M_UNION:
            size = unionUserSizeCorrection(c_union(actual));
            break;
        case M_COLLECTION:
            size = collectionUserSizeCorrection(c_collectionType(actual));
            break;
        default:
            break;
    }

    return size;
}       

STATIC unsigned int
gapi_cacheObjectUserSize (
    c_type o)
{
    unsigned int size = 0;
    c_type actual;

    actual = c_typeActualType(o);

    size = actual->size + userSizeCorrection(actual);

    TRACE (printf("gapi_cacheObjectUserSize actual->size = %d, total = %d\n", actual->size, size));

    return size;
}
    
STATIC unsigned int
gapi_cacheStructureRefCount (
    c_structure o)
{
    unsigned int count = 0;
    c_long mi;

    for (mi = 0; mi < c_arraySize(o->members); mi++) {
        count += gapi_cacheObjectRefCount(c_specifier(o->members[mi])->type);
    }
   
    return count;
}

STATIC unsigned int
gapi_cacheUnionRefCount (
    c_union o)
{
    unsigned int count = 0;
    c_long ci;

    for (ci = 0; ci < c_arraySize(o->cases); ci++) {
        count += gapi_cacheObjectRefCount(c_specifier(o->cases[ci])->type);
    }
   
    return count;
}

STATIC unsigned int
gapi_cacheCollectionRefCount (
    c_collectionType o)
{
    unsigned int count = 0;

    switch ( o->kind ) {
        case C_STRING:
        case C_SEQUENCE:
            count = 1;
            break;
        case C_ARRAY:
            count = o->maxSize * gapi_cacheObjectRefCount(c_type(c_typeActualType(o->subType)));
            break;
        default:
            break;
    }
   
    return count;
}

STATIC unsigned int
gapi_cacheObjectRefCount (
    c_type o)
{
    unsigned int count = 0;
    c_type actual;

    actual = c_typeActualType(o);

    switch ( c_baseObject(actual)->kind ) {
        case M_STRUCTURE:
            count = gapi_cacheStructureRefCount(c_structure(actual));
            break;
        case M_UNION:
            count = gapi_cacheUnionRefCount(c_union(actual));
            break;
        case M_COLLECTION:
            count = gapi_cacheCollectionRefCount(c_collectionType(actual));
            break;
        default:
            break;
    }

    return count;
}       

STATIC void
gapi_cacheSeqObjectBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopyObjectSequence objectSeqHeader;
    c_long headerIndex;

    objectSeqHeader.seqSize = o->maxSize;
    objectSeqHeader.type = c_typeActualType(o->subType);
    objectSeqHeader.baseTypeSize = objectSeqHeader.type->size;
    objectSeqHeader.userTypeSize = gapi_cacheObjectUserSize(objectSeqHeader.type);

    TRACE (printf ("SEQUENCE\n    size %d\n", o->maxSize));

    gapiCopyHeader_init ((gapiCopyHeader *)&objectSeqHeader,
                      gapiSequence,
                      sizeof(objectSeqHeader));
    headerIndex = gapi_copyCacheWrite (ctx->copyCache,
                                       &objectSeqHeader,
                                       sizeof(objectSeqHeader));
    /* Check if the subtype is in the scope of the subtype,
     * in that case it is a recursive type.
     */
    if (gapi_isDefinedInScope (o)) {
        /* make back reference */
        gapi_copyCacheBackReference (ctx->copyCache,
                                     gapi_headerIndex(c_metaObject(c_typeActualType(o->subType)),
                                     ctx));
    } else {
        gapi_metaObjectBuild (c_typeActualType(o->subType), ctx);
    }
    gapi_copyCacheUpdateSize (ctx->copyCache, headerIndex);
}

STATIC void
gapi_cacheSeqBooleanBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopySequence booleanHeader;

    TRACE (printf ("Boolean Sequence\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&booleanHeader,
                      gapiSeqBoolean,
                      sizeof(booleanHeader));
    booleanHeader.type = c_typeActualType(o->subType);
    booleanHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache,
                         &booleanHeader,
                         sizeof(booleanHeader));
}

STATIC void
gapi_cacheSeqByteBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopySequence byteHeader;

    TRACE (printf ("Byte Sequence\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&byteHeader,
                      gapiSeqByte,
                      sizeof(byteHeader));
    byteHeader.type = c_typeActualType(o->subType);
    byteHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache,
                         &byteHeader,
                         sizeof(byteHeader));
}

STATIC void
gapi_cacheSeqCharBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopySequence charHeader;

    TRACE (printf ("Char Sequence\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&charHeader,
                      gapiSeqChar,
                      sizeof(charHeader));
    charHeader.type = c_typeActualType(o->subType);
    charHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache,
                         &charHeader,
                         sizeof(charHeader));
}

STATIC void
gapi_cacheSeqShortBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopySequence shortHeader;

    TRACE (printf ("Short Sequence\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&shortHeader,
                      gapiSeqShort,
                      sizeof(shortHeader));
    shortHeader.type = c_typeActualType(o->subType);
    shortHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache,
                         &shortHeader,
                         sizeof(shortHeader));
}

STATIC void
gapi_cacheSeqIntBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopySequence intHeader;

    TRACE (printf ("Int Sequence\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&intHeader,
                      gapiSeqInt,
                      sizeof(intHeader));
    intHeader.type = c_typeActualType(o->subType);
    intHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache,
                         &intHeader,
                         sizeof(intHeader));
}

STATIC void
gapi_cacheSeqLongBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopySequence longHeader;

    TRACE (printf ("Long Sequence\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&longHeader,
                      gapiSeqLong,
                      sizeof(longHeader));
    longHeader.type = c_typeActualType(o->subType);
    longHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache,
                         &longHeader,
                         sizeof(longHeader));
}

STATIC void
gapi_cacheSeqFloatBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopySequence floatHeader;

    TRACE (printf ("Float Sequence\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&floatHeader,
                      gapiSeqFloat,
                      sizeof(floatHeader));
    floatHeader.type = c_typeActualType(o->subType);
    floatHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache, &floatHeader, sizeof(floatHeader));
}

STATIC void
gapi_cacheSeqDoubleBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopySequence doubleHeader;

    TRACE (printf ("Double Sequence\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&doubleHeader,
                      gapiSeqDouble,
                      sizeof(doubleHeader));
    doubleHeader.type = c_typeActualType(o->subType);
    doubleHeader.size = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache, &doubleHeader, sizeof(doubleHeader));
}

STATIC void
gapi_cacheStringBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopyHeader stringHeader;

    TRACE (printf ("String\n"));
    gapiCopyHeader_init (&stringHeader,
                      gapiString,
                      sizeof(stringHeader));
    gapi_copyCacheWrite (ctx->copyCache, &stringHeader, sizeof(stringHeader));
}

STATIC void
gapi_cacheBStringBuild (
    c_collectionType o,
    gapi_context ctx)
{
    gapiCopyBoundedString bstringHeader;

    TRACE (printf ("BString\n"));
    gapiCopyHeader_init ((gapiCopyHeader *)&bstringHeader,
                      gapiBString,
                      sizeof(bstringHeader));
    bstringHeader.max = o->maxSize;
    gapi_copyCacheWrite (ctx->copyCache, &bstringHeader, sizeof(bstringHeader));
}

STATIC void
gapi_cacheEnumBuild (
    c_enumeration o,
    gapi_context ctx)
{
    gapiCopyEnum copyEnum;

    copyEnum.nrOfElements = c_arraySize (o->elements);

    TRACE (printf ("Enum\n    Members # %d\n", copyEnum.nrOfElements));

    gapi_copyCacheWrite (ctx->copyCache, &copyEnum, sizeof(copyEnum));
}

void
cacheDump (
    gapiCopyHeader *ch,
    unsigned int level)
{
    unsigned int mi;
    gapiCopyStruct *csh;
    gapiCopyStructMember *csm;
    gapiCopyObjectArray *oah;
    gapiCopyType ct;
    unsigned int l;

    ct = ch->copyType;
    for (l = 0; l < level; l++) {
    printf ("  ");
    }
    printf ("  T:%d S:%d\n", ct, ch->size);
    for (l = 0; l < level; l++) {
    printf ("  ");
    }
    switch (ct) {
    case gapiBlackBox:
        printf ("  BlackBox (%d bytes)\n",((gapiCopyBlackBox *)ch)->size);
        break;
    case gapiBoolean:
        printf ("  Boolean\n");
        break;
    case gapiByte:
        printf ("  Byte\n");
        break;
    case gapiChar:
        printf ("  Char\n");
        break;
    case gapiShort:
        printf ("  Short\n");
        break;
    case gapiInt:
        printf ("  Int\n");
        break;
    case gapiLong:
        printf ("  Long\n");
        break;
    case gapiFloat:
        printf ("  Float\n");
        break;
    case gapiDouble:
        printf ("  Double\n");
        break;
    case gapiArrBoolean:
        printf ("  ArrBoolean\n");
        break;
    case gapiArrByte:
        printf ("  ArrByte\n");
        break;
    case gapiArrChar:
        printf ("  ArrChar\n");
        break;
    case gapiArrShort:
        printf ("  ArrShort\n");
        break;
    case gapiArrInt:
        printf ("  ArrInt\n");
        break;
    case gapiArrLong:
        printf ("  ArrLong\n");
        break;
    case gapiArrFloat:
        printf ("  ArrFloat\n");
        break;
    case gapiArrDouble:
        printf ("  ArrDouble\n");
        break;
    case gapiSeqBoolean:
        printf ("  SeqBoolean\n");
        break;
    case gapiSeqByte:
        printf ("  SeqByte\n");
        break;
    case gapiSeqChar:
        printf ("  SeqChar\n");
        break;
    case gapiSeqShort:
        printf ("  SeqShort\n");
        break;
    case gapiSeqInt:
        printf ("  SeqInt\n");
        break;
    case gapiSeqLong:
        printf ("  SeqLong\n");
        break;
    case gapiSeqFloat:
        printf ("  SeqFloat\n");
        break;
    case gapiSeqDouble:
        printf ("  SeqDouble\n");
        break;
    case gapiString:
        printf ("  String\n");
        break;
    case gapiBString:
        printf ("  BString\n");
        break;
    case gapiEnum:
        printf ("  Enum\n");
        break;
    case gapiStruct:
        csh = (gapiCopyStruct *)ch;
        printf ("  Struct\n");
        for (l = 0; l < level; l++) {
            printf ("  ");
        }
        printf ("  M#:%d \n", csh->nrOfMembers);
        csm = (gapiCopyStructMember *)((PA_ADDRCAST)csh + sizeof (gapiCopyStruct));
        for (mi = 0; mi < csh->nrOfMembers; mi++) {
            for (l = 0; l < level; l++) {
                printf ("  ");
            }
            printf ("  M@:%d \n", csm->memberOffset);
            ch = (gapiCopyHeader *)((PA_ADDRCAST)csm + sizeof (gapiCopyStructMember));
            cacheDump (ch, level+1);
            csm = (gapiCopyStructMember *)((PA_ADDRCAST)ch + ch->size);
        }
        break;
    case gapiUnion:
        printf ("  Union\n");
        break;
    case gapiArray:
        oah = (gapiCopyObjectArray *)ch;
        printf ("  Array\n");
        for (l = 0; l < level; l++) {
            printf ("  ");
        }
        printf ("  E#:%d TS:%d\n", oah->arraySize, oah->typeSize);
        cacheDump ((gapiCopyHeader *)((PA_ADDRCAST)oah + sizeof(gapiCopyObjectArray)), level+1);
        break;
    case gapiSequence:
        printf ("  Sequence\n");
        break;
    case gapiRecursive:
        printf ("  Recursive\n");
        break;
    }
}

void
gapi_copyCacheDump (
    gapi_copyCache copyCache)
{
    cacheDump ((gapiCopyHeader *)copyCache->cache, 0);
}


STATIC void
gapi_copyCacheBuild (
    gapi_copyCache copyCache,
    c_metaObject object)
{
    gapi_context context;

    assert (copyCache);
    assert (object);

    context = os_malloc (C_SIZEOF(gapi_context));
    if (context) {
        context->copyCache = copyCache;
        context->typeStack = c_iterNew (NULL);
        gapi_metaObjectBuild (c_type(object), context);
        gapi_copyCacheFinalize (context->copyCache);
        copyCache->userSize = gapi_cacheObjectUserSize(c_type(object));
        c_iterFree (context->typeStack);
        TRACE (gapi_copyCacheDump (context->copyCache));
    }
    os_free (context);
}
