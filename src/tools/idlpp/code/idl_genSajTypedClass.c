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
#include "idl_genSplHelper.h"
#include "idl_genJavaHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_genMetaHelper.h"
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

/* Generate string in the format "<old>:<new>;<old>:<new>". */
static os_char *
idl_packageRedirects (
    void)
{
    idl_packageRedirect redirect;
    os_uint32 count, total;
    os_size_t length;
    os_char *macro, *module, *package, *buffer;

    total = os_iterLength (idl_genJavaHelperPackageRedirects);
    length = 0;
    macro = NULL;
    for (count = 0; count < total; count++) {
        redirect = idl_packageRedirect(os_iterObject (
            idl_genJavaHelperPackageRedirects, count));
        assert (redirect != NULL);
        module = redirect->module;
        package = redirect->package;

        assert (package != NULL);
        if (module == NULL) {
            module = "";
        }

        length += strlen (module) + strlen (package) + 3; /* ';' + ':' + '\0' */
        buffer = os_malloc (length);
        memset (buffer, '\0', length);
        if (macro != NULL) {
            (void)snprintf (buffer, length, "%s;%s:%s",macro, module, package);
            os_free (macro);
        } else {
            (void)snprintf (buffer, length, "%s:%s", module, package);
        }
        macro = buffer;
    }

    if (macro == NULL) {
        macro = os_strdup ("null");
    } else {
        length = strlen (macro) + 3; /* '"' + '"' */
        buffer = os_malloc (length);
        (void)snprintf (buffer, length, "\"%s\"", macro);
        os_free (macro);
        macro = buffer;
    }

    return macro;
}

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
    struct os_stat_s tmplStat;
    unsigned int nRead;
    os_char *redirects;
    char *scopedMetaTypeName;
    const char *internalTypeName;
    const char *keyList;
    char *scopeStackJavaDot = idl_scopeStackJava(scope, ".", name);
    char *scopeStackJavaSlash = idl_scopeStackJava(scope, "/", name);
    char *typeSpecName = idl_typeSpecName(typeSpec);
    char *javaId = idl_javaId(name);
    int result = 0;
    tmplPath = os_getenv("OSPL_TMPL_PATH");
    orbPath = os_getenv("OSPL_ORB_PATH");
    if (tmplPath == NULL) {
        printf("OSPL_TMPL_PATH not defined\n");
        result = -1;
        goto err_exit;
    }
    if (orbPath == NULL) {
        printf("OSPL_ORB_PATH not defined\n");
        result = -1;
        goto err_exit;
    }

    idlpp_macroSet = idl_macroSetNew();
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("type-name", javaId));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("actual-type-name", typeSpecName));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scoped-type-name", scopeStackJavaDot));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("java-class-name", scopeStackJavaSlash));

    redirects = idl_packageRedirects ();
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("java-package-redirects", redirects));
    os_free(redirects);

    scopedMetaTypeName = idl_scopeStack(scope, "::", name);
    internalTypeName = idl_internalTypeNameForBuiltinTopic(scopedMetaTypeName);
    keyList = idl_keyResolve(idl_keyDefDefGet(), scope, name);
    if ((strlen(internalTypeName) != 0) &&
        ((keyList == NULL) ||
         (strcmp(keyList,"key") == 0))) {
        keyList = "key.localId,key.systemId";
    }
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scoped-meta-type-name", scopedMetaTypeName));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("internal-type-name", internalTypeName));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scoped-actual-type-name", idl_corbaJavaTypeFromTypeSpec(typeSpec)));
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("key-list", keyList));
    os_free(scopedMetaTypeName);

    /* Generate only in standalone mode */
    if (idl_getCorbaMode() == IDL_MODE_STANDALONE) {
        snprintf(pname, sizeof (pname), "%s%s", javaId, class_base);
        idl_openJavaPackage(scope, pname);
        if (idl_fileCur() == NULL) {
            result = -1;
            goto err_exit;
        }
        /* Prepare Interface class */
        if (generateInterfaceClass) {
            snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%ctmpl%s.java", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR, class_base);
            /* QAC EXPECT 3416; No side effects here */
            if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
                (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
                printf ("No template found or protection violation (%s)\n", tmplFileName);
                result = -1;
                goto err_exit;
            }
            /* QAC EXPECT 5007; will not use wrapper */
            idlpp_template = os_malloc(tmplStat.stat_size+1);
            tmplFile = open(tmplFileName, O_RDONLY);
            nRead = (unsigned int)read(tmplFile, idlpp_template, tmplStat.stat_size);
            memset(&idlpp_template[nRead], 0, tmplStat.stat_size+1-nRead);
            close(tmplFile);
            idlpp_macroAttrib = idl_macroAttribNew(IDL_TOKEN_START, IDL_TOKEN_OPEN, IDL_TOKEN_CLOSE);
            idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);

            te = idl_tmplExpNew(idlpp_macroSet);
            idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
            idl_streamInFree(idlpp_inStream);
            idl_tmplExpFree(te);
            os_free(idlpp_template);
        }
        idl_closeJavaPackage();

        snprintf(pname, sizeof(pname), "%s%sHolder", javaId, class_base);
        idl_openJavaPackage(scope, pname);
        if (idl_fileCur() == NULL) {
            result = -1;
            goto err_exit;
        }
        /* Prepare typeSupportHolder class */
        snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%ctmpl%sHolder.java", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR, class_base);
        /* QAC EXPECT 3416; No side effects here */
        if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
            (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
            printf ("No template found or protection violation (%s)\n", tmplFileName);
            result = -1;
            goto err_exit;
        }
        /* QAC EXPECT 5007; will not use wrapper */
        idlpp_template = os_malloc(tmplStat.stat_size+1);
        tmplFile = open(tmplFileName, O_RDONLY);
        nRead = (unsigned int)read(tmplFile, idlpp_template, tmplStat.stat_size);
        memset(&idlpp_template[nRead], 0, tmplStat.stat_size+1-nRead);
        close(tmplFile);
        idlpp_macroAttrib = idl_macroAttribNew(IDL_TOKEN_START, IDL_TOKEN_OPEN, IDL_TOKEN_CLOSE);
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        os_free(idlpp_template);

        te = idl_tmplExpNew(idlpp_macroSet);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
        idl_closeJavaPackage();

        snprintf(pname, sizeof(pname), "%s%sHelper", javaId, class_base);
        idl_openJavaPackage(scope, pname);
        if (idl_fileCur() == NULL) {
            result = -1;
            goto err_exit;
        }
        /* Prepare typeSupportHelper class */
        snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%ctmpl%sHelper.java", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR, class_base);
        /* QAC EXPECT 3416; No side effects here */
        if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
            (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
            printf ("No template found or protection violation (%s)\n", tmplFileName);
            result = -1;
            goto err_exit;
        }
        /* QAC EXPECT 5007; will not use wrapper */
        idlpp_template = os_malloc(tmplStat.stat_size+1);
        tmplFile = open(tmplFileName, O_RDONLY);
        nRead = (unsigned int)read(tmplFile, idlpp_template, tmplStat.stat_size);
        memset(&idlpp_template[nRead], 0, tmplStat.stat_size+1-nRead);
        close(tmplFile);
        idlpp_macroAttrib = idl_macroAttribNew(IDL_TOKEN_START, IDL_TOKEN_OPEN, IDL_TOKEN_CLOSE);
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        os_free(idlpp_template);

        te = idl_tmplExpNew(idlpp_macroSet);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
        idl_closeJavaPackage();

        snprintf(pname, sizeof(pname), "%s%sOperations", javaId, class_base);
        idl_openJavaPackage(scope, pname);
        if (idl_fileCur() == NULL) {
            result = idl_abort;
            goto err_exit;
        }
        /* Prepare typeSupportOperations class */
        snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%ctmpl%sOperations.java", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR, class_base);
        /* QAC EXPECT 3416; No side effects here */
        if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
            (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
            printf ("No template found or protection violation (%s)\n", tmplFileName);
            result = -1;
            goto err_exit;
        }
        /* QAC EXPECT 5007; will not use wrapper */
        idlpp_template = os_malloc(tmplStat.stat_size+1);
        tmplFile = open(tmplFileName, O_RDONLY);
        nRead = (unsigned int)read(tmplFile, idlpp_template, tmplStat.stat_size);
        memset(&idlpp_template[nRead], 0, tmplStat.stat_size+1-nRead);
        close(tmplFile);
        idlpp_macroAttrib = idl_macroAttribNew(IDL_TOKEN_START, IDL_TOKEN_OPEN, IDL_TOKEN_CLOSE);
        idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
        os_free(idlpp_template);

        te = idl_tmplExpNew(idlpp_macroSet);
        idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
        idl_streamInFree(idlpp_inStream);
        idl_tmplExpFree(te);
        idl_closeJavaPackage();
    }

    if (generateInterfaceClass) {
        /* Implementation with Impl extension */
        snprintf(pname, sizeof(pname), "%s%sImpl", javaId, class_base);
    } else {
        /* Implementation without Impl extension */
        snprintf(pname, sizeof(pname), "%s%s", javaId, class_base);
    }
    idl_openJavaPackage(scope, pname);
    if (idl_fileCur() == NULL) {
        result = -1;
        goto err_exit;
    }
    /* Prepare typeSupportStub class */
    snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%ctmpl%sImpl.java", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR, class_base);
    /* QAC EXPECT 3416; No side effects here */
    if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
        (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
        printf("No template found or protection violation (%s)\n", tmplFileName);
        result = idl_abort;
        goto err_exit;
    }
    /* QAC EXPECT 5007; will not use wrapper */
    idlpp_template = os_malloc(tmplStat.stat_size+1);
    tmplFile = open(tmplFileName, O_RDONLY);
    nRead = (unsigned int)read(tmplFile, idlpp_template, tmplStat.stat_size);
    memset(&idlpp_template[nRead], 0, tmplStat.stat_size+1-nRead);
    close(tmplFile);
    idlpp_macroAttrib = idl_macroAttribNew(IDL_TOKEN_START, IDL_TOKEN_OPEN, IDL_TOKEN_CLOSE);
    idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
    os_free(idlpp_template);

    te = idl_tmplExpNew(idlpp_macroSet);
    idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
    idl_streamInFree(idlpp_inStream);
    idl_tmplExpFree(te);
    idl_closeJavaPackage();
err_exit:    
    os_free(javaId);
    os_free(scopeStackJavaDot);
    os_free(scopeStackJavaSlash);
    

    return result;
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
    struct os_stat_s tmplStat;
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

    /* Prepare typeSupport class */
    snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%ctmplSeqHolder.java", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
    /* QAC EXPECT 3416; No side effects here */
    if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
        (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
        printf("No template found or protection violation (%s)\n", tmplFileName);
        return -1;
    }
    /* QAC EXPECT 5007; will not use wrapper */
    idlpp_template = os_malloc(tmplStat.stat_size+1);
    tmplFile = open(tmplFileName, O_RDONLY);
    nRead = (unsigned int)read(tmplFile, idlpp_template, tmplStat.stat_size);
    memset(&idlpp_template[nRead], 0, tmplStat.stat_size+1-nRead);
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
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    return idl_explore;
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
    OS_UNUSED_ARG(userData);

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
    OS_UNUSED_ARG(userData);

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
    OS_UNUSED_ARG(userData);

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
