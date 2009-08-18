/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include <jni.h>

#include <os_stdlib.h>
#include <os_heap.h>
#include <os_abstract.h>
#include <c_base.h>
#include <c_iterator.h>

#include "saj_copyCache.h"
#include "saj_utilities.h"

/*#define JNI_TRACE 1*/
#define TRACE(function)		/*function*/
#define CACHE_BLOCKSIZE		(100)
#define STATIC

C_CLASS(saj_context);
#define saj_context(o)		((saj_context)(o))

C_STRUCT(saj_context) {
    saj_copyCache 	copyCache;
    jclass		javaClass;
    JNIEnv		*javaEnv;
    c_iter		typeStack;
};

C_STRUCT(saj_copyCache) {
    void 		*cache;  /* start of cache  */
    c_long 		length;  /* length of cache */
    c_long 		iWrite;  /* write index     */
    sajReaderCopyCache	readerCache;
};

C_CLASS(saj_typeHistory);
#define saj_typeHistory(o)	((saj_typeHistory)(o))

C_STRUCT(saj_typeHistory) {
    c_metaObject	metaObject;
    c_long		cacheIndex;
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
    c_long index)
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

c_long
saj_typeHistoryIndex (
    saj_typeHistory history)
{
    return history->cacheIndex;
}

STATIC void saj_copyReaderCacheBuild (saj_copyCache copyCache, c_metaObject object, JNIEnv *env);
STATIC void saj_copyCacheBuild (saj_copyCache copyCache, c_metaObject object, JNIEnv *env);
STATIC c_long saj_copyCacheWrite (saj_copyCache copyCache, void *data, c_long size);
STATIC void saj_copyCacheWriteIndex (saj_copyCache copyCache, void *data, c_long size, c_long index);
STATIC void saj_copyCacheFinalize (saj_copyCache copyCache);
STATIC void saj_cacheHeader (sajCopyHeader *header, sajCopyType type, unsigned short size);

STATIC void saj_metaObject (c_type o, saj_context context);
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
STATIC void saj_cacheEnumBuild (c_enumeration o, saj_context ctx);
STATIC void saj_cacheArrObjectBuild (c_collectionType o, saj_context ctx);
STATIC void saj_cacheSeqObjectBuild (c_collectionType o, saj_context ctx);
STATIC void saj_scopedTypeName (c_char *buffer, c_long bufferSize, c_metaObject object, const c_char *separator);
STATIC void saj_fieldDescriptor (c_type type, char *descriptor, unsigned int size);
STATIC void saj_classDescriptor (c_type type, char *descriptor, unsigned int size);

