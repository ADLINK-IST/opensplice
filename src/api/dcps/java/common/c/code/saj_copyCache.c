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
#include <jni.h>

#include "os_stdlib.h"
#include "os_heap.h"
#include "os_abstract.h"
#include "c_base.h"
#include "c_iterator.h"

#include "sd_cdr.h"

#include "saj_copyCache.h"
#include "saj_utilities.h"
#include "saj__exception.h"

/*#define JNI_TRACE 1*/
#define TRACE(function)         /*function*/
#define CACHE_BLOCKSIZE         (100)
#define STATIC
#define SAJ_COPYCACHE_FIELDDESCRIPTOR_SIZE (512)
#define SAJ_COPYCACHE_CLASSDESCRIPTOR_SIZE (512)

C_CLASS(saj_context);
#define saj_context(o)          ((saj_context)(o))

C_STRUCT(saj_context) {
    saj_copyCache       copyCache;
    jclass              javaClass;
    JNIEnv              *javaEnv;
    c_iter              typeStack;
};

C_STRUCT(saj_copyCache) {
    void        *cache;  /* start of cache  */
    os_uint32   length;  /* length of cache */
    os_uint32   iWrite;  /* write index     */
    os_char*    orgPName;
    os_char*    tgtPName;
    sajReaderCopyCache  readerCache;
    struct sd_cdrInfo *cdrInfo;
};

C_CLASS(saj_typeHistory);
#define saj_typeHistory(o)      ((saj_typeHistory)(o))

C_STRUCT(saj_typeHistory) {
    c_metaObject    metaObject;
    os_uint32       cacheIndex;
};

static const char *java_keywords[61] = {
    /* Java keywords, list must be alphabetically ordered */
    "_abstract",
    "_assert",
    "_boolean",
    "_break",
    "_byte",
    "_case",
    "_catch",
    "_char",
    "_class",
    "_clone",
    "_const",
    "_continue",
    "_default",
    "_do",
    "_double",
    "_else",
    "_enum",
    "_equals",
    "_extends",
    "_false",
    "_final",
    "_finalize",
    "_finally",
    "_float",
    "_for",
    "_getClass",
    "_goto",
    "_hashCode",
    "_if",
    "_implements",
    "_import",
    "_instanceof",
    "_int",
    "_interface",
    "_long",
    "_native",
    "_new",
    "_notify",
    "_notifyAll",
    "_null",
    "_package",
    "_private",
    "_protected",
    "_public",
    "_return",
    "_short",
    "_static",
    "_super",
    "_switch",
    "_synchronized",
    "_this",
    "_throw",
    "_throws",
    "_toString",
    "_transient",
    "_true",
    "_try",
    "_void",
    "_volatile",
    "_wait",
    "_while"
};

const os_char *
saj_copyResultImage(
    saj_copyResult result)
{
    const char * image;

#define _SAJ_IMAGE_STR(k) ((char *)((os_address)(#k) + strlen("SAJ_COPYRESULT_")))
#define _SAJ_IMAGE_STR_CASE(k) case (k): image = _SAJ_IMAGE_STR(k); break;

    switch (result) {
    _SAJ_IMAGE_STR_CASE(SAJ_COPYRESULT_OK);
    _SAJ_IMAGE_STR_CASE(SAJ_COPYRESULT_BAD_PARAMETER);
    _SAJ_IMAGE_STR_CASE(SAJ_COPYRESULT_ERROR);
    default:
        image = "(invalid result)";
        assert(result <= SAJ_COPYRESULT_ERROR);
        break;
    }
#undef _SAJ_IMAGE_STR_CASE
#undef _SAJ_IMAGE_STR

    return image;
}

static os_char*
saj_getScopedName(
    c_metaObject object,
    const os_char separator);

static os_char*
saj_getSubstitutedScopedName(
    c_metaObject object,
    const os_char separator,
    const os_char* orgPName,
    const os_char* tarPName);

static os_boolean
saj_scopeNameRefersToDDSObject(
    const os_char* scopeName,
    const os_char separator);

static os_char*
saj_substitute(
    const os_char* org,
    const os_char* src,
    const os_char* tgt);

os_char*
saj_charToString(
    const os_char source);

static os_boolean
saj_copyCacheIsPragmaStacPossiblyDefined(
    char* fieldDescriptor, /* inout */
    os_uint32 size,
    os_boolean forClassDescriptor);

static char *
saj_dekeyedId (
    char *name)
{
    int i = 0;
    int j = sizeof(java_keywords)/sizeof(char *) - 1;
    int h;
    int r;

    while (i <= j) {
        h = (i + j) / 2;
        r = strcmp (&java_keywords[h][1], name);
        if (r == 0) {
            name = (char *)java_keywords[h];
            i = j+1; /* break while loop */
        } else if (r > 0) {
            j = h - 1;
        } else {
            i = h + 1;
        }
    }

    return name;
}

saj_typeHistory
saj_typeHistoryNew (
    const c_metaObject metaObject,
    os_uint32 index)
{
    saj_typeHistory history = os_malloc (C_SIZEOF(saj_typeHistory));

    history->metaObject = metaObject;
    history->cacheIndex = index;

    return history;
}

void
saj_typeHistoryFree (
    saj_typeHistory history)
{
    os_free (history);
}

os_uint32
saj_typeHistoryIndex (
    saj_typeHistory history)
{
    return history->cacheIndex;
}

c_metaObject
saj_typeHistoryMetaObject (
    const C_STRUCT(saj_typeHistory) *history)
{
    return history->metaObject;
}


STATIC void saj_copyReaderCacheBuild (saj_copyCache copyCache, c_metaObject object, JNIEnv *env);
STATIC void saj_copyCacheBuild (saj_copyCache copyCache, c_metaObject object, JNIEnv *env);
STATIC os_uint32 saj_copyCacheWrite (saj_copyCache copyCache, void *data, os_uint32 size);
STATIC void saj_copyCacheWriteIndex (saj_copyCache copyCache, void *data, os_uint32 size, os_uint32 index);
STATIC void saj_copyCacheFinalize (saj_copyCache copyCache);
STATIC void saj_cacheHeader (sajCopyHeader *header, sajCopyType type, os_uint32 size);

STATIC void saj_metaObject (c_type o, saj_context context, os_boolean stacRequested);
STATIC void saj_cacheStructBuild (c_structure o, saj_context context);
STATIC void saj_cacheStructMember (c_member o, saj_context context);
STATIC void saj_cacheUnionLabel (c_literal lit, saj_context ctx);
STATIC void saj_cacheUnionBuild (c_union o, saj_context context);
STATIC void saj_cacheBooleanBuild (c_primitive o, saj_context context);
STATIC void saj_cacheByteBuild (c_primitive o, saj_context context);
STATIC void saj_cacheCharBuild (c_primitive o, saj_context context);
STATIC void saj_cacheShortBuild (c_primitive o, saj_context context);
STATIC void saj_cacheIntBuild (c_primitive o, saj_context context);
STATIC void saj_cacheLongBuild (c_primitive o, saj_context context);
STATIC void saj_cacheFloatBuild (c_primitive o, saj_context context);
STATIC void saj_cacheDoubleBuild (c_primitive o, saj_context context);
STATIC void saj_cacheArrBooleanBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheArrByteBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheArrCharBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheArrCharToBStringBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheArrShortBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheArrIntBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheArrLongBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheArrFloatBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheArrDoubleBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheSeqBooleanBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheSeqByteBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheSeqCharBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheSeqShortBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheSeqIntBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheSeqLongBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheSeqFloatBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheSeqDoubleBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheStringBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheBStringBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheBStringToArrCharBuild (c_collectionType o, saj_context context);
STATIC void saj_cacheEnumBuild (c_enumeration o, saj_context ctx);
STATIC void saj_cacheArrObjectBuild (c_collectionType o, saj_context ctx, os_boolean stacRequested);
STATIC void saj_cacheSeqObjectBuild (c_collectionType o, saj_context ctx);
STATIC void saj_scopedTypeName (c_char *buffer, c_long bufferSize, c_metaObject object, const os_char separator, saj_copyCache copyCache);
STATIC void saj_fieldDescriptor (c_type type, char *descriptor, unsigned int size, saj_copyCache copyCache);
STATIC void saj_classDescriptor (c_type type, char *descriptor, unsigned int size, saj_copyCache copyCache);

static int cdrNoteCatsStac (struct sd_cdrInfo *ci, c_iter typeStack)
{
  /* typestack is a sequence of saj_typeHistory; but we need the
     c_type pointers embedded in it & we need them in root -> leaf
     order, whereas the typeStack is leaf -> root order. */
  const c_long n = c_iterLength (typeStack);
  const void **ary;
  c_long i;
  int ret;
  if ((ary = os_malloc (n * sizeof (*ary))) == NULL)
    return -1;
  c_iterArray (typeStack, (void **) ary);
  for (i = 0; i < n; i++)
    ary[i] = saj_typeHistoryMetaObject (ary[i]);
  for (i = 0; i < n / 2; i++)
  {
    const void *tmp = ary[i];
    ary[i] = ary[n-i-1];
    ary[n-i-1] = tmp;
  }
  ret = sd_cdrNoteCatsStac (ci, n, (struct c_type_s const * const *) ary);
  os_free (ary);
  return ret;
}

saj_copyCache
saj_copyCacheNew (
    JNIEnv *env,
    c_metaObject object,
    const os_char* orgPName,
    const os_char* tgtPName)
{
    saj_copyCache copyCache = os_malloc (C_SIZEOF(saj_copyCache));

    if (copyCache) {
        copyCache->cache = os_malloc (CACHE_BLOCKSIZE);
        if (copyCache->cache) {
            copyCache->length = CACHE_BLOCKSIZE;
            copyCache->iWrite = 0;
            if(orgPName)
            {
                copyCache->orgPName = os_strdup(orgPName);
            } else
            {
                copyCache->orgPName = NULL;
            }
            if(tgtPName)
            {
                copyCache->tgtPName = os_strdup(tgtPName);
            } else
            {
                copyCache->tgtPName = NULL;
            }
            copyCache->cdrInfo = sd_cdrInfoNew (c_type (object));
            saj_copyCacheBuild (copyCache, object, env);
            saj_copyReaderCacheBuild (copyCache, object, env);
        } else {
            os_free (copyCache);
            copyCache = NULL;
        }
    }
    return copyCache;
}

void
saj_copyCacheFree (
    saj_copyCache copyCache)
{
    assert (copyCache);

    sd_cdrInfoFree (copyCache->cdrInfo);
    os_free (copyCache->cache);
    os_free (copyCache->orgPName);
    os_free (copyCache->tgtPName);
    os_free (copyCache);
}

c_long
saj_copyCacheLength (
    saj_copyCache copyCache)
{
    assert (copyCache);

    return copyCache->iWrite;
}

void *
saj_copyCacheCache (
    saj_copyCache copyCache)
{
    assert (copyCache);

    return copyCache->cache;
}

struct sd_cdrInfo *
saj_copyCacheCdrInfo (
    saj_copyCache copyCache)
{
    assert (copyCache);

    return copyCache->cdrInfo;
}

sajReaderCopyCache *
saj_copyCacheReaderCache (
    saj_copyCache copyCache)
{
    assert (copyCache);

    return (&copyCache->readerCache);
}

