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
#include "idl_genIdl.h"
#include "idl_genIdlHelper.h"
#include "idl_genCxxHelper.h"
#include "idl_genSplHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_genCxxTypedInterfaces.h"
#include "idl_tmplExp.h"
#include "idl_keyDef.h"
#include "idl_dll.h"

#include "os_heap.h"
#include "os_stdlib.h"
#include "c_typebase.h"

static idl_macroAttrib idlpp_macroAttrib;
static idl_streamIn idlpp_inStream;
static c_char *idlpp_template;
static idl_macroSet idlpp_macroSet;
static c_long idlpp_indent_level = 0;

/* QAC EXPECT 0285; Need dollar here, this is specified */
#define IDL_TOKEN_START     '$'
#define IDL_TOKEN_OPEN      '('
#define IDL_TOKEN_CLOSE     ')'

static idl_action
idl_fileOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    idl_tmplExp te;
    c_char tmplFileName[1024];
    c_char *tmplPath;
    c_char *sacppPath;
    int tmplFile;
    struct os_stat_s tmplStat;
    unsigned int nRead;
    idl_cxxInterfaceSelector *interfaceSelector = (idl_cxxInterfaceSelector *) userData;

    OS_UNUSED_ARG(scope);

    tmplPath = os_getenv("OSPL_TMPL_PATH");
    if (tmplPath == NULL) {
        printf ("OSPL_TMPL_PATH not defined\n");
        return (idl_abort);
    }
    sacppPath = "SACPP";

    if (*interfaceSelector == IDL_INTERFACE_H) {
        /* Prepare file header template */
        snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%ccorbaCxxInterfaceHeader", tmplPath, OS_FILESEPCHAR, sacppPath, OS_FILESEPCHAR);

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
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew ("basename", name));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew ("DLL_IMPORTEXPORT", idl_dllGetMacro()));
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);

        /* Prepare class definition template */
        snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%ccorbaCxxInterfaceBody", tmplPath, OS_FILESEPCHAR, sacppPath, OS_FILESEPCHAR);
        idl_fileOutPrintf(idl_fileCur(), "%s\n", idl_dllGetHeader());
    } else {
        idl_fileOutPrintf(idl_fileCur(), "#include \"%s.h\"\n", name);
        idl_fileOutPrintf(idl_fileCur(), "#include \"%sDcps.h\"\n\n", name);
        snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%ccorbaCxxInterfaceImpl", tmplPath, OS_FILESEPCHAR, sacppPath, OS_FILESEPCHAR);
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
    idl_cxxInterfaceSelector *interfaceSelector = (idl_cxxInterfaceSelector *) userData;

    if (*interfaceSelector == IDL_INTERFACE_H) {
        idl_fileOutPrintf(idl_fileCur(), "#endif\n");
    }
}