saj_copyCache
saj_copyCacheNew (
    JNIEnv *env,
    c_metaObject object)
{
    saj_copyCache copyCache = os_malloc (C_SIZEOF(saj_copyCache));

    if (copyCache) {
	copyCache->cache = os_malloc (CACHE_BLOCKSIZE);
	if (copyCache->cache) {
	    copyCache->length = CACHE_BLOCKSIZE;
	    copyCache->iWrite = 0;
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

    os_free (copyCache->cache);
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

sajReaderCopyCache *
saj_copyCacheReaderCache (
    saj_copyCache copyCache)
{
    assert (copyCache);

    return (&copyCache->readerCache);
}

c_long
saj_copyCacheWrite (
    saj_copyCache copyCache,
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
	additionalLength = ((copyCache->iWrite + size)/CACHE_BLOCKSIZE) * CACHE_BLOCKSIZE;
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
    memcpy ((void *)((PA_ADDRCAST)copyCache->cache + copyCache->iWrite), data, size);
    copyCache->iWrite += size;
    return writeIndex;
}

void
saj_copyCacheWriteIndex (
    saj_copyCache copyCache,
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
saj_copyCacheUpdateSize (
    saj_copyCache copyCache,
    c_long headerIndex)
{
    short length;
    sajCopyHeader *hAddr;

    assert (copyCache);

    hAddr = (sajCopyHeader *)((PA_ADDRCAST)copyCache->cache + headerIndex);
    length = copyCache->iWrite - headerIndex;
    hAddr->size = length;
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
    saj_context context)
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
    case M_EXTENT:
    case M_MODULE:
	assert (FALSE);
        break;
    case M_PRIMITIVE:
        switch (c_primitive(o)->kind) {
	case P_BOOLEAN:
	    /* boolean */
	    saj_cacheBooleanBuild (c_primitive(o), context);
	    break;
	case P_SHORT:
	    /* short */
	    saj_cacheShortBuild (c_primitive(o), context);
	    break;
	case P_LONG:
	    /* int */
	    saj_cacheIntBuild (c_primitive(o), context);
	    break;
	case P_LONGLONG:
	    /* long */
	    saj_cacheLongBuild (c_primitive(o), context);
	    break;
	case P_USHORT:
	    /* short */
	    saj_cacheShortBuild (c_primitive(o), context);
	    break;
	case P_ULONG:
	    /* int */
	    saj_cacheIntBuild (c_primitive(o), context);
	    break;
	case P_ULONGLONG:
	    /* long */
	    saj_cacheLongBuild (c_primitive(o), context);
	    break;
	case P_CHAR:
	    /* char */
	    saj_cacheCharBuild (c_primitive(o), context);
	    break;
	case P_OCTET:
	    /* byte */
	    saj_cacheByteBuild (c_primitive(o), context);
	    break;
	case P_FLOAT:
	    /* float */
	    saj_cacheFloatBuild (c_primitive(o), context);
	    break;
	case P_DOUBLE:
	    /* double */
	    saj_cacheDoubleBuild (c_primitive(o), context);
	    break;
        default:
	    assert (FALSE);
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
                    saj_cacheArrCharBuild (c_collectionType(o), context);
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
                saj_cacheArrObjectBuild (c_collectionType(o), context);
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
        saj_metaObject (c_typeDef(o)->alias, context);
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
    unsigned short size)
{
    header->copyType = type;
    header->size = size;
}

STATIC void
saj_cacheStructBuild (
    c_structure o,
    saj_context ctx)
{
    C_STRUCT(saj_context) context;
    sajCopyStruct copyStruct;
    c_long headerIndex;
    c_long mi;
    char classDescriptor [512];
    jclass javaClass;

    classDescriptor [0] = '\0';
    saj_classDescriptor (c_type(o), classDescriptor, sizeof(classDescriptor));

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

STATIC void
saj_cacheStructMember (
    c_member o,
    saj_context ctx)
{
    sajCopyStructMember member;
    char fieldDescriptor[512];

    fieldDescriptor[0] = '\0';
    saj_fieldDescriptor (c_specifier(o)->type, fieldDescriptor, sizeof (fieldDescriptor));

    member.memberOffset = o->offset;
    member.javaFID = (*ctx->javaEnv)->GetFieldID (
	ctx->javaEnv, ctx->javaClass, saj_dekeyedId(c_specifier(o)->name), fieldDescriptor);
    saj_exceptionCheck (ctx->javaEnv);
#if JNI_TRACE
    printf ("JNI: GetFieldID (0x%x) = %d\n", ctx->javaClass, member.javaFID);
#endif

    TRACE (printf ("    Struct Member @ %d\n", member.memberOffset));
    TRACE (printf ("        Java field descriptor = %s, Field name = %s, Java FID = %x\n",
	fieldDescriptor, saj_dekeyedId(c_specifier(o)->name), (int)member.javaFID));

    saj_copyCacheWrite (ctx->copyCache, &member, sizeof(member));
    saj_metaObject (c_specifier(o)->type, ctx);
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
    saj_context ctx)
{
    sajCopyUnionCase unionCase;
    char fieldDescriptor[512];
    char attributeName[512];
    char operatorDescr[512];
    jthrowable jexception;

    fieldDescriptor[0] = '\0';
    saj_fieldDescriptor (c_specifier(o)->type, fieldDescriptor, sizeof (fieldDescriptor));

    /* Look for a setter that implicitly sets the discriminator to 1 of the appropriate cases. */
    snprintf (operatorDescr, sizeof (operatorDescr), "(%s)V", fieldDescriptor);
    unionCase.setterID = (*(ctx->javaEnv))->GetMethodID (ctx->javaEnv, ctx->javaClass,
            saj_dekeyedId(c_specifier(o)->name), operatorDescr);
    saj_exceptionCheck (ctx->javaEnv);
#if JNI_TRACE
    printf ("JNI: GetMethodID (0x%x, \"%s\", \"%s\") = %d\n", ctx->javaClass, saj_dekeyedId(c_specifier(o)->name), operatorDescr, unionCase.setterID);
#endif

    /* Look for the corresponding getter of that branch. */
    snprintf (operatorDescr, sizeof (operatorDescr), "()%s", fieldDescriptor);
    unionCase.getterID = (*(ctx->javaEnv))->GetMethodID (ctx->javaEnv, ctx->javaClass,
	saj_dekeyedId(c_specifier(o)->name), operatorDescr);
    saj_exceptionCheck (ctx->javaEnv);
#if JNI_TRACE
    printf ("JNI: GetMethodID (0x%x, \"%s\", \"%s\") = %d\n", ctx->javaClass, c_specifier(o)->name, operatorDescr, unionCase.getterID);
#endif

    /* Look for the field that holds the branch value (Only applicable to the SAJ). */
    snprintf (attributeName, sizeof(attributeName), "__%s", saj_dekeyedId(c_specifier(o)->name));
    unionCase.caseID = (*(ctx->javaEnv))->GetFieldID (ctx->javaEnv, ctx->javaClass, attributeName, fieldDescriptor);
    jexception = (*(ctx->javaEnv))->ExceptionOccurred(ctx->javaEnv);
    if (jexception)
    {
        /*  clear the exception, as the case ID is only present when java data model is generated with idlpp */
        (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);
    }
#if JNI_TRACE
    else
    {
        printf ("JNI: GetFieldID (0x%x, \"%s\", \"%s\") = %d\n", ctx->javaClass, attributeName, fieldDescriptor, unionCase.caseID);
    }
#endif

    TRACE (printf ("    Union Case\n"));
    TRACE (printf ("        Java field descriptor = %s, Field name = %s, Java setterID = %x, Java getterID = %x\n",
	fieldDescriptor, saj_dekeyedId(c_specifier(o)->name), (int)unionCase.setterID,  (int)unionCase.getterID));

    /* Also look for a possible setter that includes the discriminator as 1st parameter. */
    fieldDescriptor[0] = '\0';
    saj_fieldDescriptor (switchType, fieldDescriptor, sizeof (fieldDescriptor));
    saj_fieldDescriptor (c_specifier(o)->type, &fieldDescriptor[strlen(fieldDescriptor)], sizeof (fieldDescriptor) - strlen(fieldDescriptor));
    snprintf (operatorDescr, sizeof (operatorDescr), "(%s)V", fieldDescriptor);
    unionCase.setterWithDiscrID = (*(ctx->javaEnv))->GetMethodID (ctx->javaEnv, ctx->javaClass,
            saj_dekeyedId(c_specifier(o)->name), operatorDescr);
#if JNI_TRACE
    printf ("JNI: GetMethodID (0x%x, \"%s\", \"%s\") = %d\n", ctx->javaClass, c_specifier(o)->name, operatorDescr, unionCase.setterWithDiscrID);
#endif
    if ( (*(ctx->javaEnv))->ExceptionCheck (ctx->javaEnv))
        (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);

    saj_copyCacheWrite (ctx->copyCache, &unionCase, sizeof(unionCase));
    saj_metaObject (c_specifier(o)->type, ctx);
}

STATIC void
saj_cacheUnionCase (
    c_unionCase unionCase,
    c_type switchType,
    saj_context ctx)
{
    sajCopyUnionLabels labels;
    c_ulong l;

    labels.labelCount = c_arraySize (unionCase->labels);
    saj_copyCacheWrite (ctx->copyCache, &labels, sizeof(labels));

    TRACE (printf ("    labels count = %d\n", labels.labelCount));

    for (l = 0; l < labels.labelCount; l++) {
        saj_cacheUnionLabel (c_literal(unionCase->labels[l]), ctx);
    }
    saj_cacheUnionCaseField (unionCase, switchType, ctx);
}

STATIC void
saj_cacheUnionBuild (
    c_union o,
    saj_context ctx)
{
    C_STRUCT(saj_context) context;
    sajCopyUnion copyUnion;
    c_long headerIndex;
    c_long mi;
    char classDescriptor [512];
    char discrDescriptor [512];
    char methodDescriptor [512];
    char methodSignature [512];
    jclass javaClass;
    jthrowable jexception;

    classDescriptor [0] = '\0';
    saj_classDescriptor (c_type(o), classDescriptor, sizeof(classDescriptor));
    discrDescriptor[0] = '\0';
    saj_fieldDescriptor (c_typeActualType(o->switchType), discrDescriptor, sizeof (discrDescriptor));

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
    copyUnion.constrID = (*(ctx->javaEnv))->GetMethodID (
	context.javaEnv, copyUnion.unionClass, "<init>", "()V");
    saj_exceptionCheck (ctx->javaEnv);
#if JNI_TRACE
    printf ("JNI: GetMethodID (0x%x, \"<init>\", \"()V\") = %d\n", copyUnion.unionClass, copyUnion.constrID);
#endif
    snprintf (methodDescriptor, sizeof (methodDescriptor), "()%s", discrDescriptor);
    if (c_baseObject(c_typeActualType(o->switchType))->kind == M_ENUMERATION) {
	classDescriptor[0] = '\0';
        saj_classDescriptor (c_typeActualType(o->switchType), classDescriptor, sizeof(classDescriptor));
	javaClass = (*(ctx->javaEnv))->FindClass (ctx->javaEnv, classDescriptor);
#if JNI_TRACE
        printf ("JNI: FindClass (\"%s\") = 0x%x\n", classDescriptor, javaClass);
#endif
        copyUnion.discrClass = (*(ctx->javaEnv))->NewGlobalRef (ctx->javaEnv, (jobject)javaClass);
#if JNI_TRACE
        printf ("JNI: NewGlobalRef (0x%x) = 0x%x\n", javaClass, copyUnion.discrClass);
#endif
        (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, javaClass);
#if JNI_TRACE
        printf ("JNI: DeleteLocalRef (0x%x)\n", javaClass);
#endif
        saj_exceptionCheck (ctx->javaEnv);
        copyUnion.valueID = (*(ctx->javaEnv))->GetMethodID (
	    ctx->javaEnv, copyUnion.discrClass, "value", "()I");
        saj_exceptionCheck (ctx->javaEnv);
#if JNI_TRACE
        printf ("JNI: GetMethodID (0x%x, \"value\", \"()I\") = %d\n", copyUnion.discrClass, copyUnion.valueID);
#endif
        snprintf (methodSignature, sizeof(methodSignature), "(I)L%s;", classDescriptor);
        copyUnion.from_intID = (*(ctx->javaEnv))->GetStaticMethodID (
	    ctx->javaEnv, copyUnion.discrClass, "from_int", methodSignature);
        saj_exceptionCheck (ctx->javaEnv);
#if JNI_TRACE
        printf ("JNI: GetMethodID (0x%x, \"from_int\", \"%s\") = %d\n", copyUnion.discrClass, methodSignature, copyUnion.from_intID);
#endif
    } else {
	copyUnion.valueID = 0;
	copyUnion.from_intID = 0;
	copyUnion.discrClass = NULL;
    }
    copyUnion.discrID = (*(ctx->javaEnv))->GetFieldID (ctx->javaEnv, copyUnion.unionClass,
	"_d", discrDescriptor);
    jexception = (*(ctx->javaEnv))->ExceptionOccurred(ctx->javaEnv);

    if (jexception)
    {
        /*  clear the exception, must be CORBA-generated code. */
        (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);
    }

    switch (c_baseObject(c_typeActualType(o->switchType))->kind) {
    case M_ENUMERATION:
        {
            char discrClassDescriptor [512];

            discrClassDescriptor[0] = '\0';
            saj_classDescriptor (c_typeActualType(o->switchType), discrClassDescriptor, sizeof(discrClassDescriptor));
            snprintf (methodSignature, sizeof(methodSignature), "()L%s;", discrClassDescriptor);

            copyUnion.getDiscrMethodID = (*(ctx->javaEnv))->GetMethodID((ctx->javaEnv), copyUnion.unionClass, "discriminator", methodSignature);
            saj_exceptionCheck((ctx->javaEnv));

            snprintf (methodSignature, sizeof(methodSignature), "(L%s;)V", discrClassDescriptor);
            copyUnion.__defaultID = (*(ctx->javaEnv))->GetMethodID((ctx->javaEnv), copyUnion.unionClass, "__default", methodSignature);
            if ( (*(ctx->javaEnv))->ExceptionCheck (ctx->javaEnv))
                (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);
        }
        break;
    case M_PRIMITIVE:
        switch (c_primitive (c_typeActualType(o->switchType))->kind)
        {
        case P_BOOLEAN:
            {
                copyUnion.getDiscrMethodID = (*(ctx->javaEnv))->GetMethodID((ctx->javaEnv), copyUnion.unionClass, "discriminator", "()Z");
                saj_exceptionCheck((ctx->javaEnv));

                copyUnion.__defaultID = (*(ctx->javaEnv))->GetMethodID((ctx->javaEnv), copyUnion.unionClass, "__default", "(Z)V");
                if ( (*(ctx->javaEnv))->ExceptionCheck (ctx->javaEnv))
                    (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);
            }
            break;
        case P_CHAR:
            {
                copyUnion.getDiscrMethodID = (*(ctx->javaEnv))->GetMethodID((ctx->javaEnv), copyUnion.unionClass, "discriminator", "()C");
                saj_exceptionCheck((ctx->javaEnv));

                copyUnion.__defaultID = (*(ctx->javaEnv))->GetMethodID((ctx->javaEnv), copyUnion.unionClass, "__default", "(C)V");
                if ( (*(ctx->javaEnv))->ExceptionCheck (ctx->javaEnv))
                    (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);
            }
            break;
        case P_SHORT:
        case P_USHORT:
            {
                copyUnion.getDiscrMethodID = (*(ctx->javaEnv))->GetMethodID((ctx->javaEnv), copyUnion.unionClass, "discriminator", "()S");
                saj_exceptionCheck((ctx->javaEnv));

                copyUnion.__defaultID = (*(ctx->javaEnv))->GetMethodID((ctx->javaEnv), copyUnion.unionClass, "__default", "(S)V");
                if ( (*(ctx->javaEnv))->ExceptionCheck (ctx->javaEnv))
                    (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);
            }
            break;
        case P_LONG:
        case P_ULONG:
            {
                copyUnion.getDiscrMethodID = (*(ctx->javaEnv))->GetMethodID((ctx->javaEnv), copyUnion.unionClass, "discriminator", "()I");
                saj_exceptionCheck((ctx->javaEnv));

                copyUnion.__defaultID = (*(ctx->javaEnv))->GetMethodID((ctx->javaEnv), copyUnion.unionClass, "__default", "(I)V");
                if ( (*(ctx->javaEnv))->ExceptionCheck (ctx->javaEnv))
                    (*(ctx->javaEnv))->ExceptionClear(ctx->javaEnv);
            }
            break;
        case P_LONGLONG:
        case P_ULONGLONG:
            {
                copyUnion.getDiscrMethodID = (*(ctx->javaEnv))->GetMethodID((ctx->javaEnv), copyUnion.unionClass, "discriminator", "()J");
                saj_exceptionCheck((ctx->javaEnv));

                copyUnion.__defaultID = (*(ctx->javaEnv))->GetMethodID((ctx->javaEnv), copyUnion.unionClass, "__default", "(J)V");
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

#if JNI_TRACE
    printf ("JNI: GetFieldID (0x%x, \"_d\", \"%s\") = %d\n", copyUnion.unionClass, discrDescriptor, copyUnion.discrID);
#endif
    copyUnion.discrType = c_typeActualType(o->switchType);
    if (c_typeActualType(o->switchType)->size > c_type(o)->alignment) {
        copyUnion.casesOffset = c_typeActualType(o->switchType)->size;
    } else {
        copyUnion.casesOffset = c_type(o)->alignment;
    }

    headerIndex = saj_copyCacheWrite (context.copyCache, &copyUnion, sizeof(sajCopyUnion));
    c_iterInsert (ctx->typeStack, saj_typeHistoryNew (c_metaObject(o), headerIndex));

    TRACE (printf ("Union\n    Class name %s\n", classDescriptor));
    TRACE (printf ("    Java class %x\n", (int)copyUnion.unionClass));
    TRACE (printf ("    Constructor ID = %x\n", (int)copyUnion.constrID));

    for (mi = 0; mi < c_arraySize(o->cases); mi++) {
        saj_cacheUnionCase (o->cases[mi], o->switchType, &context);
    }
    saj_typeHistoryFree (c_iterTakeFirst (ctx->typeStack));
    saj_copyCacheUpdateSize (context.copyCache, headerIndex);
}

STATIC void
saj_cacheBooleanBuild (
    c_primitive o,
    saj_context ctx)
{
    sajCopyHeader booleanHeader;

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

    TRACE (printf ("Boolean Array\n"));
    saj_cacheHeader ((sajCopyHeader *)&booleanHeader, sajArrBoolean, sizeof(booleanHeader));
    booleanHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &booleanHeader, sizeof(booleanHeader));
}

STATIC void
saj_cacheArrByteBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray byteHeader;

    TRACE (printf ("Byte Array\n"));
    saj_cacheHeader ((sajCopyHeader *)&byteHeader, sajArrByte, sizeof(byteHeader));
    byteHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &byteHeader, sizeof(byteHeader));
}

STATIC void
saj_cacheArrCharBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray charHeader;

    TRACE (printf ("Char Array\n"));
    saj_cacheHeader ((sajCopyHeader *)&charHeader, sajArrChar, sizeof(charHeader));
    charHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &charHeader, sizeof(charHeader));
}

STATIC void
saj_cacheArrShortBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray shortHeader;

    TRACE (printf ("Short Array\n"));
    saj_cacheHeader ((sajCopyHeader *)&shortHeader, sajArrShort, sizeof(shortHeader));
    shortHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &shortHeader, sizeof(shortHeader));
}

STATIC void
saj_cacheArrIntBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray intHeader;

    TRACE (printf ("Int Array\n"));
    saj_cacheHeader ((sajCopyHeader *)&intHeader, sajArrInt, sizeof(intHeader));
    intHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &intHeader, sizeof(intHeader));
}

