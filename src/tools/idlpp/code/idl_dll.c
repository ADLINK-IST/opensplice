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
#include "idl_dll.h"
#include "c_stringSupport.h"
#include "os_heap.h"
#include "os_stdlib.h"

static os_char *idl_dllMacro = (os_char *)0;
static os_char *idl_dllHeader = (os_char *)0;
static os_char *idl_dllHeaderFile = (os_char *)0;

#define IDL_DLL_INCL_STR "#include \"%s\""
os_int32
idl_dllSetOption(
    const char *option)
{
    c_iter ieOpt;
    os_int32 result;
    os_char *incname;

    assert(option);

    ieOpt = c_splitString(option, ",");
    if (ieOpt) {
        if (c_iterLength(ieOpt) < 3) {
            idl_dllMacro  = (os_char *)c_iterTakeFirst(ieOpt);
            incname = (os_char *)c_iterTakeFirst(ieOpt);
            if (incname) {
                /* NULL terminator is covered by %s */
                idl_dllHeader = (os_char *)os_malloc(strlen(IDL_DLL_INCL_STR)+strlen(incname));
                os_sprintf(idl_dllHeader, IDL_DLL_INCL_STR, incname);
                idl_dllHeaderFile = incname;/* takes ownership */
            } else {
                /* else incname is optional */
                if (idl_dllHeader) {
                    os_free(idl_dllHeader);
                }
                idl_dllHeader = (os_char*)os_strdup("");
            }
            result = 0;
        } else {
            result = 1;
        }
        c_iterFree(ieOpt);
    } else {
        if (idl_dllMacro) {
            os_free(idl_dllMacro);
        }
        idl_dllMacro  = (os_char*)os_strdup("");
        if (idl_dllHeader) {
            os_free(idl_dllHeader);
        }
        idl_dllHeader = (os_char*)os_strdup("");
        result = 1;
    }
    return result;
}
#undef IDL_DLL_INCL_STR

const os_char *
idl_dllGetMacro(void)
{
    return (const os_char *)idl_dllMacro;
}

const os_char *
idl_dllGetHeader(void)
{
    return (const os_char *)idl_dllHeader;
}

const os_char *
idl_dllGetHeaderFile(void)
{
    return (const os_char *)idl_dllHeaderFile;
}

void
idl_dllInitialize(void)
{
    idl_dllMacro  = (os_char*)os_strdup("");
    idl_dllHeader = (os_char*)os_strdup("");
}

void
idl_dllExit(void)
{
    if (idl_dllMacro) {
        os_free(idl_dllMacro);
    }
    idl_dllMacro  = (os_char*)0;

    if (idl_dllHeader) {
        os_free(idl_dllHeader);
    }
    idl_dllHeader = (os_char*)0;
}
