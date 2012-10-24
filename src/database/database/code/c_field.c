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
#include "c__base.h"
#include "c__metabase.h"
#include "c__field.h"
#include "c_stringSupport.h"
#include "c_collection.h"
#include "c_iterator.h"
#include "os_report.h"
#include "os.h"

C_STRUCT(c_field) {
    c_valueKind kind;
    c_address   offset;
    c_string    name;
    c_array     path;
    c_array     refs;
    c_type      type;
};

c_type
c_field_t (
    c_base _this)
{
    if (_this->baseCache.fieldCache.c_field_t == NULL) {
        _this->baseCache.fieldCache.c_field_t = c_resolve(_this,"c_field");
    }
    return _this->baseCache.fieldCache.c_field_t;
}

c_collectionType
c_fieldPath_t (
    c_base _this)
{
    if (_this->baseCache.fieldCache.c_fieldPath_t == NULL) {
        _this->baseCache.fieldCache.c_fieldPath_t = c_collectionType(
                             c_metaArrayTypeNew(c_metaObject(_this),
                                                "C_ARRAY<c_base>",
                                                c_getMetaType(_this,M_BASE),
                                                0));
    }
    return _this->baseCache.fieldCache.c_fieldPath_t;
}

c_collectionType
c_fieldRefs_t (
    c_base _this)
{
    if (_this->baseCache.fieldCache.c_fieldRefs_t == NULL) {
        _this->baseCache.fieldCache.c_fieldRefs_t = c_collectionType(
                             c_metaArrayTypeNew(c_metaObject(_this),
                                                "C_ARRAY<c_address>",
                                                c_address_t(_this),
                                                0));
    }
    return _this->baseCache.fieldCache.c_fieldRefs_t;
}


void
c_fieldInit (
    c_base base)
{
    c_object scope;

    scope = c_metaDeclare((c_object)base,"c_field",M_CLASS);
        C_META_ATTRIBUTE_(c_field,scope,kind,c_valueKind_t(base));
        C_META_ATTRIBUTE_(c_field,scope,offset,c_address_t(base));
        C_META_ATTRIBUTE_(c_field,scope,name,c_string_t(base));
        C_META_ATTRIBUTE_(c_field,scope,path,c_array_t(base));
        C_META_ATTRIBUTE_(c_field,scope,refs,c_array_t(base));
        C_META_ATTRIBUTE_(c_field,scope,type,c_type_t(base));
    c__metaFinalize(scope,FALSE);
    c_free(scope);
}

c_valueKind
c_fieldValueKind (
    c_field _this)
{
    return (_this == NULL ? V_UNDEFINED : _this->kind);
}

c_array
c_fieldPath(
    c_field _this)
{
    return (_this == NULL ? NULL : _this->path);
}

c_string
c_fieldName (
    c_field _this)
{
    return (_this == NULL ? NULL : _this->name);
}

c_type
c_fieldType (
    c_field _this)
{
    return (_this == NULL ? NULL : c_keep(_this->type));
}