STATIC void
saj_cacheArrLongBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray longHeader;

    TRACE (printf ("Long Array\n"));
    saj_cacheHeader ((sajCopyHeader *)&longHeader, sajArrLong, sizeof(longHeader));
    longHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &longHeader, sizeof(longHeader));
}

STATIC void
saj_cacheArrFloatBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray floatHeader;

    TRACE (printf ("Float Array\n"));
    saj_cacheHeader ((sajCopyHeader *)&floatHeader, sajArrFloat, sizeof(floatHeader));
    floatHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &floatHeader, sizeof(floatHeader));
}

STATIC void
saj_cacheArrDoubleBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyArray doubleHeader;

    TRACE (printf ("Double Array\n"));
    saj_cacheHeader ((sajCopyHeader *)&doubleHeader, sajArrDouble, sizeof(doubleHeader));
    doubleHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &doubleHeader, sizeof(doubleHeader));
}

STATIC void
saj_cacheArrObjectBuild (
    c_collectionType o,
    saj_context ctx)
{
    saj_context context;
    sajCopyObjectArray objectArrHeader;
    char classDescriptor [512];
    c_long headerIndex;
    jclass javaClass;

    classDescriptor [0] = '\0';
    saj_classDescriptor (c_typeActualType(o->subType), classDescriptor, sizeof(classDescriptor));
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

    saj_cacheHeader ((sajCopyHeader *)&objectArrHeader, sajArray, sizeof(objectArrHeader));
    headerIndex = saj_copyCacheWrite (ctx->copyCache, &objectArrHeader, sizeof(objectArrHeader));
    context = os_malloc (C_SIZEOF(saj_context));
    context->javaEnv = ctx->javaEnv;
    context->javaClass = objectArrHeader.arrayClass;
    context->copyCache = ctx->copyCache;
    context->typeStack = ctx->typeStack;
    saj_metaObject (c_typeActualType(o->subType), context);
    saj_copyCacheUpdateSize (ctx->copyCache, headerIndex);
    os_free (context);
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
    char classDescriptor [512];
    c_long headerIndex;
    jclass javaClass;

    classDescriptor [0] = '\0';
    saj_classDescriptor (c_typeActualType(o->subType), classDescriptor, sizeof(classDescriptor));
    javaClass = (*(ctx->javaEnv))->FindClass (ctx->javaEnv, classDescriptor);
#if JNI_TRACE
    printf ("JNI: FindClass (\"%s\") = 0x%x\n", classDescriptor, javaClass);
#endif
    objectSeqHeader.seqClass = (*(ctx->javaEnv))->NewGlobalRef (ctx->javaEnv, javaClass);
#if JNI_TRACE
    printf ("JNI: NewGlobalRef (\"%s\") = 0x%x\n", javaClass, objectSeqHeader.seqClass);
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

    saj_cacheHeader ((sajCopyHeader *)&objectSeqHeader, sajSequence, sizeof(objectSeqHeader));
    headerIndex = saj_copyCacheWrite (ctx->copyCache, &objectSeqHeader, sizeof(objectSeqHeader));
    /* Check if the subtype is in the scope of the subtype, in that case it is a recursive type */
    if (saj_isDefinedInScope (o)) {
	/* make back reference */
	saj_copyCacheBackReference (ctx->copyCache, saj_headerIndex(c_metaObject(c_typeActualType(o->subType)), ctx));
    } else {
        context = os_malloc (C_SIZEOF(saj_context));
        context->javaEnv = ctx->javaEnv;
        context->javaClass = objectSeqHeader.seqClass;
        context->copyCache = ctx->copyCache;
	context->typeStack = ctx->typeStack;
        saj_metaObject (c_typeActualType(o->subType), context);
        os_free (context);
    }
    saj_copyCacheUpdateSize (ctx->copyCache, headerIndex);
}

