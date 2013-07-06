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
#include "idl_genSacTypedClassImpl.h"
#include "idl_genSacHelper.h"
#include "idl_genCHelper.h"
#include "idl_tmplExp.h"
#include "idl_keyDef.h"

#include "os_heap.h"
#include "os_stdlib.h"
#include "c_typebase.h"

static idl_macroAttrib idlpp_macroAttrib;
static idl_streamIn idlpp_inStream;
static c_char *idlpp_template;
static idl_macroSet idlpp_macroSet;
static c_long idlpp_indent_level = 0;

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
    snprintf(tmplFileName, (size_t)sizeof(tmplFileName), "%s%c%s%csacClassBodyHeader", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
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
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("basename", name));
    te = idl_tmplExpNew(idlpp_macroSet);
    idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
    idl_streamInFree(idlpp_inStream);
    idl_tmplExpFree(te);

    /* Prepare class definition template */
    snprintf(tmplFileName, (size_t)sizeof(tmplFileName), "%s%c%s%csacClassBody", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
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
    /* QAC EXPECT 2006; overview does not get better with one exit */
}

static idl_action
idl_moduleOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    return idl_explore;
}

static void
idl_sequenceObjManagDef(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec)
{
    char seqName[512];

    snprintf(seqName, sizeof(seqName), "DDS_sequence_%s", idl_scopeStack(scope, "_", name));
    if (idl_definitionExists("objManagImpl", seqName)) {
        idl_fileOutPrintf(
            idl_fileCur(),
            "extern DDS_sequence_%s *DDS_sequence_%s__alloc (void);\n",
            idl_scopeStack(scope, "_", name),
            idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(
            idl_fileCur(),
            "extern %s *DDS_sequence_%s_allocbuf (DDS_unsigned_long len);\n",
            idl_scopeStack(scope, "_", name),
            idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf (idl_fileCur(), "\n");
    } else {
        idl_definitionAdd("objManagImpl", seqName);
        /* Generate allocation routine for the sequence */
        idl_fileOutPrintf(
            idl_fileCur(),
            "DDS_sequence_%s *DDS_sequence_%s__alloc (void)\n",
            idl_scopeStack(scope, "_", name),
            idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(
            idl_fileCur(),
            "    return (DDS_sequence_%s *)DDS__malloc (DDS_sequence_free, 0, sizeof(DDS_sequence_%s));\n",
            idl_scopeStack(scope, "_", name),
            idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "}\n\n");
        /* Generate allocation routine for the sequence buffer */
        idl_fileOutPrintf(
            idl_fileCur(),
            "%s *DDS_sequence_%s_allocbuf (DDS_unsigned_long len)\n",
            idl_scopeStack(scope, "_", name),
            idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        if (idl_typeSpecHasRef(typeSpec)) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "    DDS_boolean DDS_sequence_%s_freebuf (void *buffer);\n\n",
                idl_scopeStack(scope, "_", name));
            idl_fileOutPrintf(
                idl_fileCur(),
                "    return (%s *)DDS_sequence_allocbuf (DDS_sequence_%s_freebuf, sizeof (%s), len);\n",
                idl_scopeStack(scope, "_", name),
                idl_scopeStack(scope, "_", name),
                idl_scopeStack(scope, "_", name));
        } else {
            idl_fileOutPrintf(
                idl_fileCur(),
                "    return (%s *)DDS_sequence_allocbuf (NULL, sizeof (%s), len);\n",
                idl_scopeStack(scope, "_", name),
                idl_scopeStack(scope, "_", name));
        }
        idl_fileOutPrintf(idl_fileCur(), "}\n");
        if (idl_typeSpecHasRef(typeSpec)) {
            /* Deallocation routine for the buffer is required */
            idl_fileOutPrintf(
                idl_fileCur(),
                "\nDDS_boolean DDS_sequence_%s_freebuf (void *buffer)\n",
                idl_scopeStack(scope, "_", name));
            idl_fileOutPrintf(idl_fileCur(), "{\n");
            idl_fileOutPrintf(idl_fileCur(), "    DDS_unsigned_long *count = (DDS_unsigned_long *)DDS__header (buffer);\n");
            idl_fileOutPrintf(
                idl_fileCur(),
                "    %s *b = (%s *)buffer;\n",
                idl_scopeStack(scope, "_", name),
                idl_scopeStack(scope, "_", name));
            idl_fileOutPrintf(idl_fileCur(), "    DDS_unsigned_long i;\n");
            idl_fileOutPrintf(
                idl_fileCur(),
                "    void %s__free (void *object);\n\n",
                idl_scopeStack(scope, "_", name));
            idl_fileOutPrintf(idl_fileCur(), "    for (i = 0; i < *count; i++) {\n");
            idl_fileOutPrintf(
                idl_fileCur(),
                "        %s__free (&b[i]);\n",
                idl_scopeStack(scope, "_", name));
            idl_fileOutPrintf(idl_fileCur(), "    }\n");
            idl_fileOutPrintf(idl_fileCur(), "    return TRUE;\n");
            idl_fileOutPrintf(idl_fileCur(), "}\n");
        }
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
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
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        /* keylist defined for this struct */
        idl_sequenceObjManagDef(scope, name, idl_typeSpec(structSpec));
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("type_name", idl_scopeStack(scope, "_", name)));
        snprintf(spaces, (size_t)sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew ("spaces", spaces));
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
    }
    return idl_abort;
}

static void
idl_structureClose(
    const char *name,
    void *userData)
{
}

static void
idl_structureMemberOpenClose(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec,
    void *userData)
{
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

    /* QAC EXPECT 3416; No unexpected side effects here */
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        /* keylist defined for this union */
        idl_sequenceObjManagDef(scope, name, idl_typeSpec(unionSpec));
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("type_name", idl_scopeStack(scope, "_", name)));
        snprintf(spaces, (size_t)sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew ("spaces", spaces));
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

    /* QAC EXPECT 3416; No unexpected side effects here */
    if ((idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tstruct ||
        idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tunion) &&
        idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        /* keylist defined for this union */
        idl_sequenceObjManagDef(scope, name, idl_typeSpec(defSpec));
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("type_name", idl_scopeStack(scope, "_", name)));
        snprintf(spaces, (size_t)sizeof(spaces), "%d", idlpp_indent_level*4);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("spaces", spaces));
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
    }
}

static struct idl_program
idl_genSacTypedClassImpl = {
    NULL,
    idl_fileOpen,
    NULL, /* idl_fileClose */
    idl_moduleOpen,
    NULL, /* idl_moduleClose */
    idl_structureOpen,
    idl_structureClose,
    idl_structureMemberOpenClose,
    NULL, /* 3 slots enum support */
    NULL,
    NULL,
    idl_unionOpen,
    NULL, /* idl_unionClose */
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
idl_genSacTypedClassImplProgram(
    void)
{
    return &idl_genSacTypedClassImpl;
}
