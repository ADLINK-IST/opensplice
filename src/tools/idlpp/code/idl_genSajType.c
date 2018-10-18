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
/*
   This module generates Splice type definitions related to
   an IDL input file.
*/

/**
 * @file
 * This module generates Standalone Java classes
 * related to an IDL input file.
*/

#include "os_version.h"
#include "idl_program.h"
#include "idl_scope.h"
#include "idl_genSajType.h"
#include "idl_genSplHelper.h"
#include "idl_genJavaHelper.h"
#include "idl_tmplExp.h"
#include "idl_dependencies.h"
#include "idl_genLanguageHelper.h"
#include "idl_map.h"

#include <ctype.h>
#include "c_typebase.h"
#include "os_iterator.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_abstract.h"

static idl_map map = NULL;
/** enumeration element index */
static c_long enum_element = 0;
static char *enum_def = NULL;

static char *unionSwitchType = NULL;
static char *union1stCaseValue = NULL;
static os_iter labelIter = NULL;
static os_iter labelsUsedIter = NULL;
static c_bool caseIsDefault = FALSE;

static void java_arrayDimensions(idl_typeSpec typeSeq);
static c_char *idl_valueFromLabelVal(idl_labelVal labelVal);
static void idl_ifArrayInitializeElements(idl_typeSpec typeSpec, const char *elementName);

/** @brief callback function called on opening the IDL input file.
 *
 * Generate standard file header consisting of:
 * - mutiple inclusion prevention
 * - inclusion of Splice type definition files
 * - inclusion of application specific include files related to other IDL files
 *
 * @param scope Current scope (not used)
 * @param name Name of the IDL input file
 */
static idl_action
idl_fileOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);
    /* return idl_explore to indicate that the rest of the file needs to be processed */
    return idl_explore;
}

/** @brief callback function called on module definition in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   module <module-name> {
            <module-contents>
        };
   @endverbatim
 *
 * This fuction generates the prototype of the function that
 * is responsible for loading the metadata into the database.
 * The name of the function is:
 * @verbatim
        __<scope-basename>_<scope-elements>_<module-name>__load
   @endverbatim
 * For the Splice types, no further actions are required.
 *
 * @param scope Current scope (and scope of the module definition)
 * @param name Name of the defined module
 */
static idl_action
idl_moduleOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);
    /* return idl_explore to indicate that the rest of the module needs to be processed */
    return idl_explore;
}

/** @brief callback function called on structure definition in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   struct <structure-name> {
            <structure-member-1>;
            ...              ...
            <structure-member-n>;
        };
   @endverbatim
 *
 * @param scope Current scope (and scope of the structure definition)
 * @param name Name of the structure
 * @param structSpec Specification of the struct holding the amount of members
 */
static idl_action
idl_structureOpen(
    idl_scope scope,
    const char *name,
    idl_typeStruct structSpec,
    void *userData)
{
    OS_UNUSED_ARG(structSpec);
    OS_UNUSED_ARG(userData);

    /* Open file for used scope, if needed create the directories */
    idl_openJavaPackage(scope, idl_javaId(name));
    if (idl_fileCur() == NULL) {
        return idl_abort;
    }
    /* setup iterator to hold field names and correcsponding type names */
    map = idl_mapNew(NULL, 1, 1);
    /* Write package name */
    idl_fileOutPrintf(idl_fileCur(), "public final class %s {\n\n", idl_javaId(name));
    /* return idl_explore to indicate that the rest of the structure needs to be processed */
    return idl_explore;
}

/** @brief callback function called on end of a structure definition in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        struct <structure-name> {
            <structure-member-1>
            ...              ...
            <structure-member-n>
   =>   };
   @endverbatim
 *
 * The structure is closed:
 * @verbatim
        };
   @endverbatim
 *
 * @param name Name of the structure (not used)
 */