STATIC void
saj_cacheSeqBooleanBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence booleanHeader;

    TRACE (printf ("Boolean Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)&booleanHeader, sajSeqBoolean, sizeof(booleanHeader));
    booleanHeader.type = c_typeActualType(o->subType);
    booleanHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &booleanHeader, sizeof(booleanHeader));
}

STATIC void
saj_cacheSeqByteBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence byteHeader;

    TRACE (printf ("Byte Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)&byteHeader, sajSeqByte, sizeof(byteHeader));
    byteHeader.type = c_typeActualType(o->subType);
    byteHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &byteHeader, sizeof(byteHeader));
}

STATIC void
saj_cacheSeqCharBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence charHeader;

    TRACE (printf ("Char Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)&charHeader, sajSeqChar, sizeof(charHeader));
    charHeader.type = c_typeActualType(o->subType);
    charHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &charHeader, sizeof(charHeader));
}

STATIC void
saj_cacheSeqShortBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence shortHeader;

    TRACE (printf ("Short Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)&shortHeader, sajSeqShort, sizeof(shortHeader));
    shortHeader.type = c_typeActualType(o->subType);
    shortHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &shortHeader, sizeof(shortHeader));
}

STATIC void
saj_cacheSeqIntBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence intHeader;

    TRACE (printf ("Int Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)&intHeader, sajSeqInt, sizeof(intHeader));
    intHeader.type = c_typeActualType(o->subType);
    intHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &intHeader, sizeof(intHeader));
}

