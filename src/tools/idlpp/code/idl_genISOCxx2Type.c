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
 * This module generates Standalone Cxx11 data types
 * related to an IDL input file.
*/

#include "idl_scope.h"
#include "idl_genISOCxx2Type.h"
#include "idl_genISOCxx2Helper.h"
#include "idl_genSplHelper.h"
#include "idl_genFileHelper.h"
#include "idl_tmplExp.h"

#include "idl_dependencies.h"
#include "idl_genLanguageHelper.h"
#include "idl_dll.h"
#include "idl_walk.h"

#include "vortex_os.h"
#include <ctype.h>
#include "c_base.h"
#include "c_metabase.h"
#include "c_typebase.h"

/** indentation level */
static c_long indent_level = 0;
/** enumeration element index */
static c_ulong enum_element = 0;
/* fileName in uppercase */
static char *upperName = NULL;
/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program idl_genCxx2Type;

struct ISOCxx2ScopeStack_s {
    c_type type;
    c_bool unionHasArtificialDefault;
};
typedef struct ISOCxx2ScopeStack_s ISOCxx2ScopeStack;

/** @brief callback function called on opening the IDL input file.
 *
 * Generate standard file header consisting of:
 * - inclusion of Splice type definition files
 *
 * @param scope Current scope (not used)
 * @param name Name of the IDL input file
 */
#if 0
static idl_action
idl_fileOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    c_ulong i;

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    /* Store fileName in uppercase. */
    upperName = os_malloc(strlen(name) + 1);
    for(i = 0; i < strlen(name); i++)
    {
        upperName[i] = (os_char) toupper (name[i]);
    }
    upperName[i] = '\0';
    /* Generate inclusion of standard Vortex OpenSplice type definition files */
    idl_fileOutPrintf(idl_fileCur(), "#ifndef _%s_H_\n", upperName);
    idl_fileOutPrintf(idl_fileCur(), "#define _%s_H_\n", upperName);
    idl_fileOutPrintf(idl_fileCur(), "\n");
    idl_fileOutPrintf(idl_fileCur(), "#include <dds/core/ddscore.hpp>\n");

    /* Generate inclusion of header files that this file has dependencies on. */
    for (i = 0; i < idl_depLength(idl_depDefGet()); i++) {
        const char *inclBaseName = idl_depGet (idl_depDefGet(), i);
        const char *inclBasePath;
        /* If dependency is on our builtin types, then add the relative path to the generated files. */
        if (strcmp(inclBaseName, "dds_builtinTopics") == 0 || strcmp(inclBaseName, "dds_dcps_builtintopics") == 0) {
            inclBasePath = "dds/core/detail/";
        } else {
            inclBasePath = "";
        }
        idl_fileOutPrintf(idl_fileCur(), "#include \"%s%s.h\"\n", inclBasePath, inclBaseName);
    }
    idl_fileOutPrintf(idl_fileCur(), "\n");

    /* return idl_explore to indicate that the rest of the file needs to be processed */
    return idl_explore;
}
#endif

static idl_action
idl_fileOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    c_ulong i;

    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    /* Store fileName in uppercase. */
    upperName = os_strdup(idl_genIncludeGuardFromFilename(scope, ""));

    /* Generate inclusion of standard Vortex OpenSplice type definition files */
    idl_fileOutPrintf(idl_fileCur(), "#ifndef _%s_H_\n", upperName);
    idl_fileOutPrintf(idl_fileCur(), "#define _%s_H_\n", upperName);
    idl_fileOutPrintf(idl_fileCur(), "\n");
    idl_fileOutPrintf(idl_fileCur(), "#include <dds/core/ddscore.hpp>\n");

    /* Generate inclusion of header files that this file has dependencies on. */
    for (i = 0; i < idl_depLength(idl_depDefGet()); i++) {
        const char *inclBaseName = idl_depGet (idl_depDefGet(), i);
        const char *inclBasePath;
        /* If dependency is on our builtin types, then add the relative path to the generated files. */
        if (strcmp(inclBaseName, "dds_builtinTopics") == 0 || strcmp(inclBaseName, "dds_dcps_builtintopics") == 0) {
            inclBasePath = "dds/core/detail/";
        } else {
            inclBasePath = "";
        }
        idl_fileOutPrintf(idl_fileCur(), "#include \"%s%s.h\"\n", inclBasePath, inclBaseName);
    }
    idl_fileOutPrintf(idl_fileCur(), "\n");

    /* return idl_explore to indicate that the rest of the file needs to be processed */
    return idl_explore;
}



/**
 * Just close out the file with a #endif.
 */
static void
idl_fileClose(
    void *userData)
{
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf(idl_fileCur(), "#endif /* _%s_H_ */\n", upperName);
    os_free(upperName);
    upperName = NULL;
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
 * @param scope Current scope (and scope of the module definition)
 * @param name Name of the defined module
 */
static idl_action
idl_moduleOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    char *cxxName;

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    /* Generate the C++ code that opens the namespace. */
    cxxName = idl_ISOCxx2Id(name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "namespace %s\n", cxxName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    /* Increase the indentation level. */
    indent_level++;
    os_free(cxxName);

    /* return idl_explore to indicate that the rest of the module needs to be processed */
    return idl_explore;
}

/** @brief callback function called on module termination in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        module <module-name> {
            <module-contents>
   =>   };
   @endverbatim
 *
 */
static void
idl_moduleClose(
    void *userData)
{
    OS_UNUSED_ARG(userData);

    /* Decrease the indentation level back to its original size. */
    indent_level--;

    /* Generate the C++ code that closes the namespace. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
}

typedef struct {
    c_type structType;
    c_ulong nrMembers;
    c_type *memberTypes;
    c_char **memberTypeNames;
    c_char **memberInTypes;
    c_char **memberNames;
} structMetaDescriptions;

static void
idl_structGenerateTypedefs(
        const structMetaDescriptions *smd)
{
    c_ulong i;

    /* Declare typedefs for each (anonymous) sequence type. */
    for (i = 0; i < smd->nrMembers; i++) {
        if (c_baseObjectKind(smd->memberTypes[i]) == M_COLLECTION) {
            if (c_collectionTypeKind(smd->memberTypes[i]) == OSPL_C_SEQUENCE) {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "typedef %s _%s_seq;\n\n",
                    smd->memberTypeNames[i],
                    smd->memberNames[i]);
            } else if (c_collectionTypeKind(smd->memberTypes[i]) == OSPL_C_ARRAY) {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "typedef %s _%s;\n\n",
                    smd->memberTypeNames[i],
                    smd->memberNames[i]);
            }
        }
    }
}