c_field
c_fieldNew (
    c_type type,
    const c_char *fieldName)
{
    c_array path;
    c_field field;
    c_metaObject o;
    c_long n,length;
    c_address offset;
    c_iter nameList, refsList;
    c_string name;
    c_base base;

    if ((fieldName == NULL) || (type == NULL)) {
        OS_REPORT(OS_ERROR,
                  "c_fieldNew failed",0,
                  "illegal parameter");
        return NULL;
    }

    base = c__getBase(type);
    if (base == NULL) {
        OS_REPORT(OS_ERROR,
                  "c_fieldNew failed",0,
                  "failed to retreive base");
        return NULL;
    }

    nameList = c_splitString(fieldName,".");
    length = c_iterLength(nameList);
    field = NULL;

    if (length > 0) {
        o = NULL;
        offset = 0;
        refsList = NULL;
        path = c_newArray(c_fieldPath_t(base),length);
        if (path) {
            for (n=0;n<length;n++) {
                name = c_iterTakeFirst(nameList);
                o = c_metaResolve(c_metaObject(type),name);
                os_free(name);
                if (o == NULL) {
                    c_iterWalk(nameList,(c_iterWalkAction)os_free,NULL);
                    c_iterFree(nameList);
                    c_iterFree(refsList);
                    c_free(path);
                    return NULL;
                }
                path[n] = o;
                switch (c_baseObject(o)->kind) {
                case M_ATTRIBUTE:
                case M_RELATION:
                    type = c_property(o)->type;
                    offset += c_property(o)->offset;
                break;
                case M_MEMBER:
                    type = c_specifier(o)->type;
                    offset += c_member(o)->offset;
                break;
                default:
                    c_iterWalk(nameList,(c_iterWalkAction)os_free,NULL);
                    c_iterFree(nameList);
                    c_iterFree(refsList);
                    c_free(path);
                    return NULL;
                }
                switch (c_baseObject(type)->kind) {
                case M_INTERFACE:
                case M_CLASS:
                case M_COLLECTION:
                    /*Longs are inserted in an iterator? Explanation please...*/
                    refsList = c_iterInsert(refsList,(c_voidp)offset);
                    offset = 0;
                break;
                default:
                break;
                }
            }
            if (offset > 0) {
                refsList = c_iterInsert(refsList,(c_voidp)offset);
            }


            field = c_new(c_field_t(base));
            field->name = c_stringNew(base,fieldName);
            field->path = path;
            field->type = c_keep(type);
            field->kind = c_metaValueKind(o);
            field->refs = NULL;

            if (refsList) {
                length = c_iterLength(refsList);
                field->offset = 0;
                if (length > 0) {
                    field->refs = c_newArray(c_fieldRefs_t(base),length);
                    if (field->refs) {
                        for (n=(length-1);n>=0;n--) {
                            field->refs[n] = c_iterTakeFirst(refsList);
                        }
                    } else {
                        OS_REPORT(OS_ERROR,
                                  "c_fieldNew failed",0,
                                  "failed to allocate field->refs array");
                        c_free(field);
                        field = NULL;
                    }
                }
                c_iterFree(refsList);
            } else {
                field->offset = offset;
            }
        } else {
            OS_REPORT(OS_ERROR,
                      "c_fieldNew failed",0,
                      "failed to allocate field->path array");
            c_iterWalk(nameList,(c_iterWalkAction)os_free,NULL);
            c_iterFree(nameList);
        }
        c_iterFree(nameList);
    } else {
        OS_REPORT_1(OS_ERROR,
                    "c_fieldNew failed",0,
                    "failed to process field name <%s>",
                    fieldName);
    }
    return field;
}

c_field
c_fieldConcat (
    c_field head,
    c_field tail)
{
    c_base base;
    c_long i, len1, len2, totlen;
    c_field field;

    base = c__getBase(head);

    len1 = c_arraySize(head->path);
    len2 = c_arraySize(tail->path);

    field = c_new(c_field_t(base));

    if (field) {
        field->type = c_keep(tail->type);
        field->kind = tail->kind;
        field->path = c_newArray(c_fieldPath_t(base),len1 + len2);
        for (i=0;i<len1;i++) {
            field->path[i] = c_keep(head->path[i]);
        }
        for (i=0;i<len2;i++) {
            field->path[i+len1] = c_keep(tail->path[i]);
        }

        len1 = c_arraySize(head->refs);
        len2 = c_arraySize(tail->refs);

        totlen = len1 + len2;
        if (totlen > 0) {
            field->offset = 0;
            field->refs = c_newArray(c_fieldRefs_t(base),totlen);
            if (len1) {
                for (i=0;i<len1;i++) {
                    field->refs[i] = head->refs[i];
                }
            } else {
                tail->refs[0] = (c_voidp)head->offset;
                len1 = 1;
            }
            for (i=0;i<len2;i++) {
                field->refs[i+len1] = tail->refs[i];
            }
        } else {
            field->offset = head->offset + tail->offset;
            field->refs = NULL;
        }

        len1 = strlen(head->name);
        len2 = strlen(tail->name);

        field->name = c_stringMalloc(base,len1+len2+2);
        os_sprintf(field->name,"%s.%s",head->name,tail->name);
    } else {
        OS_REPORT(OS_ERROR,
                  "database::c_fieldConcat",0,
                  "Failed to allocate c_field object.");
    }

    return field;
}