STATIC void
saj_cacheSeqLongBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence longHeader;

    TRACE (printf ("Long Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)&longHeader, sajSeqLong, sizeof(longHeader));
    longHeader.type = c_typeActualType(o->subType);
    longHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &longHeader, sizeof(longHeader));
}

STATIC void
saj_cacheSeqFloatBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence floatHeader;

    TRACE (printf ("Float Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)&floatHeader, sajSeqFloat, sizeof(floatHeader));
    floatHeader.type = c_typeActualType(o->subType);
    floatHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &floatHeader, sizeof(floatHeader));
}

STATIC void
saj_cacheSeqDoubleBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopySequence doubleHeader;

    TRACE (printf ("Double Sequence\n"));
    saj_cacheHeader ((sajCopyHeader *)&doubleHeader, sajSeqDouble, sizeof(doubleHeader));
    doubleHeader.type = c_typeActualType(o->subType);
    doubleHeader.size = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &doubleHeader, sizeof(doubleHeader));
}

STATIC void
saj_cacheStringBuild (
    c_collectionType o,
    saj_context ctx)
{
    sajCopyHeader stringHeader;

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

    TRACE (printf ("BString\n"));
    saj_cacheHeader ((sajCopyHeader *)&bstringHeader, sajBString, sizeof(bstringHeader));
    bstringHeader.max = o->maxSize;
    saj_copyCacheWrite (ctx->copyCache, &bstringHeader, sizeof(bstringHeader));
}

