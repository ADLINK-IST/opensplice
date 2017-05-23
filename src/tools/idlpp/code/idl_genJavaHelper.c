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
#include "idl_genJavaHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_scope.h"
#include "idl_tmplExp.h"
#include "idl_typeSpecifier.h"

#include "os_iterator.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_string.h"
#include "os_errno.h"

static os_equality idl_genJavaHelperComparePackageRedirect (const void *obj1, const void *obj2);

os_iter idl_genJavaHelperPackageRedirects = NULL;

/* Specify a list of all C keywords */
static const char *java_keywords[61] = {
    /* Java keywords */
    "abstract", "assert", "boolean", "break", "default", "do", "double", "enum",
    "if", "implements", "import", "private", "protected", "public", "throw",
    "throws", "transient", "byte", "case", "catch", "char", "class", "const",
    "continue", "else", "extends", "final", "finally", "float", "for", "goto",
    "instanceof", "int", "interface", "long", "native", "new", "package",
    "return", "short", "static", "super", "switch", "synchronized", "this",
    "try", "void", "volatile", "while",
    /* Java constants */
    "true", "false", "null",
    /* java.lang.Object operations */
    "clone", "equals", "finalize", "getClass", "hashCode", "notify", "notifyAll",
    "toString", "wait"
};

/* Translate an IDL identifier into a Java language identifier.
   The IDL specification states that all identifiers that match
   a Java keyword must be prepended by "_".
*/
c_char *
idl_javaId(
    const char *identifier)
{
    size_t i;
    char *javaId;
    char *helperEnding;
    char *holderEnding;
    char *operationsEnding;
    char *packageEnding;

    /* search through the Java keyword list */
    /* QAC EXPECT 5003; Bypass qactools error, why is this a violation */
    for (i = 0; i < sizeof(java_keywords)/sizeof(c_char *); i++) {
	/* QAC EXPECT 5007, 3416; will not use wrapper, no side effects here */
	if (strcmp (java_keywords[i], identifier) == 0) {
	    /* If a keyword matches the specified identifier, prepend _ */
	    /* QAC EXPECT 5007; will not use wrapper */
	    javaId = os_malloc(strlen(identifier)+1+1);
	    snprintf(javaId, strlen(identifier)+1+1, "_%s", identifier);
	    return javaId;
	}
    }
    /* Check for reserved endings
     * - <type>Helper
     * - <type>Holder
     * - <interface>Operations
     * - <type>Package
     */
    helperEnding = strstr("Helper", identifier);
    holderEnding = strstr("Holder", identifier);
    operationsEnding = strstr("Operations", identifier);
    packageEnding = strstr("Package", identifier);
    if ((helperEnding && strcmp(helperEnding, "Helper") == 0) ||
	(holderEnding && strcmp(holderEnding, "Holder") == 0) ||
	(operationsEnding && strcmp(operationsEnding, "Operations") == 0) ||
	(packageEnding && strcmp(packageEnding, "Package") == 0)) {
	javaId = os_malloc(strlen(identifier)+1+1);
	snprintf(javaId, strlen(identifier)+1+1, "_%s", identifier);
    } else  {
        /* No match with a keyword is found, thus return the identifier itself */
        javaId = os_strdup(identifier);
    }
    return javaId;
    /* QAC EXPECT 2006; performance is selected above rules here */
}

static c_char *
idl_scopeJavaElementName (
    idl_scopeElement scope)
{
    c_char *scopeName;
    c_char *scopeJavaName;

    scopeName = idl_scopeElementName(scope);
    if ((idl_scopeElementType(scope) == idl_tStruct) ||
	(idl_scopeElementType(scope) == idl_tUnion)) {
	scopeJavaName = os_malloc(strlen(scopeName) + 8);
	sprintf(scopeJavaName, "%sPackage", scopeName);
    } else {
	scopeJavaName = scopeName;
    }
    return scopeJavaName;
}

