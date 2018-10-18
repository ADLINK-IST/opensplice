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
#ifndef IDL_GENJAVAHELPER_H
#define IDL_GENJAVAHELPER_H

#include "c_typebase.h"
#include "c_iterator.h"
#include "idl_scope.h"
#include "idl_program.h"
#include "idl_typeSpecifier.h"

extern os_iter idl_genJavaHelperPackageRedirects;

OS_CLASS (idl_packageRedirect);
#define idl_packageRedirect(o) ((idl_packageRedirect)(o))

OS_STRUCT (idl_packageRedirect) {
    os_char *module;
    os_char *package;
};

c_char *idl_javaId(const char *identifier);

c_char *idl_scopeStackJava(idl_scope scope, const char *scopeSepp, const char *name);

c_char *idl_corbaJavaTypeFromTypeSpec(idl_typeSpec typeSpec);

c_char *idl_genJavaLiteralValueImage(c_value literal, c_type type);

void idl_openJavaPackage(idl_scope scope, const char *name);

void idl_closeJavaPackage(void);

c_char *idl_labelJavaEnumVal(const char *typeEnum, idl_labelEnum labelVal);

c_char *idl_sequenceIndexString(idl_typeSeq typeSeq);

c_char *idl_arrayJavaIndexString(idl_typeArray typeArray);

c_char *idl_genJavaConstantGetter(void);

os_result idl_genJavaHelperAddPackageRedirect (const os_char *optarg);
void idl_genJavaHelperFreePackageRedirects (void);

#endif /* IDL_GENJAVAHELPER_H */