os_uint32
saj_copyCacheWrite (
    saj_copyCache copyCache,
    void *data,
    os_uint32 size)
{
    os_uint32 additionalLength;
    void *newCache;
    os_uint32 writeIndex = copyCache->iWrite;

    assert (copyCache);
    assert (data);
    assert (size);

    if ((copyCache->iWrite + size) >= copyCache->length) {
        /* If new data does not fit, allocate new cache by calculating extra size needed
         * and then calculate the number of cache blocksizes needed to match this extra space.
         */

        additionalLength = (((copyCache->iWrite + size - copyCache->length) / CACHE_BLOCKSIZE) +1) * CACHE_BLOCKSIZE;
#if JNI_TRACE
    printf ("JNI: saj_copyCacheWrite cache length %d needed %d (%d + %d) extend cache with additionalLength %d \n", copyCache->length, (copyCache->iWrite + size), copyCache->iWrite, size, additionalLength);
#endif
        newCache = os_malloc(copyCache->length + additionalLength);
        if (newCache) {
            memcpy(newCache, copyCache->cache, copyCache->iWrite);
            os_free(copyCache->cache);
            copyCache->cache = newCache;
            copyCache->length = copyCache->length + additionalLength;
        } else {
            return -1;
        }
    }
    memcpy ((void *)((PA_ADDRCAST)copyCache->cache + copyCache->iWrite), data, size);
    copyCache->iWrite += size;
#if JNI_TRACE
    printf ("JNI: saj_copyCacheWrite updating write index from %d to %d\n", writeIndex, copyCache->iWrite);
#endif
    return writeIndex;
}

void
saj_copyCacheWriteIndex (
    saj_copyCache copyCache,
    void *data,
    os_uint32 size,
    os_uint32 index)
{
    assert (copyCache);
    assert (data);
    assert (size);

    memcpy ((void *)((PA_ADDRCAST)copyCache->cache + index), data, size);
}

void
saj_copyCacheUpdateSize (
    saj_copyCache copyCache,
    os_uint32 headerIndex)
{
    os_uint32 length;
    sajCopyHeader *hAddr;

    assert (copyCache);

    hAddr = (sajCopyHeader *)((PA_ADDRCAST)copyCache->cache + headerIndex);
    length = copyCache->iWrite - headerIndex;
    hAddr->size = length;

#if JNI_TRACE
    printf ("JNI: saj_copyCacheUpdateSize updating type %d to %d\n", hAddr->copyType, hAddr->size);
#endif
}

void
saj_copyCacheBackReference (
    saj_copyCache copyCache,
    c_long headerIndex)
{
    sajCopyReference ref;

    ref.header.copyType = sajRecursive;
    ref.refIndex = (unsigned int)(copyCache->iWrite - headerIndex);
    headerIndex = saj_copyCacheWrite (copyCache, &ref, sizeof(ref));
    saj_copyCacheUpdateSize (copyCache, headerIndex);

#if JNI_TRACE
    printf ("JNI: saj_copyCacheBackReference setting copyType %d\n", ref.header.copyType);
#endif
}

void
saj_copyCacheFinalize (
    saj_copyCache copyCache)
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

STATIC void
saj_metaObject(
    c_type o,
    saj_context context,
    os_boolean stacRequested)
{
    assert (o);
    assert (context);

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
    case M_ANNOTATION:
        assert(FALSE);
        break;
    case M_PRIMITIVE:
        switch (c_primitive(o)->kind) {
        case P_BOOLEAN:
            /* boolean */
            saj_cacheBooleanBuild(c_primitive(o), context);
            break;
        case P_SHORT:
            /* short */
            saj_cacheShortBuild(c_primitive(o), context);
            break;
        case P_LONG:
            /* int */
            saj_cacheIntBuild(c_primitive(o), context);
            break;
        case P_LONGLONG:
            /* long */
            saj_cacheLongBuild(c_primitive(o), context);
            break;
        case P_USHORT:
            /* short */
            saj_cacheShortBuild(c_primitive(o), context);
            break;
        case P_ULONG:
            /* int */
            saj_cacheIntBuild(c_primitive(o), context);
            break;
        case P_ULONGLONG:
            /* long */
            saj_cacheLongBuild(c_primitive(o), context);
            break;
        case P_CHAR:
            /* char */
            saj_cacheCharBuild(c_primitive(o), context);
            break;
        case P_OCTET:
            /* byte */
            saj_cacheByteBuild(c_primitive(o), context);
            break;
        case P_FLOAT:
            /* float */
            saj_cacheFloatBuild(c_primitive(o), context);
            break;
        case P_DOUBLE:
            /* double */
            saj_cacheDoubleBuild(c_primitive(o), context);
            break;
        default:
            assert(FALSE);
        }
        break;
    case M_COLLECTION:
        if (c_collectionType(o)->kind == C_STRING) {
            if (c_collectionType(o)->maxSize > 0) {
                /* bounded string */
                saj_cacheBStringBuild (c_collectionType(o), context);

            } else {
                saj_cacheStringBuild (c_collectionType(o), context);
            }
        } else if (c_collectionType(o)->kind == C_SEQUENCE) {
            /* sequence */
            if (c_baseObject(c_typeActualType(c_collectionType(o)->subType))->kind == M_PRIMITIVE) {
                switch (c_primitive(c_typeActualType(c_collectionType(o)->subType))->kind) {
                case P_BOOLEAN:
                    /* boolean */
                    saj_cacheSeqBooleanBuild (c_collectionType(o), context);
                    break;
                case P_SHORT:
                    /* short */
                    saj_cacheSeqShortBuild (c_collectionType(o), context);
                    break;
                case P_LONG:
                    /* int */
                    saj_cacheSeqIntBuild (c_collectionType(o), context);
                    break;
                case P_LONGLONG:
                    /* long */
                    saj_cacheSeqLongBuild (c_collectionType(o), context);
                    break;
                case P_USHORT:
                    /* short */
                    saj_cacheSeqShortBuild (c_collectionType(o), context);
                    break;
                case P_ULONG:
                    /* int */
                    saj_cacheSeqIntBuild (c_collectionType(o), context);
                    break;
                case P_ULONGLONG:
                    /* long */
                    saj_cacheSeqLongBuild (c_collectionType(o), context);
                    break;
                case P_CHAR:
                    /* char */
                    saj_cacheSeqCharBuild (c_collectionType(o), context);
                    break;
                case P_OCTET:
                    /* byte */
                    saj_cacheSeqByteBuild (c_collectionType(o), context);
                    break;
                case P_FLOAT:
                    /* float */
                    saj_cacheSeqFloatBuild (c_collectionType(o), context);
                    break;
                case P_DOUBLE:
                    /* double */
                    saj_cacheSeqDoubleBuild (c_collectionType(o), context);
                    break;
                default:
                    assert (FALSE);
                }
            } else {
                /** sequence of object */
                saj_cacheSeqObjectBuild (c_collectionType(o), context);
            }
        } else if (c_collectionType(o)->kind == C_ARRAY) {
            /* array */
            if (c_baseObject(c_typeActualType(c_collectionType(o)->subType))->kind == M_PRIMITIVE) {
                switch (c_primitive(c_typeActualType(c_collectionType(o)->subType))->kind) {
                case P_BOOLEAN:
                    /* boolean */
                    saj_cacheArrBooleanBuild (c_collectionType(o), context);
                    break;
                case P_SHORT:
                    /* short */
                    saj_cacheArrShortBuild (c_collectionType(o), context);
                    break;
                case P_LONG:
                    /* int */
                    saj_cacheArrIntBuild (c_collectionType(o), context);
                    break;
                case P_LONGLONG:
                    /* long */
                    saj_cacheArrLongBuild (c_collectionType(o), context);
                    break;
                case P_USHORT:
                    /* short */
                    saj_cacheArrShortBuild (c_collectionType(o), context);
                    break;
                case P_ULONG:
                    /* int */
                    saj_cacheArrIntBuild (c_collectionType(o), context);
                    break;
                case P_ULONGLONG:
                    /* long */
                    saj_cacheArrLongBuild (c_collectionType(o), context);
                    break;
                case P_CHAR:
                    /* char */
                    if(stacRequested)
                    {
                        saj_cacheBStringToArrCharBuild (c_collectionType(o), context);
                    } else
                    {
                        saj_cacheArrCharBuild (c_collectionType(o), context);
                    }
                    break;
                case P_OCTET:
                    /* byte */
                    saj_cacheArrByteBuild (c_collectionType(o), context);
                    break;
                case P_FLOAT:
                    /* float */
                    saj_cacheArrFloatBuild (c_collectionType(o), context);
                    break;
                case P_DOUBLE:
                    /* double */
                    saj_cacheArrDoubleBuild (c_collectionType(o), context);
                    break;
                default:
                    assert (FALSE);
                }
            } else {
                /** array of object */
                saj_cacheArrObjectBuild (c_collectionType(o), context, stacRequested);
            }
        }
        break;
    case M_ENUMERATION:
        saj_cacheEnumBuild (c_enumeration(o), context);
        break;
    case M_STRUCTURE:
        saj_cacheStructBuild (c_structure(o), context);
        break;
    case M_TYPEDEF:
        saj_metaObject (c_typeDef(o)->alias, context, stacRequested);
        break;
    case M_UNION:
        saj_cacheUnionBuild (c_union(o), context);
        break;
    }
}

void
saj_cacheHeader (
    sajCopyHeader *header,
    sajCopyType type,
    os_uint32 size)
{
    header->copyType = type;
    header->size = size;
#if JNI_TRACE
    printf ("JNI: saj_cacheHeader setting copyType %d, size to %d\n", header->copyType, size);
#endif
}

STATIC void
saj_cacheStructBuild (
    c_structure o,
    saj_context ctx)
{
    C_STRUCT(saj_context) context;
    sajCopyStruct copyStruct;
    os_uint32 headerIndex;
    c_long mi;
    char classDescriptor [512];
    jclass javaClass;

    classDescriptor [0] = '\0';
    saj_classDescriptor (c_type(o), classDescriptor, sizeof(classDescriptor), ctx->copyCache);

    context.copyCache = ctx->copyCache;
    context.javaEnv = ctx->javaEnv;
    context.typeStack = ctx->typeStack;
    javaClass = (*(ctx->javaEnv))->FindClass (ctx->javaEnv, classDescriptor);
#if JNI_TRACE
    printf ("JNI: FindClass (\"%s\") = 0x%x\n", classDescriptor, javaClass);
#endif
    saj_exceptionCheck (ctx->javaEnv);

    saj_cacheHeader (&copyStruct.header, sajStruct, 0);
    copyStruct.nrOfMembers = c_arraySize(o->members);
    context.javaClass = (*(ctx->javaEnv))->NewGlobalRef (ctx->javaEnv, javaClass);
#if JNI_TRACE
    printf ("JNI: NewGlobalRef (0x%x) = 0x%x\n", javaClass, context.javaClass);
#endif
    (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, javaClass);
#if JNI_TRACE
    printf ("JNI: DeleteLocalRef (0x%x)\n", javaClass);
#endif
    copyStruct.structClass = context.javaClass;
    saj_exceptionCheck (ctx->javaEnv);
    copyStruct.constrID = (*(ctx->javaEnv))->GetMethodID (
    context.javaEnv, copyStruct.structClass, "<init>", "()V");
    saj_exceptionCheck (ctx->javaEnv);
#if JNI_TRACE
    printf ("JNI: GetMethodID (0x%x, \"<init>\", \"()V\") = %d\n", copyStruct.structClass, copyStruct.constrID);
#endif
    headerIndex = saj_copyCacheWrite (context.copyCache, &copyStruct, sizeof(sajCopyStruct));
    c_iterInsert (ctx->typeStack, saj_typeHistoryNew (c_metaObject(o), headerIndex));

    TRACE (printf ("Struct\n    Class name %s\n", classDescriptor));
    TRACE (printf ("    Java class %x\n", (int)copyStruct.structClass));
    TRACE (printf ("    Constructor ID = %x\n", (int)copyStruct.constrID));

    for (mi = 0; mi < c_arraySize(o->members); mi++) {
        saj_cacheStructMember (o->members[mi], &context);
    }

    saj_typeHistoryFree (c_iterTakeFirst (ctx->typeStack));
    saj_copyCacheUpdateSize (context.copyCache, headerIndex);
}