static void
idl_structureClose(
    const char *name,
    void *userData)
{
    idl_mapIter mapIter;
    idl_typeSpec typeSpec;
    char *memberName;
    OS_UNUSED_ARG(userData);

    OS_UNUSED_ARG(userData);

    /* build constructor <type-name> () {} */
    idl_fileOutPrintf(idl_fileCur(), "\n");
    idl_fileOutPrintf(idl_fileCur(), "    public %s() {\n", idl_javaId(name));

    mapIter = idl_mapFirst(map);
    while (idl_mapIterObject(mapIter)) {
        typeSpec = idl_mapIterObject(mapIter);
        memberName = idl_mapIterKey(mapIter);
        idl_ifArrayInitializeElements(typeSpec, memberName);
        idl_mapIterNext(mapIter);
    }
    idl_fileOutPrintf(idl_fileCur(), "    }\n\n");

    /* build constructor <type-name> (<arglist>) { <assignment-list> } */
    idl_fileOutPrintf(idl_fileCur(), "    public %s(\n", idl_javaId(name));
    mapIter = idl_mapFirst(map);
    while (idl_mapIterObject(mapIter)) {
        typeSpec = idl_mapIterObject(mapIter);
        memberName = idl_mapIterKey(mapIter);
        if (idl_typeSpecType(typeSpec) == idl_tbasic) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "        %s _%s",
                idl_corbaJavaTypeFromTypeSpec(typeSpec),
                memberName);
        } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "        %s%s _%s",
                idl_corbaJavaTypeFromTypeSpec(typeSpec),
                idl_sequenceIndexString(idl_typeSeq(typeSpec)),
                memberName);
        } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "        %s%s _%s",
                idl_corbaJavaTypeFromTypeSpec(typeSpec),
                idl_arrayJavaIndexString(idl_typeArray(typeSpec)),
                memberName);
        } else {
            if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
                if (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tbasic) {
                    idl_fileOutPrintf(
                        idl_fileCur(),
                        "        %s _%s",
                        idl_corbaJavaTypeFromTypeSpec(idl_typeDefActual(idl_typeDef(typeSpec))),
                        memberName);
                } else if (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tseq) {
                    idl_fileOutPrintf(
                        idl_fileCur(),
                        "        %s%s _%s",
                        idl_corbaJavaTypeFromTypeSpec(typeSpec),
                        idl_sequenceIndexString(idl_typeSeq(idl_typeDefActual(idl_typeDef(typeSpec)))),
                        memberName);
                } else if (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tarray) {
                    idl_fileOutPrintf(
                        idl_fileCur(),
                        "        %s%s _%s",
                        idl_corbaJavaTypeFromTypeSpec(typeSpec),
                        idl_arrayJavaIndexString(idl_typeArray(idl_typeDefActual(idl_typeDef(typeSpec)))),
                        memberName);
                } else {
                    if ((idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tstruct) ||
                        (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tunion) ||
                        (idl_typeSpecType(idl_typeDefActual(idl_typeDef(typeSpec))) == idl_tenum)) {
                        idl_fileOutPrintf(
                            idl_fileCur(),
                            "        %s _%s",
                            idl_corbaJavaTypeFromTypeSpec(idl_typeDefActual(idl_typeDef(typeSpec))),
                            idl_javaId(memberName));
                    } else {
                        printf("idl_genSajType.c:idl_structureClose: Unexpected type %d\n",
                            idl_typeSpecType (typeSpec));
                    }
                }
            } else {
                if ((idl_typeSpecType(typeSpec) == idl_tstruct) ||
                    (idl_typeSpecType(typeSpec) == idl_tunion) ||
                    (idl_typeSpecType(typeSpec) == idl_tenum)) {
                    idl_fileOutPrintf(
                        idl_fileCur(),
                        "        %s _%s",
                        idl_corbaJavaTypeFromTypeSpec(typeSpec),
                        idl_javaId(memberName));
                } else {
                    printf("idl_genSajType.c:idl_structureClose: Unexpected type %d\n",
                        idl_typeSpecType(typeSpec));
                }
            }
        }
        idl_mapIterNext(mapIter);
        if (idl_mapIterObject(mapIter)) {
            idl_fileOutPrintf(idl_fileCur(), ",\n");
        } else {
            idl_fileOutPrintf(idl_fileCur(), ")\n");
        }
    }
    idl_mapIterFree(mapIter);
    idl_fileOutPrintf(idl_fileCur(), "    {\n");
    mapIter = idl_mapFirst(map);
    while (idl_mapIterObject(mapIter)) {
        idl_fileOutPrintf(
            idl_fileCur(),
            "        %s = _%s;\n",
            (char *)idl_mapIterKey(mapIter),
            (char *)idl_mapIterKey(mapIter));
        idl_mapIterNext (mapIter);
    }
    idl_mapIterFree(mapIter);
    idl_fileOutPrintf(idl_fileCur(), "    }\n\n");
    /* close class */
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    /* close file */
    idl_closeJavaPackage();
    /* remove iterator */
    idl_mapFree(map);
}

/** @brief callback function called on definition of a structure member in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        struct <structure-name> {
   =>       <structure-member-1>;
   =>       ...              ...
   =>       <structure-member-n>;
        };
   @endverbatim
 *
 * @param scope Current scope
 * @param name Name of the structure member
 * @param typeSpec Type specification of the structure member
 */
static void
idl_structureMemberOpenClose(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    /* Dereference possible typedefs first. */
    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    /* generate type-name and field-name attribute */
    if ((idl_typeSpecType(typeSpec) == idl_tbasic)) {
        /* store type-name and field-name in iterator (append) */
        idl_mapAdd(map, idl_javaId(name), typeSpec);
        if (idl_typeBasicType(idl_typeBasic (typeSpec)) == idl_string) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "    public %s %s = \"\";\n",
                idl_corbaJavaTypeFromTypeSpec(typeSpec),
                idl_javaId(name));
        } else {
            idl_fileOutPrintf(
                idl_fileCur(),
                "    public %s %s;\n",
                idl_corbaJavaTypeFromTypeSpec(typeSpec),
                idl_javaId(name));
        }
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        /* store type-name and field-name in iterator (append) */
        idl_mapAdd(map, idl_javaId(name), typeSpec);
        /* Inline sequence definition */
        idl_fileOutPrintf(
            idl_fileCur(),
            "    public %s%s %s = new %s",
            idl_corbaJavaTypeFromTypeSpec(typeSpec),
            idl_sequenceIndexString(idl_typeSeq(typeSpec)),
            idl_javaId(name),
            idl_corbaJavaTypeFromTypeSpec(typeSpec));
        java_arrayDimensions(typeSpec);
        idl_fileOutPrintf (idl_fileCur(), ";\n");
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        /* store type-name and field-name in iterator (append) */
        idl_mapAdd(map, idl_javaId(name), typeSpec);
        /* Inline array definition */
        idl_fileOutPrintf(
            idl_fileCur(),
            "    public %s%s %s = new %s",
            idl_corbaJavaTypeFromTypeSpec (typeSpec),
            idl_arrayJavaIndexString(idl_typeArray(typeSpec)),
            idl_javaId(name),
            idl_corbaJavaTypeFromTypeSpec (typeSpec));
        java_arrayDimensions(typeSpec);
        idl_fileOutPrintf (idl_fileCur(), ";\n");
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct) {
        /* store type-name and field-name in iterator (append) */
        idl_mapAdd(map, idl_javaId(name), typeSpec);
        idl_fileOutPrintf(
            idl_fileCur(),
            "    public %s %s = new %s();\n",
            idl_corbaJavaTypeFromTypeSpec(typeSpec),
            idl_javaId(name),
            idl_corbaJavaTypeFromTypeSpec(typeSpec));
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
        /* store type-name and field-name in iterator (append) */
        idl_mapAdd(map, idl_javaId(name), typeSpec);
        idl_fileOutPrintf(
            idl_fileCur(),
            "    public %s %s = %s.from_int(0);\n",
            idl_corbaJavaTypeFromTypeSpec(typeSpec),
            idl_javaId(name),
            idl_corbaJavaTypeFromTypeSpec(typeSpec));
    } else if (idl_typeSpecType (typeSpec) == idl_tunion) {
        /* store type-name and field-name in iterator (append) */
        idl_mapAdd(map, idl_javaId(name), typeSpec);
        idl_fileOutPrintf(
            idl_fileCur(),
            "    public %s %s = new %s();\n",
            idl_corbaJavaTypeFromTypeSpec(typeSpec),
            idl_javaId(name),
            idl_corbaJavaTypeFromTypeSpec(typeSpec));
    } else {
        printf("idl_genSajType.c:idl_structureMemberOpenClose: Unexpected type %d\n",
            idl_typeSpecType(typeSpec));
    }
}

