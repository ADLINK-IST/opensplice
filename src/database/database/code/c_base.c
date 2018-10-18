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
#include <stddef.h>
#include "os_stdlib.h"
#include "os_report.h"
#include "os_abstract.h"
#include "os_atomics.h"
#include "ut_avl.h"
#include "c_mmbase.h"
#include "c__mmbase.h"
#include "c__scope.h"
#include "c_sync.h"
#include "c__base.h"
#include "c_metafactory.h"
#include "c_querybase.h"
#include "c__querybase.h"
#include "c__collection.h"
#include "c__field.h"
#include "c__metabase.h"
#include "c_module.h"

/* If set the meta descriptions c_union, c_interface and c_structure will
 * have an additional blob attribute.
 */

#define TYPE_REFC_COUNTS_OBJECTS 0
#define CONFIDENCE (0x504F5448)

#define REFCOUNT_MASK            0x1ffffffu
#define REFCOUNT_FLAG_CLAMP      0x1000000u
#define REFCOUNT_FLAG_ATOMIC     0x2000000u
#define REFCOUNT_FLAG_TRACE      0x4000000u
#define REFCOUNT_FLAG_TRACETYPE  0x8000000u
#define REFCOUNT_FLAG_GARBAGE   0x10000000u
#if (REFCOUNT_FLAG_CLAMP << 1) != REFCOUNT_MASK + 1
#error "REFCOUNT_FLAG_CLAMP must be the top bit of REFCOUNT_MASK"
#endif

#define c_oid(o)    ((c_object)(C_ADDRESS(o) + HEADERSIZE))
#define c_header(o) ((c_header)(C_ADDRESS(o) - HEADERSIZE))
#define c_arrayHeader(o) ((c_arrayHeader)(C_ADDRESS(o) - ARRAYHEADERSIZE))

#define MIN_DB_SIZE     (150000)
#define MAXREFCOUNT     (50000)
#define MEMSIZE(size)   ((size)+HEADERSIZE)
#define ARRAYMEMSIZE(size)   ((size)+ARRAYHEADERSIZE)

#define ResolveType(s,t) c_type(c_metaResolve(c_metaObject(s),#t))

#define CHECK_REF (0)

#if CHECK_REF
#include "ut_trace.h" /* enable specific trace function (basic or extended) inside this header file */
/* default "v_message<kernelModule::v_topicInfo>" */
#define CHECK_REF_TYPE "v_message<kernelModule::v_topicInfo>"
#define CHECK_REF_TYPE_LEN (strlen(CHECK_REF_TYPE))
#define CHECK_REF_DEPTH (64)
#endif

#define ACTUALTYPE(_t,_type) \
        _t = _type; \
        while (c_baseObject(_t)->kind == M_TYPEDEF) { \
            _t = c_typeDef(_t)->alias; \
        };

C_CLASS(c_baseBinding);
C_CLASS(c_header);

C_STRUCT(c_header) {
#ifndef NDEBUG
    c_ulong confidence;
#ifdef OBJECT_WALK
    c_object nextObject;
    c_object prevObject;
#endif
#endif
    pa_uint32_t refCount;
    c_type type;
};

C_CLASS(c_arrayHeader);
/* The inheritance macro C_EXTEND is put at the end of the struct.
 * This is required since we must extend on the lower part of the memory.
 * Note that this requires a different approach for cast operations.
 */
C_STRUCT(c_arrayHeader) {
    c_ulonglong size;
    C_EXTENDS(c_header);
};

static const c_ulong HEADERSIZE = C_MAXALIGNSIZE(C_SIZEOF(c_header));
static const c_ulong ARRAYHEADERSIZE = C_MAXALIGNSIZE(C_SIZEOF(c_arrayHeader));

static const ut_avlTreedef_t c_base_bindings_td =
  UT_AVL_TREEDEF_INITIALIZER_INDKEY (offsetof (C_STRUCT(c_baseBinding), avlnode),
                                     offsetof (C_STRUCT(c_baseBinding), name),
                                     strcmp, 0);

static c_string
c__stringMalloc(
    c_base base,
    c_size length,
    c_bool check)
{
    c_header header;
    c_string s = NULL;

    assert(base);
    assert(length > 0);

    if (check) {
        header = (c_header)c_mmMallocThreshold(c_baseMM(base), MEMSIZE(length));
    } else {
        header = (c_header)c_mmMalloc(c_baseMM(base), MEMSIZE(length));
        if (!header) {
            abort();
        }
    }
    if (header) {
#if TYPE_REFC_COUNTS_OBJECTS
        header->type = c_keep(base->string_type);
#else
        header->type = base->string_type;
#endif
        if (base->maintainObjectCount) {
            pa_inc32(&base->string_type->objectCount);
        }
        pa_st32(&header->refCount, 1 | REFCOUNT_FLAG_ATOMIC);
        s = (c_string)c_oid(header);
        s[0] = '\0'; /* c_stringNew/Malloc always return a null-terminated string */
#ifndef NDEBUG
        header->confidence = CONFIDENCE;
#ifdef OBJECT_WALK
        header->nextObject = NULL;
        c_header(base->lastObject)->nextObject = s;
        header->prevObject = base->lastObject;
        assert(base->firstObject != NULL);
        base->lastObject = s;
#endif
#endif
    }
    return s;
}

c_string
c_stringMalloc(
    c_base base,
    c_size length)
{
    assert(base);
    assert(length > 0);

    if (length == 1){
        /* Empty strings are interned. The only valid string with length 1 is the
         * 'empty' string, so in this case return a reference to the intern. This
         * saves precious resources in case of big amounts of empty strings, since
         * each empty string does not only contain the '\0' terminator but also
         * 36-40 bytes of header information.
         */
        assert(base->emptyString[0] == '\0');
        return c_keep(base->emptyString);
    }

    return c__stringMalloc(base, length, FALSE);
}

c_string
c_stringMalloc_s(
    c_base base,
    c_size length)
{
    assert(base);
    assert(length > 0);

    if (length == 1){
        /* Empty strings are interned. The only valid string with length 1 is the
         * 'empty' string, so in this case return a reference to the intern. This
         * saves precious resources in case of big amounts of empty strings, since
         * each empty string does not only contain the '\0' terminator but also
         * 36-40 bytes of header information.
         */
        assert(base->emptyString[0] == '\0');
        return c_keep(base->emptyString);
    }

    return c__stringMalloc(base, length, TRUE);
}

static c_wstring
c__wstringMalloc(
    c_base base,
    c_size length,
    c_bool check)
{
    c_header header;
    c_wstring s = NULL;

    assert(base);
    assert(length > 0);

    if (check) {
        header = (c_header)c_mmMallocThreshold(c_baseMM(base), MEMSIZE(length*sizeof(c_wchar)));
    } else {
        header = (c_header)c_mmMalloc(c_baseMM(base), MEMSIZE(length*sizeof(c_wchar)));
    }
    if (header) {
        c_type wstring_t = c_wstring_t(base);
#if TYPE_REFC_COUNTS_OBJECTS
        header->type = c_keep(wstring_t);
#else
        header->type = wstring_t;
#endif
        if (base->maintainObjectCount) {
            pa_inc32(&wstring_t->objectCount);
        }
        pa_st32(&header->refCount, 1 | REFCOUNT_FLAG_ATOMIC);
        s = (c_wstring)c_oid(header);
        s[0] = 0; /* c_wstringNew/Malloc always return a nul-terminated string */
#ifndef NDEBUG
        header->confidence = CONFIDENCE;
#ifdef OBJECT_WALK
        header->nextObject = NULL;
        c_header(base->lastObject)->nextObject = s;
        header->prevObject = base->lastObject;
        assert(base->firstObject != NULL);
        base->lastObject = s;
#endif
#endif
    }
    return s;
}

c_wstring
c_wstringMalloc(
    c_base base,
    c_size length)
{
    assert(base);
    assert(length > 0);

    if (length == 1){
        /* Empty wstrings are interned. The only valid string with length 1 is the
         * 'empty' string, so in this case return a reference to the intern. This
         * saves precious resources in case of big amounts of empty strings, since
         * each empty string does not only contain the '\0' terminator but also
         * 36-40 bytes of header information.
         */
        assert(base->emptyWstring[0] == 0);
        return c_keep(base->emptyWstring);
    }

    return c__wstringMalloc(base, length, FALSE);
}


c_wstring
c_wstringMalloc_s(
    c_base base,
    c_size length)
{
    assert(base);
    assert(length > 0);

    if (length == 1){
        /* Empty wstrings are interned. The only valid string with length 1 is the
         * 'empty' string, so in this case return a reference to the intern. This
         * saves precious resources in case of big amounts of empty strings, since
         * each empty string does not only contain the '\0' terminator but also
         * 36-40 bytes of header information.
         */
        assert(base->emptyWstring[0] == 0);
        return c_keep(base->emptyWstring);
    }

    return c__wstringMalloc(base, length, TRUE);
}


c_string
c_stringNew(
    c_base base,
    const c_char *str)
{
    c_string s;
    size_t len;

    assert(base);

    if (str == NULL) {
        return NULL;
    }

    len = strlen(str) + 1;
    if((s = c_stringMalloc(base, len)) != NULL && len > 1){
        /* str is nul-terminated (since we could determine len), so memcpy str
         * including the nul-terminator.
         */
        memcpy(s, str, len);
    }

    /* c_wstringNew/Malloc always return a nul-terminated string */
    assert(!s || s[len - 1] == '\0');
    return s;
}

c_string
c_stringNew_s(
    c_base base,
    const c_char *str)
{
    c_string s;
     size_t len;

     assert(base);

     if (str == NULL) {
         return NULL;
     }

     len = strlen(str) + 1;
     if((s = c_stringMalloc_s(base, len)) != NULL && len > 1){
         /* str is nul-terminated (since we could determine len), so memcpy str
          * including the nul-terminator.
          */
         memcpy(s, str, len);
     }

     /* c_wstringNew/Malloc always return a nul-terminated string */
     assert(!s || s[len - 1] == '\0');
     return s;
}


/* The following Macro and subsequent instances implements the
 * following type caches in c_base and getter methods.
 *
 * c_octet_t(c_base _this)
 * c_char_t(c_base _this)
 * c_short_t(c_base _this)
 * c_long_t(c_base _this)
 * c_longlong_t(c_base _this)
 * c_uchar_t(c_base _this)
 * c_ushort_t(c_base _this)
 * c_ulong_t(c_base _this)
 * c_ulonglong_t(c_base _this)
 * c_float_t(c_base _this)
 * c_double_t(c_base _this)
 * c_string_t(c_base _this)
 * c_wchar_t(c_base _this)
 * c_wstring_t(c_base _this)
 * c_type_t(c_base _this)
 * c_valueKind_t(c_base _this)
  */

_TYPE_CACHE_(c_object)
_TYPE_CACHE_(c_voidp)
_TYPE_CACHE_(c_bool)
_TYPE_CACHE_(c_address)
_TYPE_CACHE_(c_octet)
_TYPE_CACHE_(c_char)
_TYPE_CACHE_(c_short)
_TYPE_CACHE_(c_long)
_TYPE_CACHE_(c_longlong)
_TYPE_CACHE_(c_uchar)
_TYPE_CACHE_(c_ushort)
_TYPE_CACHE_(c_ulong)
_TYPE_CACHE_(c_ulonglong)
_TYPE_CACHE_(c_float)
_TYPE_CACHE_(c_double)
_TYPE_CACHE_(c_string)
_TYPE_CACHE_(c_wchar)
_TYPE_CACHE_(c_wstring)
_TYPE_CACHE_(c_type)
_TYPE_CACHE_(c_valueKind)
_TYPE_CACHE_(pa_uint32_t)
_TYPE_CACHE_(pa_uintptr_t)
_TYPE_CACHE_(pa_voidp_t)

c_type
c_getMetaType(
    c_base base,
    c_metaKind kind)
{
    return base->metaType[kind];
}

static void
initInterface(
    c_object o)
{
    /* The vast majority of classes that get defined during initialisation have the scope already
     * allocated by c_metaDefine, but not all of them.
     */
    if(c_interface(o)->scope == NULL) {
        c_interface(o)->scope = (c_scope)c_scopeNew(c__getBase(o));
    }
    c_interface(o)->abstract = FALSE;
    c_interface(o)->inherits = NULL;
    c_interface(o)->references = NULL;
}

static c_object
initClass(
    c_object o,
    c_type baseClass)
{
    initInterface(o);
    c_class(o)->extends = c_class(baseClass);
    c_class(o)->keys = NULL;
    return o;
}

static c_object
initAnnotation(
    c_object o)
{
    initInterface(o);
    return o;
}

static c_type
c_newMetaClass(
    c_base base,
    const c_char *name,
    size_t size,
    size_t alignment)
{
    c_metaObject o,found;

    assert(base != NULL);
    assert(name != NULL);
    assert(base->metaType[M_CLASS] != NULL);

    o = c_metaDefine(c_metaObject(base),M_CLASS);
    c_metaTypeInit(o,size,alignment);
    found = c_metaBind(c_metaObject(base),name,o);
    if (found != o) {
        OS_REPORT(OS_ERROR,"c_newMetaClass failed",0,"%s already defined",name);
        assert(FALSE);
    }
    c_free(o);
    return c_type(found);
}