static void
idl_structGenerateAttributes(
        const structMetaDescriptions *smd)
{
    c_ulong i;

    /* Since the copy functions address the attributes directly, do not make them private right now. */
#if 0
    idl_printIndent(indent_level - 1);
    idl_fileOutPrintf(idl_fileCur(), "private:\n");
#endif
    /* Declare all the member attributes. */
    for (i = 0; i < smd->nrMembers; i++) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s %s_;\n", smd->memberTypeNames[i], smd->memberNames[i]);
    }
}

static void
idl_structGenerateConstructorsAndOperators(
        const structMetaDescriptions *smd,
        const c_char *name)
{
    c_ulong i;
    c_bool defValuePresent = FALSE;

    /* Start building default (empty) constructor. */
    idl_fileOutPrintf(idl_fileCur(), "\n");
    idl_printIndent(indent_level - 1);
    idl_fileOutPrintf(idl_fileCur(), "public:\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "%s()",
        name);
    indent_level += 2;
    for (i = 0; i < smd->nrMembers; i++) {
        char *defValue = idl_ISOCxx2DefaultValueFromCType(smd->memberTypes[i]);
        if (defValue) {
            if (!defValuePresent) {
                idl_fileOutPrintf(idl_fileCur(), " :\n");
                defValuePresent = TRUE;
            } else {
                idl_fileOutPrintf(idl_fileCur(), ",\n");
            }
            idl_printIndent(indent_level);
            idl_fileOutPrintf(
                idl_fileCur(),
                "%s_(%s)",
                smd->memberNames[i],
                defValue);
            os_free(defValue);
        }
    }
    idl_fileOutPrintf(idl_fileCur(), " {}\n\n");
    indent_level -= 2;

    /* Start building constructor that inits all parameters explicitly. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "explicit %s(\n",
        name);
    indent_level++;
    for (i = 0; i < smd->nrMembers; i++) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "%s %s%s",
            smd->memberInTypes[i],
            smd->memberNames[i],
            (i == (smd->nrMembers - 1) ? "" : ",\n"));
    }
    idl_fileOutPrintf(idl_fileCur(), ") : \n");
    indent_level++;
    for (i = 0; i < smd->nrMembers; i++) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "%s_(%s)%s",
            smd->memberNames[i],
            smd->memberNames[i],
            (i == (smd->nrMembers - 1) ? "" : ",\n"));
    }
    idl_fileOutPrintf(idl_fileCur(), " {}\n\n");
    indent_level -= 2;

    /* Start building the copy constructor. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s(const %s &_other) : \n", name, name);
    indent_level += 2;
    for (i = 0; i < smd->nrMembers; i++) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "%s_(_other.%s_)%s",
            smd->memberNames[i],
            smd->memberNames[i],
            (i == (smd->nrMembers - 1) ? "" : ",\n"));
    }
    idl_fileOutPrintf(idl_fileCur(), " {}\n\n");
    indent_level -= 2;

    /* Start building the move constructor. */
    idl_fileOutPrintf(idl_fileCur(), "#ifdef OSPL_DDS_CXX11\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s(%s &&_other) : \n", name, name);
    indent_level += 2;
    for (i = 0; i < smd->nrMembers; i++) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(
            idl_fileCur(),
            "%s_(::std::move(_other.%s_))%s",
            smd->memberNames[i],
            smd->memberNames[i],
            (i == (smd->nrMembers - 1) ? "" : ",\n"));
    }
    idl_fileOutPrintf(idl_fileCur(), " {}\n\n");
    indent_level -= 2;

    /* Start building the move assignment operator. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s& operator=(%s &&_other)\n", name, name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (this != &_other) {\n");
    indent_level++;
    for (i = 0; i < smd->nrMembers; i++) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s_ = ::std::move(_other.%s_);\n",
                smd->memberNames[i], smd->memberNames[i]);
    }
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return *this;\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_fileOutPrintf(idl_fileCur(), "#endif\n\n");

    /* Start building the const assignment operator. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s& operator=(const %s &_other)\n", name, name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (this != &_other) {\n");
    indent_level++;
    for (i = 0; i < smd->nrMembers; i++) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s_ = _other.%s_;\n",
                smd->memberNames[i], smd->memberNames[i]);
    }
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return *this;\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Start building the equals operator. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "bool operator==(const %s& _other) const\n", name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return ");
    indent_level++;
    for (i = 0; i < smd->nrMembers; i++) {
        if (i > 0) idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s_ == _other.%s_%s\n",
                smd->memberNames[i], smd->memberNames[i], (i == (smd->nrMembers - 1) ? ";" : " &&"));
    }
    indent_level -= 2;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Start building the not-equals operator. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "bool operator!=(const %s& _other) const\n", name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return !(*this == _other);\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}

static void
idl_structGenerateGettersSetters(
        const structMetaDescriptions *smd)
{
    c_ulong i;

    /* Start building the getter/setter methods for each attribute. */
    for (i = 0; i < smd->nrMembers; i++) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s %s() const { return this->%s_; }\n",
                smd->memberInTypes[i], smd->memberNames[i], smd->memberNames[i]);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s& %s() { return this->%s_; }\n",
                smd->memberTypeNames[i], smd->memberNames[i], smd->memberNames[i]);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "void %s(%s _val_) { this->%s_ = _val_; }\n",
                smd->memberNames[i], smd->memberInTypes[i], smd->memberNames[i]);
        if (idl_ISOCxx2IsRefType(smd->memberTypes[i])) {
            idl_fileOutPrintf(idl_fileCur(), "#ifdef OSPL_DDS_CXX11\n");
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "void %s(%s&& _val_) { this->%s_ = _val_; }\n",
                    smd->memberNames[i], smd->memberTypeNames[i], smd->memberNames[i]);
            idl_fileOutPrintf(idl_fileCur(), "#endif\n");
        }
    }
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
    c_char *cxxName;
    CxxTypeUserData *cxxUserData = (CxxTypeUserData *)userData;
    ISOCxx2ScopeStack *scopeStack;

    OS_UNUSED_ARG(scope);

    scopeStack = os_malloc(sizeof(ISOCxx2ScopeStack));
    scopeStack->type = idl_typeSpecDef(idl_typeSpec(structSpec));
    scopeStack->unionHasArtificialDefault = FALSE;

    /* Generate the code that opens a sealed class. */
    cxxName = idl_ISOCxx2Id(name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "class %s OSPL_DDS_FINAL\n",
        cxxName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public:\n");

    /* Increase the indentation level. */
    indent_level++;

    cxxUserData->typeStack = c_iterInsert(cxxUserData->typeStack, scopeStack);

    os_free(cxxName);

    /* return idl_explore to indicate that the rest of the structure needs to be processed. */
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
idl_structureClose (
    const char *name,
    void *userData)
{
    c_char *cxxName = idl_ISOCxx2Id(name);
    CxxTypeUserData *cxxUserData = (CxxTypeUserData *)userData;
    structMetaDescriptions smd;
    c_ulong i;

    /* Get the meta-data of the members of this datatype from the database. */
    ISOCxx2ScopeStack *scopeStack = c_iterTakeFirst(cxxUserData->typeStack);
    smd.structType = scopeStack->type;
    smd.nrMembers = c_structureMemberCount((c_structure) smd.structType);
    smd.memberTypes = os_malloc(smd.nrMembers * sizeof(*smd.memberTypes));
    smd.memberTypeNames = os_malloc(smd.nrMembers * sizeof(*smd.memberTypeNames));
    smd.memberInTypes = os_malloc(smd.nrMembers * sizeof(*smd.memberInTypes));
    smd.memberNames = os_malloc(smd.nrMembers * sizeof(*smd.memberNames));

    for (i = 0; i < smd.nrMembers; i++) {
        /* Get the meta-data of the attribute from the database. */
        c_member structMember = c_structureMember(smd.structType, i);
        smd.memberTypes[i] = c_memberType(structMember);
        smd.memberTypeNames[i] = idl_ISOCxx2TypeFromCType(smd.memberTypes[i]);
        smd.memberInTypes[i] = idl_ISOCxx2InTypeFromCType(smd.memberTypes[i]);
        smd.memberNames[i] = idl_ISOCxx2Id(c_specifierName(structMember));
    }

    /* Generate typedefs for all (anonymous) sequence attributes. */
    idl_structGenerateTypedefs(&smd);

    /* Create (private) struct attributes. */
    idl_structGenerateAttributes(&smd);

    /* Create constructors and operators. */
    idl_structGenerateConstructorsAndOperators(&smd, cxxName);

    /* Create the getters and setters. */
    idl_structGenerateGettersSetters(&smd);

    /* Release the meta-information about struct members. */
    for (i = 0; i < smd.nrMembers; i++) {
        os_free(smd.memberNames[i]);
        os_free(smd.memberInTypes[i]);
        os_free(smd.memberTypeNames[i]);
    }
    os_free(smd.memberNames);
    os_free(smd.memberInTypes);
    os_free(smd.memberTypeNames);
    os_free(smd.memberTypes);
    os_free(scopeStack);
    os_free(cxxName);

    /* Decrease the indentation level back to its original size. */
    indent_level--;

    /* Generate the C++ code that closes the class. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf (idl_fileCur(), "};\n\n");
}

struct unionCaseLabels {
    c_ulong nrLabels;
    c_value *labelValues;
};

typedef struct {
    c_type unionType;
    c_ulong nrBranches;
    c_type discrType;
    c_char *discrTypeName;
    c_char *discrInType;
    c_type *branchTypes;
    c_char **branchTypeNames;
    c_char **branchInTypes;
    c_char **branchNames;
    struct unionCaseLabels *branchLabels;
    c_value lowestDefaultValue;
    c_bool hasArtificialDefault;
} unionMetaDescriptions;

static void
idl_unionGenerateTypedefs(
        const unionMetaDescriptions *umd)
{
    c_ulong i;

    /* Declare typedefs for each (anonymous) sequence type. */
    for (i = 0; i < umd->nrBranches; i++) {
        if (c_baseObjectKind(umd->branchTypes[i]) == M_COLLECTION) {
            if (c_collectionTypeKind(umd->branchTypes[i]) == OSPL_C_SEQUENCE) {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "typedef %s _%s_seq;\n\n",
                    umd->branchTypeNames[i],
                    umd->branchNames[i]);
            } else if (c_collectionTypeKind(umd->branchTypes[i]) == OSPL_C_ARRAY) {
                idl_printIndent(indent_level);
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "typedef %s _%s;\n\n",
                    umd->branchTypeNames[i],
                    umd->branchNames[i]);
            }
        }
    }
}