STATIC c_baseObject
saj_copyCacheResolveTypeDef(
    c_baseObject obj)
{
    c_baseObject object;

    object = obj;
    while(object->kind == M_TYPEDEF)
    {
        object = c_baseObject(c_typeDef(object)->alias);
    }
    return object;
}

os_boolean
saj_copyCacheIsPragmaStacPossiblyDefined(
    char* fieldDescriptor, /* inout */
    os_uint32 size,
    os_boolean forClassDescriptor)
{
    os_uint32 i = 0;
    os_uint32 arraysDetected = 0;
    os_boolean charTypeDetected = OS_FALSE;
    os_boolean stacRequested = OS_FALSE;
    /* It could be that pragma stac is at play. This pragma works on bounded strings
     * as well as bounded strings within arrays. So the descriptor that failed should
     * look something like '[C' or '[[C' or '[[[C', etc. We'll search for this occurance
     * in the field descriptor. Basically only the last '[C' (i.e. the char array)
     * will need to be replaced to a string type, the other arrays ('[' chars in the
     * descriptor) can remain in tact.
     */
    for(i = 0; i < size && (arraysDetected == 0 || !charTypeDetected); i++)
    {
        if(fieldDescriptor[i] == '[')
        {
            arraysDetected++;
        } else if(arraysDetected > 0 &&
                  fieldDescriptor[i] == 'C')
        {
            charTypeDetected = OS_TRUE;
        } else
        {
            /* not relevant, exit */
            i = SAJ_COPYCACHE_FIELDDESCRIPTOR_SIZE;
        }
    }
    if(arraysDetected > 0 && charTypeDetected)
    {
        assert(i >= 2);/* should have found at least one '[' char and one 'C' char */
        /* if the field descriptor is needed for a class description and only
         * one array level is detected then it means we need to fill the
         * descriptor string following JNI class descriptor rules.
         * This means that we can not use the leading 'L' character nor the
         * trailing ';' character.
         * In all other cases, thus including class descriptor with more then
         * one level of arrays we can use the standard way
         */
        if(forClassDescriptor && arraysDetected == 1)
        {
            snprintf (&(fieldDescriptor[i-2]),size-(i-2)-1, "java/lang/String");
        } else
        {
            snprintf (&(fieldDescriptor[i-2]),size-(i-2)-1, "Ljava/lang/String;");
        }
        stacRequested = OS_TRUE;
    }
    return stacRequested;
}

STATIC void
saj_cacheStructMember (
    c_member o,
    saj_context ctx)
{
    sajCopyStructMember member;
    char fieldDescriptor[SAJ_COPYCACHE_FIELDDESCRIPTOR_SIZE];
    os_boolean bstringToCArray = OS_FALSE;
    os_boolean stacRequested = OS_FALSE;
    c_baseObject baseObject;

    fieldDescriptor[0] = '\0';
    saj_fieldDescriptor (c_specifier(o)->type, fieldDescriptor, sizeof (fieldDescriptor), ctx->copyCache);
    member.memberOffset = o->offset;
    member.javaFID = (*ctx->javaEnv)->GetFieldID (
        ctx->javaEnv,
        ctx->javaClass,
        saj_dekeyedId(c_specifier(o)->name),
        fieldDescriptor);
    if((*ctx->javaEnv)->ExceptionOccurred(ctx->javaEnv))
    {
        baseObject = saj_copyCacheResolveTypeDef(c_baseObject(o));
        baseObject = saj_copyCacheResolveTypeDef(c_baseObject(c_specifier(baseObject)->type));
        if (baseObject->kind == M_COLLECTION &&
            c_collectionType(baseObject)->kind == C_STRING &&
            c_collectionType(baseObject)->maxSize > 0)
        {
            (*ctx->javaEnv)->ExceptionClear(ctx->javaEnv);
            fieldDescriptor[0] = '[';
            fieldDescriptor[1] = 'C';
            fieldDescriptor[2] = '\0';
            member.javaFID = (*ctx->javaEnv)->GetFieldID (
                ctx->javaEnv,
                ctx->javaClass,
                saj_dekeyedId(c_specifier(o)->name),
                fieldDescriptor);
            (*ctx->javaEnv)->ExceptionDescribe(ctx->javaEnv);
            bstringToCArray = OS_TRUE;
        } else
        {
            stacRequested = saj_copyCacheIsPragmaStacPossiblyDefined(
                fieldDescriptor,
                SAJ_COPYCACHE_FIELDDESCRIPTOR_SIZE,
                OS_FALSE);
            if(stacRequested)
            {
                (*ctx->javaEnv)->ExceptionClear(ctx->javaEnv);
                member.javaFID = (*ctx->javaEnv)->GetFieldID (
                    ctx->javaEnv,
                    ctx->javaClass,
                    saj_dekeyedId(c_specifier(o)->name),
                    fieldDescriptor);
                (*ctx->javaEnv)->ExceptionDescribe(ctx->javaEnv);
            }
        }
    }
    saj_exceptionCheck (ctx->javaEnv);

#if JNI_TRACE
    printf ("JNI: GetFieldID (0x%x) = %d\n", ctx->javaClass, member.javaFID);
#endif

    TRACE (printf ("    Struct Member @ %d\n", member.memberOffset));
    TRACE (printf ("        Java field descriptor = %s, Field name = %s, Java FID = %x\n",
            fieldDescriptor,
            saj_dekeyedId(c_specifier(o)->name),
            (int)member.javaFID));

    saj_copyCacheWrite (ctx->copyCache, &member, sizeof(member));
    if(bstringToCArray)
    {
        saj_cacheArrCharToBStringBuild (c_collectionType(c_specifier(o)->type), ctx);
    } else
    {
        saj_metaObject (c_specifier(o)->type, ctx, stacRequested);
    }
}

STATIC void
saj_cacheUnionLabel (
    c_literal lit,
    saj_context ctx)
{
    sajCopyUnionLabel labelVal;

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
    case V_ADDRESS:
    case V_VOIDP:
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
    }
    TRACE (printf ("    labels value = %llu\n", labelVal.labelVal));
    saj_copyCacheWrite (ctx->copyCache, &labelVal, sizeof(labelVal));
}

STATIC void
saj_cacheUnionCaseField (
    c_unionCase o,
    c_type switchType,
    saj_context ctx,
    sajUnionType unionType)
{
    char fieldDescriptor[512];

    fieldDescriptor[0] = '\0';
    saj_fieldDescriptor (c_specifier(o)->type, fieldDescriptor, sizeof (fieldDescriptor), ctx->copyCache);

    /* Look for a setter that implicitly sets the discriminator to 1 of the appropriate cases. */
    if (unionType == SAJ_GENERIC_UNION)
    {
        sajCopyGenericUnionCase unionCase;
        char operatorDescr[512];

        snprintf (operatorDescr, sizeof (operatorDescr), "(%s)V", fieldDescriptor);
        unionCase.setterID = (*(ctx->javaEnv))->GetMethodID (ctx->javaEnv, ctx->javaClass,
                saj_dekeyedId(c_specifier(o)->name), operatorDescr);
        saj_exceptionCheck (ctx->javaEnv);
    #if JNI_TRACE
        printf ("JNI: GetMethodID (0x%x, \"%s\", \"%s\") = %d\n", ctx->javaClass, saj_dekeyedId(c_specifier(o)->name), operatorDescr, unionCase.setterID);
    #endif

        /* Look for the corresponding getter of that branch. */
        snprintf (operatorDescr, sizeof (operatorDescr), "()%s", fieldDescriptor);
        unionCase.getterID = (*(ctx->javaEnv))->GetMethodID (
                ctx->javaEnv, ctx->javaClass, saj_dekeyedId(c_specifier(o)->name), operatorDescr);
        saj_exceptionCheck (ctx->javaEnv);
    #if JNI_TRACE
        printf ("JNI: GetMethodID (0x%x, \"%s\", \"%s\") = %d\n", ctx->javaClass, c_specifier(o)->name, operatorDescr, unionCase.getterID);
    #endif

        TRACE (printf ("    Union Case\n"));
        TRACE (printf ("        Java field descriptor = %s, Java setterID = %x, Java getterID = %x\n",
        fieldDescriptor, (int)unionCase.setterID,  (int)unionCase.getterID));

        /* Also look for a possible setter that includes the discriminator as 1st parameter. */
        fieldDescriptor[0] = '\0';
        saj_fieldDescriptor (switchType, fieldDescriptor, sizeof (fieldDescriptor), ctx->copyCache);
        saj_fieldDescriptor (c_specifier(o)->type, &fieldDescriptor[strlen(fieldDescriptor)],
                sizeof (fieldDescriptor) - strlen(fieldDescriptor), ctx->copyCache);
        snprintf (operatorDescr, sizeof (operatorDescr), "(%s)V", fieldDescriptor);
        unionCase.setterWithDiscrID = (*(ctx->javaEnv))->GetMethodID (
                ctx->javaEnv, ctx->javaClass, saj_dekeyedId(c_specifier(o)->name), operatorDescr);
    #if JNI_TRACE
        printf ("JNI: GetMethodID (0x%x, \"%s\", \"%s\") = %d\n", ctx->javaClass, c_specifier(o)->name, operatorDescr, unionCase.setterWithDiscrID);
    #endif
        if ( (*(ctx->javaEnv))->ExceptionCheck (ctx->javaEnv))
        {
            (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);
        }

        saj_copyCacheWrite (ctx->copyCache, &unionCase, sizeof(unionCase));
    }
    else
    {
        sajCopySupportedUnionCase unionCase;
        char attributeName[512];

        /* Look for the field that holds the branch value (Only applicable to the SAJ). */
        snprintf (attributeName, sizeof(attributeName), "__%s", saj_dekeyedId(c_specifier(o)->name));
        unionCase.caseID = (*(ctx->javaEnv))->GetFieldID (ctx->javaEnv, ctx->javaClass, attributeName, fieldDescriptor);
        if (!unionCase.caseID)
        {
            /*  clear the exception, as the case ID is only present when java data model is generated with idlpp */
            (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);

            /* Now try to see whether the JacORB layout is a good fit. */
            unionCase.caseID = (*(ctx->javaEnv))->GetFieldID (ctx->javaEnv, ctx->javaClass, saj_dekeyedId(c_specifier(o)->name), fieldDescriptor);
            saj_exceptionCheck (ctx->javaEnv);
        }
    #if JNI_TRACE
        else
        {
            printf ("JNI: GetFieldID (0x%x, \"%s\", \"%s\") = %d\n", ctx->javaClass, attributeName, fieldDescriptor, unionCase.caseID);
        }
    #endif

        saj_copyCacheWrite (ctx->copyCache, &unionCase, sizeof(unionCase));
    }
    saj_metaObject (c_specifier(o)->type, ctx, OS_FALSE);
}

STATIC void
saj_cacheUnionCase (
    c_unionCase unionCase,
    c_type switchType,
    saj_context ctx,
    sajUnionType unionType)
{
    sajCopyUnionLabels labels;
    c_ulong l;

    labels.labelCount = c_arraySize (unionCase->labels);
    saj_copyCacheWrite (ctx->copyCache, &labels, sizeof(labels));

    TRACE (printf ("    labels count = %d\n", labels.labelCount));

    for (l = 0; l < labels.labelCount; l++) {
        saj_cacheUnionLabel (c_literal(unionCase->labels[l]), ctx);
    }
    saj_cacheUnionCaseField (unionCase, switchType, ctx, unionType);
}

