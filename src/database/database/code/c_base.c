/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "os_stdlib.h"
#include "os_report.h"
#include "os_abstract.h"
#include "c_avltree.h"
#include "c_mmbase.h"
#include "c__scope.h"
#include "c__extent.h"
#include "c_sync.h"
#include "c__base.h"
#include "c_metafactory.h"
#include "c_querybase.h"
#include "c__querybase.h"
#include "c__collection.h"
#include "c__field.h"
#include "c__metabase.h"
#include "q__parser.h"
#include "c_module.h"

/* If set the meta descriptions c_union, c_interface and c_structure will
 * have an additional blob attribute.
 */

#define CONFIDENCE (0x504F5448)

#define c_oid(o)    ((c_object)(C_ADDRESS(o) + HEADERSIZE))
#define c_header(o) ((c_header)(C_ADDRESS(o) - HEADERSIZE))
#define c_arrayHeader(o) ((c_arrayHeader)(C_ADDRESS(o) - ARRAYHEADERSIZE))

#define MIN_DB_SIZE     (150000)
#define MAXREFCOUNT     (50000)
#define MAXALIGNMENT    (C_ALIGNMENT(c_value))
#define ALIGNSIZE(size) ((((size-1)/MAXALIGNMENT)+1)*MAXALIGNMENT)
#define MEMSIZE(size)   ((size)+HEADERSIZE)
#define ARRAYMEMSIZE(size)   ((size)+ARRAYHEADERSIZE)

#define ResolveType(s,t) c_type(c_metaResolve(c_metaObject(s),#t))

#define CHECK_REF (0)

#if CHECK_REF
#define CHECK_REF_TYPE "v_indexKeyInstance<v_indexSample<PP_min_topic>,block>"
#define CHECK_REF_TYPE_LEN (strlen(CHECK_REF_TYPE))
#define CHECK_REF_DEPTH (64)
static char* CHECK_REF_FILE = NULL;

#define UT_TRACE(msgFormat, ...) do { \
    void *tr[CHECK_REF_DEPTH];\
    char **strs;\
    size_t s,i; \
    FILE* stream; \
    \
    if(!CHECK_REF_FILE){ \
        CHECK_REF_FILE = os_malloc(24); \
        os_sprintf(CHECK_REF_FILE, "mem.log"); \
    } \
    s = backtrace(tr, CHECK_REF_DEPTH);\
    strs = backtrace_symbols(tr, s);\
    stream = fopen(CHECK_REF_FILE, "a");\
    fprintf(stream, msgFormat, __VA_ARGS__);              \
    for (i=0;i<s;i++) fprintf(stream, "%s\n", strs[i]);\
    free(strs);\
    fflush(stream);\
    fclose(stream);\
  } while (0)
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
    c_ulong refCount;
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

static const c_long HEADERSIZE = ALIGNSIZE(C_SIZEOF(c_header));
static const c_long ARRAYHEADERSIZE = ALIGNSIZE(C_SIZEOF(c_arrayHeader));




C_STRUCT(c_baseBinding) {
    C_EXTENDS(c_avlNode);
    c_object object;
    c_char  *name;
};

