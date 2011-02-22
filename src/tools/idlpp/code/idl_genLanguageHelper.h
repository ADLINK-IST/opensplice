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
    IDL_LANG_COUNT /* should always be the last in the enumeration!!! */
} IDL_LANGUAGE;

typedef enum {
    IDL_MODE_UNKNOWN,
    IDL_MODE_ORB_BOUND,
    IDL_MODE_STANDALONE,
    IDL_MODE_COUNT /* should always be the last in the enumeration!!! */
} IDL_CORBA_MODE;

void idl_setLanguage(IDL_LANGUAGE language);
IDL_LANGUAGE idl_getLanguage(void);
char *idl_getLanguageStr(void);

void idl_setCorbaMode(IDL_CORBA_MODE mode);
IDL_CORBA_MODE idl_getCorbaMode(void);
char *idl_getCorbaModeStr(void);

int idl_languageAndModeSupported(void);

c_char *idl_languageId(const char *identifier);
c_char *idl_scopeStackLanguage(idl_scope scope, const char *name);
c_char *idl_corbaLanguageTypeFromTypeSpec(idl_typeSpec typeSpec);
c_char *idl_genLanguageConstGetter(void);

#endif /* IDL_GENLANGUAGEHELPER_H */