static void
createUnionAttributes(const unionMetaDescriptions *umd)
{
    c_ulong i;

    idl_printIndent(indent_level - 1);
    idl_fileOutPrintf(idl_fileCur(), "private:\n");

    /* Declare a union attribute representing the discriminator. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s m__d;\n", umd->discrTypeName);

    /* Declare a union attribute comprising of all the branch types. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "union {\n");
    indent_level++;
    for (i = 0; i < umd->nrBranches; i++) {
        idl_printIndent(indent_level);
        if (idl_ISOCxx2IsRefType(umd->branchTypes[i]))
        {
            /* Unions are not allowed to store std::string and std::vector, so in that case store a pointer instead. */
            idl_fileOutPrintf(idl_fileCur(), "%s *m_%s;\n", umd->branchTypeNames[i], umd->branchNames[i]);
        } else {
            idl_fileOutPrintf(idl_fileCur(), "%s m_%s;\n", umd->branchTypeNames[i], umd->branchNames[i]);
        }
    }
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "} _union;\n\n");

}

static void
createUnionBranchAssignmentFunction(const c_char *name, const unionMetaDescriptions *umd)
{
    c_ulong i, j, nrLabels;

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "void _copy(const %s &that)\n", name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (this != &that) {\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "m__d = that.m__d;\n");
    idl_printIndent(indent_level);
    if (c_metaValueKind(c_metaObject(c_typeActualType(umd->discrType))) == V_BOOLEAN) {
        idl_fileOutPrintf(idl_fileCur(), "switch((int)that.m__d) {\n");
    } else {
        idl_fileOutPrintf(idl_fileCur(), "switch(that.m__d) {\n");
    }
    indent_level++;
    for (i = 0; i < umd->nrBranches; i++) {
        nrLabels = umd->branchLabels[i].nrLabels;
        if (nrLabels > 0) {
            for (j = 0; j < nrLabels; j++) {
                os_char *labelValue = idl_ISOCxx2ValueFromCValue(
                        umd->discrType, umd->branchLabels[i].labelValues[j]);
                idl_printIndent(indent_level - 1);
                idl_fileOutPrintf(idl_fileCur(), "case %s:\n", labelValue);
                os_free(labelValue);
            }
        } else {
            idl_printIndent(indent_level - 1);
            idl_fileOutPrintf(idl_fileCur(), "default:\n");
        }
        idl_printIndent(indent_level);
        if (idl_ISOCxx2IsRefType(umd->branchTypes[i])) {
            idl_fileOutPrintf(idl_fileCur(), "_union.m_%s = new %s(*that._union.m_%s);\n",
                umd->branchNames[i], umd->branchTypeNames[i], umd->branchNames[i]);
        } else {
            idl_fileOutPrintf(idl_fileCur(), "_union.m_%s = that._union.m_%s;\n",
                umd->branchNames[i], umd->branchNames[i]);
        }
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "break;\n");
    }
    if (umd->hasArtificialDefault) {
        idl_printIndent(indent_level - 1);
        idl_fileOutPrintf(idl_fileCur(), "default:\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "// empty branch: nothing to be copied...\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "break;\n");
    }

    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}