/* Build a textual presentation of the provided scope stack taking the
   Java keyword identifier translation into account. Further the function
   equals "idl_scopeStack".
*/
c_char *
idl_scopeStackJava (
    idl_scope scope,
    const char *scopeSepp,
    const char *name)
{
    c_long si;
    c_long sz;
    c_char *scopeStack = NULL;
    c_char *Id;
    os_char *substitute;
    os_char *module, *package;
    os_uint32 cnt, rlen;
    idl_packageRedirect redirect;

    assert (scopeSepp != NULL);
    scopeStack = os_strdup ("");

    for (si = 0, sz = idl_scopeStackSize(scope); si < sz; si++) {
        size_t slen;

        /* Translate the scope name to a C identifier */
        Id = idl_javaId(idl_scopeJavaElementName(idl_scopeIndexed(scope, si)));
        /* allocate space for the current scope stack + the separator
           and the next scope name */
        /* QAC EXPECT 5007; will not use wrapper */
        slen = strlen (scopeStack) + strlen (scopeSepp) + strlen (Id);
        scopeStack = os_realloc(scopeStack, slen + 1);
        /* Concatenate the separator */
        /* QAC EXPECT 5007; will not use wrapper */
        if (strlen(scopeStack)) {
            os_strcat(scopeStack, scopeSepp);
        }
        /* Concatenate the scope name */
        /* QAC EXPECT 5007; will not use wrapper */
        os_strcat (scopeStack, Id);
    }

    /* idl_genJavaHelperPackageRedirects must be sorted */
    substitute = scopeStack;
    rlen = os_iterLength (idl_genJavaHelperPackageRedirects);
    for (cnt = 0; cnt < rlen && substitute == scopeStack; cnt++) {
        redirect = idl_packageRedirect (
            os_iterObject (idl_genJavaHelperPackageRedirects, cnt));
        assert (redirect != NULL);

        package = os_str_replace (redirect->package, ".", scopeSepp, 0);
        if (package != NULL) {
            if (redirect->module != NULL) {
                module = os_str_replace (redirect->module, ".", scopeSepp, 0);
                if (module == NULL) {
                    substitute = NULL;
                } else {
                    substitute = os_str_word_replace (
                        scopeStack, scopeSepp, module, package, 1);

                    if (module != redirect->module) {
                        os_free (module);
                    }
                }
            } else {
                substitute = os_malloc (
                    strlen (package) +
                    strlen (scopeSepp) +
                    strlen (scopeStack) +
                    1 /* '\0' */);
                if (substitute != NULL) {
                    (void)strcpy (substitute, package);
                    if (strlen (scopeStack)) {
                        (void)os_strcat (substitute, scopeSepp);
                        (void)os_strcat (substitute, scopeStack);
                    }
                }
            }

            if (package != redirect->package) {
                os_free (package);
            }
        }
    }

    if (substitute != scopeStack) {
        os_free (scopeStack);
        scopeStack = substitute;
    }

    if (name) {
        /* A user identifier is specified */
        /* Translate the user identifier to a Java identifier */
        Id = idl_javaId(name);
        /* allocate space for the current scope stack + the separator
           and the user identifier */
        /* QAC EXPECT 5007; will not use wrapper */
        scopeStack = os_realloc(scopeStack, strlen(scopeStack)+strlen(scopeSepp)+strlen(Id)+1);
        /* Concatenate the separator */
        /* QAC EXPECT 5007; will not use wrapper */
        if (strlen(scopeStack)) {
            os_strcat(scopeStack, scopeSepp);
        }
        /* Concatenate the user identifier */
        /* QAC EXPECT 5007; will not use wrapper */
        os_strcat (scopeStack, Id);
    }

    /* return the scope stack representation */
    return scopeStack;
}

/* Return the C specific type identifier for the
   specified type specification
*/
c_char *
idl_corbaJavaTypeFromTypeSpec (
    idl_typeSpec typeSpec)
{
    c_char *typeName = NULL;

    /* QAC EXPECT 3416; No side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        /* if the specified type is a basic type */
	switch (idl_typeBasicType(idl_typeBasic(typeSpec))) {
	case idl_short:
	case idl_ushort:
	    typeName = os_strdup("short");
	    break;
	case idl_long:
	case idl_ulong:
	    typeName = os_strdup("int");
	    break;
	case idl_longlong:
	case idl_ulonglong:
	    typeName = os_strdup("long");
	    break;
	case idl_float:
	    typeName = os_strdup("float");
	    break;
	case idl_double:
	    typeName = os_strdup("double");
	    break;
	case idl_char:
	    typeName = os_strdup("char");
	    break;
	case idl_string:
	    typeName = os_strdup("java.lang.String");
	    break;
	case idl_boolean:
	    typeName = os_strdup("boolean");
	    break;
	case idl_octet:
	    typeName = os_strdup("byte");
	    break;
	default:
	    /* No processing required, empty statement to satisfy QAC */
	    break;
	/* QAC EXPECT 2016; Default case must be empty here */
	}
        /* QAC EXPECT 3416; No side effects here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
	return idl_corbaJavaTypeFromTypeSpec (idl_typeSeqActual(idl_typeSeq (typeSpec)));
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
#if 0
	/* sequence does not have an identification */
	typeName = os_strdup ("");
	printf ("idl_corbaJavaTypeFromTypeSpec: Unexpected type handled\n");
#else
	return idl_corbaJavaTypeFromTypeSpec (idl_typeArrayActual(idl_typeArray (typeSpec)));
#endif
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
	return idl_corbaJavaTypeFromTypeSpec (idl_typeDefActual(idl_typeDef (typeSpec)));
    } else {
        /* if a user type is specified build it from its scope and its name.
	   The type should be one of idl_ttypedef, idl_tenum, idl_tstruct,
           idl_tunion.
	*/
        typeName = idl_scopeStackJava(
            idl_typeUserScope(idl_typeUser(typeSpec)),
            ".",
            idl_typeSpecName(typeSpec));
    }
    return typeName;
    /* QAC EXPECT 5101; The switch statement is simple, therefor the total complexity is low */
}

