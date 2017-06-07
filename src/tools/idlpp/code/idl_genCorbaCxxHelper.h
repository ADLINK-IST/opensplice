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
#ifndef IDL_GENCORBACXXHELPER_H
#define IDL_GENCORBACXXHELPER_H

#include "idl_program.h"
#include "c_metabase.h"
#include "c_iterator.h"

struct CxxTypeUserData_s {
    os_iter idlpp_metaList;
    os_boolean no_type_caching;
    c_iter typeStack;
};

typedef struct CxxTypeUserData_s CxxTypeUserData;

typedef struct {
    c_type type;
    c_char *descriptor;
    c_ulong nrElements;
    size_t descriptorLength;
} idl_metaCxx;

idl_metaCxx *
idl_metaCxxNew(c_type type, c_char *descriptor);

void
idl_metaCxxSerialize2XML(
        void *_metaElmnt,
        void *args);

void idl_metaCxxAddType(
        idl_scope scope,
        const char *name,
        idl_typeSpec typeSpec,
        os_iter *metaList);

idl_program idl_genCorbaCxxHelperProgram (void);

#endif /* IDL_GENCORBACXXHELPER_H */