STATIC void
saj_cacheEnumBuild (
    c_enumeration o,
    saj_context ctx)
{
    sajCopyEnum copyEnum;
    char classDescriptor [512];
    char constrSignature [512];
    jclass javaClass;

    classDescriptor [0] = '\0';
    saj_classDescriptor (c_type(o), classDescriptor, sizeof(classDescriptor));
    saj_cacheHeader ((sajCopyHeader *)&copyEnum, sajEnum, sizeof(copyEnum));
    copyEnum.nrOfElements = c_arraySize (o->elements);
    javaClass = (*(ctx->javaEnv))->FindClass (ctx->javaEnv, classDescriptor);
#if JNI_TRACE
    printf ("JNI: FindClass (\"%s\") = 0x%x\n", classDescriptor, javaClass);
#endif
    copyEnum.enumClass = (*(ctx->javaEnv))->NewGlobalRef (ctx->javaEnv, javaClass);
#if JNI_TRACE
    printf ("JNI: NewGlobalRef (\"%s\") = 0x%x\n", classDescriptor, copyEnum.enumClass);
#endif
    (*(ctx->javaEnv))->DeleteLocalRef (ctx->javaEnv, javaClass);
#if JNI_TRACE
    printf ("JNI: DeleteLocalRef (0x%x)\n", javaClass);
#endif
    saj_exceptionCheck (ctx->javaEnv);
    copyEnum.valueID = (*(ctx->javaEnv))->GetMethodID (
	ctx->javaEnv, copyEnum.enumClass, "value", "()I");
    saj_exceptionCheck (ctx->javaEnv);
#if JNI_TRACE
    printf ("JNI: GetMethodID (0x%x, \"value\", \"()I\") = %d\n", copyEnum.enumClass, copyEnum.valueID);
#endif
    snprintf (constrSignature, sizeof(constrSignature), "(I)L%s;", classDescriptor);
    copyEnum.from_intID = (*(ctx->javaEnv))->GetStaticMethodID (
	ctx->javaEnv, copyEnum.enumClass, "from_int", constrSignature);
    saj_exceptionCheck (ctx->javaEnv);
#if JNI_TRACE
    printf ("JNI: GetMethodID (0x%x, \"from_int\", \"%s\") = %d\n", copyEnum.enumClass, constrSignature, copyEnum.valueID);
#endif

    TRACE (printf ("Enum\n    Class name %s\n", classDescriptor));
    TRACE (printf ("    Members # %d\n", copyEnum.nrOfElements));
    TRACE (printf ("    Java class %x\n", (int)copyEnum.enumClass));
    TRACE (printf ("    value ID = %x\n", (int)copyEnum.valueID));
    TRACE (printf ("    from_int ID = %x\n", (int)copyEnum.from_intID));

    saj_copyCacheWrite (ctx->copyCache, &copyEnum, sizeof(copyEnum));
}

STATIC void
saj_scopeName (
    c_char *buffer,
    c_long bufferSize,
    c_metaObject object,
    const c_char *separator)
{
    c_metaObject module;

    if (object) {
        module = c_metaObject(object)->definedIn;
        if (module) {
	    if (module->name) {
	        saj_scopeName (buffer, bufferSize, module, separator);
	    }
            strncat (buffer, saj_dekeyedId(c_metaObject(object)->name), bufferSize);
	    if (c_baseObject(object)->kind == M_STRUCTURE ||
                c_baseObject(object)->kind == M_UNION) {
                strncat (buffer, "Package", bufferSize);
	    }
            strncat (buffer, separator, bufferSize);
        } /* else object is in base module, so effectively no scope */
    }
}

STATIC void
saj_scopedTypeName (
    c_char *buffer,
    c_long bufferSize,
    c_metaObject object,
    const c_char *separator)
{
    saj_scopeName (buffer, bufferSize, object->definedIn, separator);
    /*    if (strlen(buffer)) {
	strncat (buffer, separator, bufferSize);
	}*/
    strncat (buffer, saj_dekeyedId(c_metaObject(object)->name), bufferSize);
}