STATIC void
saj_cacheUnionBuild (
    c_union o,
    saj_context ctx)
{
    C_STRUCT(saj_context) context;
    sajCopyUnion copyUnion;
    os_uint32 headerIndex;
    c_long mi;
    char classDescriptor [512];
    char discrDescriptor [512];
    char methodSignature [512];
    jclass javaClass;
    jfieldID discrID;

    classDescriptor [0] = '\0';
    saj_classDescriptor (c_type(o), classDescriptor, sizeof(classDescriptor), ctx->copyCache);
    discrDescriptor[0] = '\0';
    saj_fieldDescriptor (c_typeActualType(o->switchType), discrDescriptor, sizeof (discrDescriptor), ctx->copyCache);

    context.copyCache = ctx->copyCache;
    context.javaEnv = ctx->javaEnv;
    context.typeStack = ctx->typeStack;
    javaClass = (*(ctx->javaEnv))->FindClass (ctx->javaEnv, classDescriptor);
    saj_exceptionCheck (ctx->javaEnv);
#if JNI_TRACE
    printf ("JNI: FindClass (\"%s\") = 0x%x\n", classDescriptor, javaClass);
#endif

    saj_cacheHeader (&copyUnion.header, sajUnion, 0);
    copyUnion.nrOfCases = c_arraySize(o->cases);
    context.javaClass = (*(ctx->javaEnv))->NewGlobalRef (ctx->javaEnv, javaClass);
#if JNI_TRACE
    printf ("JNI: NewGlobalRef (0x%x) = 0x%x\n", javaClass, context.javaClass);
#endif
    (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, javaClass);
#if JNI_TRACE
    printf ("JNI: DeleteLocalRef (0x%x)\n", javaClass);
#endif
    copyUnion.unionClass = context.javaClass;
    saj_exceptionCheck (ctx->javaEnv);
#if JNI_TRACE
    printf ("JNI: GetMethodID (0x%x, \"<init>\", \"()V\") = %d\n", copyUnion.unionClass, copyUnion.constrID);
#endif

    discrID = (*(ctx->javaEnv))->GetFieldID (ctx->javaEnv, copyUnion.unionClass, "_d", discrDescriptor);
    if (discrID)
    {
        copyUnion.unionType = SAJ_SUPPORTED_UNION;
        copyUnion.unionAccessors.supportedUnion.discrID = discrID;
#if JNI_TRACE
        printf ("JNI: GetFieldID (0x%x, \"_d\", \"%s\") = %d\n", copyUnion.unionClass, discrDescriptor, copyUnion.unionAccessors.supportedUnion.discrID);
#endif
    }
    else
    {
        /*  clear the exception, must be non-native code. */
        (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);

        /* Try JacORB generated layout. */
        discrID = (*(ctx->javaEnv))->GetFieldID (ctx->javaEnv, copyUnion.unionClass, "discriminator", discrDescriptor);
        if (discrID)
        {
            copyUnion.unionType = SAJ_SUPPORTED_UNION;
            copyUnion.unionAccessors.supportedUnion.discrID = discrID;
        }
        else
        {
            /*  clear the exception, must be non-JacORB code as well. */
            (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);

            copyUnion.unionType = SAJ_GENERIC_UNION;
            switch (c_baseObject(c_typeActualType(o->switchType))->kind) {
            case M_ENUMERATION:
                {
                    char discrClassDescriptor [512];

                    discrClassDescriptor[0] = '\0';
                    saj_classDescriptor (c_typeActualType(o->switchType), discrClassDescriptor, sizeof(discrClassDescriptor), ctx->copyCache);
                    snprintf (methodSignature, sizeof(methodSignature), "()L%s;", discrClassDescriptor);

                    copyUnion.unionAccessors.genericUnion.getDiscrMethodID = (*(ctx->javaEnv))->GetMethodID((ctx->javaEnv), copyUnion.unionClass, "discriminator", methodSignature);
                    saj_exceptionCheck((ctx->javaEnv));

                    snprintf (methodSignature, sizeof(methodSignature), "(L%s;)V", discrClassDescriptor);
                    copyUnion.unionAccessors.genericUnion.__defaultID = (*(ctx->javaEnv))->GetMethodID((ctx->javaEnv), copyUnion.unionClass, "__default", methodSignature);
                    if ( (*(ctx->javaEnv))->ExceptionCheck (ctx->javaEnv))
                        (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);
                }
                break;
            case M_PRIMITIVE:
                switch (c_primitive (c_typeActualType(o->switchType))->kind)
                {
                case P_BOOLEAN:
                    {
                        copyUnion.unionAccessors.genericUnion.getDiscrMethodID = (*(ctx->javaEnv))->GetMethodID(
                                (ctx->javaEnv), copyUnion.unionClass, "discriminator", "()Z");
                        saj_exceptionCheck((ctx->javaEnv));

                        copyUnion.unionAccessors.genericUnion.__defaultID = (*(ctx->javaEnv))->GetMethodID(
                                (ctx->javaEnv), copyUnion.unionClass, "__default", "(Z)V");
                        if ( (*(ctx->javaEnv))->ExceptionCheck (ctx->javaEnv))
                            (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);
                    }
                    break;
                case P_CHAR:
                    {
                        copyUnion.unionAccessors.genericUnion.getDiscrMethodID = (*(ctx->javaEnv))->GetMethodID(
                                (ctx->javaEnv), copyUnion.unionClass, "discriminator", "()C");
                        saj_exceptionCheck((ctx->javaEnv));

                        copyUnion.unionAccessors.genericUnion.__defaultID = (*(ctx->javaEnv))->GetMethodID(
                                (ctx->javaEnv), copyUnion.unionClass, "__default", "(C)V");
                        if ( (*(ctx->javaEnv))->ExceptionCheck (ctx->javaEnv))
                            (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);
                    }
                    break;
                case P_SHORT:
                case P_USHORT:
                    {
                        copyUnion.unionAccessors.genericUnion.getDiscrMethodID = (*(ctx->javaEnv))->GetMethodID(
                                (ctx->javaEnv), copyUnion.unionClass, "discriminator", "()S");
                        saj_exceptionCheck((ctx->javaEnv));

                        copyUnion.unionAccessors.genericUnion.__defaultID = (*(ctx->javaEnv))->GetMethodID(
                                (ctx->javaEnv), copyUnion.unionClass, "__default", "(S)V");
                        if ( (*(ctx->javaEnv))->ExceptionCheck (ctx->javaEnv))
                            (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);
                    }
                    break;
                case P_LONG:
                case P_ULONG:
                    {
                        copyUnion.unionAccessors.genericUnion.getDiscrMethodID = (*(ctx->javaEnv))->GetMethodID(
                                (ctx->javaEnv), copyUnion.unionClass, "discriminator", "()I");
                        saj_exceptionCheck((ctx->javaEnv));

                        copyUnion.unionAccessors.genericUnion.__defaultID = (*(ctx->javaEnv))->GetMethodID(
                                (ctx->javaEnv), copyUnion.unionClass, "__default", "(I)V");
                        if ( (*(ctx->javaEnv))->ExceptionCheck (ctx->javaEnv))
                            (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);
                    }
                    break;
                case P_LONGLONG:
                case P_ULONGLONG:
                    {
                        copyUnion.unionAccessors.genericUnion.getDiscrMethodID = (*(ctx->javaEnv))->GetMethodID(
                                (ctx->javaEnv), copyUnion.unionClass, "discriminator", "()J");
                        saj_exceptionCheck((ctx->javaEnv));

                        copyUnion.unionAccessors.genericUnion.__defaultID = (*(ctx->javaEnv))->GetMethodID(
                                (ctx->javaEnv), copyUnion.unionClass, "__default", "(J)V");
                        if ( (*(ctx->javaEnv))->ExceptionCheck (ctx->javaEnv))
                            (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);
                    }

                    break;
                default:
                    assert (0);
                }
                break;
            default:
                assert (0);
            }

            copyUnion.unionAccessors.genericUnion.constrID = (*(ctx->javaEnv))->GetMethodID (
                    context.javaEnv, copyUnion.unionClass, "<init>", "()V");
            saj_exceptionCheck (ctx->javaEnv);
        }
    }

    copyUnion.discrType = c_typeActualType(o->switchType);
    if (c_typeActualType(o->switchType)->size > c_type(o)->alignment) {
        copyUnion.casesOffset = c_typeActualType(o->switchType)->size;
    } else {
        copyUnion.casesOffset = c_type(o)->alignment;
    }

    headerIndex = saj_copyCacheWrite (context.copyCache, &copyUnion, sizeof(sajCopyUnion));
    c_iterInsert (ctx->typeStack, saj_typeHistoryNew (c_metaObject(o), headerIndex));
    if (c_baseObject(c_typeActualType(o->switchType))->kind == M_ENUMERATION) {
        saj_cacheEnumBuild(c_enumeration(c_typeActualType(o->switchType)), ctx);
    }
    copyUnion.labelsMetaOffset = context.copyCache->iWrite - headerIndex; /* Not in the copyCache yet: memcopy already done. */

    TRACE (printf ("Union\n    Class name %s\n", classDescriptor));
    TRACE (printf ("    Java class %x\n", (int)copyUnion.unionClass));
    TRACE (printf ("    Union Type = %s\n", copyUnion.unionType == SAJ_SUPPORTED_UNION ? "SAJ_SUPPORTED_UNION" : "SAJ_GENERIC_UNION"));

    for (mi = 0; mi < c_arraySize(o->cases); mi++) {
        saj_cacheUnionCase (o->cases[mi], o->switchType, &context, copyUnion.unionType);
    }
    saj_typeHistoryFree (c_iterTakeFirst (ctx->typeStack));
    saj_copyCacheUpdateSize (context.copyCache, headerIndex);

    /* Don't forget to copy the labelsMetaOffset into the copyCache. */
    ((sajCopyUnion *)((PA_ADDRCAST)context.copyCache->cache + headerIndex))->labelsMetaOffset = copyUnion.labelsMetaOffset;
}

STATIC void
saj_cacheBooleanBuild (
    c_primitive o,
    saj_context ctx)
{
    sajCopyHeader booleanHeader;

    OS_UNUSED_ARG(o);

    TRACE (printf ("Boolean\n"));
    saj_cacheHeader (&booleanHeader, sajBoolean, sizeof(booleanHeader));
    saj_copyCacheWrite (ctx->copyCache, &booleanHeader, sizeof(booleanHeader));
}

STATIC void
saj_cacheByteBuild (
    c_primitive o,
    saj_context ctx)
{
    sajCopyHeader byteHeader;

    OS_UNUSED_ARG(o);

    TRACE (printf ("Byte\n"));
    saj_cacheHeader (&byteHeader, sajByte, sizeof(byteHeader));
    saj_copyCacheWrite (ctx->copyCache, &byteHeader, sizeof(byteHeader));
}

STATIC void
saj_cacheCharBuild (
    c_primitive o,
    saj_context ctx)
{
    sajCopyHeader charHeader;

    OS_UNUSED_ARG(o);

    TRACE (printf ("Char\n"));
    saj_cacheHeader (&charHeader, sajChar, sizeof(charHeader));
    saj_copyCacheWrite (ctx->copyCache, &charHeader, sizeof(charHeader));
}

STATIC void
saj_cacheShortBuild (
    c_primitive o,
    saj_context ctx)
{
    sajCopyHeader shortHeader;

    OS_UNUSED_ARG(o);

    TRACE (printf ("Short\n"));
    saj_cacheHeader (&shortHeader, sajShort, sizeof(shortHeader));
    saj_copyCacheWrite (ctx->copyCache, &shortHeader, sizeof(shortHeader));
}