static void
c_queryCacheInit (C_STRUCT(c_queryCache) *queryCache)
{
    queryCache->c_qConst_t = NULL;
    queryCache->c_qType_t = NULL;
    queryCache->c_qVar_t = NULL;
    queryCache->c_qField_t = NULL;
    queryCache->c_qFunc_t = NULL;
    queryCache->c_qPred_t = NULL;
    queryCache->c_qKey_t = NULL;
    queryCache->c_qRange_t = NULL;
    queryCache->c_qExpr_t = NULL;
}

static void
c_fieldCacheInit (C_STRUCT(c_fieldCache) *fieldCache)
{
    fieldCache->c_field_t = NULL;
    fieldCache->c_fieldPath_t = NULL;
    fieldCache->c_fieldRefs_t = NULL;
}

static void
c_typeCacheInit (C_STRUCT(c_typeCache) *typeCache)
{
    typeCache->c_object_t = NULL;
    typeCache->c_voidp_t = NULL;
    typeCache->c_bool_t = NULL;
    typeCache->c_address_t = NULL;
    typeCache->c_octet_t = NULL;
    typeCache->c_char_t = NULL;
    typeCache->c_short_t = NULL;
    typeCache->c_long_t = NULL;
    typeCache->c_longlong_t = NULL;
    typeCache->c_uchar_t = NULL;
    typeCache->c_ushort_t = NULL;
    typeCache->c_ulong_t = NULL;
    typeCache->c_ulonglong_t = NULL;
    typeCache->c_float_t = NULL;
    typeCache->c_double_t = NULL;
    typeCache->c_string_t = NULL;
    typeCache->c_wchar_t = NULL;
    typeCache->c_wstring_t = NULL;
    typeCache->c_type_t = NULL;
    typeCache->c_valueKind_t = NULL;
    typeCache->c_member_t = NULL;
    typeCache->c_literal_t = NULL;
    typeCache->c_constant_t = NULL;
    typeCache->c_unionCase_t = NULL;
    typeCache->c_property_t = NULL;
    typeCache->pa_uint32_t_t = NULL;
    typeCache->pa_uintptr_t_t = NULL;
    typeCache->pa_voidp_t_t = NULL;
}

void
c_baseSetMaintainObjectCount (
    c_base base,
    c_bool enable)
{
    /* Not bothering with locking here, so it is undefined when
     * exactly other threads take note (but they will eventually)
     */
    base->maintainObjectCount = enable;
}

void
c_baseSetY2038Ready (
    c_base base,
    c_bool enable)
{
    /* Not bothering with locking here, so it is undefined when
     * exactly other threads take note (but they will eventually)
     */
    base->y2038Ready = enable;
}

c_bool
c_baseGetY2038Ready (
    c_base base)
{
    return base->y2038Ready;
}

os_scopeAttr
c_baseGetScopeAttr(
    c_base base)
{
    /* Use shared mutexes only when this base
     * is within shared memory.
     */
    if (c_mmMode(base->mm) == MM_SHARED) {
        return OS_SCOPE_SHARED;
    } else {
        return OS_SCOPE_PRIVATE;
    }
}

