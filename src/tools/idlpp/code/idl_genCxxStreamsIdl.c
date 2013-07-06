/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "idl_program.h"
#include "idl_genCxxStreamsIdl.h"
#include "idl_genIdlHelper.h"
#include "idl_genCxxHelper.h"
#include "idl_genSplHelper.h"
#include "idl_streamsDef.h"
#include "idl_tmplExp.h"

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
    c_char *orbPath;
    int tmplFile;
    struct os_stat tmplStat;
    unsigned int nRead;

    tmplPath = os_getenv("OSPL_TMPL_PATH");
    orbPath = os_getenv("OSPL_ORB_PATH");
    if (tmplPath == NULL) {
        printf ("OSPL_TMPL_PATH not defined\n");
        return (idl_abort);
    }
    if (orbPath == NULL) {
        printf ("OSPL_ORB_PATH not defined\n");
        return (idl_abort);
    }

    /* Prepare file header template */
    snprintf(tmplFileName, (size_t)sizeof(tmplFileName), "%s%c%s%ccorbaStreamsIdlInterfaceHeader", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
    /* QAC EXPECT 3416; No side effects here */
    if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
        (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
        printf ("No template found or protection violation (%s)\n", tmplFileName);
        return (idl_abort);
    }
    /* QAC EXPECT 5007; will not use wrapper */
    idlpp_template = os_malloc((size_t)((int)tmplStat.stat_size+1));
    tmplFile = open(tmplFileName, O_RDONLY);
    nRead = (unsigned int)read(tmplFile, idlpp_template, (size_t)tmplStat.stat_size);
    memset(&idlpp_template[nRead], 0, (size_t)((int)tmplStat.stat_size+1-nRead));
    close(tmplFile);
    idlpp_macroAttrib = idl_macroAttribNew(IDL_TOKEN_START, IDL_TOKEN_OPEN, IDL_TOKEN_CLOSE);
    idlpp_macroSet = idl_macroSetNew();
    idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
    /* Expand file header */
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("basename", name));
    te = idl_tmplExpNew(idlpp_macroSet);
    idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
    idl_streamInFree(idlpp_inStream);
    idl_tmplExpFree(te);

    /* Prepare class definition template */
    snprintf(tmplFileName, (size_t)sizeof(tmplFileName), "%s%c%s%ccorbaStreamsIdlInterface", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
    /* QAC EXPECT 3416; No side effects here */
    if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
        (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
        printf ("No template found or protection violation (%s)\n", tmplFileName);
        return (idl_abort);
    }
    /* QAC EXPECT 5007; will not use wrapper */
    idlpp_template = os_malloc((size_t)((int)tmplStat.stat_size+1));
    tmplFile = open(tmplFileName, O_RDONLY);
    nRead = (unsigned int)read(tmplFile, idlpp_template, (size_t)tmplStat.stat_size);
    memset(&idlpp_template[nRead], 0, (size_t)((int)tmplStat.stat_size+1-nRead));
    close(tmplFile);

    idlpp_indent_level = 0;

    return idl_explore;
    /* QAC EXPECT 2006; overview does not get better with one exit */
}

static void
idl_fileClose(
    void *userData)
{
    idl_fileOutPrintf(idl_fileCur(), "#endif\n");
}

static idl_action
idl_moduleOpen (
    idl_scope scope,
    const char *name,
    void *userData)
{
    c_long streamsIndex, streamsCount;
    idl_action result;

    /* Test whether the module contains a component within the pragma streams.
     * If it does not, then the module should not be generated since it will
     * contain no items (which is itself illegal idl syntax).
     *
     * Note that we are comparing against only the streams existing within this idl
     * file (idl_idlScopeStreamsList).  We do not use the result of idl_keyDefDefGet()
     * since this is a list of streams resulting from the preprocessed idl (which
     *  will include streams from other idl files that this may include).
     */
    result = idl_abort;
    streamsCount = os_iterLength(idl_idlScopeStreamsList);
    if (streamsCount != 0) {
        idl_scope moduleScope;
        idl_scopeElement newElement;

        /* the idl_scope parameter to this function does not yet include the scoping
         * for this module itself, so create a duplicate and add this scoping to it,
         * before testing whether this module contains one of the streams in this file.
         */
        moduleScope = idl_scopeDup(scope);
        newElement = idl_scopeElementNew(name, idl_tModule);
        idl_scopePush(moduleScope, newElement);

        /* Loop through the list of keys applying to this idl file and test whether
         * this particular module contains one of these keys.  If it does, generate
         * code for the module.
         */
        for (streamsIndex = 0; (streamsIndex < streamsCount) && (result == idl_abort); streamsIndex++) {
            idl_scope streamsScope = os_iterObject(idl_idlScopeStreamsList, streamsIndex);
            if (idl_scopeSub(moduleScope, streamsScope)) {
                /* Scopes match */
                result = idl_explore;
            }
        }
    }

    if (result == idl_explore) {
        idl_printIndent(idlpp_indent_level);
        idl_fileOutPrintf(idl_fileCur(), "module %s {\n", idl_cxxId(name));
        idl_fileOutPrintf(idl_fileCur(), "\n");
        idlpp_indent_level++;
    }
    return result;
}

static void
idl_moduleClose(
    void *userData)
{
    /* this is only executed if idl_moduleOpen returns 'idl_explore' */
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

    /* QAC EXPECT 3416; No side effects here */
    if (idl_streamsResolve(idl_streamsDefDefGet(), scope, name)) {
        /* streams pragma defined for this struct */
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet,
            idl_macroNew("namescope", idl_cxxId(idl_scopeElementName(idl_scopeCur(scope)))));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew ("typename", idl_cxxId(name)));
        snprintf(spaces, (size_t)sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew ("spaces", spaces));
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
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

    /* QAC EXPECT 3416; No side effects here */
    if (idl_streamsResolve(idl_streamsDefDefGet(), scope, name)) {
        /* streams pragma defined for this union */
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet,
            idl_macroNew("namescope",idl_cxxId(idl_scopeElementName(idl_scopeCur(scope)))));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", idl_cxxId(name)));
        snprintf(spaces, (size_t)sizeof(spaces), "%d", idlpp_indent_level*4);
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

    if (((idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tstruct) ||
         (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tunion)) &&
         (idl_streamsResolve(idl_streamsDefDefGet(), scope, name))) {
        /* keylist defined for this typedef of struct or union */
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet,
            idl_macroNew("namescope", idl_cxxId(idl_scopeElementName(idl_scopeCur(scope)))));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", idl_cxxId(name)));
        snprintf(spaces, (size_t)sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("spaces", spaces));
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
    }
}

static struct idl_program
idl_genCxxStreamsIdl = {
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
    NULL  /* userData */
};

idl_program
idl_genCxxStreamsIdlProgram(
    void)
{
    return &idl_genCxxStreamsIdl;
}
