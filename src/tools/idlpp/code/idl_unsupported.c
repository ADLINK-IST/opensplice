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
