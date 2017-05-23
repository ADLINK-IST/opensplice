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
#include "idl_program.h"
#include "idl_scope.h"
#include "idl_genCxxTypedClassDefs.h"
#include "idl_genCxxHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_genSplHelper.h"
#include "idl_tmplExp.h"
#include "idl_keyDef.h"
#include "idl_dll.h"

#include "os_heap.h"
#include "os_stdlib.h"
#include "c_typebase.h"
#include <ctype.h>

static idl_macroAttrib idlpp_macroAttrib;
static idl_streamIn idlpp_inStream;
static c_char *idlpp_template;
static idl_macroSet idlpp_macroSet;
static c_long idlpp_indent_level = 0;

/* QAC EXPECT 0285; Need dollar here, this is specified */
#define IDL_TOKEN_START		'$'
#define IDL_TOKEN_OPEN 		'('
#define IDL_TOKEN_CLOSE 	')'

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
    struct os_stat tmplStat;
    unsigned int nRead;

    (void) scope; /* Unused */
    (void) userData; /* Unused */

    tmplPath = os_getenv("OSPL_TMPL_PATH");
    if (tmplPath == NULL) {
        printf ("OSPL_TMPL_PATH not defined\n");
        return (idl_abort);
    }

    /* Prepare file header template */
    if (idl_getLanguage() == IDL_LANG_LITE_CXX)
    {
        if (idl_getIsISOCpp())
        {
            snprintf(tmplFileName, sizeof(tmplFileName), "%s%cliteISOCxxClassSpecHeader", tmplPath, OS_FILESEPCHAR);
        }
        else
        {
            snprintf(tmplFileName, sizeof(tmplFileName), "%s%cliteCxxClassSpecHeader", tmplPath, OS_FILESEPCHAR);
        }
    }
    else
    {
        orbPath = os_getenv("OSPL_ORB_PATH");

        if (orbPath == NULL) {
            printf ("OSPL_ORB_PATH not defined\n");
            return (idl_abort);
        }

        if (idl_getIsISOCpp())
        {
            snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%cISOCxxClassSpecHeader", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
        }
        else
        {
            snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%ccorbaCxxClassSpecHeader", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
        }
    }

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

    te = idl_tmplExpNew(idlpp_macroSet);
    idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
    idl_streamInFree(idlpp_inStream);
    idl_tmplExpFree(te);

    /* Prepare class definition template */
    if (idl_getLanguage() == IDL_LANG_LITE_CXX)
    {
        snprintf(tmplFileName, sizeof(tmplFileName), "%s%cliteCxxClassSpec", tmplPath, OS_FILESEPCHAR);
    }
    else
    {
        snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%ccorbaCxxClassSpec", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
    }
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
    memset(&idlpp_template[nRead], 0, tmplStat.stat_size+1-nRead);
    close(tmplFile);

    idlpp_indent_level = 0;

    return idl_explore;
    /* QAC EXPECT 2006; overview does not get better with one exit */
}

static void
idl_fileClose(
    void *userData)
{
    idl_macro macro;
    os_char* tmpName;
    size_t i;
    (void) userData;
    if (idl_getIsISOCpp())
    {
        macro = idl_macroSetGet (idlpp_macroSet, "basename");
        if (macro)
        {
            tmpName = os_strdup(idl_macroValue(macro));
            for(i = 0; i < strlen(tmpName); i++)
            {
                tmpName[i] = (os_char) toupper (tmpName[i]);
            }
            idl_fileOutPrintf(idl_fileCur(), "#define %s_DCPS_TYPESUPPORT_DEFINED\n", tmpName);
            os_free(tmpName);
        }
    }
    idl_fileOutPrintf(idl_fileCur(), "#undef OS_API\n");
    idl_fileOutPrintf(idl_fileCur(), "#endif\n");
}

static idl_action
idl_moduleOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    (void) scope; /* Unused */
    (void) userData; /* Unused */

    idl_printIndent(idlpp_indent_level);
    idl_fileOutPrintf(idl_fileCur(), "namespace %s {\n", idl_cxxId(name));
    idl_fileOutPrintf(idl_fileCur(), "\n");
    idlpp_indent_level++;
    return idl_explore;
}