c_string
c_stringMalloc(
    c_base base,
    c_long length)
{
    c_header header;
    c_string s = NULL;

    if (base == NULL) {
        return NULL;
    }

    header = (c_header)c_mmMalloc(c_baseMM(base), MEMSIZE(length));
    if (header) {
        header->type = c_keep(base->string_type);
        pa_increment(&base->string_type->objectCount);
        header->refCount = 1;
        s = (c_string)c_oid(header);
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
c_stringNew(
    c_base base,
    const c_char *str)
{
    c_string s;
    if (base == NULL || str == NULL) {
        return NULL;
    }

    s = c_stringMalloc(base, strlen(str) + 1);

    os_strcpy(s,str);
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
 * c_array_t(c_base _this)
 * c_type_t(c_base _this)
 * c_valueKind_t(c_base _this)
 *
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
_TYPE_CACHE_(c_array)
_TYPE_CACHE_(c_type)
_TYPE_CACHE_(c_valueKind)

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
    c_interface(o)->scope = (c_scope)c_scopeNew(c__getBase(o));
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

static c_type
c_newMetaClass(
    c_base base,
    const c_char *name,
    c_long size,
    c_long alignment)
{
    c_metaObject o,found;

    assert(base != NULL);
    assert(name != NULL);
    assert(base->metaType[M_CLASS] != NULL);

    o = c_metaDefine(c_metaObject(base),M_CLASS);
    c_metaTypeInit(o,size,alignment);
    found = c_metaBind(c_metaObject(base),name,o);
    if (found != o) {
        OS_REPORT_1(OS_ERROR,"c_newMetaClass failed",0,"%s already defined",name);
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
    typeCache->c_array_t = NULL;
    typeCache->c_type_t = NULL;
    typeCache->c_valueKind_t = NULL;
    typeCache->c_member_t = NULL;
    typeCache->c_literal_t = NULL;
    typeCache->c_constant_t = NULL;
    typeCache->c_unionCase_t = NULL;
    typeCache->c_property_t = NULL;
}

static void
c_baseInit (
    c_base base)
{
    c_type type,scopeType;
    c_header header;
    c_metaObject o,found;
    c_array members;
    c_iter labels;
    c_long size;
    c_ulong caseNumber;

    c_baseObject(base)->kind = M_MODULE;
    c_metaObject(base)->definedIn = NULL;

/** Declare class type.
    this is required because all meta meta objects are defined as class. **/
    size = MEMSIZE(C_SIZEOF(c_class));
    header = (c_header)c_mmMalloc(base->mm,size);
    if (!header) {
    return;
    }
    memset(header,0,size);
    header->refCount = 1;
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
    c_type(o)->alignment = C_ALIGNMENT(C_STRUCT(c_class));
    c_type(o)->size = C_SIZEOF(c_class);
    base->metaType[M_CLASS] = c_type(o);
    header->type = c_keep(o);

    o = c_metaObject(c_new(base->metaType[M_CLASS]));
    c_baseObject(o)->kind = M_CLASS;
    C_META_TYPEINIT_(o,c_collectionType);
    base->metaType[M_COLLECTION] = c_type(o);

/** Declare scope class type.
    this is required because all meta meta objects are managed in the base scope. **/

    scopeType = c_type(c_new(base->metaType[M_COLLECTION]));
    c_baseObject(scopeType)->kind = M_COLLECTION;
    c_collectionType(scopeType)->kind = C_SCOPE;
    c_collectionType(scopeType)->maxSize = 0;
    C_META_TYPEINIT_(scopeType,c_scope);

/** Declare base class type and initialize the base scope.
    this is required because all meta meta objects are bound to the base scope. **/

    o = c_metaObject(c_new(base->metaType[M_CLASS]));
    c_baseObject(o)->kind = M_CLASS;
    C_META_TYPEINIT_(o,c_base);
    base->metaType[M_BASE] = c_type(o);

    /* Overwrite as header->type points to type of metatype class
     * i.o. NULL
     */
    c_header(base)->type = c_keep(o);
    c_module(base)->scope = c_scope(c_new(scopeType));
    c_mutexInit(&c_module(base)->mtx, SHARED_MUTEX);
    c_scopeInit(c_module(base)->scope);

    c_queryCacheInit (&base->baseCache.queryCache);
    c_fieldCacheInit (&base->baseCache.fieldCache);
    c_typeCacheInit (&base->baseCache.typeCache);

/** Declare c_string type, this is required to be able to bind objects to names. **/
    o = c_metaObject(c_new(base->metaType[M_COLLECTION]));
    c_baseObject(o)->kind = M_COLLECTION;
    c_collectionType(o)->kind = C_STRING;
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

/** Now allocate, bind and pre-initialize all meta meta objects.
    pre-initialize will only set size and kind making the meta meta objects
    ready to be used for meta creation.
    At this point reflection will be unavailable until the meta meta objects
    are fully initialized. **/

#define _META_(b,t) c_object(c_newMetaClass(b,((const c_char *)#t),C_SIZEOF(t),C_ALIGNMENT(C_STRUCT(t))))

    /** Declare abstract meta types **/
    found = _META_(base,c_baseObject); c_free(found);
    found = _META_(base,c_operand); c_free(found);
    found = _META_(base,c_specifier); c_free(found);
    found = _META_(base,c_metaObject); c_free(found);
    found = _META_(base,c_property); c_free(found);
    found = _META_(base,c_type); c_free(found);

    /** At last set the subType of c_scope type. **/
    c_collectionType(scopeType)->subType = ResolveType(base,c_metaObject);
    c_free(scopeType); /* we can now free our local ref as we don't use it anymore */
    scopeType = NULL;

    /** Declare meta types **/
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
    base->metaType[M_EXTENT] =       _META_(base,c_extent);
    base->metaType[M_EXTENTSYNC] =   _META_(base,c_extentSync);

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
    c_free(base->metaType[M_EXTENT]);
    c_free(base->metaType[M_EXTENTSYNC]);

#undef _META_

/** Now allocation of meta objects is operational.
    For initialization of the meta meta object we need to allocate the meta objects
    for all internal types. **/

    /** Definition of the meta objects specifying all internal primitive types. **/

#define INITPRIM(s,n,k) \
    o = c_metaDeclare(c_metaObject(s),#n,M_PRIMITIVE); \
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

#undef INITPRIM

    o = c_metaDeclare(c_metaObject(base),"c_object",M_CLASS);
    c_metaFinalize(o);
    c_free(o);
    o = c_metaDeclare(c_metaObject(base),"c_any",M_CLASS);
    c_metaFinalize(o);
    c_free(o);

    c_collectionInit(base);

#define _ENUMVAL_(e,v) \
    c_enumeration(e)->elements[v] = \
    c_constantNew(c_metaObject(e),#v,c_longValue(v))

    o = c_metaDeclare(c_metaObject(base),"c_metaKind",M_ENUMERATION);
    type = c_constant_t(base);
    c_enumeration(o)->elements = c_arrayNew(type,M_COUNT);
    c_free(type);
    _ENUMVAL_(o,M_UNDEFINED);
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
    _ENUMVAL_(o,M_EXTENT);
    _ENUMVAL_(o,M_EXTENTSYNC);
    c_metaFinalize(o);
    c_free(o);

    o = c_metaDeclare(c_metaObject(base),"c_collKind",M_ENUMERATION);
    type = c_constant_t(base);
    c_enumeration(o)->elements = c_arrayNew(type,C_COUNT);
    c_free(type);
    _ENUMVAL_(o,C_UNDEFINED);
    _ENUMVAL_(o,C_LIST);
    _ENUMVAL_(o,C_ARRAY);
    _ENUMVAL_(o,C_BAG);
    _ENUMVAL_(o,C_SET);
    _ENUMVAL_(o,C_MAP);
    _ENUMVAL_(o,C_DICTIONARY);
    _ENUMVAL_(o,C_SEQUENCE);
    _ENUMVAL_(o,C_STRING);
    _ENUMVAL_(o,C_WSTRING);
    _ENUMVAL_(o,C_QUERY);
    _ENUMVAL_(o,C_SCOPE);
    c_metaFinalize(o);
    c_free(o);

    o = c_metaDeclare(c_metaObject(base),"c_primKind",M_ENUMERATION);
    type = c_constant_t(base);
    c_enumeration(o)->elements = c_arrayNew(type,P_COUNT);
    c_free(type);
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
    c_metaFinalize(o);
    c_free(o);

    o = c_metaDeclare(c_metaObject(base),"c_exprKind",M_ENUMERATION);
    type = c_constant_t(base);
    c_enumeration(o)->elements = c_arrayNew(type,E_COUNT);
    c_free(type);
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
    c_free(o);

    o = c_metaDeclare(c_metaObject(base),"c_direction",M_ENUMERATION);
    type = c_constant_t(base);
    c_enumeration(o)->elements = c_arrayNew(type,D_COUNT);
    c_free(type);
    _ENUMVAL_(o,D_UNDEFINED);
    _ENUMVAL_(o,D_IN);
    _ENUMVAL_(o,D_OUT);
    _ENUMVAL_(o,D_INOUT);
    c_metaFinalize(o);
    c_free(o);

    o = c_metaDeclare(c_metaObject(base),"c_valueKind",M_ENUMERATION);
    type = c_constant_t(base);
    c_enumeration(o)->elements = c_arrayNew(type,V_COUNT);
    c_free(type);
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
    c_free(o);

#undef _ENUMVAL_

    o = c_metaDeclare(c_metaObject(base),"c_threadId",M_STRUCTURE);
    c_metaTypeInit(o,sizeof(c_threadId),C_ALIGNMENT(c_threadId));
    c_free(o);

    o = c_metaDefine(c_metaObject(base),M_STRUCTURE);
    type = c_member_t(base);
    members = c_arrayNew(type,2);
    c_free(type);
    members[0] = (c_voidp)c_metaDefine(c_metaObject(base),M_MEMBER);
        c_specifier(members[0])->name = c_stringNew(base,"seconds");
        c_specifier(members[0])->type = c_long_t(base);
    members[1] = (c_voidp)c_metaDefine(c_metaObject(base),M_MEMBER);
        c_specifier(members[1])->name = c_stringNew(base,"nanoseconds");
        c_specifier(members[1])->type = c_ulong_t(base);
    c_structure(o)->members = members;
    c_metaObject(o)->definedIn = c_metaObject(base);
    c_metaFinalize(o);
    found = c_metaBind(c_metaObject(base),"c_time",o);
    c_free(found);
    c_free(o);

    o = c_metaDeclare(c_metaObject(base),"c_value",M_UNION);
    c_metaTypeInit(o,sizeof(struct c_value),C_ALIGNMENT(struct c_value));
    c_free(o);

/** Now all meta meta references required for initialization are available.
    The following statements will initialize all meta meta objects.
    After initialization reflection will be operational. **/

    /** Initialize abstract meta types **/

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
        type = c_string_t(base);
        C_META_ATTRIBUTE_(c_specifier,o,name,type);
        c_free(type);
        type = ResolveType(base,c_type);
        C_META_ATTRIBUTE_(c_specifier,o,type,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_metaObject,c_baseObject);
        type = c_string_t(base);
        C_META_ATTRIBUTE_(c_metaObject,o,name,type);
        c_free(type);
        type = c_voidp_t(base);
        C_META_ATTRIBUTE_(c_metaObject,o,definedIn,type);
        c_free(type);
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
        type = c_voidp_t(base);
        C_META_ATTRIBUTE_(c_type,o,base,type);
        c_free(type);
        type = ResolveType(base,c_address);
        C_META_ATTRIBUTE_(c_type,o,alignment,type);
        c_free(type);
        type = c_long_t(base);
        C_META_ATTRIBUTE_(c_type,o,objectCount,type);
        c_free(type);
        type = ResolveType(base,c_address);
        C_META_ATTRIBUTE_(c_type,o,size,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    /** Initialize meta types **/
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
            c_collectionTypeKind(type) = C_ARRAY;
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
            c_collectionTypeKind(type) = C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_literal);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_unionCase,o,labels,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_attribute,c_property);
        type = c_bool_t(base);
        C_META_ATTRIBUTE_(c_attribute,o,isReadOnly,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_relation,c_property);
        type = c_string_t(base);
        C_META_ATTRIBUTE_(c_relation,o,inverse,type);
        c_free(type);
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
            c_collectionTypeKind(type) = C_ARRAY;
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

    o = _INITCLASS_(base,c_extent,c_typeDef);
        type = c_voidp_t(base);
        C_META_ATTRIBUTE_(c_extent,o,cache,type);
        c_free(type);
        type = c_bool_t(base);
        C_META_ATTRIBUTE_(c_extent,o,sync,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_extentSync,c_extent);
        type = ResolveType(base,c_mutex);
        C_META_ATTRIBUTE_(c_extentSync,o,mutex,type);
        c_free(type);
        C_META_FINALIZE_(o);
        c_free(o);

    o = _INITCLASS_(base,c_collectionType,c_type);
        type = ResolveType(base,c_collKind);
        C_META_ATTRIBUTE_(c_collectionType,o,kind,type);
        c_free(type);
        type = c_long_t(base);
        C_META_ATTRIBUTE_(c_collectionType,o,maxSize,type);
        c_free(type);
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
            c_collectionTypeKind(type) = C_ARRAY;
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
            c_collectionTypeKind(type) = C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_unionCase);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_union,o,cases,type);
        c_free(type);
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_unionCase>");
            c_collectionTypeKind(type) = C_ARRAY;
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
            c_collectionTypeKind(type) = C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_member);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_structure,o,members,type);
        c_free(type);
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_member>");
            c_collectionTypeKind(type) = C_ARRAY;
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
        type = c_bool_t(base);
        C_META_ATTRIBUTE_(c_interface,o,abstract,type);
        c_free(type);
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_interface>");
            c_collectionTypeKind(type) = C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_interface);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_interface,o,inherits,type);
        c_free(type);
        type = c_type(c_metaDefine(c_metaObject(base),M_COLLECTION));
            c_metaObject(type)->name = c_stringNew(base,"ARRAY<c_property>");
            c_collectionTypeKind(type) = C_ARRAY;
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
            c_collectionTypeKind(type) = C_ARRAY;
            c_collectionTypeMaxSize(type) = 0;
            c_collectionTypeSubType(type) = ResolveType(base,c_string);
            c_metaFinalize(c_metaObject(type));
        C_META_ATTRIBUTE_(c_class,o,keys,type);
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
        c_union(o)->scope = NULL;

        caseNumber = 0;
        /* c_unionCaseNew transfers refCount of type and
         * c_enumValue(_SWITCH_TYPE_,"V_BOOLEAN")
         */
        type = c_address_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_ADDRESS"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "Address",type,labels);
        c_iterFree(labels);

        type = c_bool_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_BOOLEAN"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "Boolean",type,labels);
        c_iterFree(labels);

        type = c_octet_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_OCTET"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "Octet",type,labels);
        c_iterFree(labels);

        type = c_short_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_SHORT"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "Short",type,labels);
        c_iterFree(labels);

        type = c_long_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_LONG"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "Long",type,labels);
        c_iterFree(labels);

        type = c_longlong_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_LONGLONG"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "LongLong",type,labels);
        c_iterFree(labels);

        type = c_ushort_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_USHORT"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "UShort",type,labels);
        c_iterFree(labels);

        type = c_ulong_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_ULONG"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "ULong",type,labels);
        c_iterFree(labels);

        type = c_ulonglong_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_ULONGLONG"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "ULongLong",type,labels);
        c_iterFree(labels);

        type = c_float_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_FLOAT"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "Float",type,labels);
        c_iterFree(labels);

        type = c_double_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_DOUBLE"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                              "Double",type,labels);
        c_iterFree(labels);

        type = c_char_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_CHAR"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                               "Char",type,labels);
        c_iterFree(labels);

        type = c_string_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_STRING"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                               "String",type,labels);
        c_iterFree(labels);

        type = c_wchar_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_WCHAR"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                               "WChar",type,labels);
        c_iterFree(labels);

        type = c_wstring_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_WSTRING"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                               "WString",type,labels);
        c_iterFree(labels);

        type = c_string_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_FIXED"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                               "Fixed",type,labels);
        c_iterFree(labels);

        type = c_object_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_OBJECT"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                               "Object",type,labels);
        c_iterFree(labels);

        type = c_voidp_t(base);
        labels = c_iterNew(c_enumValue(_SWITCH_TYPE_,"V_VOIDP"));
        c_union(o)->cases[caseNumber++] = c_unionCaseNew(c_metaObject(base),
                                               "Voidp",type,labels);
        c_iterFree(labels);
        c_metaFinalize(o);
        c_free(o);