STATIC void
saj_cacheIntBuild (
    c_primitive o,
    saj_context ctx)
{
    sajCopyHeader intHeader;

    OS_UNUSED_ARG(o);

    TRACE (printf ("Int\n"));
    saj_cacheHeader (&intHeader, sajInt, sizeof(intHeader));
    saj_copyCacheWrite (ctx->copyCache, &intHeader, sizeof(intHeader));
}

STATIC void
saj_cacheLongBuild (
    c_primitive o,
    saj_context ctx)
{
    sajCopyHeader longHeader;

    OS_UNUSED_ARG(o);

    TRACE (printf ("Long\n"));
    saj_cacheHeader (&longHeader, sajLong, sizeof(longHeader));
    saj_copyCacheWrite (ctx->copyCache, &longHeader, sizeof(longHeader));
}

STATIC void
saj_cacheFloatBuild (
    c_primitive o,
    saj_context ctx)
{
    sajCopyHeader floatHeader;

    OS_UNUSED_ARG(o);

    TRACE (printf ("Float\n"));
    saj_cacheHeader (&floatHeader, sajFloat, sizeof(floatHeader));
    saj_copyCacheWrite (ctx->copyCache, &floatHeader, sizeof(floatHeader));
}

STATIC void
saj_cacheDoubleBuild (
    c_primitive o,
    saj_context ctx)
{
    sajCopyHeader doubleHeader;

    OS_UNUSED_ARG(o);

    TRACE (printf ("Double\n"));
    saj_cacheHeader (&doubleHeader, sajDouble, sizeof(doubleHeader));
    saj_copyCacheWrite (ctx->copyCache, &doubleHeader, sizeof(doubleHeader));
}

