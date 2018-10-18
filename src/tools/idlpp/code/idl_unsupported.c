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
#include "idl_unsupported.h"

static c_metaObject _objects[IDL_UNSUP_COUNT];

int
idl_defineUnsupportedTypes(
    c_base base)
{
    c_bool noError;

    noError = TRUE;

#define _UNSUPPORTEDTYPE_(type) if (noError) {\
        _objects[type] = c_metaDeclare(c_metaObject(base), type##_NAME , M_PRIMITIVE);\
        if (_objects[type]) {\
            c_primitive(_objects[type])->kind = P_LONG;\
            c_metaFinalize(_objects[type]);\
        } else {\
            fprintf(stderr, "ERROR: failed to declare %s\n", #type);\
            noError = FALSE;\
        } \
    }

    _UNSUPPORTEDTYPE_(IDL_UNSUP_ANY)
    _UNSUPPORTEDTYPE_(IDL_UNSUP_FIXED)
    _UNSUPPORTEDTYPE_(IDL_UNSUP_LONGDOUBLE)
    _UNSUPPORTEDTYPE_(IDL_UNSUP_OBJECT)
    _UNSUPPORTEDTYPE_(IDL_UNSUP_VALUETYPE)
    _UNSUPPORTEDTYPE_(IDL_UNSUP_VALUEBASE)
    _UNSUPPORTEDTYPE_(IDL_UNSUP_WCHAR)
    _UNSUPPORTEDTYPE_(IDL_UNSUP_WSTRING)

    return (noError==TRUE?0:1);
}

c_type
idl_unsupportedTypeGetMetadata(
    enum idl_unsupported_type unsup)
{
    return c_type(c_keep(_objects[unsup]));
}

const c_char *
idl_unsupportedTypeActualName(
    const c_char *name)
{
    return &name[strlen(IDL_UNSUP_PREFIX)];
}