static idl_action
idl_moduleOpen (
    idl_scope scope,
    const char *name,
    void *userData)
 {
     OS_UNUSED_ARG(userData);
    /* Test whether the module contains a component within the pragma keylist.
     * If it does not, then the module should not be generated since it will
     * contain no items (which is itself illegal idl syntax).
     *
     * Note that we are comparing against only the keys existing within this idl
     * file (idl_idlScopeKeyList).  We do not use the result of idl_keyDefDefGet()
     * since this is a list of keys resulting from the preprocessed idl (which
     *  will include keys from other idl files that this may include).
     */

    if (os_iterLength (idl_idlScopeKeyList) == 0) {
        return idl_abort;
    } else {
        c_ulong li = 0;
        c_bool scopesMatch = FALSE;
        idl_scope moduleScope;
        idl_scopeElement newElement;

        /* the idl_scope parameter to this function does not yet include the scoping
         * for this module itself, so create a duplicate and add this scoping to it,
         * before testing whether this module contains one of the keys in this file.
         */
        moduleScope = idl_scopeDup(scope);
        newElement = idl_scopeElementNew (name, idl_tModule);
        idl_scopePush (moduleScope, newElement);

        /* Loop through the list of keys applying to this idl file and test whether
         * this particular module contains one of these keys.  If it does, generate
         * code for the module in the Dcps.idl file.
         */
        while (li < os_iterLength (idl_idlScopeKeyList)) {
            idl_scope keyscope = os_iterObject (idl_idlScopeKeyList, li);
            scopesMatch = idl_scopeSub (moduleScope, keyscope);
            if (scopesMatch) {
                break;
            }
            li++;
        }

        if (scopesMatch == FALSE) {
            return idl_abort;
        }
    }

    idl_printIndent(idlpp_indent_level);
    idl_fileOutPrintf(idl_fileCur(), "namespace %s\n", name);
    idl_printIndent(idlpp_indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idlpp_indent_level++;
    return idl_explore;
}

static void
idl_moduleClose(
    void *userData)
{
    OS_UNUSED_ARG(userData);

    idlpp_indent_level--;
    idl_printIndent(idlpp_indent_level);
    idl_fileOutPrintf(idl_fileCur(), "};\n");
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
    OS_UNUSED_ARG(structSpec);
    OS_UNUSED_ARG(userData);

    OS_UNUSED_ARG(structSpec);
    OS_UNUSED_ARG(userData);

    /* QAC EXPECT 3416; No side effects here */
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        /* keylist defined for this struct */
        c_char *cxxName = idl_cxxId(name);
        c_char *nameScope = idl_scopeStackCxx(scope, "::", NULL);
        c_char *scopedTypeName = idl_scopeStack(scope, "::", cxxName);
        c_type structType = idl_typeSpecDef(idl_typeSpec(structSpec));
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew ("typename", cxxName));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("namescope", nameScope));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scopedtypename", scopedTypeName));
        if (idl_CxxIsRefType(structType)) {
            idl_macroSetAdd(idlpp_macroSet, idl_macroNew ("seqtmpltname", "DDS_DCPSUVLSeq"));
        } else {
            idl_macroSetAdd(idlpp_macroSet, idl_macroNew ("seqtmpltname", "DDS_DCPSUFLSeq"));
        }
        snprintf(spaces, sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew ("spaces", spaces));
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
        os_free(scopedTypeName);
        os_free(nameScope);
        os_free(cxxName);
    }
    return idl_abort;
}

static idl_action
idl_unionOpen (
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

    /* QAC EXPECT 3416; No side effects here */
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        /* keylist defined for this union */
        c_char *cxxName = idl_cxxId(name);
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", cxxName));
        snprintf(spaces, sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("spaces", spaces));
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
        os_free(cxxName);
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
        c_char *cxxName = idl_cxxId(name);
        /* keylist defined for this typedef of struct or union */
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", cxxName));
        snprintf(spaces, sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("spaces", spaces));
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
        os_free(cxxName);
    }
}

static struct idl_program
idl_genCxxInterfaces = {
    NULL,
    idl_fileOpen,
    idl_fileClose,
    idl_moduleOpen,
    idl_moduleClose,
    idl_structureOpen,
    NULL, /* idl_structureClose */
    NULL, /* idl_structureMemberOpenClose */
    NULL, /* idl_enumerationOpen */
    NULL, /* idl_enumerationClose */
    NULL, /* idl_enumerationElementOpenClose */
    idl_unionOpen,
    NULL, /* idl_unionOpen */
    NULL, /* idl_unionCaseOpenClose */
    NULL, /* idl_unionLabelsOpenClose */
    NULL, /* idl_unionLabelOpenClose */
    idl_typedefOpenClose,
    NULL, /* idl_boundedStringOpenClose */
    NULL, /* idl_sequenceOpenClose */
    NULL, /* idl_constantOpenClose */
    NULL, /* idl_artificialDefaultLabelOpenClose */
    NULL, /* idl_forwardDeclaration */
    NULL  /* userData */
};

idl_program
idl_genCxxIntefacesProgram(void *userData)
{
    idl_genCxxInterfaces.userData = userData;
    return &idl_genCxxInterfaces;
}