static c_bool
idl_unionHasCase(c_union _union, char *name) {
    c_ulong caseCount, i;
    c_unionCase _case;
    c_bool result = FALSE;

    caseCount = c_arraySize(_union->cases);

    for(i=0; i<caseCount; i++) {
        _case = _union->cases[i];
        if(!strcmp(c_specifier(_case)->name, name)) {
            result = TRUE;
            break;
        }
    }

    return result;
}

/** @brief callback function called on definition of a union in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   union <union-name> switch(<switch-type>) {
            case label1.1; .. case label1.n;
                <union-case-1>;
            case label2.1; .. case label2.n;
                ...        ...
            case labeln.1; .. case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
        };
   @endverbatim
 *
 * @param scope Current scope
 * @param name Name of the union
 * @param unionSpec Specifies the number of union cases and the union switch type
 */
static idl_action
idl_unionOpen(
    idl_scope scope,
    const char *name,
    idl_typeUnion unionSpec,
    void *userData)
{
    OS_UNUSED_ARG(userData);

    /* Open file for used scope, if needed create the directories */
    idl_openJavaPackage(scope, idl_javaId(name));
    if (idl_fileCur() == NULL) {
        return idl_abort;
    }
    unionSwitchType = idl_corbaJavaTypeFromTypeSpec(idl_typeUnionSwitchKind(unionSpec));

    /* setup iterator to hold case names and correcsponding type names */
    map = idl_mapNew(NULL, 1, 1);

    /* Write package name */
    idl_fileOutPrintf(idl_fileCur(), "import org.opensplice.dds.dcps.Utilities;\n\n");
    /* public final class <struct-name> { */
    idl_fileOutPrintf(idl_fileCur(), "public final class %s {\n\n", idl_javaId(name));
    idl_fileOutPrintf(idl_fileCur(), "    private %s _d;\n\n", unionSwitchType);
    if(idl_unionHasCase((c_union)idl_typeSpecDef((idl_typeSpec)unionSpec), "discriminator")) {
        idl_fileOutPrintf(idl_fileCur(), "    public %s _discriminator ()\n", unionSwitchType);
    }else {
        idl_fileOutPrintf(idl_fileCur(), "    public %s discriminator ()\n", unionSwitchType);
    }
    idl_fileOutPrintf(idl_fileCur(), "    {\n");
    idl_fileOutPrintf(idl_fileCur(), "        return _d;\n");
    idl_fileOutPrintf(idl_fileCur(), "    }\n\n");

    labelsUsedIter = os_iterNew(NULL);
    /* return idl_explore to indicate that the rest of the enumeration needs to be processed */
    return idl_explore;
}

/** @brief callback function called on closure of a union in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
            case label1.1; .. case label1.n;
                <union-case-1>;
            case label2.1; .. case label2.n;
                ...        ...
            case labeln.1; .. case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
   =>   };
   @endverbatim
 *
 * The union is closed:
 * @verbatim
            } _u;
        };
   @endverbatim
 * @param name Name of the union
 */
