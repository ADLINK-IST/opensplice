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
#include "idl_program.h"
#include "idl_scope.h"
#include "idl_genCxxTypedClassImpl.h"
#include "idl_genCxxHelper.h"
#include "idl_genCorbaCxxHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_genMetaHelper.h"
#include "idl_tmplExp.h"
#include "idl_keyDef.h"
#include "idl_dll.h"

#include "os_heap.h"
#include "os_stdlib.h"
#include "os_abstract.h"
#include "c_typebase.h"

static idl_macroAttrib idlpp_macroAttrib;
static idl_streamIn idlpp_inStream;
static c_char *idlpp_template;
static idl_macroSet idlpp_macroSet;
static c_long idlpp_indent_level = 0;

#define MAX_ULONG_STRLENGTH 10 /* Room for 9 digits, enough for 32-bit range. */

/* QAC EXPECT 0285; Need dollar here, this is specified */
#define IDL_TOKEN_START         '$'
#define IDL_TOKEN_OPEN          '('
#define IDL_TOKEN_CLOSE         ')'

static idl_action
idl_fileOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    idl_tmplExp te;
    c_char tmplFileName[1024];
    c_char *tmplPath;
    c_char *orbPath = NULL;
    int tmplFile;
    struct os_stat_s tmplStat;
    unsigned int nRead;
    CxxTypeUserData *cxxUserData = (CxxTypeUserData *)userData;;
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    tmplPath = os_getenv("OSPL_TMPL_PATH");
    if (tmplPath == NULL) {
        printf("OSPL_TMPL_PATH not defined\n");
        return (idl_abort);
    }

    /* Prepare file header template */
    if (idl_getLanguage() == IDL_LANG_LITE_CXX) {
        snprintf(tmplFileName, sizeof(tmplFileName), "%s%cliteCxxClassBodyHeader", tmplPath, OS_FILESEPCHAR);
    } else {
        orbPath = os_getenv("OSPL_ORB_PATH");
        if (orbPath == NULL) {
            printf("OSPL_ORB_PATH not defined\n");
            return (idl_abort);
        }
        snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%ccorbaCxxClassBodyHeader", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
    }

    /* QAC EXPECT 3416; No unexpected side effects here */
    if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
        (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
        printf("No template found or protection violation (%s)\n", tmplFileName);
        return (idl_abort);
    }
    /* QAC EXPECT 5007; will not use wrapper */
    idlpp_template = os_malloc(tmplStat.stat_size+1);
    tmplFile = open(tmplFileName, O_RDONLY);
    nRead = (unsigned int)read(tmplFile, idlpp_template, tmplStat.stat_size);
    memset(&idlpp_template[nRead], 0, tmplStat.stat_size+1-nRead);
    close(tmplFile);
    idlpp_macroAttrib = idl_macroAttribNew(IDL_TOKEN_START, IDL_TOKEN_OPEN, IDL_TOKEN_CLOSE);
    idlpp_macroSet = idl_macroSetNew();
    idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
    /* Expand file header */
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("basename", name));
    /* set dll stuff */
    idl_macroSetAdd(idlpp_macroSet,
        idl_macroNew(IDL_DLL_TMPLMACRO_MACRO_NAME, idl_dllGetMacro()));
    idl_macroSetAdd(idlpp_macroSet,
                idl_macroNew(IDL_DLL_TMPLMACRO_HEADER_NAME, idl_dllGetHeader()));
    /** @internal
     * @bug This is already 'wrong' for unions (and typedefs?) thereof
     * in the 'old' mapping. They are classes. Would try to fix but typedefs
     * queer the pitch. Compile union.idl with VS(2012?) to see warning */
    /* Macro for either keyword class or struct */
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("class_or_struct",
                idl_getIsISOCpp() && idl_getIsISOCppTypes() ? "class" : "struct"));

    te = idl_tmplExpNew(idlpp_macroSet);
    idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
    idl_streamInFree(idlpp_inStream);
    idl_tmplExpFree(te);

    if(idl_getIsISOCpp())
    {
      idl_fileOutPrintf (idl_fileCur(), "#include <org/opensplice/core/EntityRegistry.hpp>\n\n\n");
    }

    /* Prepare class definition template */
    if (idl_getLanguage() == IDL_LANG_LITE_CXX) {
        snprintf(tmplFileName, sizeof(tmplFileName), "%s%cliteCxxClassBody", tmplPath, OS_FILESEPCHAR);
    } else {
        snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%ccorbaCxxClassBody", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
    }
    /* QAC EXPECT 3416; No unexpected side effects here */
    if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
        (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
        printf("No template found or protection violation (%s)\n", tmplFileName);
        return (idl_abort);
    }
    /* QAC EXPECT 5007; will not use wrapper */
    idlpp_template = os_malloc(tmplStat.stat_size+1);
    tmplFile = open(tmplFileName, O_RDONLY);
    nRead = (unsigned int)read(tmplFile, idlpp_template, tmplStat.stat_size);
    memset(&idlpp_template[nRead], 0, tmplStat.stat_size+1-nRead);
    close(tmplFile);

    /* Generate the type descriptors for each type while not holding any locks yet. */
    if (cxxUserData->idlpp_metaList)
    {
        os_iterWalk(cxxUserData->idlpp_metaList, idl_metaCxxSerialize2XML, NULL);
    }

    idlpp_indent_level = 0;

    return idl_explore;
    /* QAC EXPECT 2006; overview does not get better with one exit */
}