STATIC void
saj_fieldDescriptor (
    c_type type,
    char *descriptor,
    unsigned int size)
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
    case M_EXTENT:
	assert (FALSE);
        break;
    case M_PRIMITIVE:
        switch (c_primitive(type)->kind) {
	case P_BOOLEAN:
	    /* boolean */
	    snprintf (descriptor, size-1, "Z");
	    break;
	case P_SHORT:
	    /* short */
	    snprintf (descriptor, size-1, "S");
	    break;
	case P_LONG:
	    /* int */
	    snprintf (descriptor, size-1, "I");
	    break;
	case P_LONGLONG:
	    /* long */
	    snprintf (descriptor, size-1, "J");
	    break;
	case P_USHORT:
	    /* short */
	    snprintf (descriptor, size-1, "S");
	    break;
	case P_ULONG:
	    /* int */
	    snprintf (descriptor, size-1, "I");
	    break;
	case P_ULONGLONG:
	    /* long */
	    snprintf (descriptor, size-1, "J");
	    break;
	case P_CHAR:
	    /* char */
	    snprintf (descriptor, size-1, "C");
	    break;
	case P_OCTET:
	    /* byte */
	    snprintf (descriptor, size-1, "B");
	    break;
	case P_FLOAT:
	    /* float */
	    snprintf (descriptor, size-1, "F");
	    break;
	case P_DOUBLE:
	    /* double */
	    snprintf (descriptor, size-1, "D");
	    break;
        default:
	    assert (FALSE);
	}
        break;
    case M_COLLECTION:
        if (c_collectionType(type)->kind == C_STRING) {
	    snprintf (descriptor, size-1, "Ljava/lang/String;");
        } else if (c_collectionType(type)->kind == C_SEQUENCE) {
            /* sequence */
	    if (c_baseObject(c_typeActualType(c_collectionType(type)->subType))->kind == M_PRIMITIVE) {
	    	snprintf (descriptor, size-1, "[");
		saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1);
	    } else {
	    	snprintf (descriptor, size-1, "[");
		saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1);
	    }
        } else if (c_collectionType(type)->kind == C_ARRAY) {
            /* array */
	    if (c_baseObject(c_typeActualType(c_collectionType(type)->subType))->kind == M_PRIMITIVE) {
	    	snprintf (descriptor, size-1, "[");
		saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1);
	    } else {
	    	snprintf (descriptor, size-1, "[");
		saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1);
	    }
        }
        break;
    case M_ENUMERATION:
	strncpy (descriptor, "L", size);
	saj_scopedTypeName (descriptor, size, c_metaObject(type), "/");
	strncat (descriptor, ";", size);
        break;
    case M_MODULE:
        break;
    case M_STRUCTURE:
	strncpy (descriptor, "L", size);
	saj_scopedTypeName (descriptor, size, c_metaObject(type), "/");
	strncat (descriptor, ";", size);
        break;
    case M_TYPEDEF:
        saj_fieldDescriptor (c_typeDef(type)->alias, descriptor, size);
        break;
    case M_UNION:
	strncpy (descriptor, "L", size);
	saj_scopedTypeName (descriptor, size, c_metaObject(type), "/");
	strncat (descriptor, ";", size);
        break;
    }
    assert (strlen (descriptor) != (size - 1));
}