static void
idl_moduleClose(
    void *userData)
{
    (void) userData; /* Unused */

    idlpp_indent_level--;
    idl_printIndent(idlpp_indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
}

static idl_action
idl_structureOpen(
    idl_scope scope,
    const char *name,
    idl_typeStruct structSpec,
    void *userData)
{
    c_char spaces[20];
    idl_tmplExp te;
    CxxTypeUserData *cxxUserData = (CxxTypeUserData *)userData;

    /* QAC EXPECT 3416; No side effects here */
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
	/* keylist defined for this struct */
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet,
            idl_macroNew("namescope", idl_cxxId(idl_scopeElementName(idl_scopeCur(scope)))));
        idl_macroSetAdd(idlpp_macroSet,
            idl_macroNew("cnamescope", idl_scopeStack(scope, "_", "")));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", idl_cxxId(name)));
        snprintf(spaces, sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("spaces", spaces));
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);

        /* Store data-type in iterator for future generation of type descriptor. */
        idl_metaCxxAddType(scope, name, idl_typeSpec(structSpec), &cxxUserData->idlpp_metaList);
    }
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

    (void) unionSpec; /* Unused */
    (void) userData; /* Unused */

    /* QAC EXPECT 3416; No side effects here */
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
	/* keylist defined for this union */
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet,
            idl_macroNew("namescope", idl_cxxId(idl_scopeElementName(idl_scopeCur(scope)))));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", idl_cxxId(name)));
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

    (void) userData; /* Unused */

    if ((idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tstruct ||
        idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tunion) &&
        idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        /* keylist defined for this typedef of struct or union */
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet,
            idl_macroNew("namescope", idl_cxxId(idl_scopeElementName(idl_scopeCur(scope)))));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", idl_cxxId(name)));
        snprintf(spaces, sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("spaces", spaces));
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
    }
}

static struct idl_program idl_genCxxTypedClassDefs;

idl_program
idl_genCxxTypedClassDefsProgram(
        CxxTypeUserData *userData)
{
    idl_genCxxTypedClassDefs.idl_getControl = NULL;
    idl_genCxxTypedClassDefs.fileOpen = idl_fileOpen;
    idl_genCxxTypedClassDefs.fileClose = idl_fileClose;
    idl_genCxxTypedClassDefs.moduleOpen = idl_moduleOpen;
    idl_genCxxTypedClassDefs.moduleClose = idl_moduleClose;
    idl_genCxxTypedClassDefs.structureOpen = idl_structureOpen;
    idl_genCxxTypedClassDefs.structureClose = NULL;
    idl_genCxxTypedClassDefs.structureMemberOpenClose = NULL;
    idl_genCxxTypedClassDefs.enumerationOpen = NULL;
    idl_genCxxTypedClassDefs.enumerationClose = NULL;
    idl_genCxxTypedClassDefs.enumerationElementOpenClose = NULL;
    idl_genCxxTypedClassDefs.unionOpen = idl_unionOpen;
    idl_genCxxTypedClassDefs.unionClose = NULL;
    idl_genCxxTypedClassDefs.unionCaseOpenClose = NULL;
    idl_genCxxTypedClassDefs.unionLabelsOpenClose = NULL;
    idl_genCxxTypedClassDefs.unionLabelOpenClose = NULL;
    idl_genCxxTypedClassDefs.typedefOpenClose = idl_typedefOpenClose;
    idl_genCxxTypedClassDefs.boundedStringOpenClose = NULL;
    idl_genCxxTypedClassDefs.sequenceOpenClose = NULL;
    idl_genCxxTypedClassDefs.constantOpenClose = NULL;
    idl_genCxxTypedClassDefs.artificialDefaultLabelOpenClose = NULL;
    idl_genCxxTypedClassDefs.userData =  userData;

    return &idl_genCxxTypedClassDefs;
}