void
c_fieldFreeRef (
    c_field field,
    c_object o)
{
    c_long i,n;
    c_voidp p = o;
    c_array refs;

    if (field->refs) {
        refs = field->refs;
        n = c_arraySize(refs)-1;
        for(i=0;i<n;i++) {
            p = *(c_voidp *)C_DISPLACE(p,refs[i]);
        }
        p = C_DISPLACE(p,refs[n]);
    } else {
        p = C_DISPLACE(p,field->offset);
    }

    switch(field->kind) {
    case V_ADDRESS:   break;
    case V_BOOLEAN:   break;
    case V_SHORT:     break;
    case V_LONG:      break;
    case V_LONGLONG:  break;
    case V_OCTET:     break;
    case V_USHORT:    break;
    case V_ULONG:     break;
    case V_ULONGLONG: break;
    case V_CHAR:      break;
    case V_WCHAR:     break;
    case V_STRING:
        c_free((c_object)(*(c_string *)p));
        (*(c_string *)p) = NULL;
    break;
    case V_WSTRING:
        c_free((c_object)(*(c_wstring *)p));
        (*(c_wstring *)p) = NULL;
    break;
    case V_FLOAT:     break;
    case V_DOUBLE:    break;
    case V_OBJECT:
        c_free(*(c_object *)p);
        (*(c_object *)p) = NULL;
    break;
    case V_VOIDP:     break;
    case V_FIXED:
    case V_UNDEFINED:
    case V_COUNT:
        OS_REPORT_1(OS_ERROR,"c_fieldFreeRef failed",0,
                    "illegal field value kind (%d)", field->kind);
        assert(FALSE);
    break;
    }
}

void
c_fieldAssign (
    c_field field,
    c_object o,
    c_value v)
{
    c_long i,n;
    c_voidp p = o;
    c_array refs;

    if (field->refs) {
        refs = field->refs;
        n = c_arraySize(refs)-1;
        for(i=0;i<n;i++) {
            p = *(c_voidp *)C_DISPLACE(p,refs[i]);
        }
        p = C_DISPLACE(p,refs[n]);
    } else {
        p = C_DISPLACE(p,field->offset);
    }
    v.kind = field->kind;

#define _VAL_(f,t) *((t *)p) = v.is.f

    switch(v.kind) {
    case V_ADDRESS:   _VAL_(Address,c_address); break;
    case V_BOOLEAN:   _VAL_(Boolean,c_bool); break;
    case V_SHORT:     _VAL_(Short,c_short); break;
    case V_LONG:      _VAL_(Long,c_long); break;
    case V_LONGLONG:  _VAL_(LongLong,c_longlong); break;
    case V_OCTET:     _VAL_(Octet,c_octet); break;
    case V_USHORT:    _VAL_(UShort,c_ushort); break;
    case V_ULONG:     _VAL_(ULong,c_ulong); break;
    case V_ULONGLONG: _VAL_(ULongLong,c_ulonglong); break;
    case V_CHAR:      _VAL_(Char,c_char); break;
    case V_WCHAR:     _VAL_(WChar,c_wchar); break;
    case V_STRING:
        c_free((c_object)(*(c_string *)p));
        _VAL_(String,c_string);
        c_keep((c_object)(*(c_string *)p));
    break;
    case V_WSTRING:
        c_free((c_object)(*(c_wstring *)p));
        _VAL_(WString,c_wstring);
        c_keep((c_object)(*(c_wstring *)p));
    break;
    case V_FLOAT:     _VAL_(Float,c_float); break;
    case V_DOUBLE:    _VAL_(Double,c_double); break;
    case V_OBJECT:
        c_free(*(c_object *)p);
        _VAL_(Object,c_object);
        c_keep(*(c_object *)p);
    break;
    case V_VOIDP:
        _VAL_(Voidp,c_voidp);
    break;
    case V_FIXED:
    case V_UNDEFINED:
    case V_COUNT:
        OS_REPORT_1(OS_ERROR,"c_fieldAssign failed",0,
                    "illegal field value kind (%d)", v.kind);
        assert(FALSE);
    break;
    }
#undef _VAL_
}