static void
c_baseInit (
    c_base base,
    c_mm mm)
{
    c_type type,scopeType;
    c_header header;
    c_metaObject o,found,_intern,temp;
    c_array members;
    c_iter labels;
    size_t size;
    c_ulong caseNumber;

    /* A c_base inherits as follows:
     *
     * c_baseObject <- c_metaObject <- c_module <- c_base
     *
     * All fields are initialised where possible before the bootstrap. */

    /* First, attach mm to base. */
    base->mm = mm;
    base->maintainObjectCount = FALSE;
    base->y2038Ready = FALSE;

    /* c_baseObject init */
    c_baseObject(base)->kind = M_MODULE;

    /* c_metaObject init */
    c_metaObject(base)->definedIn = NULL;
    c_metaObject(base)->name = NULL;

    /* c_module init */
    (void)c_mutexInit(base, &((c_module)base)->mtx);
    /* scope is initialized when scope-type is bootstrapped */

    /* c_base init */
    base->confidence = CONFIDENCE;
    ut_avlInit (&c_base_bindings_td, &base->bindings);
    (void)c_mutexInit(base, &base->bindLock);
    (void)c_mutexInit(base, &base->serLock);

    /* metaType[M_COUNT], string_type and emptyString are initialized when types
     * are available.
     */

    /* c_base init */
    c_queryCacheInit (&base->baseCache.queryCache);
    c_fieldCacheInit (&base->baseCache.fieldCache);
    c_typeCacheInit (&base->baseCache.typeCache);

#ifndef NDEBUG
#ifdef OBJECT_WALK
    base->firstObject = base;
    base->lastObject = base;
#endif
#endif

    /* Declare class type.
     * this is required because all meta meta objects are defined as class.
     */
    size = MEMSIZE(C_SIZEOF(c_class));
    header = (c_header)c_mmMalloc(base->mm,size);
    if (!header) {
    return;
    }
    memset(header,0,size);
    pa_st32(&header->refCount, 1);
    header->type = NULL;
#ifndef NDEBUG
    header->confidence = CONFIDENCE;
#endif
    o = c_metaObject(c_oid(header));
#ifndef NDEBUG
#ifdef OBJECT_WALK
    header->nextObject = NULL;
    c_header(base->lastObject)->nextObject = o;
    header->prevObject = base->lastObject;
    assert(base->firstObject != NULL);
    base->lastObject = o;
#endif
#endif
    c_type(o)->base = base;
    c_baseObject(o)->kind = M_CLASS;
    c_type (o)->alignment = C_ALIGNMENT_C_STRUCT (c_class);
    c_type(o)->size = C_SIZEOF(c_class);
    base->metaType[M_CLASS] = c_type(o);
    header->type = (c_type) o;
    c_keep(o);

    o = c_metaObject(c_new(base->metaType[M_CLASS]));
    c_baseObject(o)->kind = M_CLASS;
    C_META_TYPEINIT_(o,c_collectionType);
    base->metaType[M_COLLECTION] = c_type(o);

    /* Declare scope class type.
     * this is required because all meta meta objects are managed in the base scope.
     */

    scopeType = c_type(c_new(base->metaType[M_COLLECTION]));
    c_baseObject(scopeType)->kind = M_COLLECTION;
    c_collectionType(scopeType)->kind = OSPL_C_SCOPE;
    c_collectionType(scopeType)->maxSize = 0;
    C_META_TYPEINIT_(scopeType,c_scope);

    /* Declare base class type and initialize the base scope.
     * this is required because all meta meta objects are bound to the base scope.
     */

    o = c_metaObject(c_new(base->metaType[M_CLASS]));
    c_baseObject(o)->kind = M_CLASS;
    C_META_TYPEINIT_(o,c_base);
    base->metaType[M_BASE] = c_type(o);

    /* Overwrite as header->type points to type of metatype class
     * i.o. NULL
     */
    c_header(base)->type = c_keep(o);

    /* c_module->scope init */
    c_module(base)->scope = c_scope(c_new(scopeType));
    c_scopeInit(c_module(base)->scope);

    /* Declare c_string type, this is required to be able to bind objects to names. */
    o = c_metaObject(c_new(base->metaType[M_COLLECTION]));
    c_baseObject(o)->kind = M_COLLECTION;
    c_collectionType(o)->kind = OSPL_C_STRING;
    c_metaTypeInit(o,sizeof(c_string),C_ALIGNMENT(c_string));

    base->string_type = c_keep(o);
    found = c_metaBind(c_metaObject(base),"c_string",o);
    assert(found == o);
    c_free(found);
    c_free(o);

    o = c_metaObject(base->metaType[M_BASE]);
    found = c_metaBind(c_metaObject(base),"c_base",o);
    assert(found == o);
    c_free(found);

    o = c_metaObject(base->metaType[M_CLASS]);
    found = c_metaBind(c_metaObject(base),"c_class",o);
    assert(found == o);
    c_free(found);

    o = c_metaObject(base->metaType[M_COLLECTION]);
    found = c_metaBind(c_metaObject(base),"c_collectionType",o);
    assert(found == o);
    c_free(found);

    found = c_metaBind(c_metaObject(base),"c_scope",c_metaObject(scopeType));
    assert(found == c_metaObject(scopeType));
    c_free(found);

    /* Now allocate, bind and pre-initialize all meta meta objects.
     * pre-initialize will only set size and kind making the meta meta objects
     * ready to be used for meta creation.
     * At this point reflection will be unavailable until the meta meta objects
     * are fully initialized.
     */

    /* Initialize the interned empty string. Strings can now be allocated, so
     * even though in the baseInit no empty strings are used, initialisation
     * is needed here to guarantee the assertions of c_stringNew and
     * c_stringMalloc.
     */
    base->emptyString = c__stringMalloc(base, 1, FALSE);
    base->emptyString[0] = '\0';

#define _META_(b,t) c_object(c_newMetaClass(b,((const c_char *)#t),C_SIZEOF(t),C_ALIGNMENT_C_STRUCT(t)))

    /** Declare abstract meta types **/
    c_free(_META_(base,c_baseObject));
    c_free(_META_(base,c_operand));
    c_free(_META_(base,c_specifier));
    c_free(_META_(base,c_metaObject));

    base->baseCache.typeCache.c_property_t = _META_(base,c_property);
    base->baseCache.typeCache.c_type_t = _META_(base,c_type);

    /* At last set the subType of c_scope type. */
    c_collectionType(scopeType)->subType = ResolveType(base,c_metaObject);
    c_free(scopeType); /* we can now free our local ref as we don't use it anymore */
    scopeType = NULL;

    /* Declare meta types */
    base->metaType[M_LITERAL] =      _META_(base,c_literal);
    base->metaType[M_CONSTOPERAND] = _META_(base,c_constOperand);
    base->metaType[M_EXPRESSION] =   _META_(base,c_expression);
    base->metaType[M_PARAMETER] =    _META_(base,c_parameter);
    base->metaType[M_MEMBER] =       _META_(base,c_member);
    base->metaType[M_UNIONCASE] =    _META_(base,c_unionCase);
    base->metaType[M_ATTRIBUTE] =    _META_(base,c_attribute);
    base->metaType[M_RELATION] =     _META_(base,c_relation);
    base->metaType[M_MODULE] =       _META_(base,c_module);
    base->metaType[M_CONSTANT] =     _META_(base,c_constant);
    base->metaType[M_OPERATION] =    _META_(base,c_operation);
    base->metaType[M_TYPEDEF] =      _META_(base,c_typeDef);
    base->metaType[M_PRIMITIVE] =    _META_(base,c_primitive);
    base->metaType[M_ENUMERATION] =  _META_(base,c_enumeration);
    base->metaType[M_UNION] =        _META_(base,c_union);
    base->metaType[M_STRUCTURE] =    _META_(base,c_structure);
    base->metaType[M_EXCEPTION] =    _META_(base,c_exception);
    base->metaType[M_INTERFACE] =    _META_(base,c_interface);
    base->metaType[M_ANNOTATION] =   _META_(base,c_annotation);

    base->baseCache.typeCache.c_literal_t = base->metaType[M_LITERAL];
    base->baseCache.typeCache.c_member_t  = base->metaType[M_MEMBER];
    base->baseCache.typeCache.c_unionCase_t = base->metaType[M_UNIONCASE];
    base->baseCache.typeCache.c_constant_t = base->metaType[M_CONSTANT];

    c_free(base->metaType[M_LITERAL]);
    c_free(base->metaType[M_CONSTOPERAND]);
    c_free(base->metaType[M_EXPRESSION]);
    c_free(base->metaType[M_PARAMETER]);
    c_free(base->metaType[M_MEMBER]);
    c_free(base->metaType[M_UNIONCASE]);
    c_free(base->metaType[M_ATTRIBUTE]);
    c_free(base->metaType[M_RELATION]);
    c_free(base->metaType[M_MODULE]);
    c_free(base->metaType[M_CONSTANT]);
    c_free(base->metaType[M_OPERATION]);
    c_free(base->metaType[M_TYPEDEF]);
    c_free(base->metaType[M_PRIMITIVE]);
    c_free(base->metaType[M_ENUMERATION]);
    c_free(base->metaType[M_UNION]);
    c_free(base->metaType[M_STRUCTURE]);
    c_free(base->metaType[M_EXCEPTION]);
    c_free(base->metaType[M_INTERFACE]);
    c_free(base->metaType[M_ANNOTATION]);

#undef _META_

    /* Now allocation of meta objects is operational.
     * For initialization of the meta meta object we need to allocate the meta objects
     * for all internal types.
     */

    /* Definition of the meta objects specifying all internal primitive types. */

#define INITPRIM(s,n,k) \
    o = c_metaDeclare(c_metaObject(s),#n,M_PRIMITIVE); \
    s->baseCache.typeCache.n##_t = c_type(o); \
    c_primitive(o)->kind = k; \
    c_metaFinalize(o); \
    c_free(o)

    INITPRIM(base,c_address,   P_ADDRESS);
    INITPRIM(base,c_bool,      P_BOOLEAN);
    INITPRIM(base,c_char,      P_CHAR);
    INITPRIM(base,c_wchar,     P_WCHAR);
    INITPRIM(base,c_octet,     P_OCTET);
    INITPRIM(base,c_short,     P_SHORT);
    INITPRIM(base,c_ushort,    P_USHORT);
    INITPRIM(base,c_long,      P_LONG);
    INITPRIM(base,c_ulong,     P_ULONG);
    INITPRIM(base,c_longlong,  P_LONGLONG);
    INITPRIM(base,c_ulonglong, P_ULONGLONG);
    INITPRIM(base,c_float,     P_FLOAT);
    INITPRIM(base,c_double,    P_DOUBLE);
    INITPRIM(base,c_voidp,     P_VOIDP);
    INITPRIM(base,c_mutex,     P_MUTEX);
    INITPRIM(base,c_lock,      P_LOCK);
    INITPRIM(base,c_cond,      P_COND);
    INITPRIM(base,pa_uint32_t, P_PA_UINT32);
    INITPRIM(base,pa_uintptr_t,P_PA_UINTPTR);
    INITPRIM(base,pa_voidp_t,  P_PA_VOIDP);

#undef INITPRIM

    o = c_metaDeclare(c_metaObject(base),"c_object",M_CLASS);
    base->baseCache.typeCache.c_object_t = c_type(o);
    c_metaFinalize(o);
    c_free(o);
    o = c_metaDeclare(c_metaObject(base),"c_any",M_CLASS);
    c_metaFinalize(o);
    c_free(o);

    c_collectionInit(base);

    base->baseCache.typeCache.c_string_t = ResolveType(base,c_string);;
    base->baseCache.typeCache.c_wstring_t = ResolveType(base,c_wstring);

#define _ENUMVAL_(e,v) \
    c_enumeration(e)->elements[v] = \
    c_metaDeclareEnumElement(c_metaObject(base),#v)

#define _ENUMVAL_PREFIX_(e,v) \
    c_enumeration(e)->elements[v] = \
    c_metaDeclareEnumElement(c_metaObject(base),#v + 5 /* OSPL_ prefix */)

    o = c_metaDefine(c_metaObject(base),M_ENUMERATION);
    /* The following array of objects should be defined as array of c_constant.
     * This is not a problem and is intentional because of the side effect.
     * The side effect is that a type ARRAY<c_object> is created in the database.
     * This type will otherwise not be available but it is required for c_clone to work.
     */
    c_enumeration(o)->elements = c_arrayNew(c_object_t(base),M_COUNT);
    _ENUMVAL_(o,M_UNDEFINED);
    _ENUMVAL_(o,M_ANNOTATION);
    _ENUMVAL_(o,M_ATTRIBUTE);
    _ENUMVAL_(o,M_CLASS);
    _ENUMVAL_(o,M_COLLECTION);
    _ENUMVAL_(o,M_CONSTANT);
    _ENUMVAL_(o,M_CONSTOPERAND);
    _ENUMVAL_(o,M_ENUMERATION);
    _ENUMVAL_(o,M_EXCEPTION);
    _ENUMVAL_(o,M_EXPRESSION);
    _ENUMVAL_(o,M_INTERFACE);
    _ENUMVAL_(o,M_LITERAL);
    _ENUMVAL_(o,M_MEMBER);
    _ENUMVAL_(o,M_MODULE);
    _ENUMVAL_(o,M_OPERATION);
    _ENUMVAL_(o,M_PARAMETER);
    _ENUMVAL_(o,M_PRIMITIVE);
    _ENUMVAL_(o,M_RELATION);
    _ENUMVAL_(o,M_BASE);
    _ENUMVAL_(o,M_STRUCTURE);
    _ENUMVAL_(o,M_TYPEDEF);
    _ENUMVAL_(o,M_UNION);
    _ENUMVAL_(o,M_UNIONCASE);
    c_metaFinalize(o);
    c_metaBind(c_metaObject(base),"c_metaKind",o);
    c_free(o);

    o = c_metaDefine(c_metaObject(base),M_ENUMERATION);
    c_enumeration(o)->elements = c_arrayNew(c_constant_t(base),OSPL_C_COUNT);
    _ENUMVAL_PREFIX_(o,OSPL_C_UNDEFINED);
    _ENUMVAL_PREFIX_(o,OSPL_C_LIST);
    _ENUMVAL_PREFIX_(o,OSPL_C_ARRAY);
    _ENUMVAL_PREFIX_(o,OSPL_C_BAG);
    _ENUMVAL_PREFIX_(o,OSPL_C_SET);
    _ENUMVAL_PREFIX_(o,OSPL_C_MAP);
    _ENUMVAL_PREFIX_(o,OSPL_C_DICTIONARY);
    _ENUMVAL_PREFIX_(o,OSPL_C_SEQUENCE);
    _ENUMVAL_PREFIX_(o,OSPL_C_STRING);
    _ENUMVAL_PREFIX_(o,OSPL_C_WSTRING);
    _ENUMVAL_PREFIX_(o,OSPL_C_QUERY);
    _ENUMVAL_PREFIX_(o,OSPL_C_SCOPE);
    c_metaFinalize(o);
    c_metaBind(c_metaObject(base),"c_collKind",o);
    c_free(o);

    o = c_metaDefine(c_metaObject(base),M_ENUMERATION);
    c_enumeration(o)->elements = c_arrayNew(c_constant_t(base),P_COUNT);
    _ENUMVAL_(o,P_UNDEFINED);
    _ENUMVAL_(o,P_ADDRESS);
    _ENUMVAL_(o,P_BOOLEAN);
    _ENUMVAL_(o,P_CHAR);
    _ENUMVAL_(o,P_WCHAR);
    _ENUMVAL_(o,P_OCTET);
    _ENUMVAL_(o,P_SHORT);
    _ENUMVAL_(o,P_USHORT);
    _ENUMVAL_(o,P_LONG);
    _ENUMVAL_(o,P_ULONG);
    _ENUMVAL_(o,P_LONGLONG);
    _ENUMVAL_(o,P_ULONGLONG);
    _ENUMVAL_(o,P_FLOAT);
    _ENUMVAL_(o,P_DOUBLE);
    _ENUMVAL_(o,P_VOIDP);
    _ENUMVAL_(o,P_MUTEX);
    _ENUMVAL_(o,P_LOCK);
    _ENUMVAL_(o,P_COND);
    _ENUMVAL_(o,P_PA_UINT32);
    _ENUMVAL_(o,P_PA_UINTPTR);
    _ENUMVAL_(o,P_PA_VOIDP);
    c_metaFinalize(o);
    c_metaBind(c_metaObject(base),"c_primKind",o);
    c_free(o);

    o = c_metaDefine(c_metaObject(base),M_ENUMERATION);
    c_enumeration(o)->elements = c_arrayNew(c_constant_t(base),E_COUNT);
    _ENUMVAL_(o,E_UNDEFINED);
    _ENUMVAL_(o,E_OR);
    _ENUMVAL_(o,E_XOR);
    _ENUMVAL_(o,E_AND);
    _ENUMVAL_(o,E_SHIFTRIGHT);
    _ENUMVAL_(o,E_SHIFTLEFT);
    _ENUMVAL_(o,E_PLUS);
    _ENUMVAL_(o,E_MINUS);
    _ENUMVAL_(o,E_MUL);
    _ENUMVAL_(o,E_DIV);
    _ENUMVAL_(o,E_MOD);
    _ENUMVAL_(o,E_NOT);
    c_metaFinalize(o);
    c_metaBind(c_metaObject(base),"c_exprKind",o);
    c_free(o);

    o = c_metaDefine(c_metaObject(base),M_ENUMERATION);
    c_enumeration(o)->elements = c_arrayNew(c_constant_t(base),D_COUNT);
    _ENUMVAL_(o,D_UNDEFINED);
    _ENUMVAL_(o,D_IN);
    _ENUMVAL_(o,D_OUT);
    _ENUMVAL_(o,D_INOUT);
    c_metaFinalize(o);
    c_metaBind(c_metaObject(base),"c_direction",o);
    c_free(o);

    o = c_metaDefine(c_metaObject(base),M_ENUMERATION);
    base->baseCache.typeCache.c_valueKind_t = c_type(o);
    c_enumeration(o)->elements = c_arrayNew(c_constant_t(base),V_COUNT);
    _ENUMVAL_(o,V_UNDEFINED);
    _ENUMVAL_(o,V_ADDRESS);
    _ENUMVAL_(o,V_BOOLEAN);
    _ENUMVAL_(o,V_OCTET);
    _ENUMVAL_(o,V_SHORT);
    _ENUMVAL_(o,V_LONG);
    _ENUMVAL_(o,V_LONGLONG);
    _ENUMVAL_(o,V_USHORT);
    _ENUMVAL_(o,V_ULONG);
    _ENUMVAL_(o,V_ULONGLONG);
    _ENUMVAL_(o,V_FLOAT);
    _ENUMVAL_(o,V_DOUBLE);
    _ENUMVAL_(o,V_CHAR);
    _ENUMVAL_(o,V_STRING);
    _ENUMVAL_(o,V_WCHAR);
    _ENUMVAL_(o,V_WSTRING);
    _ENUMVAL_(o,V_FIXED);
    _ENUMVAL_(o,V_OBJECT);
    _ENUMVAL_(o,V_VOIDP);
    c_metaFinalize(o);
    c_metaBind(c_metaObject(base),"c_valueKind",o);
    c_free(o);

#undef _ENUMVAL_
#undef _ENUMVAL_PREFIX_

    {
        C_ALIGNMENT_TYPE (c_threadId);
        o = c_metaDeclare (c_metaObject (base), "c_threadId", M_STRUCTURE);
        c_metaTypeInit (o, sizeof (c_threadId), C_ALIGNMENT (c_threadId));
        c_free (o);
    }

    o = c_metaDefine(c_metaObject(base),M_STRUCTURE);
    members = c_arrayNew(c_member_t(base),2);
    members[0] = (c_voidp)c_metaDefine(c_metaObject(base),M_MEMBER);
        c_specifier(members[0])->name = c_stringNew(base,"seconds");
        c_specifier(members[0])->type = c_keep(c_long_t(base));
    members[1] = (c_voidp)c_metaDefine(c_metaObject(base),M_MEMBER);
        c_specifier(members[1])->name = c_stringNew(base,"nanoseconds");
        c_specifier(members[1])->type = c_keep(c_ulong_t(base));
    c_structure(o)->members = members;
    c_metaObject(o)->definedIn = c_metaObject(base);
    c_metaFinalize(o);
    found = c_metaBind(c_metaObject(base),"c_time",o);
    c_free(found);
    c_free(o);

    o = c_metaDefine(c_metaObject(base),M_TYPEDEF);
    c_metaObject(o)->definedIn = c_metaObject(base);
    c_typeDef(o)->alias = c_keep(c_ulonglong_t(base));
    c_metaFinalize(o);
    found = c_metaBind(c_metaObject(base),"os_timeW",o);
    c_free(found);
    c_free(o);

    o = c_metaDefine(c_metaObject(base),M_TYPEDEF);
    c_metaObject(o)->definedIn = c_metaObject(base);
    c_typeDef(o)->alias = c_keep(c_ulonglong_t(base));
    c_metaFinalize(o);
    found = c_metaBind(c_metaObject(base),"os_timeM",o);
    c_free(found);
    c_free(o);

    o = c_metaDefine(c_metaObject(base),M_TYPEDEF);
    c_metaObject(o)->definedIn = c_metaObject(base);
    c_typeDef(o)->alias = c_keep(c_ulonglong_t(base));
    c_metaFinalize(o);
    found = c_metaBind(c_metaObject(base),"os_timeE",o);
    c_free(found);
    c_free(o);

    o = c_metaDefine(c_metaObject(base),M_TYPEDEF);
    c_metaObject(o)->definedIn = c_metaObject(base);
    c_typeDef(o)->alias = c_keep(c_longlong_t(base));
    c_metaFinalize(o);
    found = c_metaBind(c_metaObject(base),"os_duration",o);
    c_free(found);
    c_free(o);

    o = c_metaDeclare(c_metaObject(base),"c_value",M_UNION);
    c_metaTypeInit(o,sizeof(struct c_value),C_ALIGNMENT(c_value));
    c_free(o);

    o = c_metaDeclare(c_metaObject(base),"c_annotation", M_CLASS);
    c_metaTypeInit(o,sizeof(struct c_annotation_s),C_ALIGNMENT_C_STRUCT(c_annotation));
    c_free(o);


    /* Now all meta meta references required for initialization are available.
     * The following statements will initialize all meta meta objects.
     * After initialization reflection will be operational.
     */

    /* Initialize abstract meta types */

#define _INITCLASS_(s,c,p) \
    c_metaObject(initClass(ResolveType(s,c),ResolveType(s,p)))

#define C_META_FINALIZE_(o) \
        c_type(o)->alignment = 0; c__metaFinalize(o,FALSE)

    o = _INITCLASS_(base,c_baseObject,NULL);
        type = ResolveType(base,c_metaKind);
        C_META_ATTRIBUTE_(c_baseObject,o,kind,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_operand,c_baseObject);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_specifier,c_baseObject);
        C_META_ATTRIBUTE_(c_specifier,o,name,c_string_t(base));
        type = ResolveType(base,c_type);
        C_META_ATTRIBUTE_(c_specifier,o,type,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_metaObject,c_baseObject);
        C_META_ATTRIBUTE_(c_metaObject,o,name,c_string_t(base));
        C_META_ATTRIBUTE_(c_metaObject,o,definedIn,c_voidp_t(base));
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_property,c_metaObject);
        type = ResolveType(base,c_type);
        C_META_ATTRIBUTE_(c_property,o,type,type);
        c_free(type);
        type = ResolveType(base,c_address);
        C_META_ATTRIBUTE_(c_property,o,offset,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_type,c_metaObject);
        C_META_ATTRIBUTE_(c_type,o,base,c_voidp_t(base));
        type = ResolveType(base,c_address);
        C_META_ATTRIBUTE_(c_type,o,alignment,type);
        c_free(type);
        C_META_ATTRIBUTE_(c_type,o,objectCount,c_long_t(base));
        type = ResolveType(base,c_address);
        C_META_ATTRIBUTE_(c_type,o,size,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    /* Initialize meta types */
    o = _INITCLASS_(base,c_literal,c_operand);
        type = ResolveType(base,c_value);
        C_META_ATTRIBUTE_(c_literal,o,value,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_constOperand,c_operand);
        type = ResolveType(base,c_constant);
        C_META_ATTRIBUTE_(c_constOperand,o,constant,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_expression,c_operand);
        type = ResolveType(base,c_exprKind);
        C_META_ATTRIBUTE_(c_expression,o,kind,type);
        c_free(type);
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_operand>");
            c_collectionTypeKind(type) = OSPL_C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_operand);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_expression,o,operands,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_parameter,c_specifier);
        type = ResolveType(base,c_direction);
        C_META_ATTRIBUTE_(c_parameter,o,mode,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_member,c_specifier);
        type = ResolveType(base,c_address);
        C_META_ATTRIBUTE_(c_member,o,offset,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_unionCase,c_specifier);
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_literal>");
            c_collectionTypeKind(type) = OSPL_C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_literal);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_unionCase,o,labels,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_attribute,c_property);
        C_META_ATTRIBUTE_(c_attribute,o,isReadOnly,c_bool_t(base));
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_relation,c_property);
        C_META_ATTRIBUTE_(c_relation,o,inverse,c_string_t(base));
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_module,c_metaObject);
        type = ResolveType(base,c_mutex);
        C_META_ATTRIBUTE_(c_module,o,mtx,type);
        c_free(type);
        type = ResolveType(base,c_scope);
        C_META_ATTRIBUTE_(c_module,o,scope,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_constant,c_metaObject);
        type = ResolveType(base,c_operand);
        C_META_ATTRIBUTE_(c_constant,o,operand,type);
        c_free(type);
        type = ResolveType(base,c_type);
        C_META_ATTRIBUTE_(c_constant,o,type,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_operation,c_metaObject);
        type = ResolveType(base,c_type);
        C_META_ATTRIBUTE_(c_operation,o,result,type);
        c_free(type);
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_parameter>");
            c_collectionTypeKind(type) = OSPL_C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_parameter);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_operation,o,parameters,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_typeDef,c_type);
        type = ResolveType(base,c_type);
        C_META_ATTRIBUTE_(c_typeDef,o,alias,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_collectionType,c_type);
        type = ResolveType(base,c_collKind);
        C_META_ATTRIBUTE_(c_collectionType,o,kind,type);
        c_free(type);
        C_META_ATTRIBUTE_(c_collectionType,o,maxSize,c_ulong_t(base));
        type = ResolveType(base,c_type);
        C_META_ATTRIBUTE_(c_collectionType,o,subType,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_primitive,c_type);
        type = ResolveType(base,c_primKind);
        C_META_ATTRIBUTE_(c_primitive,o,kind,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_enumeration,c_type);
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_constant>");
            c_collectionTypeKind(type) = OSPL_C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_constant);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_enumeration,o,elements,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_union,c_type);
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_unionCase>");
            c_collectionTypeKind(type) = OSPL_C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_unionCase);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_union,o,cases,type);
        c_free(type);
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_unionCase>");
            c_collectionTypeKind(type) = OSPL_C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_unionCase);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_union,o,references,type);
        c_free(type);
        type = ResolveType(base,c_scope);
        C_META_ATTRIBUTE_(c_union,o,scope,type);
        c_free(type);
        type = ResolveType(base,c_type);
        C_META_ATTRIBUTE_(c_union,o,switchType,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_structure,c_type);
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_member>");
            c_collectionTypeKind(type) = OSPL_C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_member);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_structure,o,members,type);
        c_free(type);
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_member>");
            c_collectionTypeKind(type) = OSPL_C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_member);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_structure,o,references,type);
        c_free(type);
        type = ResolveType(base,c_scope);
        C_META_ATTRIBUTE_(c_structure,o,scope,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_exception,c_structure);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_interface,c_type);
        C_META_ATTRIBUTE_(c_interface,o,abstract,c_bool_t(base));
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_interface>");
            c_collectionTypeKind(type) = OSPL_C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_interface);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_interface,o,inherits,type);
        c_free(type);
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_property>");
            c_collectionTypeKind(type) = OSPL_C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_property);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_interface,o,references,type);
        c_free(type);
        type = ResolveType(base,c_scope);
        C_META_ATTRIBUTE_(c_interface,o,scope,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_class,c_interface);
        type = ResolveType(base,c_interface);
        C_META_ATTRIBUTE_(c_class,o,extends,type);
        c_free(type);
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_string>");
            c_collectionTypeKind(type) = OSPL_C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_string);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_class,o,keys,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_annotation,c_interface);
        type = ResolveType(base,c_annotation);
        C_META_ATTRIBUTE_(c_annotation,o,extends,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);