STATIC void
saj_cacheArrBooleanBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray booleanHeader;
    sajCopyArray* const hdr = &booleanHeader;

    TRACE (printf ("Boolean Array\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajArrBoolean, sizeof(*hdr));
    booleanHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheArrByteBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray byteHeader;
    sajCopyArray* const hdr = &byteHeader;

    TRACE (printf ("Byte Array\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajArrByte, sizeof(*hdr));
    byteHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheArrCharBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray charHeader;
    sajCopyArray* const hdr = &charHeader;

    TRACE (printf ("Char Array\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajArrChar, sizeof(*hdr));
    charHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheArrCharToBStringBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray charHeader;
    sajCopyArray* const hdr = &charHeader;
    os_uint32 headerIndex;

    TRACE (printf ("Char Array To BString\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajArrCharToBString, sizeof(*hdr));
    charHeader.size = o->maxSize;
    headerIndex = saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));

    c_iterInsert (ctx->typeStack, saj_typeHistoryNew (c_metaObject(o), headerIndex));
    cdrNoteCatsStac (ctx->copyCache->cdrInfo, ctx->typeStack);
    saj_typeHistoryFree (c_iterTakeFirst (ctx->typeStack));
}

STATIC void
saj_cacheArrShortBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray shortHeader;
    sajCopyArray* const hdr = &shortHeader;

    TRACE (printf ("Short Array\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajArrShort, sizeof(*hdr));
    shortHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheArrIntBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray intHeader;
    sajCopyArray* const hdr = &intHeader;

    TRACE (printf ("Int Array\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajArrInt, sizeof(*hdr));
    intHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheArrLongBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray longHeader;
    sajCopyArray* const hdr = &longHeader;

    TRACE (printf ("Long Array\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajArrLong, sizeof(*hdr));
    longHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheArrFloatBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray floatHeader;
    sajCopyArray* const hdr = &floatHeader;

    TRACE (printf ("Float Array\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajArrFloat, sizeof(*hdr));
    floatHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheArrDoubleBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray doubleHeader;
    sajCopyArray* const hdr = &doubleHeader;

    TRACE (printf ("Double Array\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajArrDouble, sizeof(*hdr));
    doubleHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheArrObjectBuild (
    c_collectionType o,
    saj_context ctx,
    os_boolean stacRequested)
{
    saj_context context;
    sajCopyObjectArray objectArrHeader;
    sajCopyObjectArray* const hdr = &objectArrHeader;
    char classDescriptor [SAJ_COPYCACHE_CLASSDESCRIPTOR_SIZE];
    os_uint32 headerIndex;
    jclass javaClass;

    classDescriptor [0] = '\0';
    saj_classDescriptor (c_typeActualType(o->subType), classDescriptor, sizeof(classDescriptor), ctx->copyCache);
    if(stacRequested)
    {
        os_boolean tmp;
        tmp = saj_copyCacheIsPragmaStacPossiblyDefined(
            classDescriptor,
            SAJ_COPYCACHE_CLASSDESCRIPTOR_SIZE,
            OS_TRUE);
        assert(tmp == stacRequested); OS_UNUSED_ARG(tmp);
    }
    javaClass = (*(ctx->javaEnv))->FindClass (ctx->javaEnv, classDescriptor);
#if JNI_TRACE
    printf ("JNI: FindClass (\"%s\") = 0x%x\n", classDescriptor, javaClass);
#endif
    objectArrHeader.arrayClass = (*(ctx->javaEnv))->NewGlobalRef (ctx->javaEnv, javaClass);
#if JNI_TRACE
    printf ("JNI: NewGlobalRef (\"%s\") = 0x%x\n", javaClass, objectArrHeader.arrayClass);
#endif
    (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, javaClass);
#if JNI_TRACE
    printf ("JNI: DeleteLocalRef (0x%x)\n", javaClass);
#endif
    saj_exceptionCheck (ctx->javaEnv);
    objectArrHeader.arraySize = o->maxSize;
    objectArrHeader.typeSize = c_type(c_typeActualType(o->subType))->size;

    TRACE (printf ("Array\n    Class name %s\n", classDescriptor));
    TRACE (printf ("    size %d\n", o->maxSize));
    TRACE (printf ("    Java class %x\n", (int)objectArrHeader.arrayClass));

    saj_cacheHeader ((sajCopyHeader *)hdr, sajArray, sizeof(*hdr));
    headerIndex = saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
    c_iterInsert (ctx->typeStack, saj_typeHistoryNew (c_metaObject(o), headerIndex));

    context = os_malloc (C_SIZEOF(saj_context));
    context->javaEnv = ctx->javaEnv;
    context->javaClass = objectArrHeader.arrayClass;
    context->copyCache = ctx->copyCache;
    context->typeStack = ctx->typeStack;
    saj_metaObject (c_typeActualType(o->subType), context, stacRequested);
    saj_copyCacheUpdateSize (ctx->copyCache, headerIndex);
    assert (context->typeStack == ctx->typeStack);
    os_free (context);

    saj_typeHistoryFree (c_iterTakeFirst (ctx->typeStack));
}

STATIC int
saj_isDefinedInScope (
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
saj_headerIndex (
    c_metaObject type,
    saj_context ctx)
{
    int i;
    saj_typeHistory history;

    for (i = 0; i < c_iterLength (ctx->typeStack); i++) {
        history = c_iterObject (ctx->typeStack, i);
        if (history->metaObject == type) {
            return history->cacheIndex;
        }
    }
    return -1;
}

STATIC void
saj_cacheSeqObjectBuild (
    c_collectionType o,
    saj_context ctx)
{
    saj_context context;
    sajCopyObjectSequence objectSeqHeader;
    sajCopyObjectSequence* const hdr = &objectSeqHeader;
    char classDescriptor [512];
    os_uint32 headerIndex;
    jclass javaClass;

    classDescriptor [0] = '\0';
    saj_classDescriptor (c_typeActualType(o->subType), classDescriptor, sizeof(classDescriptor), ctx->copyCache);
    javaClass = (*(ctx->javaEnv))->FindClass (ctx->javaEnv, classDescriptor);
#if JNI_TRACE
    printf ("JNI: FindClass (\"%s\") = 0x%x\n", classDescriptor, javaClass);
#endif
    objectSeqHeader.seqClass = (*(ctx->javaEnv))->NewGlobalRef (ctx->javaEnv, javaClass);
#if JNI_TRACE
    printf ("JNI: NewGlobalRef (\"%x\") = 0x%x\n", javaClass, objectSeqHeader.seqClass);
#endif
    (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, javaClass);
#if JNI_TRACE
    printf ("JNI: DeleteLocalRef (0x%x)\n", javaClass);
#endif
    saj_exceptionCheck (ctx->javaEnv);
    objectSeqHeader.seqSize = o->maxSize;
    objectSeqHeader.typeSize = c_type(c_typeActualType(o->subType))->size;
    objectSeqHeader.type = c_typeActualType(o->subType);

    TRACE (printf ("Array\n    Class name %s\n", classDescriptor));
    TRACE (printf ("    size %d\n", o->maxSize));
    TRACE (printf ("    Java class %x\n", (int)objectSeqHeader.seqClass));

    saj_cacheHeader ((sajCopyHeader *)hdr, sajSequence, sizeof(*hdr));
    headerIndex = saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
    /* Check if the subtype is in the scope of the subtype, in that case it is a recursive type */
    if (saj_isDefinedInScope (o)) {
        /* make back reference */
        saj_copyCacheBackReference (ctx->copyCache, saj_headerIndex(c_metaObject(c_typeActualType(o->subType)), ctx));
    } else {
        c_iterInsert (ctx->typeStack, saj_typeHistoryNew (c_metaObject(o), headerIndex));

        context = os_malloc (C_SIZEOF(saj_context));
        context->javaEnv = ctx->javaEnv;
        context->javaClass = objectSeqHeader.seqClass;
        context->copyCache = ctx->copyCache;
        context->typeStack = ctx->typeStack;
        saj_metaObject (c_typeActualType(o->subType), context, OS_FALSE);
        assert (context->typeStack == ctx->typeStack);
        os_free (context);

        saj_typeHistoryFree (c_iterTakeFirst (ctx->typeStack));
    }
    saj_copyCacheUpdateSize (ctx->copyCache, headerIndex);
}

STATIC void
saj_cacheSeqBooleanBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence booleanHeader;
    sajCopySequence* const hdr = &booleanHeader;

    TRACE (printf ("Boolean Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajSeqBoolean, sizeof(*hdr));
    booleanHeader.type = c_typeActualType(o->subType);
    booleanHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheSeqByteBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence byteHeader;
    sajCopySequence* const hdr = &byteHeader;

    TRACE (printf ("Byte Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajSeqByte, sizeof(*hdr));
    byteHeader.type = c_typeActualType(o->subType);
    byteHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheSeqCharBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence charHeader;
    sajCopySequence* const hdr = &charHeader;

    TRACE (printf ("Char Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajSeqChar, sizeof(*hdr));
    charHeader.type = c_typeActualType(o->subType);
    charHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheSeqShortBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence shortHeader;
    sajCopySequence* const hdr = &shortHeader;

    TRACE (printf ("Short Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajSeqShort, sizeof(*hdr));
    shortHeader.type = c_typeActualType(o->subType);
    shortHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheSeqIntBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence intHeader;
    sajCopySequence* const hdr = &intHeader;

    TRACE (printf ("Int Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajSeqInt, sizeof(*hdr));
    intHeader.type = c_typeActualType(o->subType);
    intHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheSeqLongBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence longHeader;
    sajCopySequence* const hdr = &longHeader;

    TRACE (printf ("Long Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajSeqLong, sizeof(*hdr));
    longHeader.type = c_typeActualType(o->subType);
    longHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheSeqFloatBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence floatHeader;
    sajCopySequence* const hdr = &floatHeader;

    TRACE (printf ("Float Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajSeqFloat, sizeof(*hdr));
    floatHeader.type = c_typeActualType(o->subType);
    floatHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheSeqDoubleBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence doubleHeader;
    sajCopySequence* const hdr = &doubleHeader;

    TRACE (printf ("Double Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajSeqDouble, sizeof(*hdr));
    doubleHeader.type = c_typeActualType(o->subType);
    doubleHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheStringBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyHeader stringHeader;

    OS_UNUSED_ARG(o);

    TRACE (printf ("String\n"));
    saj_cacheHeader (&stringHeader, sajString, sizeof(stringHeader));
    saj_copyCacheWrite (ctx->copyCache, &stringHeader, sizeof(stringHeader));
}

STATIC void
saj_cacheBStringBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyBoundedString bstringHeader;
    sajCopyBoundedString* const hdr = &bstringHeader;

    TRACE (printf ("BString\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajBString, sizeof(*hdr));
    bstringHeader.max = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));
}

STATIC void
saj_cacheBStringToArrCharBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyBoundedString bstringHeader;
    sajCopyBoundedString* const hdr = &bstringHeader;
    os_uint32 headerIndex;

    TRACE (printf ("BString To Char Array\n"));
    saj_cacheHeader ((sajCopyHeader *)hdr, sajBStringToArrChar, sizeof(*hdr));
    bstringHeader.max = o->maxSize;
    headerIndex = saj_copyCacheWrite (ctx->copyCache, hdr, sizeof(*hdr));

    c_iterInsert (ctx->typeStack, saj_typeHistoryNew (c_metaObject(o), headerIndex));
    cdrNoteCatsStac (ctx->copyCache->cdrInfo, ctx->typeStack);
    saj_typeHistoryFree (c_iterTakeFirst (ctx->typeStack));
}

STATIC void
saj_cacheEnumBuild (
    c_enumeration o,
    saj_context ctx)
{
    sajCopyEnum *copyEnum;
    char classDescriptor [512];
    char constrSignature [512];
    jclass javaClass, javaClassGR;
    jmethodID from_intID, valueMethodID = NULL;
    jfieldID valueFieldID;
    size_t copyEnumSize;
    unsigned int nrOfElements;
    sajEnumType enumType;
    jint i;

    classDescriptor [0] = '\0';
    saj_classDescriptor (c_type(o), classDescriptor, sizeof(classDescriptor), ctx->copyCache);
    nrOfElements = c_arraySize (o->elements);

    javaClass = (*(ctx->javaEnv))->FindClass (ctx->javaEnv, classDescriptor);
    saj_exceptionCheck (ctx->javaEnv);
#if JNI_TRACE
    printf ("JNI: FindClass (\"%s\") = 0x%x\n", classDescriptor, javaClass);
#endif
    javaClassGR = (*(ctx->javaEnv))->NewGlobalRef (ctx->javaEnv, javaClass);
#if JNI_TRACE
    printf ("JNI: NewGlobalRef (\"%s\") = 0x%x\n", classDescriptor, javaClassGR);
#endif
    (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, javaClass);
#if JNI_TRACE
    printf ("JNI: DeleteLocalRef (0x%x)\n", javaClass);
#endif
    saj_exceptionCheck (ctx->javaEnv);


    /* Now first try to see whether the enum is accessible through an optimized and supported interface. */
    /* Start with an attempt to find the saj-specific attribute holding the enum value. */
    enumType = SAJ_SUPPORTED_ENUM;
    valueFieldID = (*ctx->javaEnv)->GetFieldID (ctx->javaEnv, javaClassGR, "__value", "I");
    if (!valueFieldID)
    {
        /* When the enum is not saj-specific, clear the exception and attempt to find the the JacORB specific attribute. */
        (*ctx->javaEnv)->ExceptionClear(ctx->javaEnv);
        valueFieldID = (*ctx->javaEnv)->GetFieldID (ctx->javaEnv, javaClassGR, "value", "I");
        if (!valueFieldID)
        {
            /* When the enum is not JacOrb specific as well, clear the exception and locate the accessor method instead. */
            (*ctx->javaEnv)->ExceptionClear(ctx->javaEnv);
            enumType = SAJ_GENERIC_ENUM;
            valueMethodID = (*(ctx->javaEnv))->GetMethodID (ctx->javaEnv, javaClassGR, "value", "()I");
            saj_exceptionCheck (ctx->javaEnv);
        }
    }

#if JNI_TRACE
    if (enumType == SAJ_GENERIC_ENUM) {
        printf ("JNI: GetMethodID (0x%x, \"value\", \"()I\") = %d\n", valueMethodID);
    } else {
        assert(enumType == SAJ_SUPPORTED_ENUM);
        printf ("JNI: GetFieldID (0x%x, \"(__)value\", \"I\") = %d\n", javaClassGR, valueFieldID);
    }
#endif

    snprintf (constrSignature, sizeof(constrSignature), "(I)L%s;", classDescriptor);
    from_intID = (*(ctx->javaEnv))->GetStaticMethodID (
            ctx->javaEnv, javaClassGR, "from_int", constrSignature);
    saj_exceptionCheck (ctx->javaEnv);

    /* In case of a supported enum, cache all the references of each label. */
    if (from_intID)
    {
        if (enumType == SAJ_SUPPORTED_ENUM)
        {
            copyEnumSize = sizeof(sajCopyEnum) + (nrOfElements - 1) * sizeof(jfieldID);
            copyEnum = os_malloc(copyEnumSize);
            if (!copyEnum)
            {
                saj_exceptionThrow(ctx->javaEnv, SAJ_EXCEPTION_TYPE_NO_MEMORY,
                        "Unable to allocate sufficient memory to build up the SAJ copyCache");
                return;
            }
            copyEnum->enumAccessors.supportedEnum.valueID = valueFieldID;
            for (i = 0; i < (jint) nrOfElements; i++)
            {
                jobject enumLabel = (*(ctx->javaEnv))->CallStaticObjectMethod (
                        ctx->javaEnv, javaClassGR, from_intID, i);
                copyEnum->enumAccessors.supportedEnum.labels[i] = (*(ctx->javaEnv))->NewGlobalRef(
                        ctx->javaEnv, enumLabel);
                (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, enumLabel);
            }
        }
        else
        {
            copyEnumSize = sizeof(sajCopyEnum);
            copyEnum = os_malloc(copyEnumSize);
            if (!copyEnum)
            {
                saj_exceptionThrow(ctx->javaEnv, SAJ_EXCEPTION_TYPE_NO_MEMORY,
                        "Unable to allocate sufficient memory to build up the SAJ copyCache");
                return;
            }
            copyEnum->enumAccessors.genericEnum.from_intID = from_intID;
            copyEnum->enumAccessors.genericEnum.valueID = valueMethodID;
        }

        saj_cacheHeader ((sajCopyHeader *)copyEnum, sajEnum, copyEnumSize);
        copyEnum->nrOfElements = nrOfElements;
        copyEnum->enumClass = javaClassGR;
        copyEnum->enumType = enumType;

#if JNI_TRACE
        if (enumType == SAJ_SUPPORTED_ENUM) {
            for (i = 0; i < nrOfElements; i++) {
                printf ("JNI: GetFieldID (0x%x, label %d = %d\n",
                        javaClassGR, i, copyEnum->enumAccessors.supportedEnum.labels[i]);
            }
        } else {
            printf ("JNI: GetMethodID (0x%x, \"from_int\", \"%s\") = %d\n",
                    javaClassGR, constrSignature, valueMethodID);
        }
#endif

        TRACE (printf ("Enum\n    Class name %s\n", classDescriptor));
        TRACE (printf ("    Members # %d\n", nrOfElements));
        TRACE (printf ("    Java class %x\n", (int)javaClassGR));
        TRACE (printf ("    enum Type = %s\n", enumType == SAJ_SUPPORTED_ENUM ? "SAJ_SUPPORTED_ENUM" : "SAJ_GENERIC_ENUM"));

        saj_copyCacheWrite (ctx->copyCache, copyEnum, copyEnumSize);
        os_free(copyEnum);
    }
}

/* ES, dds1540: This operation verifies if the scopeName refers to an object
 * in the DDS package, if so OS_TRUE is returned, otherwise OS_FALSE is
 * returned.
 *
 * @param scopeName the scoped name to search in
 * @param separator The seperator between packages
 * @return OS_TRUE if the scoped name refers to a DDS object, OS_FALSE if not.
 */
os_boolean
saj_scopeNameRefersToDDSObject(
    const os_char* scopeName,
    const os_char separator)
{
    os_boolean isDdsObject;
    os_char* ptr;
    os_char* tmp ;

    assert(scopeName);

    tmp = os_strdup(scopeName);
    ptr = strchr(tmp, separator);
    if(ptr)
    {
        *ptr = '\0';
        if(0 == strcmp(tmp, "DDS"))
        {
            ptr++;
            ptr = strchr(ptr, separator);
            if(!ptr)
            {
                isDdsObject = OS_TRUE;
            } else
            {
                isDdsObject = OS_FALSE;
            }
        } else
        {
            isDdsObject = OS_FALSE;
        }
    } else
    {
        isDdsObject = OS_FALSE;
    }
    os_free(tmp);
    return isDdsObject;
}

/* ES, dds1540: this operation will return the scoped name of an object and will
 * have applied any required prefixing or substitution.
 *
 * @param object the object of which the scoped name is required
 * @param separator The seperator of packages
 * @param orgPName The original package name to search for, may be NULL.
 * @param tarPName The package name to replace all occurances of 'orgPName' with
 *                 or if 'orgPName' is NULL, then this package name will be
 *                 prefixed to every object. May be NULL
 * @return The scoped name of the object. Must be freed! Can also return NULL.
 */
os_char*
saj_getSubstitutedScopedName(
    c_metaObject object,
    const os_char separator,
    const os_char* orgPName,
    const os_char* tarPName)
{
    os_char* org;
    os_char* orgTmp;
    os_char* chr;
    os_char* substitutedPName;
    os_char* sepPtr;
    os_char* tarPNameSubbed;
    os_char* orgPNameSubbed;
    assert(object);

    /* Step 1: determine the 'original' scoped name, without any prefixing or
     * substitution
     */
    org = saj_getScopedName(object, separator);

    /* Step 2: Determine if the scoped name refers to an object in the DDS
     * package, if so we must ignore it from any prefixing or substitution
     * instructions
     */
    if(org && !saj_scopeNameRefersToDDSObject(org, separator))
    {
        if(!orgPName && tarPName)
        {
            /* just prepend */
            substitutedPName = os_malloc(strlen(tarPName) + strlen(org) + 1 /*length seperator*/ + 1 /* \0 */);
            sepPtr = saj_charToString(separator);
            tarPNameSubbed = saj_substitute(tarPName, ".", sepPtr);
            os_strcpy(substitutedPName, tarPNameSubbed);
            substitutedPName = os_strncat(substitutedPName, &separator, 1 /*length seperator*/);
            substitutedPName = os_strncat(substitutedPName, org, strlen(org));
            os_free(tarPNameSubbed);
            os_free(sepPtr);
        } else if(orgPName && tarPName)
        {
            sepPtr = saj_charToString(separator);
            orgPNameSubbed = saj_substitute(orgPName, ".", sepPtr);
            tarPNameSubbed = saj_substitute(tarPName, ".", sepPtr);

            if (c_baseObjectKind(object) == M_MODULE) {
                /* In case object is an IDL module type, we append the separator to the object name.
                 * This is so saj_substitute function can match the object name to the package name.
                 */
                orgTmp = os_malloc(strlen(org) + strlen(sepPtr) + 1);
                os_strcpy(orgTmp, org);
                orgTmp = os_strncat(orgTmp, sepPtr, strlen(sepPtr));
                substitutedPName = saj_substitute(orgTmp, orgPNameSubbed, tarPNameSubbed);
                /* cut separator off end of substituted result string */
                chr = substitutedPName + strlen(substitutedPName) - strlen(sepPtr);
                *chr = '\0';
                os_free(orgTmp);
            } else {
                substitutedPName = saj_substitute(org, orgPNameSubbed, tarPNameSubbed);
            }
            os_free(sepPtr);
            os_free(orgPNameSubbed);
            os_free(tarPNameSubbed);
        } else
        {
            /* no prepending / substitution needed! */
            substitutedPName = org;
            org = NULL;
        }
    } else
    {
       /* excluded from prepending / substitution! */
       substitutedPName = org;/* org could be null already */
       org = NULL;
    }
    if(org)
    {
        os_free(org);
    }

    return substitutedPName;
}

/* ES, dds1540: This exact operation is copied in the
 * tools/idlpp/code/idl_genJavaHelper.c file. so any bugs fixed here, should be
 * fixed there as well!!
 */
os_char*
saj_substitute(
    const os_char* string,
    const os_char* searchFor,
    const os_char* replaceWith)
{
    os_char* result;
    os_char* ptr;
    os_char* tmp;

    tmp = os_strdup(string);
    ptr = strstr(tmp, searchFor);
    if(ptr)
    {
        os_char* before;
        os_char* after;

        before = os_malloc(ptr - tmp+1);
        *ptr = '\0';
        os_strncpy(before, tmp, (size_t)(ptr - tmp+1));
        ptr = ptr+strlen(searchFor);
        after = saj_substitute(ptr, searchFor, replaceWith);
        result = os_malloc(strlen(before) + strlen(replaceWith) + strlen (after) + 1);
        os_strcpy(result, before);
        os_strncat(result, replaceWith, strlen(replaceWith));
        os_strncat(result, after, strlen(after));
        os_free(before);
        os_free(after);
    } else
    {
        result = tmp;
        tmp = NULL;
    }
    if(tmp)
    {
        os_free(tmp);
    }
    return result;
}

os_char*
saj_charToString(
    const os_char source)
{
    os_char* result;

    result = os_malloc(2); /* 1 for the char to be converted, and 1 for the '\0' */
    memset(result, 0, 2);
    *result = source;

    return result;

}

/* ES, dds1540: This operation will return the scoped name of 'object'. This
 * operation will not take any potential substitution or prefixing rules into
 * account! For that use the 'saj_getSubstitutedScopedName' operation.
 * This operation is implemented as a two pass operation, it will first
 * calculate the length of the scoped name and then copy the scoped name in.
 * The reason for this approach is because we are always starting with the inner
 * object and have to backtrace towards the outer modules, yet the outer modules
 * need to be printed first. Now this can be solved in many ways, a two pass
 * approach was chosen here because it is avoids many mallocs and free's and
 * because we then do not have to work with some buffer with a limited length
 * which in theory could not be enough to hold the entire scoped name.
 *
 * @param object The object for which the scoped name is required.
 * @param separator The seperator of packages
 * @return The scoped name of the object. Must be freed! Can be NULL!
 */
os_char*
saj_getScopedName(
    c_metaObject object,
    const os_char separator)
{
    c_metaObject obj;
    os_char* scopeName;
    os_char* scopeNameEnd;
    os_uint32 nameLength = 0;
    os_uint32 sepLength;

    assert(object);

    if(object && object->name)
    {
        sepLength = 1 /*length seperator*/;
        /* Step 1: We need to determine the scope name length. We do this by
         * traversing all containing objects and determine the length of each of
         * their names.
         */
        obj = c_metaObject(object);
        while(obj && obj->name)
        {
            if (obj == object &&
                (c_baseObject(obj)->kind == M_STRUCTURE ||
                 c_baseObject(obj)->kind == M_UNION))
            {
                nameLength += strlen(c_metaObject(obj)->name);
                nameLength += strlen("Package");
            }
            else
            {
                nameLength += strlen(saj_dekeyedId(c_metaObject(obj)->name));
            }

            obj = c_metaObject(obj)->definedIn;
            /* Only if there is a next object, should there be a separator included
             */
            if(obj)
            {
                nameLength += sepLength;
            }
        }
        /* Step 2a: Allocate memory for the scoped name with the found size. */
        scopeName = os_malloc(nameLength + 1);
        memset(scopeName, 0, nameLength + 1);
        /* Step 2b: Move the pointer of the newly allocated string to the back, as
         * we are going reversely through the list of objects to construct the
         * scoped name.
         */
        scopeNameEnd = scopeName + nameLength - 1;
        /* Step 3: Copy the name values into the scopeName */
        obj = c_metaObject(object);
        while(obj && obj->name)
        {
            os_char* name;
            os_uint32 length;
            os_uint32 anonPackageLength;
            os_char* anonPackage;

            /* Step 3a: Get the name of the object and determine it's length */
            if (obj == object &&
                (c_baseObject(obj)->kind == M_STRUCTURE ||
                 c_baseObject(obj)->kind == M_UNION))
            {
                name = c_metaObject(obj)->name;
                anonPackage = "Package";
                anonPackageLength = strlen(anonPackage);
            } else
            {
                name = saj_dekeyedId(c_metaObject(obj)->name);
                anonPackage = NULL;
                anonPackageLength = 0;
            }
            length = strlen(name);

            /* Step 3b: Move the pointer back by the 'length' amount. I.E., we are
             * moving the pointer towards the original 'scopeName' pointer again.
             */
            scopeNameEnd = scopeNameEnd - anonPackageLength;
            assert(scopeName <= scopeNameEnd);
            if(anonPackage)
            {
                os_strncpy(scopeNameEnd, anonPackage, anonPackageLength);
            }
            scopeNameEnd = scopeNameEnd - length;
            assert(scopeName <= scopeNameEnd);
            /* Step 3c: Copy the name into the scopeName. */
            os_strncpy(scopeNameEnd, name, length);
            /* Step 3d: If we have not yet reached the original scopeName pointer
             * then we need to copy a separator into the scopeName to prepare for
             * the next name to be prepended.
             */
            if(scopeName != scopeNameEnd)
            {
                scopeNameEnd = scopeNameEnd - sepLength;
                assert(scopeName < scopeNameEnd);
                os_strncpy(scopeNameEnd, &separator, sepLength);
            }
            obj = c_metaObject(obj)->definedIn;
        }
        assert(scopeName == scopeNameEnd);
    } else
    {
        scopeName = NULL;
    }
    return scopeName;
}

STATIC void
saj_scopedTypeName (
    c_char *buffer,
    c_long bufferSize,
    c_metaObject object,
    const os_char separator,
    saj_copyCache copyCache)
{
    os_char* scopeName;
    os_char tmp[2];

    if(object->definedIn)
    {
        scopeName = saj_getSubstitutedScopedName(object->definedIn, separator, copyCache->orgPName, copyCache->tgtPName);
        if(scopeName)
        {
            os_strncat (buffer, scopeName, bufferSize);
            tmp[0] = separator;
            tmp[1] = '\0';
            os_strncat (buffer, tmp, bufferSize);
            os_free(scopeName);
        }/* else null was returned which means the object was not defined inside a module */
    }
    os_strncat (buffer, saj_dekeyedId(c_metaObject(object)->name), bufferSize);
}


STATIC void
saj_fieldDescriptor (
    c_type type,
    char *descriptor,
    unsigned int size,
    saj_copyCache copyCache)
{
    assert (type);
    assert (descriptor);

    switch (c_baseObject(type)->kind) {
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
    case M_ANNOTATION:
        assert(FALSE);
        break;
    case M_PRIMITIVE:
        switch (c_primitive(type)->kind) {
        case P_BOOLEAN:
            /* boolean */
            snprintf(descriptor, size - 1, "Z");
            break;
        case P_SHORT:
            /* short */
            snprintf(descriptor, size - 1, "S");
            break;
        case P_LONG:
            /* int */
            snprintf(descriptor, size - 1, "I");
            break;
        case P_LONGLONG:
            /* long */
            snprintf(descriptor, size - 1, "J");
            break;
        case P_USHORT:
            /* short */
            snprintf(descriptor, size - 1, "S");
            break;
        case P_ULONG:
            /* int */
            snprintf(descriptor, size - 1, "I");
            break;
        case P_ULONGLONG:
            /* long */
            snprintf(descriptor, size - 1, "J");
            break;
        case P_CHAR:
            /* char */
            snprintf(descriptor, size - 1, "C");
            break;
        case P_OCTET:
            /* byte */
            snprintf(descriptor, size - 1, "B");
            break;
        case P_FLOAT:
            /* float */
            snprintf(descriptor, size - 1, "F");
            break;
        case P_DOUBLE:
            /* double */
            snprintf(descriptor, size - 1, "D");
            break;
        default:
            assert(FALSE);
        }
        break;
    case M_COLLECTION:
        if (c_collectionType(type)->kind == C_STRING) {
            snprintf (descriptor, size-1, "Ljava/lang/String;");
        } else if (c_collectionType(type)->kind == C_SEQUENCE) {
            /* sequence */
            if (c_baseObject(c_typeActualType(c_collectionType(type)->subType))->kind == M_PRIMITIVE) {
                snprintf (descriptor, size-1, "[");
                saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1, copyCache);
            } else {
                snprintf (descriptor, size-1, "[");
                saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1, copyCache);
            }
        } else if (c_collectionType(type)->kind == C_ARRAY) {
            /* array */
            if (c_baseObject(c_typeActualType(c_collectionType(type)->subType))->kind == M_PRIMITIVE) {
                snprintf (descriptor, size-1, "[");
                saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1, copyCache);
            } else {
                snprintf (descriptor, size-1, "[");
                saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1, copyCache);
            }
        }
        break;
    case M_ENUMERATION:
        os_strncpy(descriptor, "L", size);
        saj_scopedTypeName(descriptor, size, c_metaObject(type), '/', copyCache);
        strncat(descriptor, ";", size);
        break;
    case M_MODULE:
        break;
    case M_STRUCTURE:
        os_strncpy(descriptor, "L", size);
        saj_scopedTypeName(descriptor, size, c_metaObject(type), '/', copyCache);
        strncat(descriptor, ";", size);
        break;
    case M_TYPEDEF:
        saj_fieldDescriptor(c_typeDef(type)->alias, descriptor, size, copyCache);
        break;
    case M_UNION:
        os_strncpy(descriptor, "L", size);
        saj_scopedTypeName(descriptor, size, c_metaObject(type), '/', copyCache);
        strncat(descriptor, ";", size);
        break;
    }
    assert (strlen (descriptor) != (size - 1));
}

STATIC void
saj_classDescriptor (
    c_type type,
    char *descriptor,
    unsigned int size,
    saj_copyCache copyCache)
{
    assert (type);
    assert (descriptor);

    switch (c_baseObject(type)->kind) {
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
    case M_ANNOTATION:
        assert(FALSE);
        break;
    case M_PRIMITIVE:
        switch (c_primitive(type)->kind) {
        case P_BOOLEAN:
            /* boolean */
            snprintf(descriptor, size - 1, "Z");
            break;
        case P_SHORT:
            /* short */
            snprintf(descriptor, size - 1, "S");
            break;
        case P_LONG:
            /* int */
            snprintf(descriptor, size - 1, "I");
            break;
        case P_LONGLONG:
            /* long */
            snprintf(descriptor, size - 1, "J");
            break;
        case P_USHORT:
            /* short */
            snprintf(descriptor, size - 1, "S");
            break;
        case P_ULONG:
            /* int */
            snprintf(descriptor, size - 1, "I");
            break;
        case P_ULONGLONG:
            /* long */
            snprintf(descriptor, size - 1, "J");
            break;
        case P_CHAR:
            /* char */
            snprintf(descriptor, size - 1, "C");
            break;
        case P_OCTET:
            /* byte */
            snprintf(descriptor, size - 1, "B");
            break;
        case P_FLOAT:
            /* float */
            snprintf(descriptor, size - 1, "F");
            break;
        case P_DOUBLE:
            /* double */
            snprintf(descriptor, size - 1, "D");
            break;
        default:
            assert(FALSE);
        }
        break;
    case M_COLLECTION:
        if (c_collectionType(type)->kind == C_STRING) {
            /* string */
            snprintf (descriptor, size-1, "java/lang/String");
        } else if (c_collectionType(type)->kind == C_SEQUENCE) {
            /* sequence */
            if (c_baseObject(c_typeActualType(c_collectionType(type)->subType))->kind == M_PRIMITIVE) {
                snprintf (descriptor, size-1, "[");
                saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1, copyCache);
            } else {
                snprintf (descriptor, size-1, "[");
                saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1, copyCache);
            }
        } else if (c_collectionType(type)->kind == C_ARRAY) {
            /* array */
            if (c_baseObject(c_typeActualType(c_collectionType(type)->subType))->kind == M_PRIMITIVE) {
                snprintf (descriptor, size-1, "[");
                saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1, copyCache);
            } else {
                snprintf (descriptor, size-1, "[");
                saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1, copyCache);
            }
        }
        break;
    case M_ENUMERATION:
        saj_scopedTypeName(descriptor, size, c_metaObject(type), '/', copyCache);
        break;
    case M_MODULE:
        break;
    case M_STRUCTURE:
        saj_scopedTypeName(descriptor, size, c_metaObject(type), '/', copyCache);
        break;
    case M_TYPEDEF:
        saj_classDescriptor(c_typeDef(type)->alias, descriptor, size, copyCache);
        break;
    case M_UNION:
        saj_scopedTypeName(descriptor, size, c_metaObject(type), '/', copyCache);
        break;
    }
    assert (strlen (descriptor) != (size - 1));
}

void
cacheDump (
    sajCopyHeader *ch,
    unsigned int level)
{
    unsigned int mi;
    sajCopyStruct *csh;
    sajCopyStructMember *csm;
    sajCopyObjectArray *oah;
    sajCopyType ct;
    unsigned int l;

    ct = ch->copyType;

    for (l = 0; l < level; l++) {
        printf ("  ");
    }
    printf ("T:%d S:%d\n", ct, ch->size);

    for (l = 0; l < level; l++) {
        printf ("  ");
    }

    switch (ct) {
    case sajBoolean:
        printf("Boolean\n");
        break;
    case sajByte:
        printf("Byte\n");
        break;
    case sajChar:
        printf("Char\n");
        break;
    case sajShort:
        printf("Short\n");
        break;
    case sajInt:
        printf("Int\n");
        break;
    case sajLong:
        printf("Long\n");
        break;
    case sajFloat:
        printf("Float\n");
        break;
    case sajDouble:
        printf("Double\n");
        break;
    case sajArrBoolean:
        printf("ArrBoolean\n");
        break;
    case sajArrByte:
        printf("ArrByte\n");
        break;
    case sajArrChar:
        printf("ArrChar\n");
        break;
    case sajArrCharToBString:
        printf("sajArrCharToBString\n");
        break;
    case sajArrShort:
        printf("ArrShort\n");
        break;
    case sajArrInt:
        printf("ArrInt\n");
        break;
    case sajArrLong:
        printf("ArrLong\n");
        break;
    case sajArrFloat:
        printf("ArrFloat\n");
        break;
    case sajArrDouble:
        printf("ArrDouble\n");
        break;
    case sajSeqBoolean:
        printf("SeqBoolean\n");
        break;
    case sajSeqByte:
        printf("SeqByte\n");
        break;
    case sajSeqChar:
        printf("SeqChar\n");
        break;
    case sajSeqShort:
        printf("SeqShort\n");
        break;
    case sajSeqInt:
        printf("SeqInt\n");
        break;
    case sajSeqLong:
        printf("SeqLong\n");
        break;
    case sajSeqFloat:
        printf("SeqFloat\n");
        break;
    case sajSeqDouble:
        printf("SeqDouble\n");
        break;
    case sajString:
        printf("String\n");
        break;
    case sajBString:
        printf("BString\n");
        break;
    case sajBStringToArrChar:
        printf("sajBStringToArrChar\n");
        break;
    case sajEnum:
        printf("Enum\n");
        break;
    case sajStruct:
        csh = (sajCopyStruct *) ch;
        for (l = 0; l < level; l++) {
            printf("  ");
        }
        csm = (sajCopyStructMember *) ((PA_ADDRCAST) csh + sizeof(sajCopyStruct));
        printf ("M#:%d CL:"PA_ADDRFMT" CID:"PA_ADDRFMT"\n", csh->nrOfMembers,
                (PA_ADDRCAST)csh->structClass, (PA_ADDRCAST)csh->constrID);

        for (mi = 0; mi < csh->nrOfMembers; mi++) {
            for (l = 0; l < level; l++) {
                printf("  ");
            }
            printf ("M@:%d/%d %d, FID:"PA_ADDRFMT"\n", mi+1, csh->nrOfMembers, csm->memberOffset, (PA_ADDRCAST)csm->javaFID);
            ch = (sajCopyHeader *) ((PA_ADDRCAST) csm
                    + sizeof(sajCopyStructMember));
            cacheDump(ch, level + 1);
            csm = (sajCopyStructMember *) ((PA_ADDRCAST) ch + ch->size);
        }
        break;
    case sajUnion:
        printf("Union\n");
        break;
    case sajArray:
        oah = (sajCopyObjectArray *) ch;
        printf("Array\n");
        for (l = 0; l < level; l++) {
            printf("  ");
        }
        printf("E#:%d CL:"PA_ADDRFMT" TS:%d\n", oah->arraySize, (PA_ADDRCAST) oah->arrayClass, oah->typeSize);
        cacheDump((sajCopyHeader *) ((PA_ADDRCAST) oah + oah->header.size), level + 1);
        break;
    case sajSequence:
        printf("Sequence\n");
        break;
    case sajRecursive:
        printf("Recursive\n");
        break;
    }
}

void
saj_copyCacheDump (
    saj_copyCache copyCache)
{
    cacheDump ((sajCopyHeader *)copyCache->cache, 0);
}

STATIC void
saj_copyReaderCacheBuild (
    saj_copyCache copyCache,
    c_metaObject object,
    JNIEnv *env)
{
    char tmp[512];
    sajReaderCopyCache *readerCache = saj_copyCacheReaderCache (copyCache);
    jobject javaObject;

    tmp[0] = '\0';
    saj_classDescriptor (c_type(object), tmp, sizeof(tmp), copyCache);
    javaObject = (*env)->FindClass (env, tmp);
#if JNI_TRACE
    printf ("JNI: FindClass (\"%s\") = 0x%x\n", tmp, javaObject);
#endif
    readerCache->dataClass = (*env)->NewGlobalRef (env, javaObject);
#if JNI_TRACE
    printf ("JNI: NewGlobalRef (0x%x) = 0x%x\n", javaObject, readerCache->dataClass);
#endif
    (*env)->DeleteLocalRef (env, javaObject);
#if JNI_TRACE
    printf ("JNI: DeleteLocalRef (0x%x)\n", javaObject);
#endif
    saj_exceptionCheck (env);

    readerCache->dataClass_constructor_mid = (*env)->GetMethodID (env, readerCache->dataClass, "<init>", "()V");

    tmp[0] = '\0';
    saj_classDescriptor (c_type(object), tmp, sizeof(tmp), copyCache);
    os_strncat (tmp, "Holder", sizeof(tmp));
    javaObject = (*env)->FindClass (env, tmp);
#if JNI_TRACE
    printf ("JNI: FindClass (\"%s\") = 0x%x\n", tmp, javaObject);
#endif
    readerCache->dataHolderClass = (*env)->NewGlobalRef (env, javaObject);
#if JNI_TRACE
    printf ("JNI: NewGlobalRef (0x%x) = 0x%x\n", javaObject, readerCache->dataHolderClass);
#endif
    (*env)->DeleteLocalRef (env, javaObject);
#if JNI_TRACE
    printf ("JNI: DeleteLocalRef (0x%x)\n", javaObject);
#endif
    saj_exceptionCheck (env);

    tmp[0] = '\0';
    saj_classDescriptor (c_type(object), tmp, sizeof(tmp), copyCache);
    os_strncat (tmp, "SeqHolder", sizeof(tmp));
    javaObject = (*env)->FindClass (env, tmp);
#if JNI_TRACE
    printf ("JNI: FindClass (\"%s\") = 0x%x\n", tmp, javaObject);
#endif
    if (javaObject) {
        readerCache->dataSeqHolderClass = (*env)->NewGlobalRef (env, javaObject);
#if JNI_TRACE
        printf ("JNI: NewGlobalRef (0x%x) = 0x%x\n", javaObject, readerCache->dataSeqHolderClass);
#endif
        (*env)->DeleteLocalRef (env, javaObject);
#if JNI_TRACE
        printf ("JNI: DeleteLocalRef (0x%x)\n", javaObject);
#endif
    } else {
        (*env)->ExceptionClear (env);
        readerCache->dataSeqHolderClass = NULL;
    }

    tmp[0] = '\0';
    saj_fieldDescriptor (c_type(object), &tmp[strlen(tmp)], sizeof(tmp)-strlen(tmp), copyCache);
    readerCache->dataHolder_value_fid = (*env)->GetFieldID (env, readerCache->dataHolderClass, "value", tmp);
#if JNI_TRACE
    printf ("JNI: GetFieldID (0x%x, \"%s\", \"%s\") = %d\n", readerCache->dataHolderClass, "value", tmp, readerCache->dataHolder_value_fid);
#endif
    saj_exceptionCheck (env);
    if (readerCache->dataSeqHolderClass) {
        /* sequence holder class is only generated for interfaces (datareader/datawriter) */
        os_strncpy (tmp, "[", sizeof(tmp));
        saj_fieldDescriptor (c_type(object), &tmp[strlen(tmp)], sizeof(tmp)-strlen(tmp), copyCache);
        readerCache->dataSeqHolder_value_fid = (*env)->GetFieldID (env, readerCache->dataSeqHolderClass, "value", tmp);
#if JNI_TRACE
        printf ("JNI: GetFieldID (0x%x, \"%s\", \"%s\") = %d\n", readerCache->dataSeqHolderClass, "value", tmp, readerCache->dataSeqHolder_value_fid);
#endif
        saj_exceptionCheck (env);
    }
}

STATIC void
saj_copyCacheBuild (
    saj_copyCache copyCache,
    c_metaObject object,
    JNIEnv *env)
{
    saj_context context;

    assert (copyCache);
    assert (object);

    context = os_malloc (C_SIZEOF(saj_context));
    if (context) {
        context->copyCache = copyCache;
        context->javaEnv = env;
        context->typeStack = c_iterNew (NULL);
        saj_metaObject (c_type(object), context, OS_FALSE);
        saj_copyCacheFinalize (context->copyCache);
        c_iterFree (context->typeStack);
        TRACE (saj_copyCacheDump (context->copyCache));
    }
    os_free (context);
}
