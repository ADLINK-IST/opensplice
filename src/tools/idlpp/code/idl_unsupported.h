/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
