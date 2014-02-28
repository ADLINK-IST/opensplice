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
#include "idl_genJavaHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_scope.h"
#include "idl_tmplExp.h"
#include "idl_typeSpecifier.h"

#include "os_iterator.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include <errno.h>

static c_iter originalPackageList = NULL;
static c_iter targetPackageList = NULL;
static const char* orgLastSubstituted = NULL;
static const char* tarLastSubstituted = NULL;

static os_char*
idl_genJavaHelperApplyPackageSubstitute(
    os_char* source,
    const os_char *scopeSepp);

static os_char*
idl_genJavaHelperSubstitute(
    const os_char* string,
    const os_char* searchFor,
    const os_char* replaceWith,
    c_long* replaced);

static void
idl_reportOpenError(
    char *fname)
{
    printf ("Error opening file %s for writing. Reason: %s (%d)\n", fname, strerror( errno ), errno);
    exit (-1);
}

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
    c_long i;
    char *javaId;
    char *helperEnding;
    char *holderEnding;
    char *operationsEnding;
    char *packageEnding;

    /* search through the Java keyword list */
    /* QAC EXPECT 5003; Bypass qactools error, why is this a violation */
    for (i = 0; i < (c_long)(sizeof(java_keywords)/sizeof(c_char *)); i++) {
	/* QAC EXPECT 5007, 3416; will not use wrapper, no side effects here */
	if (strcmp (java_keywords[i], identifier) == 0) {
	    /* If a keyword matches the specified identifier, prepend _ */
	    /* QAC EXPECT 5007; will not use wrapper */
	    javaId = os_malloc((size_t)((int)strlen(identifier)+1+1));
	    snprintf(javaId, (size_t)((int)strlen(identifier)+1+1), "_%s", identifier);
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
	javaId = os_malloc((size_t)((int)strlen(identifier)+1+1));
	snprintf(javaId, (size_t)((int)strlen(identifier)+1+1), "_%s", identifier);
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

/* Build a textual presenation of the provided scope stack taking the
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
    c_char *scopeStack;
    c_char *Id;

    if(orgLastSubstituted) {
        os_free(orgLastSubstituted);
        orgLastSubstituted = NULL;
    }

    if(tarLastSubstituted) {
        os_free(tarLastSubstituted);
        tarLastSubstituted = NULL;
    }

    si = 0;
    sz = idl_scopeStackSize(scope);
    if (si < sz) {
        /* The scope stack is not empty */
        /* Copy the first scope element name */
        scopeStack = os_strdup(idl_javaId(idl_scopeJavaElementName(idl_scopeIndexed(scope, si))));
        si++;
        while (si < sz) {
            /* Translate the scope name to a C identifier */
            Id = idl_javaId(idl_scopeJavaElementName(idl_scopeIndexed(scope, si)));
            /* allocate space for the current scope stack + the separator
               and the next scope name
             */
            /* QAC EXPECT 5007; will not use wrapper */
            scopeStack = os_realloc(scopeStack, (size_t)(
                             (int)strlen(scopeStack)+
                             (int)strlen(scopeSepp)+
                             (int)strlen(Id)+1));
           /* Concatenate the separator */
           /* QAC EXPECT 5007; will not use wrapper */
           os_strcat(scopeStack, scopeSepp);
           /* Concatenate the scope name */
           /* QAC EXPECT 5007; will not use wrapper */
           os_strcat (scopeStack, Id);
           si++;
        }
        if(strlen(scopeStack) > 0)
        {
            os_char* ptr;
            os_char* ptr2;
            os_char* ptr3;

            /* es, dds1540: The following code is not pretty, but time limitations
             * required it's implementation. To ensure
             * proper substitution of for example package name 'Chat' with
             * 'org.opensplice.Chat' without substitution class names like
             * 'ChatMessage' to 'org.opensplice.ChatMessage' we need to take some
             * special arrangements. We need to allow the user to state he wants to
             * replace 'Chat.' with ''org.opensplice.Chat.', as this would resolve
             * the previously stated problem.
             * However this function for getting the scope stack is called in a
             * special way if only the package names are required (without the
             * specific class at the end). In these cases package Chat would become
             * in string format 'Chat' instead of 'Chat.'. And this would cause
             * problems when doing the substitution for the directory names and
             * package directives. So to ensure substitution always goes correctly
             * we added the scopeSepp to the end of the scopeStack and input that
             * into the substitution algorithm. After the algorithm we remove the
             * added scopeSepp again.
             * So not that nicely solved, but lack of time to do it more nicely
             * (which would be to support regular expression type things) --> No.
             */
            ptr = os_malloc(strlen(scopeStack)+ strlen(scopeSepp) + 1);
            os_strcpy(ptr, scopeStack);
            os_strncat(ptr, scopeSepp, strlen(scopeSepp));
            ptr2 = idl_genJavaHelperApplyPackageSubstitute(ptr, scopeSepp);
            memset(ptr, 0, strlen(ptr));
            os_free(ptr);
            ptr3 = strrchr(ptr2, *scopeSepp);
            if(ptr3)
            {
                *ptr3 = '\0';
            }
            ptr = os_strdup(ptr2);
            memset(ptr2, 0, strlen(ptr2));
            os_free(ptr2);
            memset(scopeStack, 0, strlen(scopeStack));
            os_free(scopeStack);
            scopeStack = ptr;
        }

        if (name) {
            /* A user identifier is specified */
            /* Translate the user identifier to a Java identifier */
            Id = idl_javaId(name);
            /* allocate space for the current scope stack + the separator
               and the user identifier
             */
            /* QAC EXPECT 5007; will not use wrapper */
            scopeStack = os_realloc(scopeStack, (size_t)(
                             (int)strlen(scopeStack)+
                             (int)strlen(scopeSepp)+
                             (int)strlen(Id)+1));
           /* Concatenate the separator */
           /* QAC EXPECT 5007; will not use wrapper */
           os_strcat(scopeStack, scopeSepp);
           /* Concatenate the user identifier */
           /* QAC EXPECT 5007; will not use wrapper */
           os_strcat (scopeStack, Id);
        }
     } else {
        /* The stack is empty */
        if (name) {
            /* A user identifier is specified */
            scopeStack = os_strdup(idl_javaId(name));
        } else {
            /* make the stack represenation empty */
            scopeStack = os_strdup("");
        }
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
        if (pathName == NULL) {
            printf("Memory allocation failure when creating idl directory\n");
            return 0;
        }
        os_sprintf(pathName, "%s%s", outdir, os_fileSep());
    }
    /* make sure that we replace the os file seperator with a simple character
     * like '/', this will allow us to parse easier later.
     */
    osSep = os_fileSep();
    stackScope = idl_genJavaHelperSubstitute(fname, osSep, "/", NULL);

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
                if (pathName == NULL) {
                    printf("Memory allocation failure when trying to create idl directory %s, \
                            probably the path has grown too long\n", stackScope);
                    return 0;
                }
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
                if (pathName == NULL) {
                    printf("Memory allocation failure when trying to add the \
                            file separator to idl directory %s", stackScope);
                    return 0;
                }
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
    os_char *fname;
    os_char *package_file;

    package_file = idl_scopeStackJava(scope, os_fileSep(), name);
    fname = os_malloc(strlen (package_file) + strlen (".java") + 1);
    os_strcpy(fname, package_file);
    os_strcat(fname, ".java");
    if (idl_createDir(fname)) {
	    idl_fileSetCur(idl_fileOutNew (fname, "w"));
        if (idl_fileCur() == NULL) {
            idl_reportOpenError(fname);
        }
    } else {
	    idl_fileSetCur(NULL);
    }
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
    int sequenceStringLen;
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
    int arrayStringLen;
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

void
idl_genJavaHelperInit(
    c_iter orgPackageList,
    c_iter tarPackageList)
{
    if(orgPackageList)
    {
        originalPackageList = orgPackageList;
    }
    if(tarPackageList)
    {
        targetPackageList = tarPackageList;
    }
}

const os_char*
idl_genJavaHelperGetOrgLastSubstituted(
    void)
{
    return orgLastSubstituted;
}

const os_char*
idl_genJavaHelperGetTgtLastSubstituted(
    void)
{
    return tarLastSubstituted;
}

/* Result must be freed with os_free! */
os_char*
idl_genJavaHelperApplyPackageSubstitute(
    os_char* source,
    const os_char *scopeSepp)
{
    os_char* result = NULL;
    os_char* tarPackageNameTMP;
    os_char* orgPackageNameTMP;
    assert(source);

    /* only if 'tarPackageName' is valid should there be any substitution done */
    if(targetPackageList)
    {
        c_iterIter targetIter, originalIter;
        os_char *tarPackageName, *orgPackageName;
        c_long replaced = 0;

        targetIter = c_iterIterGet(targetPackageList);
        originalIter = c_iterIterGet(originalPackageList);
        while ((tarPackageName = c_iterNext(&targetIter))) {
            orgPackageName = c_iterNext(&originalIter);
            /* if 'orgPackageName' is NULL, then we just need to prefix */
            if(!strlen(orgPackageName))
            {
                if(tarLastSubstituted) {
                    os_free(tarLastSubstituted);
                }
                tarLastSubstituted = os_strdup(tarPackageName);
                tarPackageNameTMP = idl_genJavaHelperSubstitute(tarPackageName, ".", scopeSepp, NULL);
                if(0 != strcmp(tarPackageNameTMP+(strlen(tarPackageNameTMP)-strlen(scopeSepp)), scopeSepp))
                {
                    if(result) {
                        os_free(result);
                    }
                    result = os_malloc(strlen(tarPackageNameTMP) + strlen(scopeSepp) + strlen(source) + 1);
                    os_strcpy(result, tarPackageNameTMP);
                    result = os_strncat(result, scopeSepp, strlen(scopeSepp));
                } else
                {
                    result = os_malloc(strlen(tarPackageNameTMP) + strlen(source) + 1);
                    os_strcpy(result, tarPackageNameTMP);
                }
                result = os_strncat(result, source, strlen(source));
                os_free(tarPackageNameTMP);
            } else
            {
                /* We need to substitute all occurrences of 'orgPackageName' with
                 * 'tarPackageName'
                 */
                char *tmpResult;
                c_long tmpReplaced;
                tarPackageNameTMP = idl_genJavaHelperSubstitute(tarPackageName, ".", scopeSepp, NULL);
                orgPackageNameTMP = idl_genJavaHelperSubstitute(orgPackageName, ".", scopeSepp, NULL);
                tmpResult = idl_genJavaHelperSubstitute(source, orgPackageNameTMP, tarPackageNameTMP, &tmpReplaced);
                if(tmpReplaced >= replaced) {
                    if(result) {
                        os_free(result);
                    }
                    result = tmpResult;
                    replaced = tmpReplaced;

                    /* Set global variables indicating which was the last substitution that took place. */
                    if(orgLastSubstituted) {
                        os_free(orgLastSubstituted);
                    }
                    if(tarLastSubstituted) {
                        os_free(tarLastSubstituted);
                    }
                    orgLastSubstituted = os_strdup(orgPackageNameTMP);
                    tarLastSubstituted = os_strdup(tarPackageNameTMP);
                }
                os_free(tarPackageNameTMP);
                os_free(orgPackageNameTMP);
            }
        }
    } else
    {
        /* No substitution to perform, just return the original string */
        result = os_strdup(source);
    }
    return result;
}

/* ES, dds1540: This exact operation is copied in the
 * api/dcps/java/common/c/code/saj_copyCache.c file. so any bugs fixed here, should be
 * fixed there as well!!
 */
os_char*
idl_genJavaHelperSubstitute(
    const os_char* string,
    const os_char* searchFor,
    const os_char* replaceWith,
    c_long* replaced)
{
    os_char* result;
    os_char* ptr;
    os_char* tmp;
    if(replaced) {
        *replaced = 0;
    }

    tmp = os_strdup(string);
    ptr = strstr(tmp, searchFor);
    if(ptr && strcmp(searchFor, replaceWith))
    {
        os_char* before;
        os_char* after;
        if(replaced) {
            *replaced = strlen(searchFor); /* Return the length of the replaced string. A calling function can call this function multiple times
                                            * with multiple replacements and select the one that replaced the most characters */
        }

        before = os_malloc(ptr - tmp+1);
        *ptr = '\0';
        os_strncpy(before, tmp, (size_t)(ptr - tmp+1));
        ptr = ptr+strlen(searchFor);
        after = idl_genJavaHelperSubstitute(ptr, searchFor, replaceWith, NULL);
        result = os_malloc(strlen(before) + strlen(replaceWith) + strlen (after) + 1);
        os_strcpy(result, before);
        os_strncat(result, replaceWith, strlen(replaceWith));
        os_strncat(result, after, strlen(after));
        os_free(before);
        os_free(after);
    } else
    {
        result = tmp;
        tmp = NULL;
    }
    if(tmp)
    {
        os_free(tmp);
    }

    return result;
}