static void
idl_unionClose(
    const char *name,
    void *userData)
{
    idl_mapIter mapIter;
    idl_typeSpec typeSpec;
    char *memberName;
    OS_UNUSED_ARG(userData);

    OS_UNUSED_ARG(userData);

    mapIter = idl_mapFirst(map);
    typeSpec = idl_mapIterObject(mapIter);
    memberName = idl_mapIterKey(mapIter);

    idl_fileOutPrintf(idl_fileCur(), "    public %s () {\n", idl_javaId(name));

    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        if (idl_typeBasicType(idl_typeBasic (typeSpec)) == idl_string) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "        __%s = \"\";\n",
                idl_javaId(memberName));
        } else if (idl_typeBasicType(idl_typeBasic (typeSpec)) == idl_boolean) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "        __%s = false;\n",
                idl_javaId(memberName));
        } else if (idl_typeBasicType(idl_typeBasic (typeSpec)) == idl_char) {
            idl_fileOutPrintf(
                idl_fileCur(),
                "        __%s = '\\0';\n",
                idl_javaId(memberName));
        } else {
            idl_fileOutPrintf(
                idl_fileCur(),
                "        __%s = 0;\n",
                idl_javaId(memberName));
        }
    } else if (idl_typeSpecType(typeSpec) == idl_tseq || idl_typeSpecType(typeSpec) == idl_tarray) {
        os_size_t __memberNameLength;
        char *__memberName;

        __memberNameLength = strlen(memberName) + 2 + 1;
        __memberName = os_malloc(__memberNameLength);
        snprintf(__memberName, __memberNameLength, "__%s", memberName);
        idl_fileOutPrintf(
            idl_fileCur(),
            "        __%s = new %s",
            idl_javaId(memberName),
            idl_corbaJavaTypeFromTypeSpec(typeSpec));
        java_arrayDimensions(typeSpec);
        idl_fileOutPrintf(idl_fileCur(), ";\n");
        idl_ifArrayInitializeElements(typeSpec, __memberName);
        os_free(__memberName);
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct || idl_typeSpecType(typeSpec) == idl_tunion) {
        idl_fileOutPrintf(
            idl_fileCur(),
            "        __%s = new %s();\n",
            idl_javaId(memberName),
            idl_corbaJavaTypeFromTypeSpec(typeSpec));
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
        idl_fileOutPrintf(
            idl_fileCur(),
            "        __%s = %s.from_int(0);\n",
            idl_javaId(memberName),
            idl_corbaJavaTypeFromTypeSpec(typeSpec));
    }

    idl_fileOutPrintf(idl_fileCur(), "        _d = (%s)%s;\n", unionSwitchType, union1stCaseValue);
    idl_fileOutPrintf(idl_fileCur(), "    }\n");

    idl_mapIterFree(mapIter);
    idl_mapFree(map);
    union1stCaseValue = NULL;


    /* close class */
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    /* close file */
    idl_closeJavaPackage();
}

static char *
idl_unionCaseTypeFromTypeSpec(
    idl_typeSpec typeSpec)
{
    char typeName[512];

    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        snprintf(typeName, sizeof(typeName), "%s",
            idl_corbaJavaTypeFromTypeSpec(typeSpec));
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        snprintf (typeName, sizeof(typeName), "%s%s",
            idl_corbaJavaTypeFromTypeSpec(idl_typeSeqActual(idl_typeSeq(typeSpec))),
            idl_sequenceIndexString (idl_typeSeq(typeSpec)));
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        snprintf(typeName, sizeof(typeName), "%s%s",
            idl_corbaJavaTypeFromTypeSpec(idl_typeArrayActual(idl_typeArray(typeSpec))),
            idl_arrayJavaIndexString(idl_typeArray(typeSpec)));
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        return idl_unionCaseTypeFromTypeSpec(idl_typeDefActual(idl_typeDef(typeSpec)));
    } else {
        if ((idl_typeSpecType(typeSpec) == idl_tstruct) ||
            (idl_typeSpecType (typeSpec) == idl_tunion) ||
            (idl_typeSpecType (typeSpec) == idl_tenum)) {
            snprintf(typeName, sizeof(typeName), "%s",
                idl_corbaJavaTypeFromTypeSpec(typeSpec));
        } else {
            printf ("idl_unionCaseTypeFromTypeSpec: Unexpected type %d\n",
                idl_typeSpecType(typeSpec));
        }
    }
    return os_strdup(typeName);
}

/** @brief callback function called on definition of a union case in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
            case label1.1; .. case label1.n;
   =>           <union-case-1>;
            case label2.1; .. case label2.n;
   =>           ...        ...
            case labeln.1; .. case labeln.n;
   =>           <union-case-n>;
            default:
   =>           <union-case-m>;
        };
   @endverbatim
 *
 * @param scope Current scope (the union the union case is defined in)
 * @param name Name of the union case
 * @param typeSpec Specifies the type of the union case
 */
