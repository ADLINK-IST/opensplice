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
#include "idl_genLanguageHelper.h"
#include "idl_genCHelper.h"
#include "idl_genCxxHelper.h"
#include "idl_genSACSHelper.h"
#include "idl_genJavaHelper.h"

#include "os_stdlib.h"

static IDL_LANGUAGE lang = IDL_LANG_UNKNOWN;
static IDL_CORBA_MODE corba_mode = IDL_MODE_UNKNOWN;
/** @internal
 * @bug OSPL-3369 Handle both below as other modes and languages once
 * re-engineered and no side effects confirmed */
static os_boolean is_isocpp = OS_FALSE;
static os_boolean is_isocpp2 = OS_FALSE;
static os_boolean is_isocpp_types = OS_FALSE;
static os_boolean is_gen_equality = OS_FALSE;
static os_boolean is_isocpp_testmethods = OS_FALSE;
static os_boolean is_face = OS_FALSE;
static os_boolean is_isocpp_streams = OS_FALSE;
/* columns are the IDL_MODE attributes
   rows are the IDL_LANG attributes
*/
static int idl_supportedLanguageAndMode[IDL_LANG_COUNT][IDL_MODE_COUNT] = {
  /* IDL_LANG_UNKNOWN  */        {0, 0, 0},
  /* IDL_LANG_C        */        {0, 0, 1},
  /* IDL_LANG_CXX      */        {0, 1, 1},
  /* IDL_LANG_CS       */        {0, 0, 1},
  /* IDL_LANG_JAVA     */        {0, 1, 1},
  /* IDL_LANG_ISOCPP2  */        {0, 0, 1},
  /* IDL_LANG_LITE_CXX */        {0, 0, 1},
  /* IDL_LANG_C99      */        {0, 0, 1}
    };


void idl_setLanguage (
    IDL_LANGUAGE language)
{
    lang = language;
}

IDL_LANGUAGE
idl_getLanguage(void)
{
    return lang;
}

const char *
idl_getLanguageStr(void)
{
    const char *str = NULL;

    switch (lang) {
    case IDL_LANG_C:
        str = "C";
        break;
    case IDL_LANG_C99:
        str = "C99";
        break;
    case IDL_LANG_CXX:
    case IDL_LANG_LITE_CXX:
        str = "C++";
        break;
    case IDL_LANG_CS:
        str = "C#";
        break;
    case IDL_LANG_JAVA:
        str = "JAVA";
        break;
    case IDL_LANG_ISOCPP2:
        str = "ISOCPP";
        break;
    case IDL_LANG_UNKNOWN:
    default:
        str = "<unknown>";
        break;
    }
    return str;
}

void
idl_setCorbaMode(
    IDL_CORBA_MODE mode)
{
    corba_mode = mode;
}

IDL_CORBA_MODE
idl_getCorbaMode(void)
{
    return corba_mode;
}

const char *
idl_getCorbaModeStr(void)
{
    const char *str = NULL;

    /** @internal
     * @bug OSPL-3369 Have confirmed this is only used in output (where
     * it leaked previously) */
    if (idl_getIsISOCppTypes())
      return "ISO/IEC C++ 2003";

    switch (corba_mode) {
    case IDL_MODE_ORB_BOUND:
        str = "CORBA";
    break;
    case IDL_MODE_STANDALONE:
        str ="STANDALONE";
    break;
    case IDL_MODE_UNKNOWN:
    default:
        str = "<unknown>";
    break;
    }

    return str;
}

int
idl_languageAndModeSupported(void)
{
    int is_ok = idl_supportedLanguageAndMode[lang][corba_mode];
    if (is_ok)
    {
      /** @internal
       * @bug OSPL-3369 Temporary sanity check for isocpp settings */
      if (idl_getIsISOCppTypes()
            && lang != IDL_LANG_CXX
            && lang != IDL_LANG_ISOCPP2
            && corba_mode != IDL_MODE_STANDALONE)
      {
          is_ok = 0;
      }
    }
    return is_ok;
}