STATIC void
saj_classDescriptor (
    c_type type,
    char *descriptor,
    unsigned int size)
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
    case M_EXTENT:
	assert (FALSE);
        break;
    case M_PRIMITIVE:
        switch (c_primitive(type)->kind) {
	case P_BOOLEAN:
	    /* boolean */
	    snprintf (descriptor, size-1, "Z");
	    break;
	case P_SHORT:
	    /* short */
	    snprintf (descriptor, size-1, "S");
	    break;
	case P_LONG:
	    /* int */
	    snprintf (descriptor, size-1, "I");
	    break;
	case P_LONGLONG:
	    /* long */
	    snprintf (descriptor, size-1, "J");
	    break;
	case P_USHORT:
	    /* short */
	    snprintf (descriptor, size-1, "S");
	    break;
	case P_ULONG:
	    /* int */
	    snprintf (descriptor, size-1, "I");
	    break;
	case P_ULONGLONG:
	    /* long */
	    snprintf (descriptor, size-1, "J");
	    break;
	case P_CHAR:
	    /* char */
	    snprintf (descriptor, size-1, "C");
	    break;
	case P_OCTET:
	    /* byte */
	    snprintf (descriptor, size-1, "B");
	    break;
	case P_FLOAT:
	    /* float */
	    snprintf (descriptor, size-1, "F");
	    break;
	case P_DOUBLE:
	    /* double */
	    snprintf (descriptor, size-1, "D");
	    break;
        default:
	    assert (FALSE);
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
		saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1);
	    } else {
	    	snprintf (descriptor, size-1, "[");
		saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1);
	    }
        } else if (c_collectionType(type)->kind == C_ARRAY) {
            /* array */
	    if (c_baseObject(c_typeActualType(c_collectionType(type)->subType))->kind == M_PRIMITIVE) {
	    	snprintf (descriptor, size-1, "[");
		saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1);
	    } else {
	    	snprintf (descriptor, size-1, "[");
		saj_fieldDescriptor (c_typeActualType(c_collectionType(type)->subType), &descriptor[1], size-1);
	    }
        }
        break;
    case M_ENUMERATION:
	saj_scopedTypeName (descriptor, size, c_metaObject(type), "/");
        break;
    case M_MODULE:
        break;
    case M_STRUCTURE:
	saj_scopedTypeName (descriptor, size, c_metaObject(type), "/");
        break;
    case M_TYPEDEF:
        saj_classDescriptor (c_typeDef(type)->alias, descriptor, size);
        break;
    case M_UNION:
	saj_scopedTypeName (descriptor, size, c_metaObject(type), "/");
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
    printf ("  T:%d S:%d\n", ct, ch->size);
    for (l = 0; l < level; l++) {
	printf ("  ");
    }
    switch (ct) {
    case sajBoolean:
	printf ("  Boolean\n");
	break;
    case sajByte:
	printf ("  Byte\n");
	break;
    case sajChar:
	printf ("  Char\n");
	break;
    case sajShort:
	printf ("  Short\n");
	break;
    case sajInt:
	printf ("  Int\n");
	break;
    case sajLong:
	printf ("  Long\n");
	break;
    case sajFloat:
	printf ("  Float\n");
	break;
    case sajDouble:
	printf ("  Double\n");
	break;
    case sajArrBoolean:
	printf ("  ArrBoolean\n");
	break;
    case sajArrByte:
	printf ("  ArrByte\n");
	break;
    case sajArrChar:
	printf ("  ArrChar\n");
	break;
    case sajArrShort:
	printf ("  ArrShort\n");
	break;
    case sajArrInt:
	printf ("  ArrInt\n");
	break;
    case sajArrLong:
	printf ("  ArrLong\n");
	break;
    case sajArrFloat:
	printf ("  ArrFloat\n");
	break;
    case sajArrDouble:
	printf ("  ArrDouble\n");
	break;
    case sajSeqBoolean:
	printf ("  SeqBoolean\n");
	break;
    case sajSeqByte:
	printf ("  SeqByte\n");
	break;
    case sajSeqChar:
	printf ("  SeqChar\n");
	break;
    case sajSeqShort:
	printf ("  SeqShort\n");
	break;
    case sajSeqInt:
	printf ("  SeqInt\n");
	break;
    case sajSeqLong:
	printf ("  SeqLong\n");
	break;
    case sajSeqFloat:
	printf ("  SeqFloat\n");
	break;
    case sajSeqDouble:
	printf ("  SeqDouble\n");
	break;
    case sajString:
	printf ("  String\n");
	break;
    case sajBString:
	printf ("  BString\n");
	break;
    case sajEnum:
	printf ("  Enum\n");
	break;
    case sajStruct:
	csh = (sajCopyStruct *)ch;
	printf ("  Struct\n");
        for (l = 0; l < level; l++) {
	    printf ("  ");
        }
	printf ("  M#:%d CL:"PA_ADDRFMT" CID:"PA_ADDRFMT"\n", csh->nrOfMembers,
	    (PA_ADDRCAST)csh->structClass, (PA_ADDRCAST)csh->constrID);
	csm = (sajCopyStructMember *)((PA_ADDRCAST)csh + sizeof (sajCopyStruct));
	for (mi = 0; mi < csh->nrOfMembers; mi++) {
            for (l = 0; l < level; l++) {
	        printf ("  ");
            }
	    printf ("  M@:%d FID:"PA_ADDRFMT"\n", csm->memberOffset, (PA_ADDRCAST)csm->javaFID);
	    ch = (sajCopyHeader *)((PA_ADDRCAST)csm + sizeof (sajCopyStructMember));
	    cacheDump (ch, level+1);
	    csm = (sajCopyStructMember *)((PA_ADDRCAST)ch + ch->size);
	}
	break;
    case sajUnion:
	printf ("  Union\n");
	break;
    case sajArray:
	oah = (sajCopyObjectArray *)ch;
	printf ("  Array\n");
        for (l = 0; l < level; l++) {
	    printf ("  ");
        }
	printf ("  E#:%d CL:"PA_ADDRFMT" TS:%d\n", oah->arraySize, (PA_ADDRCAST)oah->arrayClass, oah->typeSize);
	cacheDump ((sajCopyHeader *)((PA_ADDRCAST)oah + oah->header.size), level+1);
	break;
    case sajSequence:
	printf ("  Sequence\n");
	break;
    case sajRecursive:
	printf ("  Recursive\n");
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
    saj_classDescriptor (c_type(object), tmp, sizeof(tmp));
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

    readerCache->dataClass_constructor_mid = (*env)->GetMethodID (env, readerCache->dataClass, "<init>", "()V");

    tmp[0] = '\0';
    saj_classDescriptor (c_type(object), tmp, sizeof(tmp));
    strncat (tmp, "Holder", sizeof(tmp));
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

    tmp[0] = '\0';
    saj_classDescriptor (c_type(object), tmp, sizeof(tmp));
    strncat (tmp, "SeqHolder", sizeof(tmp));
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
    saj_fieldDescriptor (c_type(object), &tmp[strlen(tmp)], sizeof(tmp)-strlen(tmp));
    readerCache->dataHolder_value_fid = (*env)->GetFieldID (env, readerCache->dataHolderClass, "value", tmp);
#if JNI_TRACE
    printf ("JNI: GetFieldID (0x%x, \"%s\", \"%s\") = %d\n", readerCache->dataHolderClass, "value", tmp, readerCache->dataHolder_value_fid);
#endif
    if (readerCache->dataSeqHolderClass) {
	/* sequence holder class is only generated for interfaces (datareader/datawriter) */
        strncpy (tmp, "[", sizeof(tmp));
        saj_fieldDescriptor (c_type(object), &tmp[strlen(tmp)], sizeof(tmp)-strlen(tmp));
        readerCache->dataSeqHolder_value_fid = (*env)->GetFieldID (env, readerCache->dataSeqHolderClass, "value", tmp);
#if JNI_TRACE
        printf ("JNI: GetFieldID (0x%x, \"%s\", \"%s\") = %d\n", readerCache->dataSeqHolderClass, "value", tmp, readerCache->dataSeqHolder_value_fid);
#endif
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
        saj_metaObject (c_type(object), context);
	saj_copyCacheFinalize (context->copyCache);
	c_iterFree (context->typeStack);
	TRACE (saj_copyCacheDump (context->copyCache));
    }
    os_free (context);
}