static void
idl_unionCaseOpenClose(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec,
    void *userData)
{
    char *labelImage;
    c_ulong nrElements, i;
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    /* store type-name and field-name in iterator (append) */
    idl_mapAdd(map, idl_javaId(name), typeSpec);

    /* Obtain name of first label and total number of labels. */
    nrElements = os_iterLength(labelIter);
    labelImage = os_iterObject(labelIter, 0);

    /* Save the 1st case label for usage with the default constructor. */
    if (!union1stCaseValue) {
        union1stCaseValue = os_iterObject(labelIter, 0);
    }

    idl_fileOutPrintf(
        idl_fileCur(),
        "    private %s __%s;\n\n",
        idl_unionCaseTypeFromTypeSpec(typeSpec),
        idl_javaId(name));
    idl_fileOutPrintf(
        idl_fileCur(),
        "    public %s %s ()\n",
        idl_unionCaseTypeFromTypeSpec (typeSpec),
        idl_javaId(name));
    idl_fileOutPrintf(idl_fileCur(), "    {\n");
    if (caseIsDefault == FALSE) {
        idl_fileOutPrintf(idl_fileCur(), "        if (_d != %s", labelImage);
        for (i = 1; i < nrElements; i++) {
            labelImage = os_iterObject(labelIter, i);
            idl_fileOutPrintf(idl_fileCur(), " &&\n            _d != %s", labelImage);
        }
        idl_fileOutPrintf(idl_fileCur(), ") {\n");
        idl_fileOutPrintf(idl_fileCur(), "            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_OPERATION, null);\n");
        idl_fileOutPrintf(idl_fileCur(), "        }\n");
    } else {
        if(os_iterLength(labelsUsedIter)) {
            labelImage = os_iterObject(labelsUsedIter, 0);
            idl_fileOutPrintf(idl_fileCur(), "        if (_d == %s", labelImage);
                nrElements = os_iterLength(labelsUsedIter);
            for (i = 1; i < nrElements; i++) {
            labelImage = os_iterObject(labelsUsedIter, i);
                idl_fileOutPrintf(idl_fileCur(), " ||\n            _d == %s", labelImage);
            }
            idl_fileOutPrintf(idl_fileCur(), ") {\n");
            idl_fileOutPrintf(idl_fileCur(), "            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_OPERATION, null);\n");
            idl_fileOutPrintf(idl_fileCur(), "        }\n");
        }
    }
    idl_fileOutPrintf(idl_fileCur(), "        return __%s;\n", idl_javaId(name));
    idl_fileOutPrintf(idl_fileCur(), "    }\n\n");
    idl_fileOutPrintf(
        idl_fileCur(),
        "    public void %s (%s val)\n",
        idl_javaId(name),
        idl_unionCaseTypeFromTypeSpec(typeSpec));
    idl_fileOutPrintf(idl_fileCur(), "    {\n");
    idl_fileOutPrintf(idl_fileCur(), "        __%s = val;\n",
	idl_javaId(name));
    labelImage = os_iterObject(labelIter, 0);
    if (labelImage) {
        idl_fileOutPrintf(idl_fileCur(), "        _d = (%s)%s;\n", unionSwitchType, labelImage);
    } else {
        /* Theoretically this branch should never be reached: also the code
         * underneath doesn't make any sense. That's why an assert has
         * now been substituted.
         */
        assert(FALSE);
    }
    idl_fileOutPrintf(idl_fileCur(), "    }\n\n");
    idl_fileOutPrintf(
        idl_fileCur(),
        "    public void %s (%s d, %s val)\n",
        idl_javaId(name),
        unionSwitchType,
        idl_unionCaseTypeFromTypeSpec (typeSpec));
    idl_fileOutPrintf(idl_fileCur(), "    {\n");
    if (caseIsDefault == FALSE) {
        labelImage = os_iterTakeFirst(labelIter);
        idl_fileOutPrintf(idl_fileCur(), "        if (d != (%s)%s", unionSwitchType, labelImage);
        labelImage = os_iterTakeFirst(labelIter);
        while (labelImage) {
            idl_fileOutPrintf(idl_fileCur(), " &&\n            d != (%s)%s", unionSwitchType, labelImage);
            labelImage = os_iterTakeFirst (labelIter);
        }
        idl_fileOutPrintf(idl_fileCur(), ") {\n");
        idl_fileOutPrintf(idl_fileCur(), "            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_OPERATION, null);\n");
        idl_fileOutPrintf(idl_fileCur(), "        }\n");
    } else {
        if(os_iterLength(labelsUsedIter)) {
            labelImage = os_iterTakeFirst(labelsUsedIter);
            idl_fileOutPrintf(idl_fileCur(), "        if (d == (%s)%s", unionSwitchType, labelImage);
            labelImage = os_iterTakeFirst(labelsUsedIter);
            while (labelImage) {
                idl_fileOutPrintf(idl_fileCur(), " ||\n            d == (%s)%s", unionSwitchType, labelImage);
                labelImage = os_iterTakeFirst(labelsUsedIter);
            }
            idl_fileOutPrintf(idl_fileCur(), ") {\n");
            idl_fileOutPrintf(idl_fileCur(), "            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_OPERATION, null);\n");
            idl_fileOutPrintf(idl_fileCur(), "        }\n");
        }
    }
    idl_fileOutPrintf(idl_fileCur(), "        __%s = val;\n", idl_javaId(name));
    idl_fileOutPrintf(idl_fileCur(), "        _d = d;\n");
    idl_fileOutPrintf(idl_fileCur(), "    }\n\n");
}

/** @brief callback function called when no default case is defined in an union
 *   for which not all possible label values are specified
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
            case label1.1; .. case label1.n;
                <union-case-1>;
            case label2.1; .. case label2.n;
                ...        ...
            case labeln.1; .. case labeln.n;
                <union-case-n>;
        };
   @endverbatim
 *
 * @param scope Current scope (the union the union case is defined in)
 * @param labelVal Default value for the label case (lowest possible not used index)
 * @param typeSpec Specifies the type of the union switch
 */
static void
idl_artificialDefaultLabelOpenClose(
    idl_scope scope,
    idl_labelVal labelVal,
    idl_typeSpec typeSpec,
    void *userData)
{
    char *labelImage;
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf(idl_fileCur(), "    public void __default ()\n");
    idl_fileOutPrintf(idl_fileCur(), "    {\n");
    idl_fileOutPrintf(idl_fileCur(), "        _d = (%s)%s;\n", unionSwitchType, idl_valueFromLabelVal(labelVal));
    idl_fileOutPrintf(idl_fileCur(), "    }\n\n");

    idl_fileOutPrintf(idl_fileCur(), "    public void __default (%s d)\n", idl_corbaJavaTypeFromTypeSpec (typeSpec));
    idl_fileOutPrintf(idl_fileCur(), "    {\n");

    if(os_iterLength(labelsUsedIter)) {
        labelImage = os_iterTakeFirst(labelsUsedIter);
        idl_fileOutPrintf(idl_fileCur(), "        if (d == (%s)%s", unionSwitchType, labelImage);
        labelImage = os_iterTakeFirst(labelsUsedIter);
        while (labelImage) {
            idl_fileOutPrintf(idl_fileCur(), " ||\n            d == (%s)%s", unionSwitchType, labelImage);
            labelImage = os_iterTakeFirst(labelsUsedIter);
        }
        idl_fileOutPrintf(idl_fileCur(), ") {\n");
        idl_fileOutPrintf(idl_fileCur(), "            throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_OPERATION, null);\n");
        idl_fileOutPrintf(idl_fileCur(), "        }\n");
    }
    idl_fileOutPrintf(idl_fileCur(), "        _d = d;\n");
    idl_fileOutPrintf(idl_fileCur(), "    }\n\n");
}