static void
createUnionBranchDestructor(const unionMetaDescriptions *umd)
{
    c_ulong i, j, nrLabels;

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "void _deleteBranch()\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);

    if (c_metaValueKind(c_metaObject(c_typeActualType(umd->discrType))) == V_BOOLEAN) {
        idl_fileOutPrintf(idl_fileCur(), "switch((int)m__d) {\n");
    } else {
        idl_fileOutPrintf(idl_fileCur(), "switch(m__d) {\n");
    }
    indent_level++;
    for (i = 0; i < umd->nrBranches; i++) {
        nrLabels = umd->branchLabels[i].nrLabels;
        if (nrLabels > 0) {
            for (j = 0; j < nrLabels; j++) {
                os_char *labelValue = idl_ISOCxx2ValueFromCValue(
                        umd->discrType, umd->branchLabels[i].labelValues[j]);
                idl_printIndent(indent_level - 1);
                idl_fileOutPrintf(idl_fileCur(), "case %s:\n", labelValue);
                os_free(labelValue);
            }
        } else {
            idl_printIndent(indent_level - 1);
            idl_fileOutPrintf(idl_fileCur(), "default:\n");
        }
        if (idl_ISOCxx2IsRefType(umd->branchTypes[i])) {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "delete _union.m_%s;\n", umd->branchNames[i]);
        }
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "break;\n");
    }
    if (umd->hasArtificialDefault) {
        idl_printIndent(indent_level - 1);
        idl_fileOutPrintf(idl_fileCur(), "default:\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "// empty branch: nothing to be deleted...\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "break;\n");
    }
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}