/*---------------------------------------------------------------*/

    o = _INITCLASS_(base,c_base,c_module);

        C_META_FINALIZE_(o);
        c_free(o);

/*---------------------------------------------------------------*/

#undef _INITCLASS_

#define _SWITCH_TYPE_ c_enumeration(c_union(o)->switchType)

    o = c_metaDeclare(c_metaObject(base),"c_value",M_UNION);
        c_union(o)->switchType = ResolveType(base,c_valueKind);
        type = ResolveType(base,c_literal);
        c_union(o)->cases = c_arrayNew(type,18);
        c_free(type);
        c_free(c_union(o)->scope);
        c_union(o)->scope = NULL;

        caseNumber = 0;
        /* c_unionCaseNew transfers refCount of type and
         * c_enumValue(_SWITCH_TYPE_,"V_BOOLEAN")
         */
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_ADDRESS"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "Address",c_address_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_BOOLEAN"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "Boolean",c_bool_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_OCTET"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "Octet",c_octet_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_SHORT"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "Short",c_short_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_LONG"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "Long",c_long_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_LONGLONG"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "LongLong",c_longlong_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_USHORT"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "UShort",c_ushort_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_ULONG"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "ULong",c_ulong_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_ULONGLONG"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "ULongLong",c_ulonglong_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_FLOAT"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "Float",c_float_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_DOUBLE"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "Double",c_double_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_CHAR"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                               "Char",c_char_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_STRING"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                               "String",c_string_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_WCHAR"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                               "WChar",c_wchar_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_WSTRING"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                               "WString",c_wstring_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_FIXED"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                               "Fixed",c_string_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_OBJECT"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                               "Object",c_object_t(base),labels);
        c_iterFree(labels);

        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_VOIDP"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                               "Voidp",c_voidp_t(base),labels);
        c_iterFree(labels);
        c_metaFinalize(o);
        c_free(o);

#undef _SWITCH_TYPE_

    /* ::<_ospl_internal> */
    _intern = c_metaDeclare(c_metaObject(base), "_ospl_internal", M_MODULE);

    /* Declare builtin-annotations
     *
     * Note: these builtin annotation objects are not yet finalized. There is currently
     * no need because creation of annotations won't be supported until version >=7.
     */

    /* ::ID */
    o = c_metaDefine(c_metaObject(_intern), M_ANNOTATION);
    initAnnotation(o);
    found = c_metaDeclare(o, "value", M_ATTRIBUTE);
    c_property(found)->type = c_ulong_t(base);
    c_free(found);
    c_metaBind(c_metaObject(_intern), "ID", o);
    c_free(o);

    /* ::Optional */
    o = c_metaDefine(c_metaObject(_intern), M_ANNOTATION);
    initAnnotation(o);
    found = c_metaDeclare(o, "value", M_ATTRIBUTE);
    c_property(found)->type = c_bool_t(base);
    c_free(found);
    c_metaBind(c_metaObject(_intern), "optional", o);
    c_free(o);

    /* ::Key */
    o = c_metaDefine(c_metaObject(_intern), M_ANNOTATION);
    initAnnotation(o);
    found = c_metaDeclare(o, "value", M_ATTRIBUTE);
    c_property(found)->type = c_bool_t(base);
    c_free(found);
    c_metaBind(c_metaObject(_intern), "Key", o);
    c_free(o);

    /* ::Shared */
    o = c_metaDefine(c_metaObject(_intern), M_ANNOTATION);
    initAnnotation(o);
    found = c_metaDeclare(o, "value", M_ATTRIBUTE);
    c_property(found)->type = c_bool_t(base);
    c_free(found);
    c_metaBind(c_metaObject(_intern), "Shared", o);
    c_free(o);

    /* ::BitBound */
    o = c_metaDefine(c_metaObject(_intern), M_ANNOTATION);
    initAnnotation(o);
    found = c_metaDeclare(o, "value", M_ATTRIBUTE);
    c_property(found)->type = c_ushort_t(base);
    c_free(found);
    c_metaBind(c_metaObject(_intern), "BitBound", o);
    c_free(o);

    /* ::Value */
    o = c_metaDefine(c_metaObject(_intern), M_ANNOTATION);
    initAnnotation(o);
    found = c_metaDeclare(o, "value", M_ATTRIBUTE);
    c_property(found)->type = c_ulong_t(base);
    c_free(found);
    c_metaBind(c_metaObject(_intern), "Value", o);
    c_free(o);

    /* ::BitSet */
    o = c_metaDefine(c_metaObject(_intern), M_ANNOTATION);
    initAnnotation(o);
    c_metaBind(c_metaObject(_intern), "BitSet", o);
    c_free(o);

    /* ::Nested */
    o = c_metaDefine(c_metaObject(_intern), M_ANNOTATION);
    initAnnotation(o);
    found = c_metaDeclare(o, "value", M_ATTRIBUTE);
    c_property(found)->type = c_bool_t(base);
    c_free(found);
    c_metaBind(c_metaObject(_intern), "Nested", o);
    c_free(o);

    temp = c_metaDefine(c_metaObject(base),M_ENUMERATION);
    c_enumeration(temp)->elements = c_arrayNew(c_constant_t(base),3);
    c_enumeration(temp)->elements[0] = c_metaDeclareEnumElement(c_metaObject(base),"FINAL_EXTENSIBILITY");
    c_enumeration(temp)->elements[1] = c_metaDeclareEnumElement(c_metaObject(base),"EXTENSIBLE_EXTENSIBILITY");
    c_enumeration(temp)->elements[2] = c_metaDeclareEnumElement(c_metaObject(base),"MUTABLE_EXTENSIBILITY");
    c_metaFinalize(temp);
    c_metaBind(c_metaObject(base), "ExtensibilityKind", temp);

    /* ::Extensibility */
    o = c_metaDefine(c_metaObject(_intern), M_ANNOTATION);
    initAnnotation(o);
    found = c_metaDeclare(o, "value", M_ATTRIBUTE);
    /* Transfer refCount of temp here, so no c_free on it later. */
    c_property(found)->type = c_type(temp);
    c_free(found);
    c_metaBind(c_metaObject(_intern), "Extensibility", o);
    c_free(o);

    /* ::MustUnderstand */
    o = c_metaDefine(c_metaObject(_intern), M_ANNOTATION);
    initAnnotation(o);
    found = c_metaDeclare(o, "value", M_ATTRIBUTE);
    c_property(found)->type = c_bool_t(base);
    c_free(found);
    c_metaBind(c_metaObject(_intern), "MustUnderstand", o);
    c_free(o);

    /* ::Verbatim */
    o = c_metaDefine(c_metaObject(_intern), M_ANNOTATION);
    initAnnotation(o);
    c_metaBind(c_metaObject(_intern), "Verbatim", o);

    found = c_metaObject(c_metaDefine(c_metaObject(base), M_COLLECTION));
    c_collectionType(found)->kind = OSPL_C_STRING;
    c_collectionType(found)->subType = c_char_t(base);
    c_collectionType(found)->maxSize = 32;
    c_metaObject(found)->definedIn = c_metaObject(base);
    c_metaFinalize(found);
    temp = c_metaBind(c_metaObject(base), "C_STRING<32>", found);
    c_free(found);

    found = c_metaDeclare(o, "language", M_ATTRIBUTE);
    /* Transfer refCount of temp here, so no c_free on it later. */
    c_property(found)->type = c_type(temp);
    c_free(found);

    found = c_metaObject(c_metaDefine(c_metaObject(base), M_COLLECTION));
    c_collectionType(found)->kind = OSPL_C_STRING;
    c_collectionType(found)->subType = c_char_t(base);
    c_collectionType(found)->maxSize = 128;
    c_metaObject(found)->definedIn = c_metaObject(base);
    c_metaFinalize(found);
    temp = c_metaBind(c_metaObject(base), "C_STRING<128>", found);
    c_free(found);

    found = c_metaDeclare(o, "placement", M_ATTRIBUTE);
    /* Transfer refCount of temp here, so no c_free on it later. */
    c_property(found)->type = c_type(temp);
    c_free(found);

    found = c_metaDeclare(o, "text", M_ATTRIBUTE);
    c_property(found)->type = c_string_t(base);
    c_free(found);
    c_free(o);

    c_fieldInit(base);
    c_querybaseInit(base);
}