#undef _SWITCH_TYPE_

    c_fieldInit(base);
    c_querybaseInit(base);
}

static c_equality
c_compareBindName(c_baseBinding b1, c_baseBinding b2, const c_char *name);

c_base
c_create (
    const c_char *name,
    void *address,
    c_size size,
    c_size threshold)
{
    c_mm  mm;
    c_base base;
    c_base tempbase;
    c_header header;

    if ((size != 0) && (size < MIN_DB_SIZE)) {
        OS_REPORT_2(OS_ERROR,"c_base::c_create",0,
                    "Specified memory size (%d) is too small to occupy a c_base instance,"
                    "required minimum size is %d bytes.",
                    size,MIN_DB_SIZE);
        return NULL;
    }
    mm = c_mmCreate(address,size, threshold);

    header = (c_header)c_mmMalloc(mm, MEMSIZE(C_SIZEOF(c_base)));
    if (header) {
#ifndef NDEBUG
        header->confidence = CONFIDENCE;
#ifdef OBJECT_WALK
        header->nextObject = NULL;
        header->prevObject = NULL;
#endif
#endif
        header->refCount = 1;
        header->type = NULL;
        tempbase = (c_base)c_oid(header);
        base = (c_base)c_mmBind(mm, name, tempbase);
        if (base != tempbase) {
            const c_char *_name;
            if (name) {
                _name = name;
            } else {
                _name = "NULL";
            }
            OS_REPORT_4(OS_ERROR,
                        "c_base::c_create",0,
                        "Internal error, memory management seems corrupted.\n"
                        "             mm = 0x%x, name = %s,\n"
                        "             tempbase = 0x%x, base = 0x%x",
                        mm, name, tempbase, base);
            c_mmFree(mm, tempbase);
            return NULL;
        }
        base->mm = mm;
        base->confidence = CONFIDENCE;
        base->bindings = c_avlTreeNew(base->mm, 0);
#ifndef NDEBUG
#ifdef OBJECT_WALK
        base->firstObject = tempbase;
        base->lastObject = tempbase;
#endif
#endif
        c_mutexInit(&base->serLock,SHARED_MUTEX);
        c_mutexInit(&base->bindLock,SHARED_MUTEX);
        c_mutexInit(&base->extentBugLock,SHARED_MUTEX);

        c_baseInit(base);

        c_bind(base,"c_baseModule");
        c_metaObject(base)->name = NULL;

        q_parserInit();
    }
    return base;
}