static void
createUnionConstructorsAndOperators(const c_char *name, const unionMetaDescriptions *umd)
{
    c_char *discrValue, *branchValue = NULL;
    c_ulong i, j = 0;

    idl_printIndent(indent_level - 1);
    idl_fileOutPrintf(idl_fileCur(), "public:\n");

    /* Start building default (empty) constructor. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s() :\n", name);

    /* Check whether there is a default branch (either implicit or explicit). */
    if (umd->lowestDefaultValue.kind != V_UNDEFINED) {
        /* If there is a default branch, pick its lowest discriminator value. */
        discrValue = idl_ISOCxx2ValueFromCValue(umd->discrType, umd->lowestDefaultValue);
        /* Check whether there is an explicit default branch. */
        for (i = 0; i < umd->nrBranches; i++) {
            if (umd->branchLabels[i].nrLabels == 0) {
                if (idl_ISOCxx2IsRefType(umd->branchTypes[i])) {
                    const char *refTemplate = "new %s()";
                    size_t len = strlen(refTemplate) + strlen(umd->branchTypeNames[i]) + 1;
                    branchValue = os_malloc(len);
                    snprintf(branchValue, len, refTemplate, umd->branchTypeNames[i]);
                } else {
                    /* If there is an explicit default branch, pick the default value for its type. */
                    branchValue = idl_ISOCxx2DefaultValueFromCType(umd->branchTypes[i]);
                }
                break;
            }
        }
    } else {
        /* If there is no default branch, pick the first discriminator for the first branch available. */
        i = 0;
        discrValue = idl_ISOCxx2ValueFromCValue(umd->discrType, umd->branchLabels[i].labelValues[0]);
        if (idl_ISOCxx2IsRefType(umd->branchTypes[0])) {
            const char *refTemplate = "new %s()";
            size_t len = strlen(refTemplate) + strlen(umd->branchTypeNames[i]) + 1;
            branchValue = os_malloc(len);
            snprintf(branchValue, len, refTemplate, umd->branchTypeNames[i]);
        } else {
            branchValue = idl_ISOCxx2DefaultValueFromCType(umd->branchTypes[0]);
        }
    }
    idl_printIndent(indent_level + 2);
    idl_fileOutPrintf(idl_fileCur(), "m__d(%s)", discrValue);
    os_free(discrValue);
    if (branchValue) {
        idl_fileOutPrintf(idl_fileCur(), "\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_printIndent(indent_level + 1);
        idl_fileOutPrintf(idl_fileCur(), "_union.m_%s = %s;\n", umd->branchNames[i], branchValue);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "}\n\n");
        os_free(branchValue);
    } else {
        idl_fileOutPrintf(idl_fileCur(), " {}\n\n");
    }

    /* Start building destructor. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "~%s()\n", name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_printIndent(indent_level + 1);
    idl_fileOutPrintf(idl_fileCur(), "_deleteBranch();\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Start building the copy constructor. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s(const %s &_other)\n", name, name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_printIndent(indent_level + 1);
    idl_fileOutPrintf(idl_fileCur(), "_copy(_other);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Start building the move constructor. */
    idl_fileOutPrintf(idl_fileCur(), "#ifdef OSPL_DDS_CXX11\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s(%s &&_other)\n", name, name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "::std::memcpy(this, &_other, sizeof(%s));\n", name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "::std::memset(&_other, 0, sizeof(%s));\n", name);
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Start building the move assignment operator. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s& operator=(%s &&_other)\n", name, name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (this != &_other) {\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "::std::memcpy(this, &_other, sizeof(%s));\n", name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "::std::memset(&_other, 0, sizeof(%s));\n", name);
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return *this;\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_fileOutPrintf(idl_fileCur(), "#endif\n\n");

    /* Start building the const assignment operator. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s& operator=(const %s &_other)\n", name, name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "_deleteBranch();\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "_copy(_other);\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return *this;\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Start building the equals operator. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "bool operator==(const %s& _other) const\n", name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (this != &_other) {\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "if (m__d != _other.m__d) {\n");
    idl_printIndent(indent_level + 1);
    idl_fileOutPrintf(idl_fileCur(), "return false;\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "} else {\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "switch (m__d) {\n");
    indent_level++;
    for (i = 0; i < umd->nrBranches; i++) {
        for (j = 0; j < umd->branchLabels[i].nrLabels; j++) {
            os_char *labelValue = idl_ISOCxx2ValueFromCValue(
                    umd->discrType, umd->branchLabels[i].labelValues[j]);
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "case %s:\n", labelValue);
            os_free(labelValue);
        }
        if (j == 0) {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "default: /* In case of explicit default branch. */\n");
        }
        idl_printIndent(indent_level + 1);
        if (idl_ISOCxx2IsRefType(umd->branchTypes[i])) {
            idl_fileOutPrintf(idl_fileCur(), "return (*_union.m_%s == *_other._union.m_%s);\n",
                    umd->branchNames[i], umd->branchNames[i]);
        } else {
            idl_fileOutPrintf(idl_fileCur(), "return (_union.m_%s == _other._union.m_%s);\n",
                    umd->branchNames[i], umd->branchNames[i]);
        }
        idl_printIndent(indent_level + 1);
        idl_fileOutPrintf(idl_fileCur(), "break;\n");
    }
    if (umd->hasArtificialDefault) {
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "default: /* In case of implicit default branch. */\n");
        idl_printIndent(indent_level + 1);
        idl_fileOutPrintf(idl_fileCur(), "return true; /* In case of implicit default branch. */\n");
        idl_printIndent(indent_level + 1);
        idl_fileOutPrintf(idl_fileCur(), "break;\n");
    }
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "} else {\n");
    idl_printIndent(indent_level + 1);
    idl_fileOutPrintf(idl_fileCur(), "return true;\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");

    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Start building the not-equals operator. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "bool operator!=(const %s& _other) const\n", name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return !(*this == _other);\n");
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}

