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
#ifndef IDL_GENLANGUAGEHELPER_H
#define IDL_GENLANGUAGEHELPER_H

#include "c_typebase.h"

#include "idl_scope.h"
#include "idl_program.h"

typedef enum {
    IDL_LANG_UNKNOWN,
    IDL_LANG_C,
    IDL_LANG_CXX,
    IDL_LANG_CS,
    IDL_LANG_JAVA,
    IDL_LANG_ISOCPP2,
    /** IDL_LANG_ISOCPP, I am not adding this yet. I wish to re-use C++ with tweaks @bug OSPL-3369 */
    IDL_LANG_LITE_CXX,
    IDL_LANG_C99,
    IDL_LANG_COUNT /* should always be the last in the enumeration!!! */
} IDL_LANGUAGE;

typedef enum {
    IDL_MODE_UNKNOWN,
    IDL_MODE_ORB_BOUND,
    IDL_MODE_STANDALONE,
    /** IDL_MODE_NEW,  I am not adding this yet. I wish to re-use C++ with tweaks @bug OSPL-3369
                      Also: I don't like the designation 'New' but we're using vanilla
                     get_opt and all the good characters are taken.
                     This way we can re-use for Java 5 PSM */
    IDL_MODE_COUNT /* should always be the last in the enumeration!!! */
} IDL_CORBA_MODE;


typedef struct idl_program_args {
    os_boolean no_type_caching;
} idl_program_args;

void idl_setLanguage(IDL_LANGUAGE language);
IDL_LANGUAGE idl_getLanguage(void);
const char *idl_getLanguageStr(void);

void idl_setCorbaMode(IDL_CORBA_MODE mode);
IDL_CORBA_MODE idl_getCorbaMode(void);
const char *idl_getCorbaModeStr(void);

int idl_languageAndModeSupported(void);

c_char *idl_languageId(const char *identifier);
c_char *idl_scopeStackLanguage(idl_scope scope, const char *name);
c_char *idl_corbaLanguageTypeFromTypeSpec(idl_typeSpec typeSpec);
c_char *idl_genLanguageConstGetter(void);

void idl_setIsISOCpp(os_boolean itIs);
os_boolean idl_getIsISOCpp(void);

void idl_setIsISOCpp2(os_boolean itIs);
os_boolean idl_getIsISOCpp2();

void idl_setIsISOCppTypes(os_boolean itIs);
os_boolean idl_getIsISOCppTypes(void);

void idl_setIsGenEquality(os_boolean itIs);
os_boolean idl_getIsGenEquality(void);

void idl_setIsISOCppTestMethods(os_boolean itIs);
os_boolean idl_getIsISOCppTestMethods(void);

void idl_setIsISOCppStreams(os_boolean itIs);
os_boolean idl_getIsISOCppStreams(void);

void idl_setIsFace(os_boolean itIs);
os_boolean idl_getIsFace(void);


#endif /* IDL_GENLANGUAGEHELPER_H */