c_base
c_create (
    const c_char *name,
    void *address,
    c_size size,
    c_size threshold)
{
    c_mm  mm;
    c_base base = NULL;
    c_base tempbase;
    c_header header;

    if ((size != 0) && (size < MIN_DB_SIZE)) {
        OS_REPORT(OS_ERROR,"c_base::c_create",0,
                    "Specified memory size (%"PA_PRIuSIZE") is too small to occupy a c_base instance,"
                    "required minimum size is %d bytes.",
                    size,MIN_DB_SIZE);
        return NULL;
    }
    mm = c_mmCreate(address,size, threshold);
    if (mm == NULL) {
        /* error is reported in c_mmCreate */
        return NULL;
    }
    header = (c_header)c_mmMalloc(mm, MEMSIZE(C_SIZEOF(c_base)));
    if (header) {
#ifndef NDEBUG
        header->confidence = CONFIDENCE;
#ifdef OBJECT_WALK
        header->nextObject = NULL;
        header->prevObject = NULL;
#endif
#endif
        pa_st32(&header->refCount, 1);
        header->type = NULL; /* Will be set in c_baseInit after bootstrapping */
        tempbase = (c_base)c_oid(header);
        base = (c_base)c_mmBind(mm, name, tempbase);
        if (base != tempbase) {
            OS_REPORT(OS_ERROR,
                        "c_base::c_create",0,
                        "Internal error, memory management seems corrupted.\n"
                        "             mm = 0x%"PA_PRIxADDR", name = %s,\n"
                        "             tempbase = 0x%"PA_PRIxADDR", base = 0x%"PA_PRIxADDR"",
                        (os_address)mm, name ? name : "(null)", (os_address)tempbase, (os_address)base);
            c_mmFree(mm, header);
            return NULL;
        }

        c_baseInit(base, mm);

        ospl_c_bind(base,"c_baseModule");
    }
    return base;
}

static void deleteGarbage(c_base base);

void
c_destroy (
    c_base _this)
{
    c_mm mm = _this->mm;
    deleteGarbage(_this);
    c_mmDestroy(mm);
}

void
c_detach (
    c_base _this)
{
    c_mmSuspend (_this->mm);
}

c_base
c_open (
    const c_char *name,
    void *address)
{
    c_base base;
    c_mm mm;
    c_size namelength;

    namelength = strlen(name);

    if (namelength == 0) {
        return c_create("HEAPDATABASE", NULL, 0, 0);
    }
    mm = c_mmCreate(address,0, 0);
    if (mm == NULL) {
        /* error is reported in c_mmCreate */
        return NULL;
    }
    base = (c_base)c_mmLookup(mm, name);
    if (base == NULL) {
        OS_REPORT(OS_ERROR,
                    "c_base::c_open",0,
                    "segment %s not found",name);
        return NULL;
    }

    if (base->confidence != CONFIDENCE) {
        OS_REPORT(OS_ERROR,
                    "c_base::c_open",0,
                    "confidence mismatch: %d <> %d",
                    base->confidence,CONFIDENCE);
        return NULL;
    }

    return base;
}

c_object
ospl_c_bind (
    c_object object,
    const c_char *name)
{
    ut_avlIPath_t p;
    c_baseBinding binding;
    c_base base;

    base = c_header(object)->type->base;
    assert(base->confidence == CONFIDENCE);

    if ((binding = ut_avlLookupIPath (&c_base_bindings_td, &base->bindings, name, &p)) != NULL) {
        return binding->object;
    } else if ((binding = (c_baseBinding) c_mmMalloc (base->mm, sizeof (*binding))) == NULL) {
        return NULL;
    } else {
        binding->name = c_stringNew(base,name);
        binding->object = c_keep(object);
        ut_avlInsertIPath (&c_base_bindings_td, &base->bindings, binding, &p);
        return binding->object;
    }
}

c_object
c_unbind (
    c_base base,
    const c_char *name)
{
    ut_avlDPath_t p;
    c_baseBinding binding;
    assert(base->confidence == CONFIDENCE);
    if ((binding = ut_avlLookupDPath (&c_base_bindings_td, &base->bindings, name, &p)) == NULL) {
        return NULL;
    } else {
        c_object object = binding->object;
        ut_avlDeleteDPath (&c_base_bindings_td, &base->bindings, binding, &p);
        c_free(binding->name);
        c_mmFree(base->mm, binding);
        return object;
    }
}

c_object
c_lookup (
    c_base base,
    const c_char *name)
{
    c_baseBinding found;
    assert(base->confidence == CONFIDENCE);
    c_mutexLock(&base->bindLock);
    found = ut_avlLookup (&c_base_bindings_td, &base->bindings, name);
    c_mutexUnlock(&base->bindLock);
    if (found != NULL) {
        return c_keep(found->object);
    } else {
        return NULL;
    }
}

c_type
c_resolve (
    c_base base,
    const c_char *typeName)
{
    c_metaObject o;

    assert(base->confidence == CONFIDENCE);

    o = c_metaResolve(c_metaObject(base),typeName);
    if (c_objectIsType(c_baseObject(o))) {
        return c_type(o);
    }
    c_free(o);
    return NULL;
}

os_size_t
c_memsize (
    c_type type)
{
    return MEMSIZE(c_typeSize(type));
}

c_memoryThreshold
c_baseGetMemThresholdStatus(
    c_base _this)
{
    return c_mmbaseGetMemThresholdStatus(_this->mm);
}

c_bool
c_baseMakeMemReservation(
    c_base _this,
    os_address amount)
{
    if (amount != C_MM_RESERVATION_NO_CHECK) {
        return c_mmbaseMakeReservation(_this->mm, amount);
    }
    return TRUE;
}

void
c_baseReleaseMemReservation(
    c_base _this,
    os_address amount)
{
    if (amount != C_MM_RESERVATION_NO_CHECK) {
        c_mmbaseReleaseReservation(_this->mm, amount);
    }
}

static c_object
c__newCommon (
    c_type type,
    c_bool check)
{
    c_header header;
    c_object o;
    os_size_t size;
    os_uint32 traceType;

    assert(type);

    if (c_baseObjectKind (type) != M_COLLECTION) {
        size = type->size;
    } else {
#ifndef NDEBUG
        if (c_collectionTypeKind (type) == OSPL_C_ARRAY ||
            c_collectionTypeKind (type) == OSPL_C_SEQUENCE)
        {
            OS_REPORT(OS_ERROR,
                      "Database c_new",0,
                      "c_new cannot create OSPL_C_ARRAY nor OSPL_C_SEQUENCE, "
                      "use c_newArray or c_newSequence respectively");
            assert(0);
        }
#endif
        size = c_typeSize (type);
    }
    assert (size > 0);

    if (check) {
        header = (c_header) c_mmMallocThreshold (type->base->mm, MEMSIZE(size));
    } else {
        header = (c_header) c_mmMalloc (type->base->mm, MEMSIZE(size));
        if (!header) {
            abort();
        }
    }
    if (c_unlikely (header == NULL)) {
        return NULL;
    }

    traceType = (pa_ld32(&c_header(type)->refCount) & REFCOUNT_FLAG_TRACETYPE);
    pa_st32(&header->refCount, 1);
    if (traceType) {
        pa_or32(&header->refCount, REFCOUNT_FLAG_TRACE);
        c_mmTrackObject (type->base->mm, header, C_MMTRACKOBJECT_CODE_MIN + 2);
    }
#if TYPE_REFC_COUNTS_OBJECTS
    header->type = c_keep(type);
#else
    header->type = type;
#endif
    if (type->base->maintainObjectCount) {
        pa_inc32(&type->objectCount);
    }
#ifndef NDEBUG
    header->confidence = CONFIDENCE;
#ifdef OBJECT_WALK
    header->nextObject = NULL;
    c_header(type->base->lastObject)->nextObject = o;
    header->prevObject = type->base->lastObject;
    assert(type->base->firstObject != NULL);
    type->base->lastObject = o;
#endif
#endif

    o = c_oid (header);
    memset (o, 0, size);
#if CHECK_REF
    ACTUALTYPE(type,header->type);
    if (type && c_metaObject(type)->name) {
        if (strlen(c_metaObject(type)->name) == CHECK_REF_TYPE_LEN) {
            if (strncmp(c_metaObject(type)->name, CHECK_REF_TYPE, strlen(CHECK_REF_TYPE)) == 0) {
                UT_TRACE("\n\n============ New(%p) =============\n", o);
            }
        }
    }
#endif
    return o;
}

c_object
c_new (
    c_type type)
{
    return c__newCommon(type, FALSE);
}

c_object
c_new_s(
    c_type type)
{
    return c__newCommon(type, TRUE);
}

/*
 * Use the macro definitions c_newArray and c_newSequence instead, respectively
 * for creating an array or a sequence for legibility.
 */
static c_object
c__newBaseArrayObjectCommon (
    c_collectionType arrayType,
    c_ulong size,
    c_bool check)
{
    c_size allocSize;
    c_object o = NULL;

    assert(arrayType);
    assert(
        (c_collectionTypeKind(arrayType) == OSPL_C_ARRAY && size > 0)
        ||
        (c_collectionTypeKind(arrayType) == OSPL_C_SEQUENCE)
        );

    if ((c_collectionTypeKind(arrayType) == OSPL_C_ARRAY)
         || (c_collectionTypeKind(arrayType) == OSPL_C_SEQUENCE)) {
        if (    (c_collectionTypeKind(arrayType) == OSPL_C_ARRAY && size > 0)
                ||
                (c_collectionTypeKind(arrayType) == OSPL_C_SEQUENCE) ) {
            c_arrayHeader hdr;
            c_header header;
            c_base base;
            c_type subType;

            subType = c_collectionTypeSubType(arrayType);

            switch(c_baseObjectKind(subType)) {
            case M_INTERFACE:
            case M_CLASS:
            case M_ANNOTATION:
                allocSize = size * sizeof(void *);
            break;
            default:
                if (subType->size == 0) {
                    subType->size = sizeof(void *);
                }
                allocSize = size * subType->size;
            break;
            }

            base = c_type(arrayType)->base;
            if (check) {
                hdr = (c_arrayHeader)c_mmMallocThreshold(base->mm, ARRAYMEMSIZE(allocSize));
            } else {
                hdr = (c_arrayHeader)c_mmMalloc(base->mm, ARRAYMEMSIZE(allocSize));
                if (!hdr) {
                    abort();
                }
            }

            if (hdr) {
                os_uint32 traceType;

                hdr->size = size;
                header = (c_header)&hdr->_parent;

                traceType = (pa_ld32(&c_header(arrayType)->refCount) & REFCOUNT_FLAG_TRACETYPE);
                pa_st32(&header->refCount, 1);
                if (traceType) {
                    pa_or32(&header->refCount, REFCOUNT_FLAG_TRACE);
                    c_mmTrackObject (base->mm, header, C_MMTRACKOBJECT_CODE_MIN + 2);
                }
                /* Keep reference to our type */
#if TYPE_REFC_COUNTS_OBJECTS
                header->type = c_keep(c_type(arrayType));
#else
                header->type = c_type(arrayType);
#endif
                if (base->maintainObjectCount) {
                    pa_inc32(&header->type->objectCount);
                }

                o = c_oid(header);

                /* When an array is freed via c_free, it also checks whether it contains
                 * references and if they need to be freed. If the user did not fill the whole array
                 * a c_free on garbage data could be performed, which causes undefined behaviour.
                 * Therefore the whole array is set on 0.
                 */
                memset(o, 0, allocSize);

#ifndef NDEBUG
                header->confidence = CONFIDENCE;
#ifdef OBJECT_WALK
                header->nextObject = NULL;
                c_header(arrayType->base->lastObject)->nextObject = o;
                header->prevObject = arrayType->base->lastObject;
                assert(arrayType->base->firstObject != NULL);
                arrayType->base->lastObject = o;
#endif
#endif
            }
        } else {
            OS_REPORT(OS_ERROR,
                        "Database c_newBaseArrayObject",0,
                        "Illegal size %d specified", size);
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "Database c_newBaseArrayObject",0,
                  "Specified type is not an array nor a sequence");
    }
    return o;
}

c_object
c_newBaseArrayObject (
    c_collectionType arrayType,
    c_ulong size)
{
    return c__newBaseArrayObjectCommon(arrayType, size, FALSE);
}


c_object
c_newBaseArrayObject_s (
    c_collectionType arrayType,
    c_ulong size)
{
    return c__newBaseArrayObjectCommon(arrayType, size, TRUE);
}

