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
/** \file services/serialization/code/sd_misc.c
 *  \brief Implementation of functions for common use by all serializer
 *         descendants.
 */

/* Interface */
#include "sd_misc.h"
/* Implementation */
#include "os_abstract.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "c_base.h"
#include "sd__confidence.h"
#include "sd_stringsXML.h"

/** \brief Function for selecting the correct switch value for a given
 *         union object
 */
static c_value
sd_unionDetermineSwitchValue(
    c_type switchType,
    c_object object)
{
    c_value switchValue;

    switch (c_baseObject(switchType)->kind) {
    case M_PRIMITIVE:
        switch (c_primitive(switchType)->kind) {
#define __CASE__(prim, type) case prim: switchValue = type##Value(*((type *)object)); break;
        __CASE__(P_ADDRESS,c_address)
        __CASE__(P_BOOLEAN,c_bool)
        __CASE__(P_CHAR,c_char)
        __CASE__(P_SHORT,c_short)
        __CASE__(P_USHORT,c_ushort)
        __CASE__(P_LONG,c_long)
        __CASE__(P_ULONG,c_ulong)
        __CASE__(P_LONGLONG,c_longlong)
        __CASE__(P_ULONGLONG,c_ulonglong)
#undef __CASE__
        default:
            switchValue = c_undefinedValue();
            SD_CONFIDENCE(FALSE);
        break;
        }
    break;
    case M_ENUMERATION:
        switchValue = c_longValue(*(c_long *)object);
    break;
    default:
        switchValue = c_undefinedValue();
        SD_CONFIDENCE(FALSE);
    break;
    }

    return switchValue;
/* QAC EXPECT 5101; cyclomatic complexity is high because of switch statement */
}


/** \brief Function for selecting the correct case label for a given switch
 *         value
 */
static c_unionCase
sd_unionDetermineLabel(
    c_union v_union,
    c_value switchValue)
{
    c_unionCase result = NULL;
    c_unionCase deflt;
    c_unionCase currentCase;
    c_literal label;
    int i,j, nLabels;

    /* Determine corresponding label */
    result = NULL;
    deflt = NULL;
    for (i=0; (i<c_arraySize(v_union->cases)) && !result; i++) {
        currentCase = c_unionCase(v_union->cases[i]);
        nLabels = c_arraySize(currentCase->labels);
        if (nLabels > 0) {
            for (j=0; (j<nLabels) && !result; j++) {
                label = c_literal(currentCase->labels[j]);

                if (c_valueCompare(switchValue, label->value) == C_EQ) {
                    result = currentCase;
                }
            }
        } else {
            deflt = currentCase;
        }
    }

    if (!result) {
        result = deflt;
    }

    return result;
}


/** \brief Helper function for determining the active case label of a union by
 *         inspecting the value of the switch field and comparing its values
 *         to those of the case labels.
 */

c_unionCase
sd_unionDetermineActiveCase(
    c_union v_union,
    c_object object)
{
    c_value switchValue;
    c_type switchType;

    switchType = c_typeActualType(v_union->switchType);
    /* Determine value of the switch field */
    switchValue = sd_unionDetermineSwitchValue(switchType, object);

    /* Determine the label corresponding to this field */
    return sd_unionDetermineLabel(v_union, switchValue);
}


/** \brief Function for determining the scoped typename of an object. */

#define SD_TYPENAME_ANON "anonymous"
c_char *
sd_getScopedTypeName(
    c_type type,
    const c_char *moduleSep)
{
    c_string typeName;
    c_metaObject module;
    c_string moduleName;
    c_ulong nameLen;
    c_char *result;
    c_type actualType;

    actualType = type;
    typeName = c_metaName(c_metaObject(actualType));
    if (typeName) {
        module = c_metaModule(c_metaObject(actualType));
        if (module) {
            moduleName = c_metaName(c_metaObject(module));
            nameLen = (moduleName ? strlen(moduleName) : 0) +
                      (moduleName ? strlen(moduleSep) : 0) +
                      strlen(typeName) +
                      1U /* '\0' */;
            result = (c_char *)os_malloc(nameLen);
            snprintf(result, nameLen, "%s%s%s",
                    moduleName ? moduleName : "",
                    moduleName ? moduleSep : "",
                    typeName);
            c_free(moduleName);
            c_free(module);
        } else {
            result = sd_stringDup(typeName);
        }
        c_free(typeName);
    } else {
        result = sd_stringDup(SD_TYPENAME_ANON);
    }

    return result;
}
#undef SD_TYPENAME_ANON


/** \brief Commonly used function for copying a string using os_malloc for
 *         allocation
 */
char *
sd_stringDup(
    const char *string)
{
    unsigned int size;
    char *result = NULL;

    if (string) {
        size = strlen(string);
        size++; /* '\0'*/
        result = os_malloc(size);
        if (result) {
             os_strncpy(result, string, size);
        }
    }

    return result;
}

c_bool
sd_stringToLong (
    const c_char *str,
    c_long       *retval)
{
    c_bool result = FALSE;
    c_char *endp;

    *retval = strtol(str, &endp, 10);
    if ( endp != str ) {
        result = TRUE;
    }

    return result;
}

c_bool
sd_stringToLongLong (
    const c_char *str,
    c_longlong   *retval)
{
    c_bool result = FALSE;
    c_char *endp;

    *retval = os_strtoll(str, &endp, 10);
    if ( endp != str ) {
        result = TRUE;
    }

    return result;
}

c_bool
sd_stringToBoolean (
    const c_char *str,
    c_bool       *retval)
{
    c_bool result = FALSE;

    if ( strcmp(str, "False") == 0 ) {
        result = TRUE;
        *retval = FALSE;
    } else if ( strcmp(str, "True") == 0 ) {
        result = TRUE;
        *retval = TRUE;
    } else {
        result = FALSE;
    }

    return result;
}

c_bool
sd_stringToAddress(
    const c_char *str,
    c_address    *retval)
{
    c_bool result = FALSE;
    int scanCount;

    scanCount = sscanf(str, PA_ADDRFMT, retval);
    if (scanCount == 1) {
        result = TRUE;
    }

    return result;
}