c_value
c_fieldValue(
    c_field field,
    c_object o)
{
    c_value v;
    c_long i,n;
    c_array refs;
    c_voidp p = o;

    /* initialize v! variable v has to be initialized as parts of the
     * union might not be initialized.
     * Example:
     * kind = V_LONG;
     * is.Long = 1;
     * then the last 4 bytes have not been initialized,
     * since the double field is 8 bytes, making the
     * c_value structure effectively 12 bytes.
     * This causes a lot of MLK's in purify.
     */
#ifndef NDEBUG
    memset(&v, 0, sizeof(v));
#endif

    if (field->refs) {
        refs = field->refs;
        n = c_arraySize(refs)-1;
        for(i=0;i<n;i++) {
            p = C_DISPLACE(p,refs[i]);
            if (p == NULL) {
                v.kind = V_UNDEFINED;
                return v;
            }
            p = *(c_voidp *)p;
        }
        if(p == NULL){
            v.kind = V_UNDEFINED;
            return v;
        }
        else {
            p = C_DISPLACE(p,refs[n]);
        }
    } else {
        p = C_DISPLACE(p,field->offset);
    }

#if 0
    v.kind = field->kind;
    switch(v.kind) {
    case V_STRING:
    case V_WSTRING:
    case V_OBJECT:
        v.is.Object = *(c_object *)p;
/*        memcpy(&v.is,p,field->type->size);*/
        c_keep(v.is.Object);
    break;
    case V_FIXED:
    case V_UNDEFINED:
    case V_COUNT:
    break;
    default:
        v.is.LongLong = *(c_longlong *)p;
/*        memcpy(&v.is,p,field->type->size);*/
    break;
    }
    return v;
#else
#define _VAL_(f,t) v.is.f = *(t *)p

    v.kind = field->kind;

    switch(field->kind) {
    case V_ADDRESS:   _VAL_(Address,c_address); break;
    case V_BOOLEAN:   _VAL_(Boolean,c_bool); break;
    case V_SHORT:     _VAL_(Short,c_short); break;
    case V_LONG:      _VAL_(Long,c_long); break;
    case V_LONGLONG:  _VAL_(LongLong,c_longlong); break;
    case V_OCTET:     _VAL_(Octet,c_octet); break;
    case V_USHORT:    _VAL_(UShort,c_ushort); break;
    case V_ULONG:     _VAL_(ULong,c_ulong); break;
    case V_ULONGLONG: _VAL_(ULongLong,c_ulonglong); break;
    case V_CHAR:      _VAL_(Char,c_char); break;
    case V_WCHAR:     _VAL_(WChar,c_wchar); break;
    case V_STRING:    _VAL_(String,c_string); c_keep(v.is.String); break;
    case V_WSTRING:   _VAL_(WString,c_wstring); c_keep(v.is.WString); break;
    case V_FLOAT:     _VAL_(Float,c_float); break;
    case V_DOUBLE:    _VAL_(Double,c_double); break;
    case V_OBJECT:    _VAL_(Object,c_object); c_keep(v.is.Object); break;
    case V_VOIDP:     _VAL_(Voidp,c_voidp); break;
    case V_FIXED:
    case V_UNDEFINED:
    case V_COUNT:
        OS_REPORT_1(OS_ERROR,"c_fieldAssign failed",0,
                    "illegal field value kind (%d)", v.kind);
        assert(FALSE);
    break;
    }

    return v;

#undef _VAL_
#endif
}

void
c_fieldCopy(
    c_field srcfield,
    c_object src,
    c_field dstfield,
    c_object dst)
{
    c_long i,n;
    c_array refs;
    c_voidp srcp = src;
    c_voidp dstp = dst;

    if (srcfield->refs) {
        refs = srcfield->refs;
        n = c_arraySize(refs)-1;
        for(i=0;i<n;i++) {
            srcp = *(c_voidp *)C_DISPLACE(srcp,refs[i]);
        }
        srcp = C_DISPLACE(srcp,refs[n]);
    } else {
        srcp = C_DISPLACE(srcp,srcfield->offset);
    }

    if (dstfield->refs) {
        refs = dstfield->refs;
        n = c_arraySize(refs)-1;
        for(i=0;i<n;i++) {
            dstp = *(c_voidp *)C_DISPLACE(dstp,refs[i]);
        }
        dstp = C_DISPLACE(dstp,refs[n]);
    } else {
        dstp = C_DISPLACE(dstp,dstfield->offset);
    }
    memcpy(dstp,srcp,dstfield->type->size);
    if ((dstfield->kind == V_STRING) ||
        (dstfield->kind == V_WSTRING) ||
        (dstfield->kind == V_FIXED)) {
        c_keep(*(c_string *)dstp);
    }
}