/** @brief Generate a string representaion the literal value of a label
 * in metadata terms.
 *
 * @param labelVal Specifies the kind and the value of the label
 * @return String representing the image of \b labelVal
 */
#define maxLabelNameSize 100

static c_char *
idl_valueFromLabelVal (
    idl_labelVal labelVal)
{
    c_char *labelName;

    if (idl_labelValType(idl_labelVal(labelVal)) == idl_lenum) {
        labelName = idl_labelJavaEnumVal(unionSwitchType, idl_labelEnum(labelVal));
    } else {
        labelName = os_malloc(maxLabelNameSize);
        switch (idl_labelValueVal(idl_labelValue(labelVal)).kind) {
	    case V_CHAR:
            snprintf(labelName, maxLabelNameSize, "%u",
                idl_labelValueVal(idl_labelValue(labelVal)).is.Char);
		break;
	    case V_SHORT:
            snprintf(labelName, maxLabelNameSize, "%d",
                idl_labelValueVal(idl_labelValue(labelVal)).is.Short);
		break;
	    case V_USHORT:
            snprintf(labelName, maxLabelNameSize, "%d",
                idl_labelValueVal(idl_labelValue(labelVal)).is.UShort);
		break;
	    case V_LONG:
            snprintf(labelName, maxLabelNameSize, "%d",
                idl_labelValueVal(idl_labelValue(labelVal)).is.Long);
		break;
	    case V_ULONG:
            snprintf(labelName, maxLabelNameSize, "%d",
                idl_labelValueVal(idl_labelValue(labelVal)).is.ULong);
		break;
	    case V_LONGLONG:
            snprintf(labelName, maxLabelNameSize, "%"PA_PRId64"L",
                idl_labelValueVal(idl_labelValue(labelVal)).is.LongLong);
		break;
	    case V_ULONGLONG:
            snprintf(labelName, maxLabelNameSize, "%"PA_PRIu64"L",
                idl_labelValueVal(idl_labelValue(labelVal)).is.ULongLong);
		break;
	    case V_BOOLEAN:
    		/* QAC EXPECT 3416; No side effect here */
		if (idl_labelValueVal(idl_labelValue(labelVal)).is.Boolean == TRUE) {
		    snprintf(labelName, maxLabelNameSize, "true");
		} else {
		    snprintf(labelName, maxLabelNameSize, "false");
		}
		break;
	    default:
		break;
        }
    }
    return labelName;
}

/** @brief callback function called on definition of the union case labels in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
   =>       case label1.1; .. case label1.n;
                <union-case-1>;
   =>       case label2.1; .. case label2.n;
                ...        ...
   =>       case labeln.1; .. case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
        };
   @endverbatim
 *
 * @param scope Current scope (the union the labels are defined in)
 * @param labelSpec Specifies the number of labels of the union case
 */
static void
idl_unionLabelOpenClose(
    idl_scope ownScope,
    idl_labelVal labelVal,
    void *userData)
{
    OS_UNUSED_ARG(ownScope);
    OS_UNUSED_ARG(userData);

    if (idl_labelValType(labelVal) != idl_ldefault) {
        caseIsDefault = FALSE;
        labelIter = os_iterAppend(labelIter, idl_valueFromLabelVal(labelVal));
        labelsUsedIter = os_iterAppend(labelsUsedIter, idl_valueFromLabelVal(labelVal));
    } else {
        caseIsDefault = TRUE;
        labelIter = os_iterAppend(labelIter,
	    idl_valueFromLabelVal(idl_labelDefaultAlternative(idl_labelDefault(labelVal))));
    }
}

/** @brief callback function called on definition of the union case labels in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
   =>       case label1.1; .. case label1.n;
                <union-case-1>;
   =>       case label2.1; .. case label2.n;
                ...        ...
   =>       case labeln.1; .. case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
        };
   @endverbatim
 *
 * @param scope Current scope (the union the labels are defined in)
 * @param labelSpec Specifies the number of labels of the union case
 */
static void
idl_unionLabelsOpenClose(
    idl_scope scope,
    idl_labelSpec labelSpec,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(labelSpec);
    OS_UNUSED_ARG(userData);

    labelIter = os_iterNew(NULL);
}

/** @brief callback function called on definition of an enumeration.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   enum <enum-name> {
            <enum-element-1>;
            ...          ...
            <enum-element-n>;
        };
   @endverbatim
 *
 * @param scope Current scope
 * @param name Name of the enumeration
 * @param enumSpec Specifies the number of elements in the enumeration
 */
static idl_action
idl_enumerationOpen(
    idl_scope scope,
    const char *name,
    idl_typeEnum enumSpec,
    void *userData)
{
    OS_UNUSED_ARG(userData);

    /* Open file for used scope, if needed create the directories */
    idl_openJavaPackage(scope, idl_javaId(name));
    if (idl_fileCur() == NULL) {
        return idl_abort;
    }
    /* Write package name */
#if 0
    idl_fileOutPrintf(idl_fileCur(), "/*\n");
    idl_fileOutPrintf(idl_fileCur(), " * Generated by OpenSpliceDDS "OSPL_VERSION_STR"\n");
    idl_fileOutPrintf(idl_fileCur(), " */\n\n");
#endif
    idl_fileOutPrintf(idl_fileCur(), "import org.opensplice.dds.dcps.Utilities;\n\n");
    /* public final class <enumeration-name> { */
    idl_fileOutPrintf(idl_fileCur(), "public class %s {\n\n", idl_javaId(name));
    idl_fileOutPrintf(idl_fileCur(), "    private int __value;\n");
    idl_fileOutPrintf(
        idl_fileCur(),
        "    private static int __size = %d;\n",
        idl_typeEnumNoElements (enumSpec));
    enum_def = idl_scopeStackJava(scope, ".", name);
    idl_fileOutPrintf(
        idl_fileCur(),
        "    private static %s[] __array = new %s[__size];\n\n",
        enum_def,
        enum_def);
    enum_element = 0;
    /* return idl_explore to indicate that the rest of the structure needs to be processed */
    return idl_explore;
}