static void
idl_fileClose(
    void *userData)
{
    OS_UNUSED_ARG(userData);
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
    c_char spaces[128];
    CxxTypeUserData *cxxUserData = (CxxTypeUserData *)userData;
    idl_tmplExp te;
    os_char intToStr[MAX_ULONG_STRLENGTH];
    OS_UNUSED_ARG(structSpec);
    OS_UNUSED_ARG(userData);

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        char *scopedTypeName = idl_scopeStack(scope, "::", name);
        const char *internalTypeName = idl_internalTypeNameForBuiltinTopic(scopedTypeName);
        const char *keyList = idl_keyResolve(idl_keyDefDefGet(), scope, name);
        if ((strlen(internalTypeName) != 0) &&
            ((keyList == NULL) ||
             (strcmp(keyList,"key") == 0))) {
            keyList = "key.localId,key.systemId";
        }

        /* keylist defined for this struct */
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("namescope", idl_scopeStackCxx(scope, "::", NULL)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", idl_cxxId(name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scopedtypename", scopedTypeName));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("internaltypename", internalTypeName));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("uniquetypename", idl_scopeStack(scope, "_", name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("keyList", keyList));

        if (cxxUserData->idlpp_metaList) {
            idl_metaCxx *metaElmnt = os_iterTakeFirst(cxxUserData->idlpp_metaList);
            idl_macroSetAdd(idlpp_macroSet, idl_macroNew("meta-descriptor", metaElmnt->descriptor));
            snprintf(intToStr, MAX_ULONG_STRLENGTH, "%u", metaElmnt->nrElements);
            idl_macroSetAdd(idlpp_macroSet, idl_macroNew("meta-descriptorArrLength", intToStr));
            snprintf(intToStr, MAX_ULONG_STRLENGTH, "%" PA_PRIuSIZE, metaElmnt->descriptorLength);
            idl_macroSetAdd(idlpp_macroSet, idl_macroNew("meta-descriptorLength", intToStr));
        }

        snprintf(spaces, sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("spaces", spaces));
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);

        os_free(scopedTypeName);

        if(idl_getIsISOCpp())
        {
          if (idl_keyResolve(idl_keyDefDefGet(), idl_typeUserScope(idl_typeUser(structSpec)), name) != NULL)
          {
              char* sampleType;

              sampleType = idl_corbaCxxTypeFromTypeSpec(idl_typeSpec(structSpec));
              idl_fileOutPrintf(idl_fileCur(),
                                  "\n\nINSTANTIATE_TYPED_REGISTRIES(%s)\n\n",
                                      sampleType);
              os_free(sampleType);
          }
        }
    }

    /* No need to traverse individual members of this struct. */
    return idl_abort;
}