static void
createUnionDiscrGetterSetter(const unionMetaDescriptions *umd)
{
    c_bool checkDefault = FALSE;
    c_ulong i, j;
    c_ulong lblCount = 0;

    /* Generate a getter for the discriminator. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s _d() const\n", umd->discrInType);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_printIndent(indent_level + 1);
    idl_fileOutPrintf(idl_fileCur(), "return m__d;\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Generate a setter for the discriminator. */
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "void _d(%s val)\n", umd->discrInType);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    for (i = 0; i < umd->nrBranches; i++) {
        lblCount += umd->branchLabels[i].nrLabels;
    }

    if (lblCount > 0) {
        if (c_metaValueKind(c_metaObject(c_typeActualType(umd->discrType))) == V_BOOLEAN) {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    bool valid = (val == m__d);\n");
        } else {
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    bool valid = true;\n");

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    switch (val) {\n");
            for (i = 0; i < umd->nrBranches; i++) {
                for (j = 0; j < umd->branchLabels[i].nrLabels; j++) {
                    os_char *labelValue = idl_ISOCxx2ValueFromCValue(
                            umd->discrType, umd->branchLabels[i].labelValues[j]);
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "    case %s:\n", labelValue);
                    os_free(labelValue);
                }
                for (j = 0; j < umd->branchLabels[i].nrLabels; j++) {
                    os_char *labelValue = idl_ISOCxx2ValueFromCValue(
                            umd->discrType, umd->branchLabels[i].labelValues[j]);
                    if (j == 0) {
                        idl_printIndent(indent_level);
                        idl_fileOutPrintf(idl_fileCur(), "        if (m__d != %s", labelValue);
                    } else {
                        idl_fileOutPrintf(idl_fileCur(), " &&\n");
                        idl_printIndent(indent_level );
                        idl_fileOutPrintf(idl_fileCur(), "              m__d != %s", labelValue);
                    }
                    os_free(labelValue);
                }
                if (umd->branchLabels[i].nrLabels > 0) {
                    idl_fileOutPrintf(idl_fileCur(), ") {\n");
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "            valid = false;\n");
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "        }\n");
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "        break;\n");
                }
            }

            if (c_baseObjectKind(c_typeActualType(umd->discrType)) == M_ENUMERATION) {
                if (lblCount > 0 && (lblCount < c_arraySize(c_enumeration(c_typeActualType(umd->discrType))->elements))) {
                    checkDefault = TRUE;
                }
            } else {
                checkDefault = TRUE;
            }

            if (checkDefault) {
                int n = 0;
                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "    default:\n");

                for (i = 0; i < umd->nrBranches; i++) {
                    for (j = 0; j < umd->branchLabels[i].nrLabels; j++) {
                        os_char *labelValue = idl_ISOCxx2ValueFromCValue(
                                umd->discrType, umd->branchLabels[i].labelValues[j]);
                        if (n == 0) {
                            idl_printIndent(indent_level);
                            idl_fileOutPrintf(idl_fileCur(), "        if (m__d == %s", labelValue);
                            n = 1;
                        } else {
                            idl_fileOutPrintf(idl_fileCur(), " ||\n");
                            idl_printIndent(indent_level );
                            idl_fileOutPrintf(idl_fileCur(), "              m__d == %s", labelValue);
                        }
                        os_free(labelValue);
                    }
                }

                if (n != 0) {
                    idl_fileOutPrintf(idl_fileCur(), ") {\n");
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "            valid = false;\n");
                    idl_printIndent(indent_level);
                    idl_fileOutPrintf(idl_fileCur(), "        }\n");
                }

                idl_printIndent(indent_level);
                idl_fileOutPrintf(idl_fileCur(), "        break;\n");
            }

            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "    }\n\n");
        }

        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    if (!valid) {\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "        throw dds::core::PreconditionNotMetError(\"New discriminator value does not match current discriminator\");\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "    }\n\n");
    }

    idl_printIndent(indent_level + 1);
    idl_fileOutPrintf(idl_fileCur(), "m__d = val;\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}

static void createUnionGetterBody(const unionMetaDescriptions *umd, c_ulong i)
{
    os_char *labelValue;
    c_ulong j, k;
    c_bool ifClause = FALSE;

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    idl_printIndent(indent_level);
    if (umd->branchLabels[i].nrLabels > 0) {
        for (j = 0; j < umd->branchLabels[i].nrLabels; j++) {
            labelValue = idl_ISOCxx2ValueFromCValue(
                    umd->discrType, umd->branchLabels[i].labelValues[j]);
            if (j == 0) {
                idl_fileOutPrintf(idl_fileCur(), "if (m__d == %s", labelValue);
                ifClause = TRUE;
                indent_level++;
            } else {
                idl_fileOutPrintf(idl_fileCur(), "|| \n");
                idl_printIndent(indent_level + 1);
                idl_fileOutPrintf(idl_fileCur(), "m__d == %s", labelValue);
            }
            os_free(labelValue);
        }
        if (ifClause) {
            idl_fileOutPrintf(idl_fileCur(), ") {\n");
            idl_printIndent(indent_level);
        }
    } else {
        for (j = 0; j < umd->nrBranches; j++) {
            for (k = 0; k < umd->branchLabels[j].nrLabels; k++) {
                labelValue = idl_ISOCxx2ValueFromCValue(
                        umd->discrType, umd->branchLabels[j].labelValues[k]);
                if (j == 0 && k == 0) {
                    idl_fileOutPrintf(idl_fileCur(), "if (m__d != %s", labelValue);
                    ifClause = TRUE;
                    indent_level++;
                } else {
                    idl_fileOutPrintf(idl_fileCur(), "&& \n");
                    idl_printIndent(indent_level + 1);
                    idl_fileOutPrintf(idl_fileCur(), "m__d != %s", labelValue);
                }
                os_free(labelValue);
            }
        }
        if (ifClause) {
            idl_fileOutPrintf(idl_fileCur(), ") {\n");
            idl_printIndent(indent_level);
        }
    }
    if (idl_ISOCxx2IsRefType(umd->branchTypes[i])) {
        idl_fileOutPrintf(idl_fileCur(), "return *_union.m_%s;\n", umd->branchNames[i]);
    } else {
        idl_fileOutPrintf(idl_fileCur(), "return _union.m_%s;\n", umd->branchNames[i]);
    }
    if (ifClause) {
        idl_printIndent(indent_level - 1);
        idl_fileOutPrintf(idl_fileCur(), "} else {\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "throw dds::core::PreconditionNotMetError(\"Requested branch does not match current discriminator\");\n");
        indent_level--;
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "}\n");
    }
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}

static void createUnionSetterBody(const unionMetaDescriptions *umd, c_ulong i)
{
    os_char *labelValue;

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    indent_level++;
    if (umd->branchLabels[i].nrLabels > 0) {
        labelValue = idl_ISOCxx2ValueFromCValue(
                            umd->discrType, umd->branchLabels[i].labelValues[0]);
    } else {
        labelValue = idl_ISOCxx2ValueFromCValue(umd->discrType, umd->lowestDefaultValue);
    }
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "_deleteBranch();\n");
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "m__d = %s;\n", labelValue);
    idl_printIndent(indent_level);
    if (idl_ISOCxx2IsRefType(umd->branchTypes[i])) {
        idl_fileOutPrintf(idl_fileCur(), "_union.m_%s = new %s(val);\n", umd->branchNames[i], umd->branchTypeNames[i]);
    } else {
        idl_fileOutPrintf(idl_fileCur(), "_union.m_%s = val;\n", umd->branchNames[i]);
    }
    os_free(labelValue);
    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
}

