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
#ifndef IDL_UNSUPPORTED_H_
#define IDL_UNSUPPORTED_H_

#include "c_base.h"

#define IDL_UNSUP_PREFIX                "#idl_unsup_"
#define IDL_UNSUP_ANY_NAME              IDL_UNSUP_PREFIX "any"
#define IDL_UNSUP_FIXED_NAME            IDL_UNSUP_PREFIX "fixed"
#define IDL_UNSUP_LONGDOUBLE_NAME       IDL_UNSUP_PREFIX "long double"
#define IDL_UNSUP_OBJECT_NAME           IDL_UNSUP_PREFIX "Object"
#define IDL_UNSUP_VALUETYPE_NAME        IDL_UNSUP_PREFIX "valuetype"
#define IDL_UNSUP_VALUEBASE_NAME        IDL_UNSUP_PREFIX "ValueBase"
#define IDL_UNSUP_WCHAR_NAME            IDL_UNSUP_PREFIX "wchar"
#define IDL_UNSUP_WSTRING_NAME          IDL_UNSUP_PREFIX "wstring"

enum idl_unsupported_type {
    IDL_UNSUP_UNDEFINED,
    IDL_UNSUP_ANY,
    IDL_UNSUP_FIXED,
    IDL_UNSUP_LONGDOUBLE,
    IDL_UNSUP_OBJECT,
    IDL_UNSUP_VALUETYPE,
    IDL_UNSUP_VALUEBASE,
    IDL_UNSUP_WCHAR,
    IDL_UNSUP_WSTRING,
    IDL_UNSUP_COUNT
};

int idl_defineUnsupportedTypes(c_base base);
c_type idl_unsupportedTypeGetMetadata(enum idl_unsupported_type unsup);
const c_char *idl_unsupportedTypeActualName(const c_char *name);
 
#endif /*IDL_UNSUPPORTED_H_*/
