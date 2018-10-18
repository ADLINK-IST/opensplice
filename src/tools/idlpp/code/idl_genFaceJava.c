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
#include "idl_genJavaHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_genMetaHelper.h"
#include "idl_tmplExp.h"

#include "idl_keyDef.h"
#include "idl_typeSpecifier.h"

#include "os_heap.h"
#include "os_stdlib.h"
#include "os_abstract.h"

#include <ctype.h>

static idl_macroSet    idlpp_macroSet     = NULL;
static idl_macroAttrib idlpp_macroAttrib  = NULL;
static c_long          idlpp_indent_level = 0;

/* QAC EXPECT 0285; Need dollar here, this is specified */
#define IDL_TOKEN_START         '$'
#define IDL_TOKEN_OPEN          '('
#define IDL_TOKEN_CLOSE         ')'

#define MAX_ULONG_STRLENGTH 10 /* Room for 9 digits, enough for 32-bit range. */

static char *strnpatsubst(
    char *dest,
    const char *src,
    const char *pattern,
    const char *subst,
    size_t n)
{
    char *r;
    size_t i, j, k;

    r = strstr(src, pattern);
    if (r) {
        for (i=0; i<n && &src[i] != r; i++) {
            dest[i] = src[i];
        }
        j = i;
        for (k=0 ; i<n && subst[i] != '\0'; i++, k++) {
            dest[i] = subst[k];
        }
        for (k=j+strlen(pattern); i<n && src[k] != '\0'; i++, k++) {
            dest[i] = src[k];
        }
        for ( ; i < n; i++) {
            dest[i] = '\0';
        }
        r = dest;
    }

    return r;
}

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
    char *scopeName = idl_scopeStackJava(scope, ".", NULL);

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    idlpp_macroSet = idl_macroSetNew();
    idlpp_indent_level = 0;

    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scope-name", scopeName));
    os_free(scopeName);

    return idl_explore;
}

/**
 * Just close out the file with an endif.
 */
static void
idl_fileClose(
    void *userData)
{
    OS_UNUSED_ARG(userData);

    if (idlpp_macroAttrib) {
        idl_macroAttribFree(idlpp_macroAttrib);
        idlpp_macroAttrib = NULL;
    }
    if (idlpp_macroSet) {
        idl_macroSetFree(idlpp_macroSet);
        idlpp_macroSet = NULL;
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

static int
idl_genFaceInterface(
    idl_scope scope,
    const char *name)
{
    char *package;
    char tmplFileName[1024];
    char *tmplPath;
    char tmplFacePath[64];
    char *template;
    char *scopedTypeName;
    idl_streamIn inStream;
    idl_tmplExp te;
    size_t i;
    char fname[128];
    char *fullfname;
    char *javaId;
    struct os_stat_s tmplStat;
    os_size_t nRead;
    FILE *fp;

    const char *tmplFiles[] = {
        "tmplTS.java",
        "tmplRead_Callback.java",
        "tmplRead_CallbackHolder.java",
        "tmplRead_CallbackOperations.java"
    };

    package = idl_scopeStackJava(scope, ".", NULL);
    if (package != NULL && strlen (package) > 0) {
        os_size_t size = strlen(package) + strlen("package") + 3;
        char *pstr = os_malloc(size);
        snprintf(pstr, size, "package %s;", package);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("package-line", pstr));
        os_free(pstr);
    } else {
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("package-line", ""));
    }
    os_free(package);
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("type-name", name));
    scopedTypeName = idl_scopeStackJava(scope, ".", name);
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("scoped-type-name", scopedTypeName));
    os_free(scopedTypeName);

    tmplPath = os_getenv("OSPL_TMPL_PATH");
    if (tmplPath == NULL) {
        printf("OSPL_TMPL_PATH not defined\n");
        return -1;
    }
    snprintf(tmplFacePath, sizeof(tmplFacePath), "FACE%cSAJ", OS_FILESEPCHAR);

    for (i = 0; i<sizeof(tmplFiles)/sizeof(tmplFiles[0]); i++) {
        snprintf(tmplFileName, sizeof(tmplFileName), "%s%c%s%c%s", tmplPath, OS_FILESEPCHAR, tmplFacePath, OS_FILESEPCHAR, tmplFiles[i]);
        /* QAC EXPECT 3416; No side effects here */
        if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
            (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
            printf ("No template found or protection violation (%s)\n", tmplFileName);
            return -1;
        }
        /* QAC EXPECT 5007; will not use wrapper */
        template = os_malloc(tmplStat.stat_size+1);
        fp = fopen(tmplFileName, "r");
        if (fp) {
            nRead = fread(template, 1, tmplStat.stat_size, fp);
            memset(&template[nRead], 0, tmplStat.stat_size+1-nRead);
            (void)fclose(fp);
        } else {
            printf ("failed to open file \'%s\'", tmplFileName);
            os_free(template);
            return -1;
        }

        javaId = idl_javaId(name);
        strnpatsubst(fname, tmplFiles[i], "tmpl", javaId, sizeof(fname));
        fullfname = idl_scopeStackJava(scope, os_fileSep(), fname);
        /* Directory should already exist when coming here */
        idl_fileSetCur(idl_fileOutNew (fullfname, "w"));
        if (idl_fileCur() == NULL) {
            idl_fileOpenError(fullfname);
        }
        os_free(javaId);
        os_free(fullfname);

        idlpp_macroAttrib = idl_macroAttribNew(IDL_TOKEN_START, IDL_TOKEN_OPEN, IDL_TOKEN_CLOSE);
        inStream = idl_streamInNew(template, idlpp_macroAttrib);
        te = idl_tmplExpNew(idlpp_macroSet);
        idl_tmplExpProcessTmpl(te, inStream, idl_fileCur());
        idl_streamInFree(inStream);
        idl_tmplExpFree(te);

        os_free(template);
        idl_fileOutFree(idl_fileCur());
    }
    return 0;
}

static idl_action
idl_structureOpen(
    idl_scope scope,
    const char *name,
    idl_typeStruct structSpec,
    void *userData)
{
    OS_UNUSED_ARG(structSpec);
    OS_UNUSED_ARG(userData);

    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        idl_genFaceInterface(scope, name);
    }
    return idl_abort;
}

static struct idl_program idl_genMyActions = {
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
    NULL, /* idl_unionOpen */
    NULL, /* idl_unionOpen */
    NULL, /* idl_unionCaseOpenClose */
    NULL, /* idl_unionLabelsOpenClose */
    NULL, /* idl_unionLabelOpenClose */
    NULL, /* idl_typedefOpenClose */
    NULL, /* idl_boundedStringOpenClose */
    NULL, /* idl_sequenceOpenClose */
    NULL, /* idl_constantOpenClose */
    NULL, /* idl_artificialDefaultLabelOpenClose */
    NULL  /* userData */
};

idl_program
idl_genFaceJavaTmplProgram (
    void)
{
    return &idl_genMyActions;
}