/* Translate an IDL identifier into a language specific identifier.
   The IDL specification states that all identifiers that match
   a language keyword must be prepended by a language specific prefix.
*/
c_char *
idl_languageId(
    const char *identifier)
{
    c_char *id = NULL;

    switch (lang) {
    case IDL_LANG_C:
    case IDL_LANG_C99:
        id = idl_cId (identifier);
    break;
    case IDL_LANG_CXX:
    case IDL_LANG_ISOCPP2:
    case IDL_LANG_LITE_CXX:
        id = idl_cxxId (identifier);
    break;
    case IDL_LANG_CS:
        id = idl_CsharpId (identifier, FALSE, FALSE);
    break;
    case IDL_LANG_JAVA:
        id = idl_javaId (identifier);
    break;
    case IDL_LANG_UNKNOWN:
    default:
    break;
    }
    return id;
}

/* Build a textual presenation of the provided scope stack taking the
   language keyword identifier translation into account. Further the function
   equals "idl_scopeStack".
*/
c_char *
idl_scopeStackLanguage(
    idl_scope scope,
    const char *name)
{
    c_char *id = NULL;

    switch (lang) {
    case IDL_LANG_C:
    case IDL_LANG_C99:
        id = idl_scopeStackC (scope, "_", name);
    break;
    case IDL_LANG_CXX:
    case IDL_LANG_ISOCPP2:
        id = idl_scopeStackCxx (scope, "::", name);
    break;
    case IDL_LANG_CS:
        id = idl_scopeStackCsharp(scope, ".", name);
    break;
    case IDL_LANG_JAVA:
        id = idl_scopeStackJava (scope, ".", name);
    break;
    case IDL_LANG_UNKNOWN:
    default:
    break;
    }
    return id;
}

/* Return the Language specific type identifier for the
   specified type specification
*/
c_char *
idl_corbaLanguageTypeFromTypeSpec (
    idl_typeSpec typeSpec)
{
    c_char *id = NULL;

    switch (lang) {
    case IDL_LANG_C:
    case IDL_LANG_C99:
        id = idl_corbaCTypeFromTypeSpec (typeSpec);
    break;
    case IDL_LANG_CXX:
    case IDL_LANG_ISOCPP2:
    case IDL_LANG_LITE_CXX:
        id = idl_corbaCxxTypeFromTypeSpec (typeSpec);
    break;
    case IDL_LANG_JAVA:
        id = idl_corbaJavaTypeFromTypeSpec (typeSpec);
    break;
    case IDL_LANG_UNKNOWN:
    default:
    break;
    }
    return id;
}

/* Return the language specific constant value getter */
c_char *
idl_genLanguageConstGetter(void)
{
    c_char *getter = NULL;

    switch (lang) {
    case IDL_LANG_C:
    case IDL_LANG_C99:
        getter = idl_genCConstantGetter();
    break;
    case IDL_LANG_CXX:
    case IDL_LANG_ISOCPP2:
    case IDL_LANG_LITE_CXX:
        getter = idl_genCxxConstantGetter();
    break;
    case IDL_LANG_CS:
        getter = idl_genCsharpConstantGetter();
    break;
    case IDL_LANG_JAVA:
        getter = idl_genJavaConstantGetter();
    break;
    case IDL_LANG_UNKNOWN:
    default:
    break;
    }

    return getter;
}

void
idl_setIsISOCpp(os_boolean itIs)
{
    is_isocpp = itIs;
}

os_boolean
idl_getIsISOCpp()
{
    return is_isocpp;
}

void
idl_setIsISOCpp2(os_boolean itIs)
{
    is_isocpp2 = itIs;
}

os_boolean
idl_getIsISOCpp2()
{
    return is_isocpp2;
}

void
idl_setIsISOCppTypes(os_boolean itIs)
{
    if(!idl_getIsISOCppStreams())
        is_isocpp_types = itIs;
}

os_boolean
idl_getIsISOCppTypes()
{
    return !idl_getIsISOCppStreams() ? is_isocpp_types : OS_FALSE;
}

void
idl_setIsGenEquality(os_boolean itIs)
{
    is_gen_equality = itIs;
}

os_boolean
idl_getIsGenEquality()
{
    return is_gen_equality;
}

void
idl_setIsISOCppTestMethods(os_boolean itIs)
{
    is_isocpp_testmethods = itIs;
}

os_boolean
idl_getIsISOCppTestMethods()
{
    return is_isocpp_testmethods;
}

void
idl_setIsFace(os_boolean itIs)
{
    is_face = itIs;
}

os_boolean
idl_getIsFace()
{
    return is_face;
}

void
idl_setIsISOCppStreams(os_boolean itIs)
{
    is_isocpp_streams = itIs;
}

os_boolean
idl_getIsISOCppStreams()
{
    return is_isocpp_streams;
}
