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
#include "idl_scope.h"
#include "idl_dll.h"
#include "idl_streamsDef.h"
#include "idl_genCxxHelper.h"
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
        printf("OSPL_TMPL_PATH not defined\n");
        return (idl_abort);
    }
    if (orbPath == NULL) {
        printf("OSPL_ORB_PATH not defined\n");
        return (idl_abort);
    }

    /* Prepare file header template */
    snprintf(tmplFileName, (size_t)sizeof(tmplFileName), "%s%c%s%ccorbaCxxStreamsClassBodyHeader", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
    /* QAC EXPECT 3416; No unexpected side effects here */
    if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
        (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
        printf("No template found or protection violation (%s)\n", tmplFileName);
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
    idl_macroSetAdd(idlpp_macroSet,
        idl_macroNew("basename", name));
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
    snprintf(tmplFileName, (size_t)sizeof(tmplFileName), "%s%c%s%ccorbaCxxStreamsClassBody", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
    /* QAC EXPECT 3416; No unexpected side effects here */
    if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
        (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
        printf("No template found or protection violation (%s)\n", tmplFileName);
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
}

static idl_action
idl_moduleOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    return idl_explore;
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

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_streamsResolve(idl_streamsDefDefGet(), scope, name)) {
        /* streams pragma defined for this struct */
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("namescope", idl_scopeStackCxx(scope, "::", NULL)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", idl_cxxId(name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scopedtypename", idl_scopeStack(scope, "::", name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("uniquetypename", idl_scopeStack(scope, "_", name)));
        snprintf(spaces, (size_t)sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("spaces", spaces));
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
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

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_streamsResolve(idl_streamsDefDefGet(), scope, name)) {
        /* streams pragma defined for this union */
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("namescope", idl_scopeStackCxx(scope, "::", NULL)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", idl_cxxId(name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scopedtypename", idl_scopeStack(scope, "::", name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("uniquetypename", idl_scopeStack(scope, "_", name)));
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
        /* streams pragma defined for this typedef of struct or union */
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("namescope", idl_scopeStackCxx(scope, "::", NULL)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", idl_cxxId(name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scopedtypename", idl_scopeStack(scope, "::", name)));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("uniquetypename", idl_scopeStack(scope, "_", name)));
        snprintf(spaces, (size_t)sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("spaces", spaces));
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
    }
}

static struct idl_program
idl_genCxxStreamsImpl = {
    NULL,                   /* idl_programControl *(* idl_getControl)(void *userData); */
    idl_fileOpen,           /* idl_action (* fileOpen)(idl_scope ownScope, const char *name, void *userData); */
    NULL,                   /* void (* fileClose)(void *userData); */
    idl_moduleOpen,         /* idl_action (* moduleOpen)(idl_scope ownScope, const char *name, void *userData); */
    NULL,                   /* void (* moduleClose)(void *userData); */
    idl_structureOpen,      /* idl_action (* structureOpen)(idl_scope ownScope, const char *name, idl_typeStruct structSpec, void *userData); */
    NULL,                   /* void (* structureClose)(const char *name, void *userData); */
    NULL,                   /* void (* structureMemberOpenClose)(idl_scope ownScope, const char *name, idl_typeSpec typeSpec, void *userData); */
    NULL,                   /* idl_action (* enumerationOpen)(idl_scope ownScope, const char *name, idl_typeEnum enumSpec, void *userData); */
    NULL,                   /* void (* enumerationClose)(const char *name, void *userData); */
    NULL,                   /* void (* enumerationElementOpenClose)(idl_scope ownScope, const char *name, void *userData); */
    idl_unionOpen,          /* idl_action (* unionOpen)(idl_scope ownScope, const char *name, idl_typeUnion unionSpec, void *userData); */
    NULL,                   /* void (* unionClose)(const char *name, void *userData); */
    NULL,                   /* void (* unionCaseOpenClose)(idl_scope ownScope, const char *name, idl_typeSpec typeSpec, void *userData); */
    NULL,                   /* void (* unionLabelsOpenClose)(idl_scope ownScope, idl_labelSpec labelSpec, void *userData); */
    NULL,                   /* void (* unionLabelOpenClose)(idl_scope ownScope, idl_labelVal labelVal, void *userData); */
    idl_typedefOpenClose,   /* void (* typedefOpenClose)(idl_scope ownScope, const char *name, idl_typeDef typeDef, void *userData); */
    NULL,                   /* void (* boundedStringOpenClose)(idl_scope ownScope, idl_typeBasic typeBasic, void *userData); */
    NULL,                   /* void (* sequenceOpenClose)(idl_scope ownScope, idl_typeSeq typeSeq, void *userData); */
    NULL,                   /* void (* constantOpenClose)(idl_scope ownScope, idl_constSpec constantSpec, void *userData); */
    NULL,                   /* void (* artificialDefaultLabelOpenClose)(idl_scope ownScope, idl_labelVal labelVal, idl_typeSpec typeSpec, void *userData); */
    NULL                    /* void *userData; */
};

idl_program
idl_genCxxStreamsImplProgram(
    void)
{
    return &idl_genCxxStreamsImpl;
}
