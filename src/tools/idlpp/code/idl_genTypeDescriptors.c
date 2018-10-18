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
#include "idl_genTypeDescriptors.h"
#include "idl_genCorbaCxxHelper.h"
#include "idl_keyDef.h"

static idl_action
idl_fileOpen (
    idl_scope scope,
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    return idl_explore;
}

static void
idl_fileClose(
    void *userData)
{
    CxxTypeUserData *cxxUserData = (CxxTypeUserData *)userData;

    /* Generate the type descriptors for each type while not holding any locks yet. */
    if (cxxUserData->idlpp_metaList)
    {
        os_iterWalk(cxxUserData->idlpp_metaList, idl_metaCxxSerialize2XML, NULL);
    }
}

static idl_action
idl_moduleOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    return idl_explore;
}

static idl_action
idl_structureOpen(
    idl_scope scope,
    const char *name,
    idl_typeStruct structSpec,
    void *userData)
{
    CxxTypeUserData *cxxUserData = (CxxTypeUserData *)userData;

    /* QAC EXPECT 3416; No side effects here */
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        /* keylist defined for this struct */
        /* Store data-type in iterator for future generation of type descriptor. */
        idl_metaCxxAddType(scope, name, idl_typeSpec(structSpec), &cxxUserData->idlpp_metaList);
    }
    return idl_abort;
}

static struct idl_program idl_genCxxTypedClassDefs;

idl_program
idl_genTypeDescriptorsProgram(
        void *userData)
{
    idl_genCxxTypedClassDefs.idl_getControl = NULL;
    idl_genCxxTypedClassDefs.fileOpen = idl_fileOpen;
    idl_genCxxTypedClassDefs.fileClose = idl_fileClose;
    idl_genCxxTypedClassDefs.moduleOpen = idl_moduleOpen;
    idl_genCxxTypedClassDefs.moduleClose = NULL;
    idl_genCxxTypedClassDefs.structureOpen = idl_structureOpen;
    idl_genCxxTypedClassDefs.structureClose = NULL;
    idl_genCxxTypedClassDefs.structureMemberOpenClose = NULL;
    idl_genCxxTypedClassDefs.enumerationOpen = NULL;
    idl_genCxxTypedClassDefs.enumerationClose = NULL;
    idl_genCxxTypedClassDefs.enumerationElementOpenClose = NULL;
    idl_genCxxTypedClassDefs.unionOpen = NULL;
    idl_genCxxTypedClassDefs.unionClose = NULL;
    idl_genCxxTypedClassDefs.unionCaseOpenClose = NULL;
    idl_genCxxTypedClassDefs.unionLabelsOpenClose = NULL;
    idl_genCxxTypedClassDefs.unionLabelOpenClose = NULL;
    idl_genCxxTypedClassDefs.typedefOpenClose = NULL;
    idl_genCxxTypedClassDefs.boundedStringOpenClose = NULL;
    idl_genCxxTypedClassDefs.sequenceOpenClose = NULL;
    idl_genCxxTypedClassDefs.constantOpenClose = NULL;
    idl_genCxxTypedClassDefs.artificialDefaultLabelOpenClose = NULL;
    idl_genCxxTypedClassDefs.userData =  userData;

    return &idl_genCxxTypedClassDefs;
}