/* FIXME: replace by os_mkpath once it's merged? */
static int
idl_createDir (
    os_char* fname)
{
    char *pathName;
    os_result statRes;
    struct os_stat statbuf;
    char* outdir;
    os_char* stackScope;
    os_char* token;
    const os_char* osSep;

    /* to allow nested modules which may potentially lead to very long paths,
     * pathName is not allocated fixed-size on stack
     * but dynamically on heap
     */
    pathName = os_malloc(1);
    pathName[0] = '\0';
    outdir = idl_dirOutCur();
    if(outdir){
        pathName = os_realloc(pathName, strlen(outdir) + strlen(os_fileSep()) + 1);
        os_sprintf(pathName, "%s%s", outdir, os_fileSep());
    }
    /* make sure that we replace the os file seperator with a simple character
     * like '/', this will allow us to parse easier later.
     */
    osSep = os_fileSep();
    stackScope = os_str_replace (fname, osSep, "/", 0);

    if(stackScope[0] != '\0'){ /* strlen(stackScope) > 0 */
        do
        {
            token = strchr(stackScope, '/');
            /* make sure this is not the last part of the file name, for example
             * for the file name org/opensplice/foo.java we only want to create
             * directories for org and opensplice, the foo.java part is not a
             * directory. So we can simply state if no '/' char can be
             * found then we must be at the end part of the file name!
             */
            if(token)
            {
                *token = '\0';
                token++;
                /* reallocate pathName to include stackScope */
                pathName = os_realloc(pathName, strlen(pathName) + strlen(stackScope) + 1);
                os_strcat (pathName, stackScope);
                stackScope = token;
                statRes = os_stat (pathName, &statbuf);
                if (statRes == os_resultFail) {
                    /* @todo                                                      */
                    /* Assume the file does not exist. On some platforms          */
                    /* a check to see if errno == ENOENT would be more conclusive */
                    /* That fails on WIN32 however because stat is not fully      */
                    /* compatible. Only an os_stat implementation can solve that. */

                    os_mkdir(pathName, 0777);
                    statRes = os_stat(pathName, &statbuf);
                } else {
                    if (!OS_ISDIR(statbuf.stat_mode)) {
                        printf ("File %s already exists, but is not a directory\n", pathName);
                        os_free(pathName);
                        return 0;
                    }
                }
                if (statRes == os_resultFail) {
                    printf ("Error when creating directory %s\n", pathName);
                    os_free(pathName);
                    return 0;
                }
                /* reallocate pathName to include os_fileSep() */
                pathName = os_realloc(pathName, strlen(pathName) + strlen(stackScope) + strlen(os_fileSep()) + 1);
                os_strcat (pathName, os_fileSep());
            }
        } while(token);
    }
    /* free the memory allocated to pathName */
    os_free(pathName);
    return 1;
}

void
idl_openJavaPackage (
    idl_scope scope,
    const char *name)
{
    os_char *fname = NULL;
    os_char *package_file = NULL;
    os_char *package = NULL;

    package_file = idl_scopeStackJava(scope, os_fileSep(), name);
    fname = os_malloc(strlen (package_file) + strlen (".java") + 1);
    os_strcpy(fname, package_file);
    os_strcat(fname, ".java");
    if (idl_createDir(fname)) {
	    idl_fileSetCur(idl_fileOutNew (fname, "w"));
        if (idl_fileCur() == NULL) {
            idl_fileOpenError(fname);
        }
        /* Add package header if applicable */
        package = idl_scopeStackJava(scope, ".", NULL);
        if (package != NULL && strlen (package) == 0) {
            os_free (package);
            package = NULL;
        }
        if (package != NULL) {
            idl_fileOutPrintf(idl_fileCur(), "package %s;\n", package);
            idl_fileOutPrintf(idl_fileCur(), "\n");
        }
    } else {
	    idl_fileSetCur(NULL);
    }
    os_free(package);
    os_free(package_file);
    os_free(fname);
}