c_object
c_mem2object(
    c_voidp mem,
    c_type type)
{
    c_object o;
    c_type t;
    C_STRUCT(c_header) *header = (C_STRUCT(c_header) *)mem;

    pa_st32(&header->refCount, 1);
    header->type = c_keep(type);

    o = c_oid(header);
    ACTUALTYPE(t, type);
    memset(o,0,c_typeSize(t));

#ifndef NDEBUG
    header->confidence = CONFIDENCE;
#ifdef OBJECT_WALK
    header->nextObject = NULL;
    c_header(t->base->lastObject)->nextObject = o;
    header->prevObject = t->base->lastObject;
    assert(t->base->firstObject != NULL);
    t->base->lastObject = o;
#endif
#endif
    return o;
}

c_voidp
c_object2mem (
    c_object object)
{
    c_header header;

    if (object == NULL) {
        return NULL;
    }

    header = c_header(object);
    assert((pa_ld32(&header->refCount) & REFCOUNT_MASK) == 0);
#ifndef NDEBUG
    assert(header->confidence == CONFIDENCE);
#endif

    return header;
}

#ifndef NDEBUG
void
c__assertValidDatabaseObject(
    c_voidp o)
{
    c_header header;
    assert(o);
    header = c_header(o);
    assert(header->confidence == CONFIDENCE);
    assert((pa_ld32(&header->refCount) & REFCOUNT_MASK) > 0);
}
#endif

/**
 * Object Free functionality
 *
 * The c_free decreases the refcount of a database object and releases claimed memory resources
 * when the refcount reaches zero, in other words the object is no longer reachable / used.
 *
 * The free will also free other referenced objects when an object is destroyed, this traversal
 * of references is solved in an iterative algorithm and not recursively to avoid stack memory depletion.
 *
 * The algorithm works by:
 * - first initialize a freelist intended to hold any references that are encountered that also need to be freed.
 * - enter the processing loop
 * - then decrease the refcount of the object.
 * - if the refcount reaches zero then add all direct referenced objects to the freelist.
 * - release the objects memory resources.
 * - get the next object from the freelist and goto the start of the loop.
 *
 * Performance considerations:
 * The freelist is ia stretchy fifo queue that is implemented as an array allocated on heap to avoid
 * memory allocation and deallocation, however when the number of elements exceeds the array size (REFBUF_SIZE)
 * then the freelist will allocate additional buffers forming a chain of buffers.
 * Allocated buffers in the chain that become empty when taking object from the queue are freed on the fly.
 * The size of the buffer becomes less effective the greater it gets and considering that the initial buffer
 * is allocated on stack it should not be too big.
 */

#define REFBUF_SIZE (256)

/* The class c_refbuf implements the chainable object buffer. */
C_CLASS(c_refbuf);
C_STRUCT(c_refbuf) {
    c_object element[REFBUF_SIZE];
    c_refbuf next;
};

#define c_refbuf(o) ((c_refbuf)o)

/* The class c_freelist implements the stretchy fifo queue used by the iterative free algorithm. */
C_CLASS(c_freelist);
C_STRUCT(c_freelist) {
    C_EXTENDS(c_refbuf);  /* Initial object buffer that is initialized on stack. */
    c_refbuf tail_buf;    /* Tail buffer of the queue, objects are read from this buffer. */
    c_refbuf head_buf;    /* Head buffer of the queue, objects are inserted in this buffer. */
    os_uint32 tail_idx;   /* Tail buffer index of the queue, oldest object that is next to be read. */
    os_uint32 head_idx;   /* Head buffer index of the queue, place where the next object will be inserted. */
};

/* This function is used to initialize a new freelist heap object. */
static void
c_freelistInit(
    c_freelist freelist)
{
    c_refbuf(freelist)->next = NULL;
    freelist->tail_buf = c_refbuf(freelist);
    freelist->head_buf = c_refbuf(freelist);
    freelist->tail_idx = 0;
    freelist->head_idx = 0;
}

/* This function inserts an object at the head of the freelist. */
static void
c_freelistInsert(
    c_freelist freelist,
    c_object object)
{
    if (object == NULL) return;
    assert(c_header(object)->confidence == CONFIDENCE);
    if (freelist->head_idx == REFBUF_SIZE) {
        freelist->head_buf->next = os_malloc(sizeof(C_STRUCT(c_refbuf)));
        freelist->head_buf = freelist->head_buf->next;
        freelist->head_buf->next = NULL;
        freelist->head_idx = 0;
    }
    freelist->head_buf->element[freelist->head_idx++] = object;
}

/* This function will remove the oldest object from the freelist. */
static c_object
c_freelistTake(
    c_freelist freelist)
{
    c_object object = NULL;
    c_refbuf freebuf;

    if (freelist->tail_idx == REFBUF_SIZE) {
        /* Reached the end of a buffer so it is empty and if allocated can be freed. */
        freelist->tail_idx = 0;
        freebuf = freelist->tail_buf;
        freelist->tail_buf = freelist->tail_buf->next;

        if (freebuf != c_refbuf(freelist)) {
            /* The empty buffer was allocated so free it now. */
            assert(freebuf == c_refbuf(freelist)->next);
            os_free(freebuf);
        }
        c_refbuf(freelist)->next = freelist->tail_buf;
        if (freelist->tail_buf == NULL) {
            /* If the tail_buf is NULL then the queue is empty meaning that it can conveniently
             * be reinitialized as a new freelist.
             */
            c_freelistInit(freelist);
        }
    }
    if (freelist->head_idx > freelist->tail_idx || freelist->head_buf != freelist->tail_buf )
    {
        /* the freelist is not empty so take an object from the freelist. */
        object = freelist->tail_buf->element[freelist->tail_idx++];
    } else if (freelist->tail_buf != c_refbuf(freelist)) {
        /* The freelist is empty but also has an allocated buffer so free it now. */
        os_free(freelist->tail_buf);
    }
    return object;
}

static c_bool _c_freeReferences(c_metaObject metaObject, c_object o, c_freelist freelist);
#define c_freeReferences(m,o,l) ((m && o)? _c_freeReferences(m,o,l):TRUE)
#define freeReference(p,t,l) (p?_freeReference(p,t,l):TRUE)

static c_bool
_freeReference (
    c_voidp *p,
    c_type type,
    c_freelist freelist)
{
    c_type t = type;

    assert(p);
    assert(t);

    while (c_baseObject(t)->kind == M_TYPEDEF) {
        t = c_typeDef(t)->alias;
    }
    switch (c_baseObject(t)->kind) {
    case M_CLASS:
    case M_INTERFACE:
    case M_ANNOTATION:
        c_freelistInsert(freelist, c_object(*p));
    break;
    case M_BASE:
    case M_COLLECTION:
        if ((c_collectionType(t)->kind == OSPL_C_ARRAY) &&
            (c_collectionType(t)->maxSize != 0)) {
            c_freeReferences(c_metaObject(t), p, freelist);
        } else {
            c_freelistInsert(freelist, c_object(*p));
        }
    break;
    case M_EXCEPTION:
    case M_STRUCTURE:
    case M_UNION:
        c_freeReferences(c_metaObject(type),p, freelist);
    break;
    case M_PRIMITIVE:
        switch (c_primitive(t)->kind) {
        case P_MUTEX:
            c_mutexDestroy((c_mutex *)p);
        break;
        case P_LOCK:
            c_lockDestroy((c_lock *)p);
        break;
        case P_COND:
            c_condDestroy((c_cond *)p);
        break;
        default:
        break;
        }
    break;
    default:
        OS_REPORT(OS_ERROR,
                  "freeReference",0,
                  "illegal object detected");
        assert(FALSE);
        return FALSE;
    }
    return TRUE;
}

static c_bool
_c_freeReferences (
    c_metaObject metaObject,
    c_object o,
    c_freelist freelist)
{
    c_type type;
    c_class cls;
    c_array references, labels, ar;
    c_property property;
    c_member member;
    c_ulong i,j,length;
    c_size size;
    c_ulong nrOfRefs,nrOfLabs;
    c_value v;

    assert(metaObject);
    assert(o);

    switch (c_baseObject(metaObject)->kind) {
    case M_CLASS:
        /* Check if type is an enumeration and in that case set the type of its enum constant to NULL.
         * This is because the existing cyclic reference between enumeration and its constant are
         * deliberately not ref counted otherwise the enumeration would be kept alive forever.
         * Now we need to correct this otherwise freeing the constants will decrease the refcount
         * of the enumeration type below zero.
         * This solution will not win the beauty contest, a better solution would be to implement a
         * c_enumConstant instead of using c_constant but that is considerably more work to implement.
         */
        if (metaObject == c_metaObject(c_type(metaObject)->base->metaType[M_ENUMERATION])) {
            c_array elements = ((c_enumeration)o)->elements;
            length = c_arraySize(elements);
            for (i=0; i<length; i++) {
                c_constant(elements[i])->type = NULL;
            }
        }
        cls = c_class(metaObject);
        while (cls) {
            length = c_arraySize(c_interface(cls)->references);
            for (i=0;i<length;i++) {
                property = c_property(c_interface(cls)->references[i]);
                type = property->type;
                freeReference(C_DISPLACE(o,property->offset),type, freelist);
            }
            cls = cls->extends;
        }
    break;
    case M_INTERFACE:
    case M_ANNOTATION:
        length = c_arraySize(c_interface(metaObject)->references);
        for (i=0;i<length;i++) {
            property = c_property(c_interface(metaObject)->references[i]);
            type = property->type;
            freeReference(C_DISPLACE(o,property->offset),type, freelist);
        }
    break;
    case M_EXCEPTION:
    case M_STRUCTURE:
        length = c_arraySize(c_structure(metaObject)->references);
        for (i=0;i<length;i++) {
            member = c_member(c_structure(metaObject)->references[i]);
            type = c_specifier(member)->type;
            freeReference(C_DISPLACE(o,member->offset),type, freelist);
        }
    break;
    case M_UNION:
#define _CASE_(k,t) case k: v = t##Value(*((t *)o)); break
        switch (c_metaValueKind(c_metaObject(c_union(metaObject)->switchType))) {
        _CASE_(V_BOOLEAN,   c_bool);
        _CASE_(V_OCTET,     c_octet);
        _CASE_(V_SHORT,     c_short);
        _CASE_(V_LONG,      c_long);
        _CASE_(V_LONGLONG,  c_longlong);
        _CASE_(V_USHORT,    c_ushort);
        _CASE_(V_ULONG,     c_ulong);
        _CASE_(V_ULONGLONG, c_ulonglong);
        _CASE_(V_CHAR,      c_char);
        _CASE_(V_WCHAR,     c_wchar);
        default:
            OS_REPORT(OS_ERROR,
                      "c_freeReferences",0,
                      "illegal union switch type detected");
            assert(FALSE);
            return FALSE;
        }
#undef _CASE_
        references = c_union(metaObject)->references;
        if (references != NULL) {
            i=0; type=NULL;
            nrOfRefs = c_arraySize(references);
            while ((i<nrOfRefs) && (type == NULL)) {
                labels = c_unionCase(references[i])->labels;
                j=0;
                nrOfLabs = c_arraySize(labels);
                while ((j<nrOfLabs) && (type == NULL)) {
                    if (c_valueCompare(v,c_literal(labels[j])->value) == C_EQ) {
                        c_freeReferences(c_metaObject(references[i]),
                                         C_DISPLACE(o,c_type(metaObject)->alignment), freelist);
                        type = c_specifier(references[i])->type;
                    }
                    j++;
                }
                i++;
            }
        }
    break;
    case M_COLLECTION:
        switch (c_collectionType(metaObject)->kind) {
        case OSPL_C_ARRAY:
        case OSPL_C_SEQUENCE:
            ACTUALTYPE(type,c_collectionType(metaObject)->subType);
            ar = (c_array)o;

            if(c_collectionType(metaObject)->kind == OSPL_C_ARRAY
                && c_collectionType(metaObject)->maxSize > 0){
                length = c_collectionType(metaObject)->maxSize;
            } else {
                length = c_arraySize(ar);
            }

            if (c_typeIsRef(type)) {
                for (i=0;i<length;i++) {
                    c_freelistInsert(freelist, ar[i]);
                }
            } else {
                if (c_typeHasRef(type)) {
                    size = type->size;
                    for (i=0;i<length;i++) {
                        freeReference(C_DISPLACE(ar,(i*size)),type, freelist);
                    }
                }
            }
        break;
        case OSPL_C_SCOPE:
            c_scopeDeinit(c_scope(o));
        break;
        default:
            c_clear(o);
        break;
        }
    break;
    case M_BASE:
    break;
    case M_TYPEDEF:
        c_freeReferences(c_metaObject(c_typeDef(metaObject)->alias),o, freelist);
    break;
    case M_ATTRIBUTE:
    case M_RELATION:
        ACTUALTYPE(type,c_property(metaObject)->type);
        freeReference(C_DISPLACE(o,c_property(metaObject)->offset),type, freelist);
    break;
    case M_MEMBER:
        ACTUALTYPE(type,c_specifier(metaObject)->type);
        freeReference(C_DISPLACE(o,c_member(metaObject)->offset),type, freelist);
    break;
    case M_UNIONCASE:
        ACTUALTYPE(type,c_specifier(metaObject)->type);
        freeReference(o,type, freelist);
    break;
    case M_MODULE:
        c_freelistInsert(freelist, c_module(o)->scope);
    break;
    case M_PRIMITIVE:
        /* Do nothing */
    break;
    default:
        OS_REPORT(OS_ERROR,
                  "c_freeReferences",0,
                  "illegal meta object specified");
        assert(FALSE);
        return FALSE;
    }
    return TRUE;
}