static void
createUnionBranchGettersSetters(const unionMetaDescriptions *umd)
{
    c_ulong i;

    /* Start building the getter/setter methods for each attribute. */
    for (i = 0; i < umd->nrBranches; i++) {

        /* Build const-getter. */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s %s() const\n",
                umd->branchInTypes[i], umd->branchNames[i]);
        createUnionGetterBody(umd, i);

        /* Build ref-getter. */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "%s& %s()\n",
                umd->branchTypeNames[i], umd->branchNames[i]);
        createUnionGetterBody(umd, i);

        /* Build setter. */
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "void %s(%s val)\n",
                umd->branchNames[i], umd->branchInTypes[i]);
        createUnionSetterBody(umd, i);
        idl_fileOutPrintf(idl_fileCur(), "\n");

        /* When appropriate, build setter with move semantics. */
        if (idl_ISOCxx2IsRefType(umd->branchTypes[i])) {
            idl_fileOutPrintf(idl_fileCur(), "#ifdef OSPL_DDS_CXX11\n");
            idl_printIndent(indent_level);
            idl_fileOutPrintf(idl_fileCur(), "void %s(%s &&val)\n",
                    umd->branchNames[i], umd->branchTypeNames[i]);
            createUnionSetterBody(umd, i);
            idl_fileOutPrintf(idl_fileCur(), "#endif\n\n");
        }
    }

}

static void
createUnionImplicitDefaultSetter(const unionMetaDescriptions *umd)
{
    c_ulong i;
    os_char *labelValue;

    /* Check if all possible discriminant values are covered. */
    if (umd->lowestDefaultValue.kind != V_UNDEFINED)
    {
        /* Check if an explicit default already exists. */
        for (i = 0; i < umd->nrBranches; i++) {
            if (umd->branchLabels[i].nrLabels == 0) return;
        }

        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "void _default()\n");
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        labelValue = idl_ISOCxx2ValueFromCValue(umd->discrType, umd->lowestDefaultValue);
        idl_printIndent(indent_level + 1);
        idl_fileOutPrintf(idl_fileCur(), "m__d = %s;\n", labelValue);
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "}\n");
        os_free(labelValue);
    }
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
    c_char *cxxName;
    CxxTypeUserData *cxxUserData = (CxxTypeUserData *)userData;
    ISOCxx2ScopeStack *scopeStack;

    OS_UNUSED_ARG(scope);

    /* Set expectation of implicit default branch to FALSE.
     * idl_artificialDefaultLabelOpenClose will set it to TRUE when encountered.
     */
    scopeStack = os_malloc(sizeof(ISOCxx2ScopeStack));
    scopeStack->type = idl_typeSpecDef(idl_typeSpec(unionSpec));
    scopeStack->unionHasArtificialDefault = FALSE;


    /* Generate the code that opens a sealed class. */
    cxxName = idl_ISOCxx2Id(name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(
        idl_fileCur(),
        "class %s OSPL_DDS_FINAL\n",
        cxxName);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "public:\n");

    /* Increase the indentation level. */
    indent_level++;
    cxxUserData->typeStack = c_iterInsert(cxxUserData->typeStack, scopeStack);
    os_free(cxxName);

    return idl_explore;
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
    CxxTypeUserData *cxxUserData;
    ISOCxx2ScopeStack *scopeStack;

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(labelVal);
    OS_UNUSED_ARG(typeSpec);

    cxxUserData = (CxxTypeUserData *)userData;
    scopeStack = (ISOCxx2ScopeStack *)  c_iterObject(cxxUserData->typeStack, 0);
    scopeStack->unionHasArtificialDefault = TRUE;
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
idl_unionClose (
    const char *name,
    void *userData)
{
    c_char *cxxName = idl_ISOCxx2Id(name);
    CxxTypeUserData *cxxUserData = (CxxTypeUserData *)userData;
    unionMetaDescriptions umd;
    c_ulong i, j, nrLabels;

    /* Get the meta-data of the members of this union from the database. */
    ISOCxx2ScopeStack *scopeStack = c_iterTakeFirst(cxxUserData->typeStack);
    umd.unionType = scopeStack->type;
    umd.nrBranches = c_unionUnionCaseCount(umd.unionType);
    umd.discrType = c_unionUnionSwitchType(umd.unionType);
    umd.discrTypeName = idl_ISOCxx2TypeFromCType(umd.discrType);
    umd.discrInType = idl_ISOCxx2InTypeFromCType(umd.discrType);
    umd.branchTypes = os_malloc(umd.nrBranches * sizeof(*umd.branchTypes));
    umd.branchTypeNames = os_malloc(umd.nrBranches * sizeof(*umd.branchTypeNames));
    umd.branchInTypes = os_malloc(umd.nrBranches * sizeof(*umd.branchInTypes));
    umd.branchNames = os_malloc(umd.nrBranches * sizeof(*umd.branchNames));
    umd.branchLabels = os_malloc(umd.nrBranches * sizeof(*umd.branchLabels));
    umd.lowestDefaultValue = idl_ISOCxx2LowestUnionDefaultValue(umd.unionType);
    umd.hasArtificialDefault = scopeStack->unionHasArtificialDefault;
    for (i = 0; i < umd.nrBranches; i++) {
        /* Get the meta-data of the branches from the database. */
        c_unionCase branch = c_unionUnionCase(umd.unionType, i);
        umd.branchTypes[i] = c_unionCaseType(branch);
        umd.branchTypeNames[i] = idl_ISOCxx2TypeFromCType(umd.branchTypes[i]);
        umd.branchInTypes[i] = idl_ISOCxx2InTypeFromCType(umd.branchTypes[i]);
        umd.branchNames[i] = idl_ISOCxx2Id(c_specifierName(branch));
        nrLabels = c_arraySize(branch->labels);
        umd.branchLabels[i].nrLabels = nrLabels;
        if (nrLabels > 0) {
            umd.branchLabels[i].labelValues = os_malloc(sizeof(c_value) * nrLabels);
            for (j = 0; j < nrLabels; j++) {
                umd.branchLabels[i].labelValues[j] = c_literal(branch->labels[j])->value;
            }
        } else {
            umd.branchLabels[i].labelValues = NULL;
        }
    }

    /* Generate typedefs for all (anonymous) sequence branch types. */
    idl_unionGenerateTypedefs(&umd);

    /* Generate the union attributes. */
    createUnionAttributes(&umd);

    /* Create utility function to assign branch from one union to another. */
    createUnionBranchAssignmentFunction(name, &umd);

    /* Create a utility function that deletes the current branch member if appropriate. */
    createUnionBranchDestructor(&umd);

    /* Generate union constructors */
    createUnionConstructorsAndOperators(cxxName, &umd);

    /* Generate the discriminator getter/setter. */
    createUnionDiscrGetterSetter(&umd);

    /* Generate the branch getters/setters. */
    createUnionBranchGettersSetters(&umd);

    /* Generate the implicit default setter (if appropriate) */
    createUnionImplicitDefaultSetter(&umd);

    /* Release the meta-information about union branches. */
    for (i = 0; i < umd.nrBranches; i++) {
        os_free(umd.branchNames[i]);
        os_free(umd.branchInTypes[i]);
        os_free(umd.branchTypeNames[i]);
        os_free(umd.branchLabels[i].labelValues);
    }
    os_free(umd.branchLabels);
    os_free(umd.discrInType);
    os_free(umd.discrTypeName);
    os_free(umd.branchNames);
    os_free(umd.branchInTypes);
    os_free(umd.branchTypeNames);
    os_free(umd.branchTypes);
    os_free(scopeStack);
    os_free(cxxName);

    indent_level--;
    idl_printIndent(indent_level); idl_fileOutPrintf(idl_fileCur(), "};\n\n");
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
    //c_char *scopedName = idl_scopeStack(scope, "_", name);
    c_char *cxxName = idl_ISOCxx2Id(name);
    idl_type typedefType = idl_typeSpecType(idl_typeDefRefered(defSpec));
    c_char *typeName;

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    switch(typedefType) {
    case idl_tbasic:
    case idl_ttypedef:
    case idl_tenum:
    case idl_tstruct:
    case idl_tunion:
    case idl_tarray:
    case idl_tseq:
        /* generate code for a standard mapping or a basic type mapping */
        typeName = idl_ISOCxx2TypeFromTypeSpec(idl_typeSpec(idl_typeDefRefered(defSpec)));
        idl_printIndent(indent_level);
        idl_fileOutPrintf(idl_fileCur(), "typedef %s %s;\n\n", typeName, cxxName);
        os_free(typeName);
        break;
    default:
        assert(0); /* idl_typedefOpenClose: Unsupported typedef type */
        break;
    }
    os_free(cxxName);
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
    char *cxxName;

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    cxxName = idl_ISOCxx2Id(name);
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "OSPL_ENUM %s {\n", cxxName);

    enum_element = idl_typeEnumNoElements(enumSpec);
    indent_level++;
    os_free(cxxName);

    /* return idl_explore to indicate that the rest of the enumeration needs to be processed */
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
idl_enumerationClose (
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    indent_level--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "};\n\n");
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
    char *labelName;

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    /* Translate the remaining label into their C++11 representation. */
    labelName = idl_ISOCxx2Id(name);
    enum_element--;
    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s%s\n", labelName, (enum_element > 0) ? "," : "");
    os_free(labelName);
}