void
idl_closeJavaPackage (
    void)
{
    if (idl_fileCur()) {
	idl_fileOutFree(idl_fileCur());
    }
}

c_char *
idl_labelJavaEnumVal (
    const char *typeEnum,
    idl_labelEnum labelVal)
{
    c_char *scopedName;

    scopedName = os_malloc(strlen(typeEnum) + strlen(idl_labelEnumImage(labelVal)) + 2);
    os_strcpy(scopedName, typeEnum);
    os_strcat(scopedName, ".");
    os_strcat(scopedName,idl_labelEnumImage(labelVal));

    return scopedName;
}

c_char *
idl_sequenceIndexString (
    idl_typeSeq typeSeq)
{
    c_char *sequenceString;
    os_size_t sequenceStringLen;
    c_char *sequenceStringPrev;

    if (idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_tseq) {
        sequenceStringPrev = idl_sequenceIndexString(idl_typeSeq(idl_typeSeqType (typeSeq)));
        sequenceStringLen = strlen(sequenceStringPrev) + 3;
        sequenceString = os_malloc(sequenceStringLen);
        snprintf(sequenceString, sequenceStringLen, "[]%s", sequenceStringPrev);
    } else if (idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_tarray) {
        sequenceStringPrev = idl_arrayJavaIndexString(idl_typeArray(idl_typeSeqType (typeSeq)));
        sequenceStringLen = strlen(sequenceStringPrev) + 3;
        sequenceString = os_malloc(sequenceStringLen);
        snprintf(sequenceString, sequenceStringLen, "[]%s", sequenceStringPrev);
    } else if (idl_typeSpecType(idl_typeSeqType(typeSeq)) == idl_ttypedef &&
	idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeSeqType(typeSeq)))) == idl_tseq) {
        sequenceStringPrev = idl_sequenceIndexString(idl_typeSeq(idl_typeDefActual(idl_typeDef(idl_typeSeqType(typeSeq)))));
        sequenceStringLen = strlen(sequenceStringPrev) + 3;
        sequenceString = os_malloc(sequenceStringLen);
        snprintf(sequenceString, sequenceStringLen, "[]%s", sequenceStringPrev);
    } else if (idl_typeSpecType(idl_typeSeqType (typeSeq)) == idl_ttypedef &&
	idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeSeqType(typeSeq)))) == idl_tarray) {
        sequenceStringPrev = idl_arrayJavaIndexString(idl_typeArray(idl_typeDefActual(idl_typeDef(idl_typeSeqType(typeSeq)))));
        sequenceStringLen = strlen(sequenceStringPrev) + 3;
        sequenceString = os_malloc(sequenceStringLen);
        snprintf(sequenceString, sequenceStringLen, "[]%s", sequenceStringPrev);
    } else {
        sequenceString = os_malloc(3);
        snprintf(sequenceString, 3, "[]");
    }
    return sequenceString;
}

c_char *
idl_arrayJavaIndexString (
    idl_typeArray typeArray)
{
    c_char *arrayString;
    os_size_t arrayStringLen;
    c_char *arrayStringPrev;

    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray) {
        arrayStringPrev = idl_arrayJavaIndexString(idl_typeArray(idl_typeArrayType(typeArray)));
        arrayStringLen = strlen(arrayStringPrev) + 3;
        arrayString = os_malloc(arrayStringLen);
        snprintf(arrayString, arrayStringLen, "[]%s", arrayStringPrev);
    } else if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tseq) {
        arrayStringPrev = idl_sequenceIndexString(idl_typeSeq(idl_typeArrayType (typeArray)));
        arrayStringLen = strlen(arrayStringPrev) + 3;
        arrayString = os_malloc(arrayStringLen);
        snprintf(arrayString, arrayStringLen, "[]%s", arrayStringPrev);
    } else if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_ttypedef &&
	idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeArrayType(typeArray)))) == idl_tarray) {
        arrayStringPrev = idl_arrayJavaIndexString(idl_typeArray(idl_typeDefActual(idl_typeDef(idl_typeArrayType(typeArray)))));
        arrayStringLen = strlen(arrayStringPrev) + 3;
        arrayString = os_malloc(arrayStringLen);
        snprintf(arrayString, arrayStringLen, "[]%s", arrayStringPrev);
    } else if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_ttypedef &&
	idl_typeSpecType(idl_typeDefActual(idl_typeDef(idl_typeArrayType(typeArray)))) == idl_tseq) {
        arrayStringPrev = idl_sequenceIndexString(idl_typeSeq(idl_typeDefActual(idl_typeDef(idl_typeArrayType(typeArray)))));
        arrayStringLen = strlen(arrayStringPrev) + 3;
        arrayString = os_malloc(arrayStringLen);
        snprintf(arrayString, arrayStringLen, "[]%s", arrayStringPrev);
    } else {
        arrayString = os_malloc(3);
        snprintf(arrayString, 3, "[]");
    }
    return arrayString;
}