void
c_destroy (
    c_base _this)
{
    /* for future use */
}

c_base
c_open (
    const c_char *name,
    void *address)
{
    c_base base;
    c_mm mm;
    c_long namelength;

    namelength = strlen(name);

    if (namelength == 0) {
        return c_create("HEAPDATABASE", NULL, 0, 0);
    }
    mm = c_mmCreate(address,0, 0);
    base = (c_base)c_mmLookup(mm, name);
    if (base == NULL) {
        OS_REPORT_1(OS_ERROR,
                    "c_base::c_open",0,
                    "segment %s not found",name);
        return NULL;
    }

    if (base->confidence != CONFIDENCE) {
        OS_REPORT_2(OS_ERROR,
                    "c_base::c_open",0,
                    "confidence mismatch: %d <> %d",
                    base->confidence,CONFIDENCE);
        return NULL;
    }

    q_parserInit();

    return base;
}

c_object
c_bind (
    c_object object,
    const c_char *name)
{
    c_baseBinding binding, found;
    c_base base;
    c_object result = NULL;

    base = c_header(object)->type->base;

    assert(base->confidence == CONFIDENCE);
    binding = (c_baseBinding)c_mmMalloc(base->mm,C_SIZEOF(c_baseBinding));
    if (binding) {
        binding->name = c_stringNew(base,name);
        binding->object = c_keep(object);

        found = (c_baseBinding)c_avlTreeInsert(base->bindings,
                                               binding,
                                               (c_equality(*)())c_compareBindName,
                                               NULL);

        if (found != binding) {
            c_free(binding->object);
            c_free(binding->name);
            c_mmFree(base->mm, binding);
        }
    if (found) {
        result = found->object;
    }
    }
    return result;
}