static idl_action
idl_unionOpen(
    idl_scope scope,
    const char *name,
    idl_typeUnion unionSpec,
    void *userData)
{
    c_char spaces[20];
    idl_tmplExp te;
    OS_UNUSED_ARG(unionSpec);
    OS_UNUSED_ARG(userData);

    OS_UNUSED_ARG(unionSpec);
    OS_UNUSED_ARG(userData);

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        /* keylist defined for this union */
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("namescope", idl_scopeStackCxx(scope, "::", NULL)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", idl_cxxId(name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scopedtypename", idl_scopeStack(scope, "::", name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("uniquetypename", idl_scopeStack(scope, "_", name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("keyList", idl_keyResolve(idl_keyDefDefGet(), scope, name)));
        snprintf(spaces, sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("spaces", spaces));
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
    }
    return idl_abort;
}

static void
idl_typedefOpenClose(
    idl_scope scope,
    const char *name,
    idl_typeDef defSpec,
    void *userData)
{
    c_char spaces[20];
    idl_tmplExp te;
    OS_UNUSED_ARG(userData);

    OS_UNUSED_ARG(userData);

    if ((idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tstruct ||
        idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tunion) &&
        idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        /* keylist defined for this typedef of struct or union */
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("namescope", idl_scopeStackCxx(scope, "::", NULL)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", idl_cxxId(name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scopedtypename", idl_scopeStack(scope, "::", name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("uniquetypename", idl_scopeStack(scope, "_", name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("keyList", idl_keyResolve(idl_keyDefDefGet(), scope, name)));
        snprintf(spaces, sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("spaces", spaces));
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
    }
}

static struct idl_program idl_genCxxTypedClassImpl;

idl_program
idl_genCxxTypedClassImplProgram(
        CxxTypeUserData *userData)
{
    idl_genCxxTypedClassImpl.idl_getControl = NULL;
    idl_genCxxTypedClassImpl.fileOpen = idl_fileOpen;
    idl_genCxxTypedClassImpl.fileClose = idl_fileClose;
    idl_genCxxTypedClassImpl.moduleOpen = idl_moduleOpen;
    idl_genCxxTypedClassImpl.moduleClose = NULL;
    idl_genCxxTypedClassImpl.structureOpen = idl_structureOpen;
    idl_genCxxTypedClassImpl.structureClose = NULL;
    idl_genCxxTypedClassImpl.structureMemberOpenClose = NULL;
    idl_genCxxTypedClassImpl.enumerationOpen = NULL;
    idl_genCxxTypedClassImpl.enumerationClose = NULL;
    idl_genCxxTypedClassImpl.enumerationElementOpenClose = NULL;
    idl_genCxxTypedClassImpl.unionOpen = idl_unionOpen;
    idl_genCxxTypedClassImpl.unionClose = NULL;
    idl_genCxxTypedClassImpl.unionCaseOpenClose = NULL;
    idl_genCxxTypedClassImpl.unionLabelsOpenClose = NULL;
    idl_genCxxTypedClassImpl.unionLabelOpenClose = NULL;
    idl_genCxxTypedClassImpl.typedefOpenClose = idl_typedefOpenClose;
    idl_genCxxTypedClassImpl.boundedStringOpenClose = NULL;
    idl_genCxxTypedClassImpl.sequenceOpenClose = NULL;
    idl_genCxxTypedClassImpl.constantOpenClose = NULL;
    idl_genCxxTypedClassImpl.artificialDefaultLabelOpenClose = NULL;
    idl_genCxxTypedClassImpl.userData = userData;

    return &idl_genCxxTypedClassImpl;
}
