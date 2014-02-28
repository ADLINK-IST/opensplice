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
#ifndef IDL_GENJAVAHELPER_H
#define IDL_GENJAVAHELPER_H

#include "c_typebase.h"
#include "c_iterator.h"
#include "idl_scope.h"
#include "idl_program.h"
#include "idl_typeSpecifier.h"

c_char *idl_javaId(const char *identifier);

c_char *idl_scopeStackJava(idl_scope scope, const char *scopeSepp, const char *name);

c_char *idl_corbaJavaTypeFromTypeSpec(idl_typeSpec typeSpec);

void idl_openJavaPackage(idl_scope scope, const char *name);

void idl_closeJavaPackage(void);

c_char *idl_labelJavaEnumVal(const char *typeEnum, idl_labelEnum labelVal);

c_char *idl_sequenceIndexString(idl_typeSeq typeSeq);

c_char *idl_arrayJavaIndexString(idl_typeArray typeArray);

c_char *idl_genJavaConstantGetter(void);

const os_char*
idl_genJavaHelperGetTgtLastSubstituted(
    void);

const os_char*
idl_genJavaHelperGetOrgLastSubstituted(
    void);

void
idl_genJavaHelperInit(
    c_iter originalPackageName,
    c_iter targetPackageName);

#endif /* IDL_GENJAVAHELPER_H */