void
c_fieldClone(
    c_field srcfield,
    c_object src,
    c_field dstfield,
    c_object dst)
{
    c_long i,n;
    c_array refs;
    c_voidp srcp = src;
    c_voidp dstp = dst;

    if (srcfield->refs) {
        refs = srcfield->refs;
        n = c_arraySize(refs)-1;
        for(i=0;i<n;i++) {
            srcp = *(c_voidp *)C_DISPLACE(srcp,refs[i]);
        }
        srcp = C_DISPLACE(srcp,refs[n]);
    } else {
        srcp = C_DISPLACE(srcp,srcfield->offset);
    }

    if (dstfield->refs) {
        refs = dstfield->refs;
        n = c_arraySize(refs)-1;
        for(i=0;i<n;i++) {
            dstp = *(c_voidp *)C_DISPLACE(dstp,refs[i]);
        }
        dstp = C_DISPLACE(dstp,refs[n]);
    } else {
        dstp = C_DISPLACE(dstp,dstfield->offset);
    }
    if ((dstfield->kind == V_STRING) ||
        (dstfield->kind == V_WSTRING) ||
        (dstfield->kind == V_FIXED)) {
    	dstp = c_stringNew(c_getBase(dstfield), srcp);
    } else {
        memcpy(dstp,srcp,dstfield->type->size);
    }
}

c_equality
c_fieldCompare (
    c_field field1,
    c_object src1,
    c_field field2,
    c_object src2)
{
    c_long i,n,r;
    c_array refs;
    c_voidp p1 = src1;
    c_voidp p2 = src2;
    c_equality result;

    result = C_NE;

    if (field1->refs) {
        refs = field1->refs;
        n = c_arraySize(refs)-1;
        for(i=0;i<n;i++) {
            p1 = *(c_voidp *)C_DISPLACE(p1,refs[i]);
        }
        p1 = C_DISPLACE(p1,refs[n]);
    } else {
        p1 = C_DISPLACE(p1,field1->offset);
    }

    if (field2->refs) {
        refs = field2->refs;
        n = c_arraySize(refs)-1;
        for(i=0;i<n;i++) {
            p2 = *(c_voidp *)C_DISPLACE(p2,refs[i]);
        }
        p2 = C_DISPLACE(p2,refs[n]);
    } else {
        p2 = C_DISPLACE(p2,field2->offset);
    }
#define _CMP_(t) ((*(t*)p1)<(*(t*)p2)?C_LT:((*(t*)p1)>(*(t*)p2)?C_GT:C_EQ))

    switch(field1->kind) {
    case V_ADDRESS:   result = _CMP_(c_address); break;
    case V_BOOLEAN:   result = _CMP_(c_bool); break;
    case V_SHORT:     result = _CMP_(c_short); break;
    case V_LONG:      result = _CMP_(c_long); break;
    case V_LONGLONG:  result = _CMP_(c_longlong); break;
    case V_OCTET:     result = _CMP_(c_octet); break;
    case V_USHORT:    result = _CMP_(c_ushort); break;
    case V_ULONG:     result = _CMP_(c_ulong); break;
    case V_ULONGLONG: result = _CMP_(c_ulonglong); break;
    case V_CHAR:      result = _CMP_(c_char); break;
    case V_WCHAR:     result = _CMP_(c_wchar); break;
    case V_FLOAT:     result = _CMP_(c_float); break;
    case V_DOUBLE:    result = _CMP_(c_double); break;
    case V_VOIDP:     result = _CMP_(c_voidp); break;
    case V_OBJECT:    result = _CMP_(c_object); break;
    case V_STRING:
    case V_WSTRING:
    case V_FIXED:     p1 = (p1?*(c_voidp*)p1:NULL);
                      p2 = (p2?*(c_voidp*)p2:NULL);
                      if (((c_address)p1) == ((c_address)p2)) {
                          result = C_EQ;
                      } else if (p1 == NULL) {
                          result = C_LT;
                      } else if (p2 == NULL) {
                          result = C_GT;
                      } else {
                          r = strcmp(p1,p2);
                          if (r>0) {
                              result = C_GT;
                          } else if (r<0) {
                              result = C_LT;
                          } else {
                              result = C_EQ;
                          }
                      }
    break;
    case V_UNDEFINED:
    case V_COUNT:
        OS_REPORT_1(OS_ERROR,"c_fieldCompare failed",0,
                    "illegal field value kind (%d)", field1->kind);
        assert(FALSE);
        result = C_NE;
    break;
    }
    return result;
#undef _CMP_
}