c_object
c_unbind (
    c_base base,
    const c_char *name)
{
    c_baseBinding binding;
    c_object object;

    assert(base->confidence == CONFIDENCE);

    binding = (c_baseBinding)c_avlTreeRemove(base->bindings,
                                             NULL,
                                             (c_equality(*)())c_compareBindName,
                                             (void *)name,
                                             NULL,NULL);
    if (binding == NULL) {
        return NULL;
    }
    object = binding->object;
    c_mmFree(base->mm, binding->name);
    c_mmFree(base->mm, binding);
    return object;
}

c_object
c_lookup (
    c_base base,
    const c_char *name)
{
    c_baseBinding found;
    assert(base->confidence == CONFIDENCE);

    c_mutexLock(&base->bindLock);
    found = (c_baseBinding)c_avlTreeFind(base->bindings,
                                         NULL,
                                         (c_equality(*)())c_compareBindName,
                                         (void *)name);
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

c_long
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

c_object
c_new (
    c_type type)
{
    c_header header = NULL;
    c_object o;
    c_long size;

    assert(type);

    size = c_typeSize(type);

    if ((c_baseObjectKind(type) == M_COLLECTION) &&
            ((c_collectionTypeKind(type) == C_ARRAY)
                || (c_collectionTypeKind(type) == C_SEQUENCE))) {
        OS_REPORT(OS_ERROR,
                    "Database c_new",0,
                    "c_new cannot create C_ARRAY nor C_SEQUENCE, use c_newArray or c_newSequence respectively");
        assert(0); /* crash in dev, so that the error is easily found */
        o = NULL;
    } else if (size <= 0) {
        OS_REPORT_1(OS_ERROR,
                    "Database c_new",0,
                    "Illegal size %d specified",size);
        o = NULL;
    } else {
        header = (c_header)c_mmMalloc(type->base->mm, MEMSIZE(size));
        if (header) {
            header->refCount = 1;
            header->type = c_keep(type);
            pa_increment(&header->type->objectCount);
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
            o = c_oid(header);
            memset(o,0,size);
#if CHECK_REF
            if ((c_baseObject(header->type)->kind == M_EXTENT) ||
                (c_baseObject(header->type)->kind == M_EXTENTSYNC)) {
                c_type t;
                t = c_extentType(c_extent(header->type));
                ACTUALTYPE(type,t);
                c_free(t);
            } else {
                ACTUALTYPE(type,header->type);
            }
            if (type && c_metaObject(type)->name) {
                if (strlen(c_metaObject(type)->name) >= CHECK_REF_TYPE_LEN) {
                    if (strncmp(c_metaObject(type)->name, CHECK_REF_TYPE, strlen(CHECK_REF_TYPE)) == 0) {
                        UT_TRACE("\n\n============ New(0x%x) =============\n", o);
                    }
                }
            }
#endif
        }
    }
    return o;
}

/*
 * Use the macro definitions c_newArray and c_newSequence instead, respectively
 * for creating an array or a sequence for legibility.
 */
c_object
c_newBaseArrayObject (
    c_collectionType arrayType,
    c_long size)
{
    c_long allocSize;
    c_object o = NULL;

    assert(arrayType);
    assert(
        (c_collectionTypeKind(arrayType) == C_ARRAY && size > 0)
        ||
        (c_collectionTypeKind(arrayType) == C_SEQUENCE && size >= 0)
        );

    if ((c_collectionTypeKind(arrayType) == C_ARRAY)
         || (c_collectionTypeKind(arrayType) == C_SEQUENCE)) {
        if (    (c_collectionTypeKind(arrayType) == C_ARRAY && size > 0)
                ||
                (c_collectionTypeKind(arrayType) == C_SEQUENCE && size >= 0) ) {
            c_arrayHeader hdr;
            c_header header;
            c_type subType;

            subType = c_collectionTypeSubType(arrayType);

            switch(c_baseObjectKind(subType)) {
            case M_INTERFACE:
            case M_CLASS:
                allocSize = size * sizeof(void *);
            break;
            default:
                if (subType->size == 0) {
                    subType->size = sizeof(void *);
                }
                allocSize = size * subType->size;
            break;
            }
            hdr = (c_arrayHeader)c_mmMalloc(c_type(arrayType)->base->mm,
                                            ARRAYMEMSIZE(allocSize));

            if (hdr) {
                hdr->size = size;
                header = (c_header)&hdr->_parent;

                header->refCount = 1;
                /* Keep reference to our type */
                header->type = c_keep(c_type(arrayType));
                pa_increment(&header->type->objectCount);

                o = c_oid(header);

                /*
                 * When an array is freed via c_free, it also checks whether it contains
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
            OS_REPORT_1(OS_ERROR,
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
c_mem2object(
    c_voidp mem,
    c_type type)
{
    c_object o;
    c_type t;
    C_STRUCT(c_header) *header = (C_STRUCT(c_header) *)mem;

    header->refCount = 1;
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
    assert(header->refCount == 0);
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
    assert(o);
    assert(c_header(o)->confidence == CONFIDENCE);
    assert(c_header(o)->refCount > 0);
}
#endif

static c_bool _c_freeReferences(c_metaObject metaObject, c_object o);
#define c_freeReferences(m,o) ((m && o)? _c_freeReferences(m,o):TRUE)
#define freeReference(p,t) (p?_freeReference(p,t):TRUE)

static c_bool
_freeReference (
    c_voidp *p,
    c_type type)
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
        c_free(c_object(*p));
    break;
    case M_BASE:
    case M_COLLECTION:
        if ((c_collectionType(t)->kind == C_ARRAY) &&
            (c_collectionType(t)->maxSize != 0)) {
            c_freeReferences(c_metaObject(t), p);
        } else {
            c_free(c_object(*p));
        }
    break;
    case M_EXCEPTION:
    case M_STRUCTURE:
    case M_UNION:
        c_freeReferences(c_metaObject(type),p);
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
    case M_EXTENT:
    case M_EXTENTSYNC:
        c_free(c_object(*p));
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
    c_object o)
{
    c_type type;
    c_class cls;
    c_array references, labels, ar;
    c_property property;
    c_member member;
    c_long i,j,length,size;
    c_long nrOfRefs,nrOfLabs;
    c_value v;

    assert(metaObject);
    assert(o);

    switch (c_baseObject(metaObject)->kind) {
#if 0
    case M_CLASS:
        c_freeReferences(c_metaObject(c_class(metaObject)->extends),o);
    case M_INTERFACE:
        length = c_arraySize(c_interface(metaObject)->references);
        for (i=0;i<length;i++) {
            property = c_property(c_interface(metaObject)->references[i]);
            type = property->type;
            freeReference(C_DISPLACE(o,property->offset),type);
        }
    break;
#else
    case M_CLASS:
        /* Extent is a special case, so check */
        if ((c_getMetaType(c_type(metaObject)->base, M_EXTENT)==c_type(metaObject)) ||
            (c_getMetaType(c_type(metaObject)->base, M_EXTENTSYNC))==c_type(metaObject)) {
            c_extentFree(c_extent(o));
        } else {
            cls = c_class(metaObject);
            while (cls) {
                length = c_arraySize(c_interface(cls)->references);
                for (i=0;i<length;i++) {
                    property = c_property(c_interface(cls)->references[i]);
                    type = property->type;
                    freeReference(C_DISPLACE(o,property->offset),type);
                }
                cls = cls->extends;
            }
        }
    break;
    case M_INTERFACE:
        length = c_arraySize(c_interface(metaObject)->references);
        for (i=0;i<length;i++) {
            property = c_property(c_interface(metaObject)->references[i]);
            type = property->type;
            freeReference(C_DISPLACE(o,property->offset),type);
        }
    break;
#endif
    case M_EXCEPTION:
    case M_STRUCTURE:
        length = c_arraySize(c_structure(metaObject)->references);
        for (i=0;i<length;i++) {
            member = c_member(c_structure(metaObject)->references[i]);
            type = c_specifier(member)->type;
            freeReference(C_DISPLACE(o,member->offset),type);
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
        break;
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
                                         C_DISPLACE(o,c_type(metaObject)->alignment));
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
        case C_ARRAY:
        case C_SEQUENCE:
            ACTUALTYPE(type,c_collectionType(metaObject)->subType);
            ar = (c_array)o;

            if(c_collectionType(metaObject)->kind == C_ARRAY
                && c_collectionType(metaObject)->maxSize > 0){
                length = c_collectionType(metaObject)->maxSize;
            } else {
                length = c_arraySize(ar);
            }

            if (c_typeIsRef(type)) {
                for (i=0;i<length;i++) {
                    c_free(ar[i]);
                }
            } else {
                if (c_typeHasRef(type)) {
                    size = type->size;
                    for (i=0;i<length;i++) {
                        freeReference(C_DISPLACE(ar,(i*size)),type);
                    }
                }
            }
        break;
        case C_SCOPE:
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
        c_freeReferences(c_metaObject(c_typeDef(metaObject)->alias),o);
    break;
    case M_ATTRIBUTE:
    case M_RELATION:
        ACTUALTYPE(type,c_property(metaObject)->type);
        freeReference(C_DISPLACE(o,c_property(metaObject)->offset),type);
    break;
    case M_MEMBER:
        ACTUALTYPE(type,c_specifier(metaObject)->type);
        freeReference(C_DISPLACE(o,c_member(metaObject)->offset),type);
    break;
    case M_UNIONCASE:
        ACTUALTYPE(type,c_specifier(metaObject)->type);
        freeReference(o,type);
    break;
    case M_MODULE:
        c_free(c_module(o)->scope);
    break;
    case M_PRIMITIVE:
        /* Do nothing */
    break;
    case M_EXTENT:
    case M_EXTENTSYNC:
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
/*
 * Function used in OS_REPORT in c_free
 */
static const c_char *
metaKindImage (
    c_metaKind kind)
{
#define _CASE_(o) case o: return #o
    switch (kind) {
    _CASE_(M_UNDEFINED);
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
    _CASE_(M_EXTENT);
    _CASE_(M_EXTENTSYNC);
    _CASE_(M_COUNT);
    default:
        return "Unknown metaKind specified";
            }
#undef _CASE_
}
#endif /* NDEBUG */

void
c_free (
    c_object object)
{
    c_header header;
    c_type type, headerType;
    os_uint32 safeCount;

    if (object == NULL) {
        return;
    }

    header = c_header(object);
    assert(header->confidence == CONFIDENCE);
    if (header->refCount == 0) {
#ifndef NDEBUG
#if CHECK_REF
        UT_TRACE("\n\n===========Free(0x%x) already freed =======\n", (unsigned int)object);
#endif
        OS_REPORT_3(OS_ERROR,
                    "Database",0,
                    "Object (%p) of type '%s', kind '%s' already deleted",
                    object,
                    c_metaName(c_metaObject(header->type)),
                    metaKindImage(c_baseObject(header->type)->kind));
        assert(FALSE);
#endif /* NDEBUG */
        return; /* cyclic reference detection */
    }
    safeCount = pa_decrement(&header->refCount);
    /* Take a local pointer, since header->type pointer will be deleted */
    headerType = header->type;
    if ((c_baseObject(headerType)->kind == M_EXTENT) ||
        (c_baseObject(headerType)->kind == M_EXTENTSYNC)) {
        c_type t;
        t = c_extentType(c_extent(headerType));
        ACTUALTYPE(type,t);
        c_free(t);
    } else {
        ACTUALTYPE(type,headerType);
    }

#if CHECK_REF
    if (type && c_metaObject(type)->name) {
        if (strlen(c_metaObject(type)->name) >= CHECK_REF_TYPE_LEN) {
            if (strncmp(c_metaObject(type)->name, CHECK_REF_TYPE, strlen(CHECK_REF_TYPE)) == 0) {
                UT_TRACE("\n\n============ Free(0x%x): %d -> %d =============\n",
                        object, safeCount+1, safeCount);
            }
        }
    }
#endif

    if (safeCount == 0) {
        c_freeReferences(c_metaObject(type),object);
#ifndef NDEBUG
        {
            c_long size;

#ifdef OBJECT_WALK
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
#endif

            size = c_typeSize(type);
            if (size < 0) {
                OS_REPORT(OS_ERROR,"c_free",0,"illegal object size ( size <= 0 )");
                assert(FALSE);
            }
        }
#endif
        if ((c_baseObject(headerType)->kind == M_EXTENT) ||
            (c_baseObject(headerType)->kind == M_EXTENTSYNC)) {
            c_extentDelete(c_extent(headerType), object);
        } else {
            if ((c_baseObjectKind(type) == M_COLLECTION) &&
                ((c_collectionTypeKind(type) == C_ARRAY) ||
                (c_collectionTypeKind(type) == C_SEQUENCE))) {
                c_arrayHeader hdr;

                hdr = c_arrayHeader(object);
#ifndef NDEBUG
                {
                    c_long size;
                    size = c_arraySize(object);
                    memset(hdr,0,ARRAYMEMSIZE(size));
    }
#endif
                c_mmFree(type->base->mm, hdr);
            } else {
#ifndef NDEBUG
#if 0
               {
                  /* Disabled so that we can detect mutex and conditon signatures
                     when the memory is freed */
                  c_long size;
                  size = c_typeSize(type,object);
                  memset(header,0,MEMSIZE(size));
               }
#endif
#endif
               c_mmFree(type->base->mm, header);
            }
            /* Do not use type, as it refers to an actual type, while
             * we incremented the header->type.
             */
            pa_decrement(&headerType->objectCount);
        }
        c_free(headerType); /* free the header->type */
    }
}

void
c__free(
    c_object object)
{
    c_header header;
    c_type type,extType;

    header = c_header(object);
    extType = c_extentType(c_extent(header->type));
    ACTUALTYPE(type,extType);
    c_free(extType);
    assert(header->refCount == 0);
    if ((c_baseObjectKind(type) == M_COLLECTION) &&
        ((c_collectionTypeKind(type) == C_ARRAY) ||
         (c_collectionTypeKind(type) == C_SEQUENCE))) {
        c_arrayHeader hdr;

        hdr = c_arrayHeader(object);
#ifndef NDEBUG
        {
            c_long size;
            size = c_typeSize(type);
            memset(hdr,0,ARRAYMEMSIZE(size));
        }
#endif
        c_mmFree(type->base->mm, hdr);
    } else {
#ifndef NDEBUG
#if 0
       {
          /* Disabled so that we can detect mutex and conditon signatures
             when the memory is freed */
          c_long size;
          size = c_typeSize(type,object);
          memset(header,0,MEMSIZE(size));
       }
#endif
#endif
       c_mmFree(type->base->mm, header);
    }
}

c_object
c_keep (
    c_object object)
{
    c_header header;

    if (object == NULL) {
        return NULL;
    }

    header = c_header(object);
    assert(header->confidence == CONFIDENCE);
    assert(header->refCount > 0);
#if CHECK_REF
    if (header->type) {
        c_type type;
        if ((c_baseObject(header->type)->kind == M_EXTENT) ||
            (c_baseObject(header->type)->kind == M_EXTENTSYNC)) {
            c_type t;
            t = c_extentType(c_extent(header->type));
            ACTUALTYPE(type,t);
            c_free(t);
        } else {
            ACTUALTYPE(type,header->type);
        }
        if (type && c_metaObject(type)->name) {
            if (strlen(c_metaObject(type)->name) >= CHECK_REF_TYPE_LEN) {
                if (strncmp(c_metaObject(type)->name, CHECK_REF_TYPE, CHECK_REF_TYPE_LEN) == 0) {
                    UT_TRACE("\n\n============ Keep(0x%x): %d -> %d =============\n",
                        object, header->refCount, header->refCount+1);
                }
            }
        }
    }
#endif
    pa_increment(&header->refCount);

    return object;
}

static c_equality
c_compareBindName (
    c_baseBinding b1,
    c_baseBinding b2,
    const c_char *name)
{
    c_long result;
    if (b2 == NULL) {
        result = strcmp(b1->name, name);
    } else {
        result = strcmp(b1->name, b2->name);
    }
    if (result == 0) {
        return C_EQ;
    }
    if (result < 0) {
        return C_LT;
    }
    return C_GT;
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
    if( (c_baseObject(c_header(object)->type)->kind == M_EXTENT) ||
        (c_baseObject(c_header(object)->type)->kind == M_EXTENTSYNC)){
        type = c_typeDef(c_header(object)->type)->alias;
    } else {
        type = c_header(object)->type;
    }
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
    return c_header(object)->refCount;
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

void
c_baseOnOutOfMemory(
    c_base base,
    c_baseOutOfMemoryAction action,
    c_voidp arg)
{
    if (base) {
        c_mm mm;
        mm = c_baseMM(base);
        if (mm) {
            c_mmOnOutOfMemory(mm,action,arg);
        }
    }
}

void
c_baseOnLowOnMemory(
    c_base base,
    c_baseLowOnMemoryAction action,
    c_voidp arg)
{
    if (base) {
        c_mm mm;
        mm = c_baseMM(base);
        if (mm) {
            c_mmOnLowOnMemory(mm,action,arg);
        }
    }
}

c_long
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
                    (c_collectionTypeKind(c_header(_this)->type) == C_ARRAY)
                    || (c_collectionTypeKind(c_header(_this)->type) == C_SEQUENCE)
               )
            );
        return (c_long)(c_arrayHeader(_this)->size);
    } else {
        return 0;
    }
}

c_long
c_sequenceSize(
    c_sequence _this)
{
    if (_this) {
        assert(c_header(_this)->confidence == CONFIDENCE);
        assert(c_baseObjectKind(c_header(_this)->type) == M_COLLECTION &&
                c_collectionTypeKind(c_header(_this)->type) == C_SEQUENCE);
        return (c_long)(c_arrayHeader(_this)->size);
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
                        (!c_objectIsType(type)))
                    {
                        /* Invalid array hdr, so try if it is a normal header. */
                        header = (c_header)hdr;
                        addr = c_mmCheckPtr(c_baseMM(_this), c_header(header->type));
                        if (addr != NULL) {
                            type = c_oid(addr);
                            if ((type != NULL) &&
                                (type->base == _this) &&
                                (c_objectIsType(type)))
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
                            (c_objectIsType(type)))
                        {
                            /* We have a valid address! */
                            o = c_oid(header);
                        }
                    }
                }
                hdr = (c_arrayHeader)(C_ADDRESS(hdr) - 4);
            }
        } else {
            OS_REPORT_1(OS_ERROR,"c_baseCheckPtr", 0,
                        "Could not resolve Memory Manager for Database (0x%x)",
                        _this);
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

#if CHECK_REF
#undef UT_TRACE
#endif