static void
idl_constantOpenClose (
    idl_scope scope,
    idl_constSpec constantSpec,
    void *userData)
{
    char *constTypeName = idl_ISOCxx2TypeFromTypeSpec(idl_constSpecTypeGet(constantSpec));
    char *cxxConstName = idl_ISOCxx2Id(idl_constSpecName(constantSpec));
    char *constImage = idl_constSpecImage(constantSpec);

    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(userData);

    idl_printIndent(indent_level);
    idl_fileOutPrintf(idl_fileCur(), "const %s %s = %s;\n\n", constTypeName, cxxConstName, constImage);
    os_free(constImage);
    os_free(cxxConstName);
    os_free(constTypeName);

}

/**
 * Standard control structure to specify that anonymous
 * type definitions are to be processed inline with the
 * type itself in contrast with the setting of idl_prior.
*/
static idl_programControl idl_genISOCxx2LoadControl = {
    idl_inline
};

/** @brief return the program control structure for the splice type generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    OS_UNUSED_ARG(userData);

    return &idl_genISOCxx2LoadControl;
}

/** @brief return the callback table for the splice type generation functions.
 */
idl_program
idl_genISOCxx2TypeProgram(
    CxxTypeUserData *userData)
{
    idl_genCxx2Type.idl_getControl                  = idl_getControl;
    idl_genCxx2Type.fileOpen                        = idl_fileOpen;
    idl_genCxx2Type.fileClose                       = idl_fileClose;
    idl_genCxx2Type.moduleOpen                      = idl_moduleOpen;
    idl_genCxx2Type.moduleClose                     = idl_moduleClose;
    idl_genCxx2Type.structureOpen                   = idl_structureOpen;
    idl_genCxx2Type.structureClose                  = idl_structureClose;
    idl_genCxx2Type.structureMemberOpenClose        = NULL;
    idl_genCxx2Type.enumerationOpen                 = idl_enumerationOpen;
    idl_genCxx2Type.enumerationClose                = idl_enumerationClose;
    idl_genCxx2Type.enumerationElementOpenClose     = idl_enumerationElementOpenClose;
    idl_genCxx2Type.unionOpen                       = idl_unionOpen;
    idl_genCxx2Type.unionClose                      = idl_unionClose;
    idl_genCxx2Type.unionCaseOpenClose              = NULL;
    idl_genCxx2Type.unionLabelsOpenClose            = NULL;
    idl_genCxx2Type.unionLabelOpenClose             = NULL;
    idl_genCxx2Type.typedefOpenClose                = idl_typedefOpenClose;
    idl_genCxx2Type.boundedStringOpenClose          = NULL;
    idl_genCxx2Type.sequenceOpenClose               = NULL;
    idl_genCxx2Type.constantOpenClose               = idl_constantOpenClose;
    idl_genCxx2Type.artificialDefaultLabelOpenClose = idl_artificialDefaultLabelOpenClose;
    idl_genCxx2Type.userData                        = userData;

    return &idl_genCxx2Type;
}
