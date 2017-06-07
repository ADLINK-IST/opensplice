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
#ifndef IDL_GENISOCXX2HELPER_H
#define IDL_GENISOCXX2HELPER_H

#include "c_typebase.h"

#include "idl_scope.h"
#include "idl_program.h"

c_char *idl_ISOCxx2Id(const char *identifier);

c_char *idl_scopeStackISOCxx2(idl_scope scope, const char *scopeSepp, const char *name);

c_char *idl_ISOCxx2TypeFromTypeSpec(idl_typeSpec typeSpec);

c_char *idl_ISOCxx2TypeFromCType(c_type t);

c_char *idl_ISOCxx2DefaultValueFromCType(c_type t);

c_char *idl_ISOCxx2ValueFromCValue(c_type t, c_value v);

c_char *idl_ISOCxx2InTypeFromCType(c_type t);

c_bool idl_ISOCxx2IsRefType(c_type t);

c_value idl_ISOCxx2LowestUnionDefaultValue(c_type t);

#endif /* IDL_GENISOCXX2HELPER_H */
