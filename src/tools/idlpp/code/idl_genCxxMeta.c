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
#include "idl_genCxxMeta.h"
#include "idl_genCxxHelper.h"
#include "idl_genMetaHelper.h"
#include "idl_tmplExp.h"
#include "idl_keyDef.h"

#include "os_heap.h"
#include "os_stdlib.h"
#include "os_iterator.h"

static idl_macroAttrib idlpp_macroAttrib;
static idl_streamIn idlpp_inStream;
static c_char *idlpp_template;
static idl_macroSet idlpp_macroSet;
static os_iter idlpp_metaList = NULL;

/* QAC EXPECT 0285; Need dollar here, this is specified */
#define IDL_TOKEN_START		'$'
#define IDL_TOKEN_OPEN 		'('
#define IDL_TOKEN_CLOSE 	')'

#define MAX_ULONG_STRLENGTH 10 /* Room for 9 digits, enough for 32-bit range. */

C_CLASS(idl_meta);
C_STRUCT(idl_meta) {
    idl_scope scope;
    char *name;
    char *actual_name;
    c_type type;
};

static int
idl_genMeta(
    idl_meta meta)
{
    idl_tmplExp te;
    c_char tmplFileName[1024];
    c_char *tmplPath;
    c_char *orbPath;
    c_char *metaXML;
    c_char arrLengthStr[MAX_ULONG_STRLENGTH];
    c_ulong nrElements;
    int tmplFile;
    struct os_stat tmplStat;
    unsigned int nRead;

    tmplPath = os_getenv("OSPL_TMPL_PATH");
    orbPath = os_getenv("OSPL_ORB_PATH");
    if (tmplPath == NULL) {
        printf ("OSPL_TMPL_PATH not defined\n");
        return -1;
    }
    if (orbPath == NULL) {
        printf ("OSPL_ORB_PATH not defined\n");
        return -1;
    }

    idlpp_macroSet = idl_macroSetNew();
    idl_macroSetAdd(idlpp_macroSet, 
        idl_macroNew("namescope",
        idl_scopeStackCxx(meta->scope, "::", NULL)));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("typename", meta->name));
    idl_macroSetAdd(idlpp_macroSet,
        idl_macroNew("scopedtypename",
        idl_scopeStackCxx(meta->scope, "::", meta->name)));
    idl_macroSetAdd(idlpp_macroSet,
        idl_macroNew("uniquetypename",
        idl_scopeStackCxx(meta->scope, "_", meta->name)));
    metaXML = idl_genXMLmeta(meta->type);
    idl_macroSetAdd(idlpp_macroSet,
        idl_macroNew("meta-descriptor",
        idl_cutXMLmeta(metaXML, &nrElements)));
    snprintf(arrLengthStr, MAX_ULONG_STRLENGTH, "%d", nrElements);
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("meta-descriptorArrLength", arrLengthStr));
    os_free(metaXML);

    if (idl_fileCur() == NULL) {
        return -1;
    }
    snprintf(tmplFileName, (size_t)sizeof(tmplFileName), "%s%c%s%ccorbaCxxMetaDescription", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
    /* QAC EXPECT 3416; No side effects here */
    if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
        (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
        printf ("No template found or protection violation (%s)\n", tmplFileName);
        return -1;
    }
    /* QAC EXPECT 5007; will not use wrapper */
    idlpp_template = os_malloc((size_t)((int)tmplStat.stat_size+1));
    tmplFile = open(tmplFileName, O_RDONLY);
    nRead = (unsigned int)read(tmplFile, idlpp_template, (size_t)tmplStat.stat_size);
    memset(&idlpp_template[nRead], 0, (size_t)((int)tmplStat.stat_size+1-nRead));
    close(tmplFile);
    idlpp_macroAttrib = idl_macroAttribNew(IDL_TOKEN_START, IDL_TOKEN_OPEN, IDL_TOKEN_CLOSE);
    idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);

    te = idl_tmplExpNew(idlpp_macroSet);
    idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
    idl_tmplExpFree(te);

    return 0;
}

static void
newMeta(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec)
{
    idl_meta meta;

    meta = os_malloc(C_SIZEOF(idl_meta));
    if (meta) {
        meta->scope = idl_scopeDup(scope);
        meta->name = os_strdup(name);
        if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
            meta->actual_name = idl_typeSpecName(idl_typeDefActual(idl_typeDef(typeSpec)));
        } else {
            meta->actual_name = idl_typeSpecName(typeSpec);
        }
        meta->type = idl_typeSpecDef(typeSpec);
    }
    idlpp_metaList = os_iterAppend(idlpp_metaList, meta);
}

static idl_action
idl_fileOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    return idl_explore;
}

static void
idl_fileClose(
    void *userData)
{
    idl_meta meta;

    meta = os_iterTakeFirst(idlpp_metaList);
    while (meta) {
        idl_genMeta(meta);
        idl_scopeFree(meta->scope);
        os_free(meta->name);
        os_free(meta->actual_name);
        meta = os_iterTakeFirst(idlpp_metaList);
    }
}

static idl_action
idl_moduleOpen (
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
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        newMeta(scope, name, idl_typeSpec(structSpec));
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
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        newMeta(scope, name, idl_typeSpec(unionSpec));
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
    if ((idl_typeSpecType(idl_typeDefActual (defSpec)) == idl_tstruct) ||
        (idl_typeSpecType(idl_typeDefActual (defSpec)) == idl_tunion)) {
        if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
            newMeta(scope, name, idl_typeSpec(defSpec));
        }
    }
}

static struct idl_program
idl_genCxxMeta = {
    NULL,
    idl_fileOpen,
    idl_fileClose,
    idl_moduleOpen,
    NULL, /* idl_moduleClose */
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
idl_genCxxMetaProgram(
    void)
{
    return &idl_genCxxMeta;
}
