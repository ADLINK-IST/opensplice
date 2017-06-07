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
#ifndef IDL_PROGRAM_H
#define IDL_PROGRAM_H

#include "idl_scope.h"
#include "idl_constSpecifier.h"

#include "os_iterator.h"

typedef enum {
    idl_prior,
    idl_inline
} idl_structuredScopeWalk;

typedef struct {
    idl_structuredScopeWalk structScopeWalk;
} idl_programControl;

typedef struct idl_program {
    idl_programControl *(* idl_getControl)(void *userData);
    idl_action (* fileOpen)(idl_scope ownScope, const char *name, void *userData);
    void (* fileClose)(void *userData);
    idl_action (* moduleOpen)(idl_scope ownScope, const char *name, void *userData);
    void (* moduleClose)(void *userData);
    idl_action (* structureOpen)(idl_scope ownScope, const char *name, idl_typeStruct structSpec, void *userData);
    void (* structureClose)(const char *name, void *userData);
    void (* structureMemberOpenClose)(idl_scope ownScope, const char *name, idl_typeSpec typeSpec, void *userData);
    idl_action (* enumerationOpen)(idl_scope ownScope, const char *name, idl_typeEnum enumSpec, void *userData);
    void (* enumerationClose)(const char *name, void *userData);
    void (* enumerationElementOpenClose)(idl_scope ownScope, const char *name, void *userData);
    idl_action (* unionOpen)(idl_scope ownScope, const char *name, idl_typeUnion unionSpec, void *userData);
    void (* unionClose)(const char *name, void *userData);
    void (* unionCaseOpenClose)(idl_scope ownScope, const char *name, idl_typeSpec typeSpec, void *userData);
    void (* unionLabelsOpenClose)(idl_scope ownScope, idl_labelSpec labelSpec, void *userData);
    void (* unionLabelOpenClose)(idl_scope ownScope, idl_labelVal labelVal, void *userData);
    void (* typedefOpenClose)(idl_scope ownScope, const char *name, idl_typeDef typeDef, void *userData);
    void (* boundedStringOpenClose)(idl_scope ownScope, idl_typeBasic typeBasic, void *userData);
    void (* sequenceOpenClose)(idl_scope ownScope, idl_typeSeq typeSeq, void *userData);
    void (* constantOpenClose)(idl_scope ownScope, idl_constSpec constantSpec, void *userData);
    void (* artificialDefaultLabelOpenClose)(idl_scope ownScope, idl_labelVal labelVal, idl_typeSpec typeSpec, void *userData);
    void *userData;
} *idl_program;

#endif /* IDL_PROGRAM_H */