#ifndef NDEBUG
/* Function used in OS_REPORT in c_free */
static const c_char *
metaKindImage (
    c_metaKind kind)
{
#define _CASE_(o) case o: return #o
    switch (kind) {
    _CASE_(M_UNDEFINED);
    _CASE_(M_ANNOTATION);
    _CASE_(M_ATTRIBUTE);
    _CASE_(M_CLASS);
    _CASE_(M_COLLECTION);
    _CASE_(M_CONSTANT);
    _CASE_(M_CONSTOPERAND);
    _CASE_(M_ENUMERATION);
    _CASE_(M_EXCEPTION);
    _CASE_(M_EXPRESSION);
    _CASE_(M_INTERFACE);
    _CASE_(M_LITERAL);
    _CASE_(M_MEMBER);
    _CASE_(M_MODULE);
    _CASE_(M_OPERATION);
    _CASE_(M_PARAMETER);
    _CASE_(M_PRIMITIVE);
    _CASE_(M_RELATION);
    _CASE_(M_BASE);
    _CASE_(M_STRUCTURE);
    _CASE_(M_TYPEDEF);
    _CASE_(M_UNION);
    _CASE_(M_UNIONCASE);
    _CASE_(M_COUNT);
    default:
        return "Unknown metaKind specified";
            }
#undef _CASE_
}
#endif

void
c_free (
    c_object object)
{
    c_header header;
    c_type type, headerType;
    C_STRUCT(c_freelist) freelist;
    os_uint32 safeCount;
#if CHECK_REF
    c_bool matchesRefRequest = FALSE;
#endif

    if (object == NULL) {
        return;
    }

    c_freelistInit(&freelist);

    while (object) {
        header = c_header (object);

#ifndef NDEBUG
        assert(header->confidence == CONFIDENCE);
        if ((pa_ld32(&header->refCount) & REFCOUNT_MASK) == 0) {
#if CHECK_REF
            UT_TRACE("\n\n===========Free(%p) already freed =======\n", object);
#endif
            OS_REPORT(OS_ERROR,
                        "Database",0,
                        "Object (%p) of type '%s', kind '%s' already deleted",
                        object,
                        c_metaName(c_metaObject(header->type)),
                        metaKindImage(c_baseObject(header->type)->kind));
            assert(0);
        }
#endif /* NDEBUG */

#if CHECK_REF
        /* Take a local pointer, since header->type pointer will be deleted */
        headerType = header->type;
        ACTUALTYPE(type,headerType);
        if (type && c_metaObject(type)->name) {
          if (strlen(c_metaObject(type)->name) >= CHECK_REF_TYPE_LEN) {
            if (strncmp(c_metaObject(type)->name, CHECK_REF_TYPE, strlen(CHECK_REF_TYPE)) == 0) {
                matchesRefRequest = TRUE;
            }
          }
        }
#endif
        safeCount = pa_dec32_nv(&header->refCount);

        if ((safeCount & REFCOUNT_MASK) != 0) {
            if (safeCount & REFCOUNT_FLAG_TRACE) {
                c_type headerType = header->type, type;
                void *block;
                ACTUALTYPE (type, headerType);
                if ((c_baseObjectKind (type) == M_COLLECTION) &&
                    ((c_collectionTypeKind (type) == OSPL_C_ARRAY) ||
                     (c_collectionTypeKind (type) == OSPL_C_SEQUENCE)))
                {
                    block = c_arrayHeader (object);
                } else {
                    block = header;
                }
                c_mmTrackObject (type->base->mm, block, C_MMTRACKOBJECT_CODE_MIN + 1);
            }
        } else {
            c_base base;

            /* Take a local pointer, since header->type pointer will be deleted */
#if ! CHECK_REF
            headerType = header->type;
            ACTUALTYPE(type,headerType);
#endif

            base = type->base;
            if (!(safeCount & REFCOUNT_FLAG_ATOMIC)) {
                c_freeReferences(c_metaObject(type),object, &freelist);
            }
#ifndef NDEBUG
#ifdef OBJECT_WALK
            {
                c_object *prevNext;
                c_object *nextPrev;

                if (header->prevObject != NULL) {
                    prevNext = &c_header(header->prevObject)->nextObject;
                } else {
                    prevNext = &type->base->firstObject;
                }
                if (header->nextObject != NULL) {
                    nextPrev = &c_header(header->nextObject)->prevObject;
                } else {
                    nextPrev = &type->base->lastObject;
                }
                *prevNext = header->nextObject;
                *nextPrev = header->prevObject;
            }
#endif
#endif

            if ((c_baseObjectKind(type) == M_COLLECTION) &&
                ((c_collectionTypeKind(type) == OSPL_C_ARRAY) ||
                 (c_collectionTypeKind(type) == OSPL_C_SEQUENCE))) {
                c_arrayHeader hdr;

                hdr = c_arrayHeader(object);
#ifdef OSPL_STRICT_MEM
                {
                    c_long size;
                    size = c_arraySize(object);
                    memset(hdr,0xff,ARRAYMEMSIZE(size));
                }
#endif
                if (safeCount & REFCOUNT_FLAG_TRACE) {
                    c_mmTrackObject (base->mm, hdr, C_MMTRACKOBJECT_CODE_MIN + 3);
                }
                c_mmFree(base->mm, hdr);
            } else {
#ifdef OSPL_STRICT_MEM
                {
                    /* Only when OSPL_STRICT_MEM has been set so that we can abort on the detection of illegal usage
                     * of a deleted mutex or conditon, without blocking the calling thread indefinitely
                     */
                    c_long size;
                    size = c_typeSize(type);
                    memset(header,0xff,MEMSIZE(size));
                }
#endif
                if (safeCount & REFCOUNT_FLAG_TRACE) {
                    c_mmTrackObject (base->mm, header, C_MMTRACKOBJECT_CODE_MIN + 3);
                }
                c_mmFree(base->mm, header);
            }
            /* Do not use type, as it refers to an actual type, while
             * we incremented the header->type.
             */
            if (base->maintainObjectCount) {
                /* Since no special actions need to be performed on going to 0, the
                 * return-value of the decrement isn't needed.
                 */
                (void) pa_dec32_nv(&headerType->objectCount);
            }
#if TYPE_REFC_COUNTS_OBJECTS
            c_free(headerType); /* free the header->type */
#endif
        }

#if CHECK_REF
        if (matchesRefRequest) {
            UT_TRACE("\n\n============ Free(%p): %d -> %d =============\n",
                     object, (safeCount & REFCOUNT_MASK)+1, safeCount & REFCOUNT_MASK);
        }
#endif
        object = c_freelistTake(&freelist);
    }
}

c_object
c_keep (
    c_object object)
{
    c_header header;
    os_uint32 oldCount;

    if (object == NULL) {
        return NULL;
    }

    header = c_header(object);
    assert(header->confidence == CONFIDENCE);
#if CHECK_REF
    if (header->type) {
        c_type type;
        ACTUALTYPE(type,header->type);
        if (type && c_metaObject(type)->name) {
            if (strlen(c_metaObject(type)->name) >= CHECK_REF_TYPE_LEN) {
                if (strncmp(c_metaObject(type)->name, CHECK_REF_TYPE, CHECK_REF_TYPE_LEN) == 0) {
                    os_uint32 refc = pa_ld32(&header->refCount) & REFCOUNT_MASK;
                    UT_TRACE("\n\n============ Keep(%p): %d -> %d =============\n", object, refc, refc+1);
                }
            }
        }
    }
#endif
    oldCount = pa_ld32(&header->refCount);
    assert((oldCount & REFCOUNT_MASK) > 0);
    pa_inc32(&header->refCount); /* FIXME: should add pa_inc32_ov and use it here */
    if (oldCount & REFCOUNT_FLAG_CLAMP) {
        /* Protect against wrap-around of reference count by forcing
         * the refcount up once half the range (24 bits) has been used
         * up. The memory will then never be freed, but a memory leak
         * is still better than a crash. This resets the flags, which
         * is fine for the current set of flags
         */
        pa_st32 (&header->refCount, (REFCOUNT_FLAG_CLAMP | (REFCOUNT_FLAG_CLAMP >> 1)));
    }
    if (oldCount & REFCOUNT_FLAG_TRACE) {
        c_type headerType = header->type, type;
        void *block;
        ACTUALTYPE (type, headerType);
        if ((c_baseObjectKind (type) == M_COLLECTION) && ((c_collectionTypeKind (type) == OSPL_C_ARRAY) || (c_collectionTypeKind (type) == OSPL_C_SEQUENCE))) {
            block = c_arrayHeader (object);
        } else {
            block = header;
        }
        c_mmTrackObject (type->base->mm, block, C_MMTRACKOBJECT_CODE_MIN + 0);
    }

    return object;
}

void
c_baseTraceObject (
    c_object object)
{
    c_header header;
    if (object != NULL) {
        header = c_header(object);
        assert(header->confidence == CONFIDENCE);
        pa_or32(&header->refCount, REFCOUNT_FLAG_TRACE);
    }
}

void
c_baseTraceObjectsOfType (
    c_type type)
{
    c_header header;
    assert(c_instanceOf(type, "c_type"));
    header = c_header(type);
    assert(header->confidence == CONFIDENCE);
    pa_or32(&header->refCount, REFCOUNT_FLAG_TRACETYPE);
}

c_mm
c_baseMM(
    c_base base)
{
    if (base == NULL) {
        return NULL;
    }
    return base->mm;
}

c_type
c_getType(
    c_object object)
{
    c_type type;
    if (object == NULL) {
        return NULL;
    }
    type = c_header(object)->type;
    return type;
}

c_base
c_getBase(
    c_object object)
{
    if (object == NULL) {
        return NULL;
    }
    return c_header(object)->type->base;
}

c_long
c_refCount (
    c_object object)
{
    if (object == NULL) {
        return 0;
    }
    return (c_long) (pa_ld32(&c_header(object)->refCount) & REFCOUNT_MASK);
}

void
c_baseSerLock(
    c_base base)
{
    c_mutexLock(&base->serLock);
}

void
c_baseSerUnlock(
    c_base base)
{
    c_mutexUnlock(&base->serLock);
}

c_ulong
c_arraySize(
    c_array _this)
{
    if (_this) {
        assert(c_header(_this)->confidence == CONFIDENCE);
        assert(
            /* For backward compatibility purposes this function can still be used
             * to obtain the length of sequences as well, but new code should use
             * the c_sequenceSize operation instead.
             */
            (c_baseObjectKind(c_header(_this)->type) == M_COLLECTION)
            && (
                    (c_collectionTypeKind(c_header(_this)->type) == OSPL_C_ARRAY)
                    || (c_collectionTypeKind(c_header(_this)->type) == OSPL_C_SEQUENCE)
               )
            );
        return (c_ulong)(c_arrayHeader(_this)->size);
    } else {
        return 0;
    }
}

c_ulong
c_sequenceSize(
    c_sequence _this)
{
    if (_this) {
        assert(c_header(_this)->confidence == CONFIDENCE);
        assert(c_baseObjectKind(c_header(_this)->type) == M_COLLECTION &&
                c_collectionTypeKind(c_header(_this)->type) == OSPL_C_SEQUENCE);
        return (c_ulong)(c_arrayHeader(_this)->size);
    } else {
        return 0;
    }
}

c_object
c_baseCheckPtr(
    c_base _this,
    void *ptr)
{
    c_object o;
    c_type type;
    c_arrayHeader hdr;
    c_header header;
    c_voidp addr;
    c_mm mm;

    o = NULL;
    if (_this != NULL) {
        mm = c_baseMM(_this);
        if (mm != NULL) {
            /* First assume that the address refers to an array object.
             * Note that not the array headre size is substracted but
             * the header size instead. This is not correct but is
             * compensated by shifting the address at the end of the loop.
             * Fixing this issue requires a partial redesign of this
             * operation.
             */
            hdr = c_mmCheckPtr(c_baseMM(_this), c_header(ptr));
            while ((hdr != NULL) && (o == NULL)) {
                /* Check if header->type refers to valid type. */
                header = &hdr->_parent;
                addr = c_mmCheckPtr(c_baseMM(_this), c_header(header->type));
                if (addr != NULL) {
                    type = c_oid(addr);
                    if ((type == NULL) ||
                        (type->base != _this) ||
                        (!c_objectIsType(c_baseObject(type))))
                    {
                        /* Invalid array hdr, so try if it is a normal header. */
                        header = (c_header)hdr;
                        addr = c_mmCheckPtr(c_baseMM(_this), c_header(header->type));
                        if (addr != NULL) {
                            type = c_oid(addr);
                            if ((type != NULL) &&
                                (type->base == _this) &&
                                (c_objectIsType(c_baseObject(type))))
                            {
                                /* We have a valid address! */
                                o = c_oid(header);
                            }
                        }
                    } else {
                        /* We have a valid address! */
                        o = c_oid(header);
                    }
                } else {
                    /* Invalid array hdr, so try if it is a normal header. */
                    header = (c_header)hdr;
                    addr = c_mmCheckPtr(c_baseMM(_this), c_header(header->type));
                    if (addr != NULL) {
                        type = c_oid(addr);
                        if ((type != NULL) &&
                            (type->base == _this) &&
                            (c_objectIsType(c_baseObject(type))))
                        {
                            /* We have a valid address! */
                            o = c_oid(header);
                        }
                    }
                }
                hdr = (c_arrayHeader)(C_ADDRESS(hdr) - 4);
            }
        } else {
            OS_REPORT(OS_ERROR,"c_baseCheckPtr", 0,
                        "Could not resolve Memory Manager for Database (0x%"PA_PRIxADDR")",
                        (os_address)_this);
        }
    } else {
        OS_REPORT(OS_ERROR,"c_baseCheckPtr", 0,
                  "Bad Parameter: Database = NULL");
    }
    return o;
}