/** @brief callback function called on closure of an enumeration in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        enum <enum-name> {
            <enum-element-1>;
            ...          ...
            <enum-element-n>;
   =>   };
   @endverbatim
 *
 * @param name Name of the enumeration
 */
static void
idl_enumerationClose(
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf(idl_fileCur(), "    public int value ()\n");
    idl_fileOutPrintf(idl_fileCur(), "    {\n");
    idl_fileOutPrintf(idl_fileCur(), "        return __value;\n");
    idl_fileOutPrintf(idl_fileCur(), "    }\n\n");
    idl_fileOutPrintf(
        idl_fileCur(),
        "    public static %s from_int (int value)\n",
        enum_def);
    idl_fileOutPrintf(idl_fileCur(), "    {\n");
    idl_fileOutPrintf(idl_fileCur(), "        if (value >= 0 && value < __size) {\n");
    idl_fileOutPrintf(idl_fileCur(), "            return __array[value];\n");
    idl_fileOutPrintf(idl_fileCur(), "        }\n");
    idl_fileOutPrintf(idl_fileCur(), "        throw Utilities.createException(Utilities.EXCEPTION_TYPE_BAD_OPERATION, null);\n");
    idl_fileOutPrintf(idl_fileCur(), "    }\n\n");
    idl_fileOutPrintf(
        idl_fileCur(),
        "    protected %s (int value)\n",
        idl_javaId(name));
    idl_fileOutPrintf(idl_fileCur(), "    {\n");
    idl_fileOutPrintf(idl_fileCur(), "        __value = value;\n");
    idl_fileOutPrintf(idl_fileCur(), "        __array[__value] = this;\n");
    idl_fileOutPrintf(idl_fileCur(), "    }\n");
    /* close class */
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    /* close file */
    idl_closeJavaPackage();
}

/** @brief callback function called on definition of an enumeration element in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        enum <enum-name> {
   =>       <enum-element-1>,
   =>       ...          ...
   =>       <enum-element-n>
        };
   @endverbatim
 *
 * For the last element generate:
 * @verbatim
        <element-name>
   @endverbatim
 * For any but the last element generate:
 * @verbatim
	<element-name>,
   @endverbatim
 *
 * @param scope Current scope
 * @param name Name of the enumeration element
 */
static void
idl_enumerationElementOpenClose (
    idl_scope scope,
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf(
        idl_fileCur(),
        "    public static final int _%s = %d;\n",
        idl_javaId(name),
        enum_element);
    idl_fileOutPrintf(
        idl_fileCur(),
        "    public static final %s %s = new %s(_%s);\n\n",
        enum_def,
        idl_javaId(name),
        enum_def,
        idl_javaId(name));
    enum_element++;
}

/** @brief generate dimension of an array or a sequence
 *
 * java_arrayDimensions is a local support function to generate
 * the dimensions of a java Array representing an IDL sequence or
 * array. Since Sequences will always be initialized to 0 elements,
 * its dimensions will always be assigned 0. Arrays will be assigned
 * their IDL dimensions.
 *
 * @param typeSeq Specifies the type of the sequence
 */
static void
java_arrayDimensions(
    idl_typeSpec typeSpec)
{
    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }
    if (idl_typeSpecType(typeSpec) == idl_tarray) {
        idl_typeArray typeArray = idl_typeArray(typeSpec);
        idl_fileOutPrintf (idl_fileCur(), "[%d]", idl_typeArraySize(typeArray));
        java_arrayDimensions(idl_typeArrayType(typeArray));
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
        idl_fileOutPrintf(idl_fileCur(), "[0]");
        java_arrayDimensions(idl_typeSeqType(idl_typeSeq(typeSpec)));
    }
}

/** @brief function to find out whether the elements of an array
 * require initialization.
 *
 * idl_arrayElementsNeedInitialization is a local support function to find out
 * whether the specified array is of a type for which all elements need to be
 * initialized individually. If the array is of a primitive type, this is never
 * necessary, but if the array contains a reference type and no underlying
 * sequences (which will be initialized to 0 elements) then for loops will
 * need to be created to explicitly initialize all of these attributes.
 *
 * @param typeSpec Specifies the attribute type that needs to be investigated.
 */
static int
idl_arrayElementsNeedInitialization(
    idl_typeArray typeArray)
{
    int initRequired = FALSE;

    /* Obtain the type of the array. */
    idl_typeSpec typeSpec = idl_typeArrayType(typeArray);

    /* Resolve potential typedefs. */
    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    if (idl_typeSpecType(typeSpec) == idl_tarray) {
        initRequired = idl_arrayElementsNeedInitialization(idl_typeArray(typeSpec));
    } else if ( idl_typeSpecType(typeSpec) == idl_tstruct ||
                idl_typeSpecType(typeSpec) == idl_tunion ||
                idl_typeSpecType(typeSpec) == idl_tenum ||
                ( idl_typeSpecType(typeSpec) == idl_tbasic &&
                  idl_typeBasicType(idl_typeBasic (typeSpec)) == idl_string) ) {
        initRequired = TRUE;
    }

    return initRequired;
}

/** @brief generate initialization of array elements.
 *
 * idl_arrayElementInit generates for-loops that initialize
 * each attribute of an array explicitly.
 *
 * @param typeArray Specifies the type of the array
 */
