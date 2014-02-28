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
#include "idl_genSplHelper.h"
#include "idl_genJavaHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_tmplExp.h"
#include "idl_keyDef.h"

#include "os_heap.h"
#include "os_stdlib.h"
#include "c_typebase.h"

static idl_macroAttrib idlpp_macroAttrib;
static idl_streamIn idlpp_inStream;
static c_char *idlpp_template;
static idl_macroSet idlpp_macroSet;

/* QAC EXPECT 0285; Need dollar here, this is specified */
#define IDL_TOKEN_START		'$'
#define IDL_TOKEN_OPEN 		'('
#define IDL_TOKEN_CLOSE 	')'

static int
idl_genInterface(
    idl_scope scope,
    const char *name,
    char *class_base,
    idl_typeSpec typeSpec,
    c_bool generateInterfaceClass)
{
    idl_tmplExp te;
    c_char tmplFileName[1024];
    c_char pname[1024];
    c_char *tmplPath;
    c_char *orbPath;
    int tmplFile;
    struct os_stat tmplStat;
    unsigned int nRead;

    tmplPath = os_getenv("OSPL_TMPL_PATH");
    orbPath = os_getenv("OSPL_ORB_PATH");
    if (tmplPath == NULL) {
        printf("OSPL_TMPL_PATH not defined\n");
        return -1;
    }
    if (orbPath == NULL) {
        printf("OSPL_ORB_PATH not defined\n");
        return -1;
    }

    idlpp_macroSet = idl_macroSetNew();
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("type-name", idl_javaId(name)));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("actual-type-name", idl_typeSpecName(typeSpec)));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scoped-type-name", idl_scopeStackJava(scope, ".", name)));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("java-class-name", idl_scopeStackJava(scope, "/", name)));
    if(idl_genJavaHelperGetOrgLastSubstituted())
    {
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("java-org-package-name", idl_genJavaHelperGetOrgLastSubstituted()));
    } else
    {
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("java-org-package-name", "null"));
    }
    if(idl_genJavaHelperGetTgtLastSubstituted())
    {
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("java-tgt-package-name", idl_genJavaHelperGetTgtLastSubstituted()));
    } else
    {
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("java-tgt-package-name", "null"));
    }
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scoped-meta-type-name", idl_scopeStack(scope, "::", name)));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scoped-actual-type-name", idl_corbaJavaTypeFromTypeSpec(typeSpec)));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("key-list", idl_keyResolve(idl_keyDefDefGet(), scope, name)));


    /* Generate only in standalone mode */
    if (idl_getCorbaMode() == IDL_MODE_STANDALONE) {
        snprintf(pname, sizeof (pname), "%s%s", idl_javaId(name), class_base);
        idl_openJavaPackage(scope, pname);
        if (idl_fileCur() == NULL) {
            return -1;
        }
        if (idl_scopeStackSize(scope) > 0) {
            idl_fileOutPrintf(idl_fileCur(), "package %s;\n", idl_scopeStackJava (scope, ".", NULL));
            idl_fileOutPrintf(idl_fileCur(), "\n");
        }
        /* Prepare Interface class */
        if (generateInterfaceClass) {
            snprintf(tmplFileName, (size_t)sizeof(tmplFileName), "%s%c%s%ctmpl%s.java", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR, class_base);
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
            idl_streamInFree(idlpp_inStream);
            idl_tmplExpFree(te);
        }
        idl_closeJavaPackage();

        snprintf(pname, sizeof(pname), "%s%sHolder", idl_javaId(name), class_base);
        idl_openJavaPackage(scope, pname);
        if (idl_fileCur() == NULL) {
            return -1;
        }
        if (idl_scopeStackSize(scope) > 0) {
            idl_fileOutPrintf(idl_fileCur(), "package %s;\n", idl_scopeStackJava(scope, ".", NULL));
            idl_fileOutPrintf(idl_fileCur(), "\n");
        }
        /* Prepare typeSupportHolder class */
        snprintf(tmplFileName, (size_t)sizeof(tmplFileName), "%s%c%s%ctmpl%sHolder.java", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR, class_base);
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
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
        idl_closeJavaPackage();

        snprintf(pname, sizeof(pname), "%s%sHelper", idl_javaId(name), class_base);
        idl_openJavaPackage(scope, pname);
        if (idl_fileCur() == NULL) {
            return -1;
        }
        if (idl_scopeStackSize(scope) > 0) {
            idl_fileOutPrintf(idl_fileCur(), "package %s;\n", idl_scopeStackJava(scope, ".", NULL));
            idl_fileOutPrintf(idl_fileCur(), "\n");
        }
        /* Prepare typeSupportHelper class */
        snprintf(tmplFileName, (size_t)sizeof(tmplFileName), "%s%c%s%ctmpl%sHelper.java", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR, class_base);
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
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
        idl_closeJavaPackage();

        snprintf(pname, sizeof(pname), "%s%sOperations", idl_javaId(name), class_base);
        idl_openJavaPackage(scope, pname);
        if (idl_fileCur() == NULL) {
            return (idl_abort);
        }
        if (idl_scopeStackSize(scope) > 0) {
            idl_fileOutPrintf(idl_fileCur(), "package %s;\n", idl_scopeStackJava(scope, ".", NULL));
            idl_fileOutPrintf(idl_fileCur(), "\n");
        }
        /* Prepare typeSupportOperations class */
        snprintf(tmplFileName, (size_t)sizeof(tmplFileName), "%s%c%s%ctmpl%sOperations.java", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR, class_base);
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
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
        idl_closeJavaPackage();
    }

    if (generateInterfaceClass) {
        /* Implementation with Impl extension */
        snprintf(pname, sizeof(pname), "%s%sImpl", idl_javaId(name), class_base);
    } else {
        /* Implementation without Impl extension */
        snprintf(pname, sizeof(pname), "%s%s", idl_javaId(name), class_base);
    }
    idl_openJavaPackage(scope, pname);
    if (idl_fileCur() == NULL) {
        return -1;
    }
    if (idl_scopeStackSize(scope) > 0) {
        idl_fileOutPrintf(idl_fileCur(), "package %s;\n", idl_scopeStackJava(scope, ".", NULL));
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
    /* Prepare typeSupportStub class */
    snprintf(tmplFileName, (size_t)sizeof(tmplFileName), "%s%c%s%ctmpl%sImpl.java", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR, class_base);
    /* QAC EXPECT 3416; No side effects here */
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
    idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);

    te = idl_tmplExpNew(idlpp_macroSet);
    idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
    idl_streamInFree(idlpp_inStream);
    idl_tmplExpFree(te);
    idl_closeJavaPackage();

    return 0;
}