#ifndef NDEBUG
#ifdef OBJECT_WALK
void
c_baseObjectWalk(
    c_base base,
    c_baseWalkAction action,
    c_baseWalkActionArg arg)
{
    c_object object;

    assert(base != NULL);
    assert(action != NULL);

    object = base->firstObject;
    while (object != NULL) {
        action(object, arg);
        object = c_header(object)->nextObject;
    }
}
#endif
#endif
#undef ResolveType

/*******
 * Following operations involve garbage collection at database destruction to avoid leakage.
 * The Database itself has some reference counting flaws that causes some memory leakage.
 * The following deleteGarbage function is therefore created to deal with this leakage at
 * database destruction. The deleteGarbage function is called by the c_destruct operation.
 * This function will walk over all remaining objects and collect one reference to each object
 * in a trashcan and afterwards free the allocated memory of each object referenced by the trashcan.
 *******/

#define OBJECTTYPE(t,o) ACTUALTYPE(t,c_header(o)->type)
#define CHECKOBJECT(o) assert(o == NULL || c_header(o)->confidence == CONFIDENCE)

C_CLASS(c_trashcan);
C_STRUCT(c_trashcan) {
    c_iter trash;
    c_iter arrays;
    c_iter scopes;
    c_iter collections;
    c_iter references;
};

static void walkReferences(c_metaObject metaObject, c_object object, c_trashcan trashcan);

static void
collectScopeGarbage(
c_metaObject o,
c_voidp trashcan)
{
    CHECKOBJECT(o);
    ((c_trashcan)trashcan)->references = c_iterAppend(((c_trashcan)trashcan)->references, c_keep(o));
}

static c_bool
collectGarbage(
c_object o,
c_voidp trashcan)
{
    CHECKOBJECT(o);
    ((c_trashcan)trashcan)->references = c_iterAppend(((c_trashcan)trashcan)->references, c_keep(o));
    return TRUE;
}

static void
collectReferenceGarbage(
    c_voidp *p,
    c_type type,
    c_trashcan trashcan)
{
    c_type t = type;

    if (p == NULL || t == NULL) return;

    while (c_baseObject(t)->kind == M_TYPEDEF) {
        t = c_typeDef(t)->alias;
    }
    switch (c_baseObject(t)->kind) {
    case M_CLASS:
    case M_INTERFACE:
    case M_ANNOTATION:
        CHECKOBJECT(*p);
        trashcan->references = c_iterAppend(trashcan->references, c_object(*p));
    break;
    case M_BASE:
    case M_COLLECTION:
        if ((c_collectionTypeKind(t) == OSPL_C_ARRAY) &&
            (c_collectionTypeMaxSize(t) != 0))
        {
            walkReferences((c_metaObject)t, p, trashcan);
        } else {
            CHECKOBJECT(*p);
            trashcan->references = c_iterAppend(trashcan->references, c_object(*p));
        }
    break;
    case M_EXCEPTION:
    case M_STRUCTURE:
    case M_UNION:
        walkReferences(c_metaObject(type),p, trashcan);
    break;
    case M_PRIMITIVE:
    break;
    default:
    break;
    }
}

static void
walkReferences(
    c_metaObject metaObject,
    c_object o,
    c_trashcan trashcan)
{
    c_type type;
    c_class cls;
    c_array references, labels, ar;
    c_property property;
    c_member member;
    c_ulong i,j,length;
    c_size size;
    c_ulong nrOfRefs,nrOfLabs;
    c_value v;
    c_bool fixType = FALSE;

    if (metaObject == NULL || o == NULL) return;

    switch (c_baseObjectKind(metaObject)) {
    case M_CLASS:
        cls = c_class(metaObject);
        while (cls) {
            length = c_arraySize(c_interface(cls)->references);
            for (i=0;i<length;i++) {
                property = c_property(c_interface(cls)->references[i]);
                type = property->type;
                collectReferenceGarbage(C_DISPLACE(o,property->offset),type, trashcan);
            }
            cls = cls->extends;
        }
    break;
    case M_INTERFACE:
    case M_ANNOTATION:
        length = c_arraySize(c_interface(metaObject)->references);
        for (i=0;i<length;i++) {
            property = c_property(c_interface(metaObject)->references[i]);
            type = property->type;
            collectReferenceGarbage(C_DISPLACE(o,property->offset),type, trashcan);
        }
    break;
    case M_EXCEPTION:
    case M_STRUCTURE:
        length = c_arraySize(c_structure(metaObject)->references);
        for (i=0;i<length;i++) {
            member = c_member(c_structure(metaObject)->references[i]);
            type = c_specifier(member)->type;
            collectReferenceGarbage(C_DISPLACE(o,member->offset),type, trashcan);
        }
    break;
    case M_UNION:
#define _CASE_(k,t) case k: v = t##Value(*((t *)o)); break
        switch (c_metaValueKind(c_metaObject(c_union(metaObject)->switchType))) {
        _CASE_(V_BOOLEAN,   c_bool);
        _CASE_(V_OCTET,     c_octet);
        _CASE_(V_SHORT,     c_short);
        _CASE_(V_LONG,      c_long);
        _CASE_(V_LONGLONG,  c_longlong);
        _CASE_(V_USHORT,    c_ushort);
        _CASE_(V_ULONG,     c_ulong);
        _CASE_(V_ULONGLONG, c_ulonglong);
        _CASE_(V_CHAR,      c_char);
        _CASE_(V_WCHAR,     c_wchar);
        default:
            return;
        }
#undef _CASE_
        references = c_union(metaObject)->references;
        if (references != NULL) {
            i=0; type=NULL;
            nrOfRefs = c_arraySize(references);
            while ((i<nrOfRefs) && (type == NULL)) {
                labels = c_unionCase(references[i])->labels;
                j=0;
                nrOfLabs = c_arraySize(labels);
                while ((j<nrOfLabs) && (type == NULL)) {
                    if (c_valueCompare(v,c_literal(labels[j])->value) == C_EQ) {
                        walkReferences(c_metaObject(references[i]),
                                       C_DISPLACE(o,c_type(metaObject)->alignment), trashcan);
                        type = c_specifier(references[i])->type;
                    }
                    j++;
                }
                i++;
            }
        }
    break;
    case M_COLLECTION:
        switch (c_collectionTypeKind(metaObject)) {
        case OSPL_C_ARRAY:
        case OSPL_C_SEQUENCE:
            ACTUALTYPE(type,((c_collectionType)metaObject)->subType);
            /* Some internal database arrays have an incorrect subtype.
             * This should be fixed but for now detect and correct during
             * garbage collection.
             */
            if (type == c_getMetaType(c_getBase(type),M_BASE)) {
                fixType = TRUE;
            }
            ar = (c_array)o;

            if (c_collectionTypeKind(metaObject) == OSPL_C_ARRAY &&
                c_collectionTypeMaxSize(metaObject) > 0)
            {
                length = ((c_collectionType)metaObject)->maxSize;
            } else {
                length = c_arraySize(ar);
            }

            if (c_typeIsRef(type)) {
                for (i=0;i<length;i++) {
                    if (fixType) {
                        OBJECTTYPE(type, ar[i]);
                    }
                    /* Need to check type again as it might have been fixed */
                    if (c_typeIsRef(type)) {
                        CHECKOBJECT(ar[i]);
                        trashcan->references = c_iterAppend(trashcan->references, ar[i]);
                    } else {
                        walkReferences(c_metaObject(type),ar[i],trashcan);
                    }
                }
            } else {
                if (c_typeHasRef(type)) {
                    size = type->size;
                    for (i=0;i<length;i++) {
                        walkReferences(c_metaObject(type),C_DISPLACE(ar,(i*size)),trashcan);
                    }
                }
            }
        break;
        case OSPL_C_SCOPE:
            c_scopeWalk(c_scope(o), collectScopeGarbage, trashcan);
        break;
        case OSPL_C_STRING:
        break;
        default:
            (void)c_walk(o, collectGarbage, trashcan);
        break;
        }
    break;
    case M_BASE:
    break;
    case M_TYPEDEF:
        walkReferences(c_metaObject(c_typeDef(metaObject)->alias),o, trashcan);
    break;
    case M_ATTRIBUTE:
    case M_RELATION:
        ACTUALTYPE(type,c_property(metaObject)->type);
        collectReferenceGarbage(C_DISPLACE(o,c_property(metaObject)->offset),type, trashcan);
    break;
    case M_MEMBER:
        ACTUALTYPE(type,c_specifier(metaObject)->type);
        collectReferenceGarbage(C_DISPLACE(o,c_member(metaObject)->offset),type, trashcan);
    break;
    case M_UNIONCASE:
        ACTUALTYPE(type,c_specifier(metaObject)->type);
        collectReferenceGarbage(o,type, trashcan);
    break;
    case M_MODULE:
        CHECKOBJECT(c_module(o)->scope);
        trashcan->references = c_iterAppend(trashcan->references, c_module(o)->scope);
    break;
    case M_PRIMITIVE:
        /* Do nothing */
    break;
    default:
    break;
    }
}

typedef struct bindArg {
    c_mm mm;
    c_trashcan trashcan;
} *bindArgp;

static void freeBindings (void *binding, void *arg)
{
    c_baseBinding b = (c_baseBinding)binding;
    bindArgp a = (bindArgp)arg;
    c_header header;
    c_type type;
    c_object o;

    a->trashcan->references = c_iterInsert(a->trashcan->references, b->object);
    while ((o = c_iterTakeFirst(a->trashcan->references)) != NULL) {
        header = c_header(o);
        /* skip already freed or corrupted */
        assert(header->confidence == CONFIDENCE);
        if ((pa_ld32(&header->refCount) & REFCOUNT_FLAG_GARBAGE) == 0) {
            pa_or32(&header->refCount, REFCOUNT_FLAG_GARBAGE);

            OBJECTTYPE(type,o);
            walkReferences(c_metaObject(type),o,a->trashcan);
            if (c_baseObjectKind(type) == M_COLLECTION) {
                switch (c_collectionTypeKind(type)) {
                case OSPL_C_ARRAY:
                case OSPL_C_SEQUENCE:
                    a->trashcan->arrays = c_iterInsert(a->trashcan->arrays, o);
                break;
                case OSPL_C_SCOPE:
                    a->trashcan->scopes = c_iterInsert(a->trashcan->scopes, o);
                break;
                case OSPL_C_STRING:
                    a->trashcan->trash = c_iterInsert(a->trashcan->trash, o);
                break;
                default:
                    a->trashcan->collections = c_iterInsert(a->trashcan->collections, o);
                }
            } else {
                a->trashcan->trash = c_iterInsert(a->trashcan->trash, o);
            }
        }
    }
    c_free(b->name);
    c_mmFree(a->mm, b);
}

static void
deleteGarbage(
    c_base base)
{
    C_STRUCT(c_trashcan) trashcan;
    c_mm mm;
    c_object trash;
    struct bindArg barg;

    if (base == NULL) return;

    assert(base->confidence == CONFIDENCE);

    mm = base->mm;

    trashcan.trash = NULL;
    trashcan.arrays = NULL;
    trashcan.scopes = NULL;
    trashcan.collections = NULL;
    trashcan.references = NULL;

    barg.trashcan = &trashcan;
    barg.mm = mm;

    ut_avlFreeArg (&c_base_bindings_td, &base->bindings, freeBindings, &barg);
    OS_REPORT_NOW(OS_INFO,"Database close",0,-1,"Removed %d objects",c_iterLength(trashcan.trash));

    while ((trash = c_iterTakeFirst(trashcan.scopes)) != NULL)
    {
        c_scopeClean(c_scope(trash));
        c_mmFree(mm, c_header(trash));
    }
    while ((trash = c_iterTakeFirst(trashcan.collections)) != NULL)
    {
        c_clear(trash);
        c_mmFree(mm, c_header(trash));
    }
    while ((trash = c_iterTakeFirst(trashcan.arrays)) != NULL)
    {
        c_mmFree(mm, c_arrayHeader(trash));
    }
    while ((trash = c_iterTakeFirst(trashcan.trash)) != NULL)
    {
        c_mmFree(mm, c_header(trash));
    }
    c_iterFree(trashcan.trash);
    c_iterFree(trashcan.arrays);
    c_iterFree(trashcan.scopes);
    c_iterFree(trashcan.collections);
    c_iterFree(trashcan.references);
}