c_char *
idl_genJavaConstantGetter(void)
{
    return os_strdup(".value");
}

static os_equality
idl_genJavaHelperComparePackageRedirect (
    const void *obj1,
    const void *obj2)
{
    idl_packageRedirect rdr1, rdr2;
    os_equality eq = OS_EQ;
    os_int ret;

    assert (obj1 != NULL);
    assert (obj2 != NULL);

    rdr1 = idl_packageRedirect (obj1);
    rdr2 = idl_packageRedirect (obj2);

    if (rdr1->module != rdr2->module) {
        if (rdr1->module == NULL) {
            eq = OS_LT;
        } else if (rdr2->module == NULL) {
            eq = OS_GT;
        } else {
            ret = strcmp (rdr1->module, rdr2->module);
            if (ret < 0) {
                eq = OS_LT;
            } else if (ret > 0) {
                eq = OS_GT;
            }
        }
    }

    return eq;
}

os_result
idl_genJavaHelperAddPackageRedirect (
    const os_char *optarg)
{
    idl_packageRedirect exists, redirect = NULL;
    os_char *colon;
    os_char *module = NULL;
    os_char *package = NULL;
    os_char *trim = NULL;
    os_result result = os_resultSuccess;

    assert (optarg != NULL);

    colon = os_index (optarg, ':');
    if (colon != NULL) {
        if (colon == optarg) {
            module = NULL;
        } else {
            module = os_strndup (optarg, (size_t) (colon - optarg));
            if (module != NULL) {
                if ((trim = os_str_trim (module, "./:")) != module) {
                    os_free (module);
                }
                module = trim;
            }
            if (module == NULL) {
                result = os_resultFail;
            } else if (strlen (module) == 0) {
                os_free (module);
                module = NULL;
            }
        }

        package = os_strdup (colon + 1);
        if ((trim = os_str_trim (package, "./:")) != package) {
            os_free (package);
        }
        package = trim;

        if (strlen (package) == 0) {
            result = os_resultInvalid;
        }

        redirect = os_malloc (OS_SIZEOF (idl_packageRedirect));
        if (result == os_resultSuccess) {
            redirect->module = module;
            redirect->package = package;

            exists = os_iterResolve (
                idl_genJavaHelperPackageRedirects,
               &idl_genJavaHelperComparePackageRedirect,
                redirect);
            if (exists != NULL) {
                result = os_resultInvalid;
            } else {
                idl_genJavaHelperPackageRedirects = os_iterAppend (
                    idl_genJavaHelperPackageRedirects,
                    redirect);
                /* idl_genJavaHelperPackageRedirect must be sorted */
                os_iterSort (
                    idl_genJavaHelperPackageRedirects,
                   &idl_genJavaHelperComparePackageRedirect,
                    OS_FALSE);
            }
        }

        if (result != os_resultSuccess) {
            os_free (module);
            os_free (package);
            os_free (redirect);
        }
    } else {
        result = os_resultInvalid;
    }

    return result;
}

void
idl_genJavaHelperFreePackageRedirects (
    void)
{
    idl_packageRedirect redirect;
    os_iter redirects;
    void *object;

    redirects = idl_genJavaHelperPackageRedirects;
    if (redirects != NULL) {
        for (object = os_iterTakeFirst (redirects);
             object != NULL;
             object = os_iterTakeFirst (redirects))
        {
            redirect = idl_packageRedirect (object);
            os_free (redirect->module);
            os_free (redirect->package);
            os_free (redirect);
        }
        os_iterFree (redirects);
    }

    idl_genJavaHelperPackageRedirects = NULL;
}