static int
idl_genTypeSeqHolder(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec)
{
    idl_tmplExp te;
    c_char tmplFileName[1024];
    c_char pname[1024];
    c_char *tmplPath;
    c_char *orbPath;
    int tmplFile;
    struct os_stat tmplStat;
    unsigned int nRead;

    tmplPath = os_getenv("OSPL_TMPL_PATH");
    orbPath = os_getenv("OSPL_ORB_PATH");
    if (tmplPath == NULL) {
        printf("OSPL_TMPL_PATH not defined\n");
        return -1;
    }
    if (orbPath == NULL) {
        printf("OSPL_ORB_PATH not defined\n");
        return -1;
    }

    idlpp_macroSet = idl_macroSetNew();
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("type-name", idl_javaId(name)));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("actual-type-name", idl_typeSpecName(typeSpec)));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scoped-type-name", idl_scopeStackJava(scope, ".", name)));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scoped-actual-type-name", idl_corbaJavaTypeFromTypeSpec(typeSpec)));

    snprintf(pname, sizeof (pname), "%sSeqHolder", idl_javaId(name));
    idl_openJavaPackage(scope, pname);
    if (idl_fileCur() == NULL) {
        return -1;
    }
    if (idl_scopeStackSize(scope) > 0) {
        idl_fileOutPrintf(idl_fileCur(), "package %s;\n", idl_scopeStackJava(scope, ".", NULL));
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
    /* Prepare typeSupport class */
    snprintf(tmplFileName, (size_t)sizeof(tmplFileName), "%s%c%s%ctmplSeqHolder.java", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
    /* QAC EXPECT 3416; No side effects here */
    if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
        (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
        printf("No template found or protection violation (%s)\n", tmplFileName);
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
    idl_streamInFree(idlpp_inStream);
    idl_tmplExpFree(te);
    idl_closeJavaPackage();

    return 0;
}

static idl_action
idl_fileOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
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
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        idl_genInterface(scope, name, "TypeSupport", idl_typeSpec(structSpec), FALSE);
        idl_genInterface(scope, name, "DataReader", idl_typeSpec(structSpec), TRUE);
        idl_genInterface(scope, name, "DataReaderView", idl_typeSpec(structSpec), TRUE);
        idl_genInterface(scope, name, "DataWriter", idl_typeSpec(structSpec), TRUE);
        if (idl_getCorbaMode() == IDL_MODE_STANDALONE) {
            idl_genTypeSeqHolder(scope, name, idl_typeSpec(structSpec));
        }
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
        idl_genInterface(scope, name, "TypeSupport", idl_typeSpec(unionSpec), FALSE);
        idl_genInterface(scope, name, "DataReader", idl_typeSpec(unionSpec), TRUE);
        idl_genInterface(scope, name, "DataReaderView", idl_typeSpec(unionSpec), TRUE);
        idl_genInterface(scope, name, "DataWriter", idl_typeSpec(unionSpec), TRUE);
        if (idl_getCorbaMode() == IDL_MODE_STANDALONE) {
            idl_genTypeSeqHolder(scope, name, idl_typeSpec(unionSpec));
        }
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
        (idl_typeSpecType (idl_typeDefActual (defSpec)) == idl_tunion)) {
        if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
            idl_genInterface(scope, name, "TypeSupport", idl_typeSpec(defSpec), FALSE);
            idl_genInterface(scope, name, "DataReader", idl_typeSpec(defSpec), TRUE);
            idl_genInterface(scope, name, "DataReaderView", idl_typeSpec(defSpec), TRUE);
            idl_genInterface(scope, name, "DataWriter", idl_typeSpec(defSpec), TRUE);
            if (idl_getCorbaMode() == IDL_MODE_STANDALONE) {
                idl_genTypeSeqHolder(scope, name, idl_typeDefActual(defSpec));
            }
        }
    }
}

static struct idl_program
idl_genSajTypedClass = {
    NULL,
    idl_fileOpen,
    NULL, /* idl_fileClose */
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
idl_genSajTypedClassProgram(
    void)
{
    return &idl_genSajTypedClass;
}
