/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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

#include <os_iterator.h>
#include <os_heap.h>
#include <os_stdlib.h>

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
           strcat(scopeStack, scopeSepp);
           /* Concatenate the scope name */
           /* QAC EXPECT 5007; will not use wrapper */
           strcat (scopeStack, Id);
           si++;
        }
        if (name) {
            /* A user identifier is specified */
            /* Translate the user identifier to a C identifier */
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
           strcat(scopeStack, scopeSepp);
           /* Concatenate the user identifier */
           /* QAC EXPECT 5007; will not use wrapper */
           strcat (scopeStack, Id);
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
    c_char *typeName;

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
    idl_scope scope)
{
    idl_scopeElement se;
    c_long si;
    char pathName[OS_PATH_MAX];
    os_result statRes;
    struct os_stat statbuf;
    char* outdir;
    pathName[0] = '\0';
    
    outdir = idl_dirOutCur();
    
    if(outdir){
        sprintf(pathName, "%s%s", outdir, os_fileSep());
    }
    
    for (si = 0; si < idl_scopeStackSize(scope); si++) {
	se = idl_scopeIndexed (scope, si);
	strcat (pathName, idl_javaId(idl_scopeJavaElementName(se)));
	statRes = os_stat (pathName, &statbuf);
	if ((statRes == os_resultFail)) {
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
	        return 0;
	    }
	}
	if (statRes == os_resultFail) {
	    printf ("Error when creating directory %s\n", pathName);
	    return 0;
	}
	strcat (pathName, os_fileSep());
    }
    return 1;
}
    
void
idl_openJavaPackage (
    idl_scope scope,
    const char *name)
{
    char *fname;
    char *package_file;
    
    package_file = os_malloc(strlen (name) + strlen (".java") + 1);
    strcpy(package_file, name);
    strcat(package_file, ".java");
    fname = idl_scopeStackJava(scope, os_fileSep(), package_file);
    if (idl_createDir(scope)) {
	idl_fileSetCur(idl_fileOutNew (fname, "w"));
    } else {
	idl_fileSetCur(NULL);
    }
    os_free(package_file);
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
    strcpy(scopedName, typeEnum);
    strcat(scopedName, ".");
    strcat(scopedName,idl_labelEnumImage(labelVal));

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
        sequenceStringPrev = idl_sequenceIndexString(idl_typeArray(idl_typeDefActual(idl_typeDef(idl_typeSeqType(typeSeq)))));
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