static void
idl_arrayElementInit(
    idl_typeSpec typeSpec,
    const char *elementName,
    int dimCount,
    int indent)
{
    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    if (idl_typeSpecType(typeSpec) == idl_tarray) {
        idl_fileOutPrintf(
            idl_fileCur(),
            "%*sfor(int i%d = 0; i%d < %d; i%d++) {\n",
            indent,
            "",
            dimCount,
            dimCount,
            idl_typeArraySize(idl_typeArray(typeSpec)),
            dimCount);
        idl_arrayElementInit(
            idl_typeArrayType(idl_typeArray(typeSpec)),
            elementName,
            dimCount + 1,
            indent + 4);
        idl_fileOutPrintf(
            idl_fileCur(),
            "%*s}\n",
            indent,
            "");
    } else {
        int j;

        idl_fileOutPrintf(
            idl_fileCur(),
            "%*s%s",
            indent,
            "",
            elementName);
        for (j =  1; j < dimCount; j++) {
            idl_fileOutPrintf(idl_fileCur(), "[i%d]", j);
        }
        if ( idl_typeSpecType(typeSpec) == idl_tbasic &&
             idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string) {
            idl_fileOutPrintf(
                idl_fileCur(),
                " = \"\";\n");
         } else if ( idl_typeSpecType(typeSpec) == idl_tunion ||
                     idl_typeSpecType(typeSpec) == idl_tstruct ) {
            idl_fileOutPrintf(
                idl_fileCur(),
                " = new %s();\n",
                idl_corbaJavaTypeFromTypeSpec(typeSpec));
        } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
            idl_fileOutPrintf(
                idl_fileCur(),
                " = %s.from_int(0);\n",
                idl_corbaJavaTypeFromTypeSpec(typeSpec));
        }
    }
}

/** @brief generate initialization of array elements.
 *
 * idl_ifArrayInitializeElements is a local support function to initialize
 * the elements of non-primitive array types, if appropriate.
 *
 * @param typeSpec Type of the attribute that might need to be initialized.
 * @param elementName Name of the attribute that might need to be initialized.
 */
static void
idl_ifArrayInitializeElements(
    idl_typeSpec typeSpec,
    const char *elementName)
{
    int dimCount = 1;
    int indent = 8;

    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    if ( idl_typeSpecType(typeSpec) == idl_tarray) {
        if (idl_arrayElementsNeedInitialization(idl_typeArray(typeSpec))) {
            idl_arrayElementInit(
                typeSpec,
                elementName,
                dimCount,
                indent);
        }
    }
}


/** @brief callback function called on definition of a named type in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   typedef <type-name> <name>;
   @endverbatim
 *
 * @param scope Current scope
 * @param name Specifies the name of the type
 * @param defSpec Specifies the type of the named type
 */
static void
idl_typedefOpenClose(
    idl_scope scope,
    const char *name,
    idl_typeDef defSpec,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(defSpec);
    OS_UNUSED_ARG(userData);
}

static void
idl_sequenceOpenClose (
    idl_scope scope,
    idl_typeSeq typeSeq,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(typeSeq);
    OS_UNUSED_ARG(userData);
}

static void
idl_constantOpenClose(
    idl_scope scope,
    idl_constSpec constantSpec,
    void *userData)
{
    OS_UNUSED_ARG(userData);
    /* Open file for used scope, if needed create the directories */
    idl_openJavaPackage(scope, idl_javaId(idl_constSpecName(constantSpec)));
    /* Write package name */
    idl_fileOutPrintf(idl_fileCur(), "public interface %s {\n",
    idl_constSpecName(constantSpec));

    if (idl_typeSpecType(idl_constSpecTypeGet(constantSpec)) == idl_tbasic &&
        idl_typeBasicType(idl_typeBasic(idl_constSpecTypeGet(constantSpec))) == idl_string) {
        idl_fileOutPrintf(
            idl_fileCur(),
            "    public static final %s value = %s;\n",
            idl_corbaJavaTypeFromTypeSpec(idl_constSpecTypeGet(constantSpec)),
            idl_constSpecImage (constantSpec));
    } else {
        idl_fileOutPrintf(
            idl_fileCur(),
            "    public static final %s value = (%s)(%s);\n",
            idl_corbaJavaTypeFromTypeSpec(idl_constSpecTypeGet(constantSpec)),
            idl_corbaJavaTypeFromTypeSpec(idl_constSpecTypeGet(constantSpec)),
            idl_constSpecImage(constantSpec));
    }
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_closeJavaPackage();
}

/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
*/
static idl_programControl idl_genSajLoadControl = {
    idl_prior
};

/** @brief return the program control structure for the splice type generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    OS_UNUSED_ARG(userData);
    return &idl_genSajLoadControl;
}

/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program
idl_genSajType = {
    idl_getControl,
    idl_fileOpen,
    NULL, /* idl_fileClose */
    idl_moduleOpen,
    NULL, /* idl_moduleClose */
    idl_structureOpen,
    idl_structureClose,
    idl_structureMemberOpenClose,
    idl_enumerationOpen,
    idl_enumerationClose,
    idl_enumerationElementOpenClose,
    idl_unionOpen,
    idl_unionClose,
    idl_unionCaseOpenClose,
    idl_unionLabelsOpenClose,
    idl_unionLabelOpenClose,
    idl_typedefOpenClose,
    NULL, /* idl_boundedStringOpenClose */
    idl_sequenceOpenClose,
    idl_constantOpenClose,
    idl_artificialDefaultLabelOpenClose,
    NULL /* userData */
};

/** @brief return the callback table for the splice type generation functions.
 */
idl_program
idl_genSajTypeProgram(
    void)
{
    return &idl_genSajType;
}
