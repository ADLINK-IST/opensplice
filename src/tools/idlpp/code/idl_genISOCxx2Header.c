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
/*
   This module generates include file for the C++ API
*/

#include "idl_program.h"
#include "idl_scope.h"
#include "idl_genCorbaCxxHelper.h"
#include "idl_genISOCxx2Header.h"
#include "idl_genCxxHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_genMetaHelper.h"
#include "idl_genFileHelper.h"
#include "idl_tmplExp.h"
/* TBD */
#include "idl_keyDef.h"
#include "idl_typeSpecifier.h"

#include "os_heap.h"
#include "os_stdlib.h"
#include "os_abstract.h"

#include <ctype.h>

static idl_macroAttrib idlpp_macroAttrib;
static idl_streamIn idlpp_inStream;
static c_char *idlpp_template;
static idl_macroSet idlpp_macroSet;
static c_long idlpp_indent_level = 0;

/* QAC EXPECT 0285; Need dollar here, this is specified */
#define IDL_TOKEN_START         '$'
#define IDL_TOKEN_OPEN          '('
#define IDL_TOKEN_CLOSE         ')'

#define MAX_ULONG_STRLENGTH 10 /* Room for 9 digits, enough for 32-bit range. */

/**
 * fileOpen callback. This works by calling an existing function instead of making
 * another copy of the same source in this location.
 * @return idl_explore to state that the rest of the file needs to be processed
 */
static idl_action
idl_fileOpen (
    idl_scope scope,
    const char *name,
    void *userData)
{
    idl_tmplExp te;
    c_char tmplFileName [1024];
    c_char *tmplPath;
    c_char *orbPath;
    int tmplFile;
    struct os_stat_s tmplStat;
    unsigned int nRead;

    OS_UNUSED_ARG(userData);

    tmplPath = os_getenv ("OSPL_TMPL_PATH");
    orbPath = os_getenv ("OSPL_ORB_PATH");
    if (tmplPath == NULL) {
        printf ("OSPL_TMPL_PATH not defined\n");
        return (idl_abort);
    }
    if (orbPath == NULL) {
        printf ("OSPL_ORB_PATH not defined\n");
        return (idl_abort);
    }

    /* Prepare file header template */
    snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%cISOCPP2MainInclude", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
    /* QAC EXPECT 3416; No unexpected side effects here */
    if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
        (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
        printf ("No template found or protection violation (%s)\n", tmplFileName);
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
    os_free(idlpp_template);
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("basename", name));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("basename_upper", idl_genIncludeGuardFromFilename(scope, "")));
    te = idl_tmplExpNew(idlpp_macroSet);
    idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
    idl_streamInFree(idlpp_inStream);
    idl_tmplExpFree(te);

    /* Prepare class definition template */
    snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%cISOCPP2ClassBody", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
    /* QAC EXPECT 3416; No side effects here */
    if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
        (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
        printf ("No template found or protection violation (%s)\n", tmplFileName);
        return (idl_abort);
    }
    /* QAC EXPECT 5007; will not use wrapper */
    idlpp_template = os_malloc(tmplStat.stat_size+1);
    tmplFile = open(tmplFileName, O_RDONLY);
    nRead = (unsigned int)read(tmplFile, idlpp_template, tmplStat.stat_size);
    (void)memset(&idlpp_template[nRead], 0, tmplStat.stat_size+1-nRead);
    close(tmplFile);

    idlpp_indent_level = 0;

    return idl_explore;
}

/**
 * Just close out the file with an endif.
 */
static void
idl_fileClose(
    void *userData)
{
    const c_char* baseName;
    idl_macro macro;

    OS_UNUSED_ARG(userData);

    macro = idl_macroSetGet (idlpp_macroSet, "basename_upper");
    if (macro)
    {
        baseName = idl_macroValue(macro);
        idl_fileOutPrintf(idl_fileCur(), "#endif /* ISOCPP2_%s_H */\n", baseName);
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


/**
 * Not yet required
 * @return idl_explore - Apparently this means process the rest of the file.
 */
static idl_action
idl_structureOpen(
    idl_scope scope,
    const char *name,
    idl_typeStruct structSpec,
    void *userData)
{
    c_char spaces[20];
    idl_tmplExp te;
    os_char intToStr[MAX_ULONG_STRLENGTH];
    CxxTypeUserData *cxxUserData = (CxxTypeUserData *)userData;

    if (idl_keyResolve(idl_keyDefDefGet(), idl_typeUserScope(idl_typeUser(structSpec)), name) != NULL)
    {
        /* keylist defined for this struct */
        char *scopedTypeName = idl_scopeStack(scope, "::", name);
        const char *internalTypeName = idl_internalTypeNameForBuiltinTopic(scopedTypeName);
        const char *keyList = idl_keyResolve(idl_keyDefDefGet(), scope, name);
        if ((strlen(internalTypeName) != 0) &&
            ((keyList == NULL) ||
             (strcmp(keyList,"key") == 0))) {
            keyList = "key.localId,key.systemId";
        }

        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet,
            idl_macroNew("namescope", idl_cxxId(idl_scopeElementName(idl_scopeCur(scope)))));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", idl_cxxId(name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scopedtypename", scopedTypeName));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("internaltypename",
                strlen(internalTypeName) > 0 ? internalTypeName : scopedTypeName));
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

        /* Store data-type in iterator for future generation of type descriptor. */
        idl_metaCxxAddType(scope, name, idl_typeSpec(structSpec), &cxxUserData->idlpp_metaList);
        os_free(scopedTypeName);
    }
    return idl_explore;
}

static struct idl_program idl_genMyActions;

/**
 * Actions to generate a Foo_DCPS.hpp for the ISO C++ PSM from idl Foo.idl
 */
idl_program
idl_genISOCxx2HeaderProgram (
    void *userData)
{
    idl_genMyActions.idl_getControl = NULL;
    idl_genMyActions.fileOpen = idl_fileOpen;
    idl_genMyActions.fileClose = idl_fileClose;
    idl_genMyActions.moduleOpen = idl_moduleOpen;
    idl_genMyActions.moduleClose = NULL;
    idl_genMyActions.structureOpen = idl_structureOpen;
    idl_genMyActions.structureClose = NULL;
    idl_genMyActions.structureMemberOpenClose = NULL;
    idl_genMyActions.enumerationOpen = NULL;
    idl_genMyActions.enumerationClose = NULL;
    idl_genMyActions.enumerationElementOpenClose = NULL;
    idl_genMyActions.unionOpen = NULL;
    idl_genMyActions.unionClose = NULL;
    idl_genMyActions.unionCaseOpenClose = NULL;
    idl_genMyActions.unionLabelsOpenClose = NULL;
    idl_genMyActions.unionLabelOpenClose = NULL;
    idl_genMyActions.typedefOpenClose = NULL;
    idl_genMyActions.boundedStringOpenClose = NULL;
    idl_genMyActions.sequenceOpenClose = NULL;
    idl_genMyActions.constantOpenClose = NULL;
    idl_genMyActions.artificialDefaultLabelOpenClose = NULL;
    idl_genMyActions.userData =  userData;

    return &idl_genMyActions;
}
