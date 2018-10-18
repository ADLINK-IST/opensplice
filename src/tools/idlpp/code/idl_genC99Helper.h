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
#ifndef IDL_GENC99HELPER_H
#define IDL_GENC99HELPER_H

#include "c_typebase.h"

#include "idl_program.h"

#include "idl_genSacHelper.h"

c_char *idl_scopedC99TypeIdent (const idl_typeSpec typeSpec);

c_char *idl_c99TypeFromTypeSpec (const idl_typeSpec typeSpec);

char *idl_c99SequenceElementIdent (const idl_typeSpec typeSpec);

char *idl_c99SequenceIdent (const idl_typeSeq typeSeq);

char *idl_c99SequenceIdentScoped (const idl_scope scope, const idl_typeSeq typeSeq);

c_bool idl_c99SequenceSupportFunctionsExist (const idl_scope scope,
                                             const idl_typeSeq typeSeq, const char *elementName);

#endif /* IDL_GENC99HELPER_H */
