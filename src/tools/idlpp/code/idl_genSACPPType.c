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
/*
   This module generates type definitions related to
   an IDL input file.
   For implementation consideration see end of file.
*/

#include "idl_program.h"
/**
 * @file
 * This module generates Standalone C++ data types
 * related to an IDL input file.
*/

#include "idl_scope.h"
#include "idl_genSACPPType.h"
#include "idl_genCxxHelper.h"
#include "idl_genSplHelper.h"
#include "idl_tmplExp.h"
#include "idl_dependencies.h"
#include "idl_genLanguageHelper.h"
#include "idl_dll.h"

#include "os.h"
#include <ctype.h>
#include "c_typebase.h"
#include "ut_stack.h"

#define MAX_BUFFER (256)

struct idl_genSACPPData {
    /** indentation level */
    c_long indent_level;
    /** enumeration element index */
    c_long enum_element;
    char *enum_def;
    ut_stack structSpecStack;
    char buffer[MAX_BUFFER];
};

static struct idl_genSACPPData idl_genSACPPData;

struct idl_genSACPPCodeBuffer {
    c_ulong nrOfLines;
    c_ulong currentLine;
    char **lines;
};

/* this structure contains the C++ struct definition
 * from the idl structure.
 */
struct idl_genSACPPStructSpec {
    struct idl_genSACPPCodeBuffer codebuffer;
    c_bool hasRef;
};

/* @brief generate dimension of an array slice
 *
 * arraySliceDimensions is a local support function to generate
 * the array dimensions of an array slice
 *
 * @param typeArray Specifies the type of the array
 */
static void
idl_arraySliceDimensions(
    idl_typeArray typeArray)
{
    if (idl_typeSpecType(idl_typeArrayType(typeArray)) == idl_tarray &&
        idl_typeSpecType(idl_typeArrayType(idl_typeArray(idl_typeArrayType(typeArray)))) == idl_tarray) {
        idl_arraySliceDimensions(idl_typeArray(idl_typeArrayType(typeArray)));
    }
    idl_fileOutPrintf(idl_fileCur(), "[%d]", idl_typeArraySize(typeArray));
}

/* @brief generate dimension of an array
 *
 * arrayDimensions is a local support function to generate
 * the array dimensions of an array
 *
 * @param typeArray Specifies the type of the array
 */
static char *
idl_arrayDimensions(
    idl_typeArray typeArray,
    struct idl_genSACPPData *arg)
{
    char *dims;
    c_ulong len;
    idl_typeSpec ts;

    dims = (char *)os_malloc(1);
    dims[0] = '\0';
    len = 1;
    ts = idl_typeSpec(typeArray);
    while (idl_typeSpecType(ts) == idl_tarray) {
        snprintf(arg->buffer, MAX_BUFFER, "[%d]", idl_typeArraySize(idl_typeArray(ts)));
        len += strlen(arg->buffer);
        dims = os_realloc(dims, len);
        dims = os_strcat(dims, arg->buffer);
        ts = idl_typeArrayType(typeArray);
    }
    return dims;
}

static void
idl_arrayTypeDefs(
    idl_typeArray typeArray,
    const char *name,
    const char *arrayDims,
    struct idl_genSACPPData *arg)
{
    idl_typeSpec actualType;
    idl_typeSpec subType;

    actualType = idl_typeArrayActual(typeArray);
    subType = idl_typeArrayType(typeArray);

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"typedef %s %s_slice;\n",
        idl_corbaCxxTypeFromTypeSpec(actualType), name);
    if (idl_typeSpecType(subType) == idl_tarray) {
        idl_arraySliceDimensions(idl_typeArray(subType));
    }

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"typedef %s %s%s;\n",
        idl_corbaCxxTypeFromTypeSpec(actualType),
        name,
        arrayDims);

    if (idl_typeSpecType(subType) == idl_tseq) {
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"typedef DDS_DCPS_VLArray_out< %s, %s_slice, %s_var, %s_uniq_> %s_out;\n",
            name, name, name, name, name);
    } else {
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"typedef %s %s_out;\n", name, name);
    }

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"static %s_slice *%s_alloc();\n", name, name);

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"static void %s_free(%s_slice *);\n", name, name);

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"static void %s_copy(%s_slice *to, const %s_slice *from);\n",
        name, name, name);

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"static %s_slice *%s_dup(const %s_slice *from);\n",
        name, name, name);

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"struct %s_uniq_ {};\n", name);

    if (idl_typeSpecType(subType) == idl_tarray) {
        /* Multi-dimensional array */
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"typedef DDS_DCPS_MArray_var< %s, %s_slice, struct %s_uniq_ > %s_var;\n",
            name, name, name, name);
    } else {
        if (idl_typeSpecType(subType) == idl_tseq) {
            /* array of sequence */
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"typedef DDS_DCPS_VArray_var< %s, %s_slice, struct %s_uniq_ > %s_var;\n",
                name, name, name, name);
        } else {
            /* any other subtype */
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"typedef DDS_DCPS_FArray_var< %s, %s_slice, struct %s_uniq_ > %s_var;\n",
                name, name, name, name);
        }
    }
}

static void
idl_seqTypeDefs(
    idl_typeSeq typeSeq,
    const char *name,
    struct idl_genSACPPData *arg)
{
    char *subTypeName;
    idl_typeSpec seqType;

    seqType = idl_typeSeqType(typeSeq);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"struct %s_uniq_ {};\n", name);

    if ((idl_typeSpecType(seqType) == idl_tbasic) &&
        (idl_typeBasicType(idl_typeBasic(seqType)) == idl_string)) {
        idl_printIndent(arg->indent_level);
        if (idl_typeSeqMaxSize(typeSeq) == 0) {
            idl_fileOutPrintf(idl_fileCur(),"typedef DDS_DCPSUStrSeqT<struct %s_uniq_> %s;\n",
               name, name);
        } else {
            idl_fileOutPrintf(idl_fileCur(),"typedef DDS_DCPSBStrSeq< %d > %s;\n",
                idl_typeSeqMaxSize(typeSeq), name);
        }
    } else {
        subTypeName = idl_corbaCxxTypeFromTypeSpec(seqType);
        idl_printIndent(arg->indent_level);
        if (idl_typeSeqMaxSize(typeSeq) == 0) {
            idl_fileOutPrintf(idl_fileCur(),"typedef DDS_DCPSUFLSeq< %s, struct %s_uniq_ > %s;\n",
               subTypeName, name, name);
        } else {
            idl_fileOutPrintf(idl_fileCur(),"typedef DDS_DCPSBFLSeq< %s, %s, %d > %s;\n",
                subTypeName, subTypeName, idl_typeSeqMaxSize(typeSeq), name);
        }
        os_free(subTypeName);
    }
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"typedef DDS_DCPSSequence_var< %s > %s_var;\n",
        name, name);

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(),"typedef DDS_DCPSSequence_out< %s > %s_out;\n",
        name, name);

}

/** @brief generate name which will be used as a macro to prevent multiple inclusions
 *
 * From the specified basename create a macro which will
 * be used to prevent multiple inclusions of the generated
 * header file. The basename characters are translated
 * into uppercase characters and the append string is
 * appended to the macro.
 */
static c_char *
idl_macroFromBasename(
    const char *basename,
    const char *append)
{
    static c_char macro[200];
    c_long i;

    for (i = 0; i < (c_long)strlen(basename); i++) {
        macro[i] = toupper(basename[i]);
        macro[i+1] = '\0';
    }
    os_strncat(macro, append, (size_t)((int)sizeof(macro)-(int)strlen(append)));

    return macro;
}

static os_equality
defName(
    void *iterElem,
    void *arg)
{
    if (strcmp((char *)iterElem, (char *)arg) == 0) {
        return OS_EQ;
    }
    return OS_NE;
}

/* @brief callback function called on opening the IDL input file.
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
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;
    int i;

    /* First initialize userData */
    arg->indent_level = 0;
    arg->enum_element = 0;
    arg->enum_def = NULL;
    arg->structSpecStack = ut_stackNew(UT_STACK_DEFAULT_INC);

    /* Generate multiple inclusion prevention code */
    idl_fileOutPrintf(idl_fileCur(), "#ifndef %s\n", idl_macroFromBasename(name, "DCPS_H"));
    idl_fileOutPrintf(idl_fileCur(), "#define %s\n", idl_macroFromBasename(name, "DCPS_H"));
    idl_fileOutPrintf(idl_fileCur(), "\n");
    /* Generate inclusion of standard OpenSplice DDS type definition files */
    idl_fileOutPrintf(idl_fileCur(), "#include <sacpp_mapping.h>\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
#ifndef RP
    /* Generate code for inclusion of application specific include files */
    for (i = 0; i < idl_depLength(idl_depDefGet()); i++) {
        idl_fileOutPrintf(idl_fileCur(), "#include \"%s.h\"\n", idl_depGet(idl_depDefGet(), i));
    }
    if (idl_depLength(idl_depDefGet()) > 0) {
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
#endif

    /* Setup dll stuff */
    idl_fileOutPrintf(idl_fileCur(), "%s\n", idl_dllGetHeader());
    /* return idl_explore to indicate that the rest of the file needs to be processed */
    return idl_explore;
}

/* @brief callback function called on closing the IDL input file.
 *
 * Generate standard file footer consisting of:
 * - mutiple inclusion prevention closure
 */
static void
idl_fileClose (
    void *userData)
{
    /* Generate closure of multiple inclusion prevention code */
    idl_fileOutPrintf(idl_fileCur(), "#endif\n");

    /* \TODO: deinit userData */
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
 * This maps on namespaces in c++
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
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "namespace %s {\n", name);
    arg->indent_level++;
    /* return idl_explore to indicate that the rest of the module needs to be processed */
    return idl_explore;
}

static void
idl_moduleClose(
    void *userData)
{
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;

    arg->indent_level--;
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n");
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
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;
    struct idl_genSACPPStructSpec *output;

    output = (struct idl_genSACPPStructSpec *)os_malloc(sizeof(struct idl_genSACPPStructSpec));
    output->codebuffer.nrOfLines = idl_typeStructNoMembers(structSpec);
    output->codebuffer.lines = (char **)os_malloc(output->codebuffer.nrOfLines*sizeof(char *));
    output->codebuffer.currentLine = 0;
    ut_stackPush(arg->structSpecStack, (void *)output);

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "struct %s {\n", name);
    arg->indent_level++;
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
idl_structureClose (
    const char *name,
    void *userData)
{
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;
    struct idl_genSACPPStructSpec *structSpec;
    c_ulong line;

    structSpec = (struct idl_genSACPPStructSpec *)ut_stackPop(arg->structSpecStack);
    assert(structSpec);
    assert(structSpec->codebuffer.nrOfLines == structSpec->codebuffer.currentLine);
    for (line=0;line<structSpec->codebuffer.currentLine;line++) {
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(), structSpec->codebuffer.lines[line]);
        os_free(structSpec->codebuffer.lines[line]);
    }
    os_free(structSpec->codebuffer.lines);

    arg->indent_level--;
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "};\n");

    /* generate _var and  _out definitions */
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "typedef DDS_DCPSStruct_var< %s > %s_var;\n", name, name);
    /* _out type depends on contents of structure:
     * If structure contains variable length types (e.g. any, string, wstring, seq, etc)
     * it should be defined as a special type for proper memory management.
     * Else it can be defined as a standard c++ ref.
     */
    idl_printIndent(arg->indent_level);
    if (structSpec->hasRef) {
        idl_fileOutPrintf(idl_fileCur(), "typedef DDS_DCPSStruct_out< %s > %s_out;\n\n", name, name);
    } else {
        idl_fileOutPrintf(idl_fileCur(), "typedef %s &%s_out;\n\n", name, name);
    }
    os_free(structSpec);
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
idl_structureMemberOpenClose (
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec,
    void *userData)
{
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;
    struct idl_genSACPPStructSpec *structSpec;
    char *memberName;
    char *seqName;
    char *dims;

    memberName = idl_cxxId(name);
    structSpec = (struct idl_genSACPPStructSpec *)ut_stackPop(arg->structSpecStack);

    switch (idl_typeSpecType(typeSpec)) {
    case idl_tbasic:
        if (idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string) {
            snprintf(arg->buffer,MAX_BUFFER,"::DDS::String_mgr %s;\n", memberName);
        } else {
            snprintf(arg->buffer,MAX_BUFFER,"%s %s;\n",
            idl_corbaCxxTypeFromTypeSpec(typeSpec),
                memberName);
        }
    break;
    case idl_tenum:
    case idl_ttypedef:
    case idl_tstruct:
    case idl_tunion:
        snprintf(arg->buffer,MAX_BUFFER,"%s %s;\n",
            idl_corbaCxxTypeFromTypeSpec(typeSpec),
            memberName);
    break;
    case idl_tarray:
        /* generate code for an array mapping */
        snprintf(arg->buffer,MAX_BUFFER,"_%s_array",memberName);
        seqName = os_strdup(arg->buffer);
        dims = idl_arrayDimensions(idl_typeArray(typeSpec), arg);
        idl_arrayTypeDefs(idl_typeArray(typeSpec), seqName, dims, arg);
        snprintf(arg->buffer, MAX_BUFFER, "_%s_array %s;\n", memberName, memberName);
        os_free(dims);
        os_free(seqName);
    break;
    case idl_tseq:
        snprintf(arg->buffer,MAX_BUFFER,"_%s_seq",memberName);
        seqName = os_strdup(arg->buffer);
        idl_seqTypeDefs(idl_typeSeq(typeSpec), seqName, arg);
        snprintf(arg->buffer, MAX_BUFFER, "%s %s;\n", seqName, memberName);
        os_free(seqName);
    break;
    default:
        printf("idl_structureMemberOpenClose: Unsupported structure member type (member name = %s, type name = %s)\n",
            name, idl_scopedTypeName(typeSpec));
    }
    structSpec->codebuffer.lines[structSpec->codebuffer.currentLine++] = os_strdup(arg->buffer);

    ut_stackPush(arg->structSpecStack, structSpec);
    os_free(memberName);
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
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;
    struct idl_genSACPPCodeBuffer *codebuffer;
    char *switchTypeName;

    codebuffer = (struct idl_genSACPPCodeBuffer *)os_malloc(sizeof(struct idl_genSACPPCodeBuffer));
    switchTypeName = idl_corbaCxxTypeFromTypeSpec(idl_typeUnionSwitchKind(unionSpec));

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "class %s {\n",name);
    idl_fileOutPrintf(idl_fileCur(), "public:\n");
    arg->indent_level++;

    /* Setter for discriminant */
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "void _d(%s val) {\n", switchTypeName);
    arg->indent_level++;
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "m__d = val;\n");
    arg->indent_level--;
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");

    /* Getter for discriminant */
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s  _d() const {\n", switchTypeName);
    arg->indent_level++;
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "return m__d;\n");
    arg->indent_level--;
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
#ifndef RP
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "%s m__d;\n", switchTypeName);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "union {\n");
    codebuffer->currentLine = 0;
    codebuffer->nrOfLines = idl_typeUnionNoCases(unionSpec);
    codebuffer->lines = (char **)os_malloc(codebuffer->nrOfLines*sizeof(char*));
    ut_stackPush(arg->structSpecStack, codebuffer);
#else
    codebuffer->currentLine = 0;
    codebuffer->nrOfLines = idl_typeUnionNoCases(unionSpec)+2;
    codebuffer->lines = (char **)os_malloc(codebuffer->nrOfLines*sizeof(char*));
    snprintf(arg->buffer, MAX_BUFFER, "%s m__d;\n", switchTypeName);
    codebuffer->lines[codebuffer->currentLine++] = os_strdup(arg->buffer);
    codebuffer->lines[codebuffer->currentLine++] = os_strdup("union {\n");
    ut_stackPush(arg->structSpecStack, codebuffer);
#endif
    os_free(switchTypeName);
    /* return idl_explore to indicate that the rest of the union needs to be processed */
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
idl_unionClose (
    const char *name,
    void *userData)
{
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;
    struct idl_genSACPPCodeBuffer *codebuffer;
    c_ulong line;

    codebuffer = (struct idl_genSACPPCodeBuffer *)ut_stackPop(arg->structSpecStack);
    assert(codebuffer);
    assert(codebuffer->nrOfLines == codebuffer->currentLine);
    arg->indent_level++;
    for (line=0;line<codebuffer->currentLine;line++) {
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(), codebuffer->lines[line]);
        os_free(codebuffer->lines[line]);
    }
    arg->indent_level--;
    os_free(codebuffer->lines);
    os_free(codebuffer);

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "};\n");
    arg->indent_level--;
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "};\n");

    /* Generate _var and _out types */
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "typedef DDS_DCPSStruct_var< %s > %s_var;\n", name, name);

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "typedef %s &%s_out;\n\n", name, name);
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
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;
    struct idl_genSACPPCodeBuffer *codebuffer;
    char *dims;
    char *seqName;

    /* \TODO: add getter/setter for unioncase */
    codebuffer = (struct idl_genSACPPCodeBuffer *)ut_stackPop(arg->structSpecStack);
    switch (idl_typeSpecType(typeSpec)) {
    case idl_tbasic:
        if (idl_typeBasicType(idl_typeBasic(typeSpec)) == idl_string) {
            snprintf(arg->buffer,MAX_BUFFER,"::DDS::String_mgr %s;\n", name);
        } else {
            snprintf(arg->buffer,MAX_BUFFER,"%s %s;\n",
            idl_corbaCxxTypeFromTypeSpec(typeSpec),
                name);
        }
    break;
    case idl_tenum:
    case idl_ttypedef:
    case idl_tstruct:
    case idl_tunion:
        snprintf(arg->buffer,MAX_BUFFER,"%s %s;\n",
        idl_corbaCxxTypeFromTypeSpec(typeSpec),
            name);
    break;
    case idl_tarray:
        /* generate code for an array mapping */
        snprintf(arg->buffer,MAX_BUFFER,"_%s_array", name);
        seqName = os_strdup(arg->buffer);
        dims = idl_arrayDimensions(idl_typeArray(typeSpec), arg);
        idl_arrayTypeDefs(idl_typeArray(typeSpec), seqName, dims, arg);
        snprintf(arg->buffer, MAX_BUFFER, "%s %s;", seqName, name);
        os_free(dims);
        os_free(seqName);
    break;
    case idl_tseq:
        snprintf(arg->buffer,MAX_BUFFER,"_%s_seq",name);
        seqName = os_strdup(arg->buffer);
        idl_seqTypeDefs(idl_typeSeq(typeSpec), seqName, arg);
        snprintf(arg->buffer, MAX_BUFFER, "%s %s;", seqName, name);
        os_free(seqName);
    break;
    default:
        printf("idl_unionCaseOpenClose: Unsupported union member type (member name = %s, type name = %s)\n",
            name, idl_scopedTypeName(typeSpec));
    }
    codebuffer->lines[codebuffer->currentLine++] = os_strdup(arg->buffer);
    ut_stackPush(arg->structSpecStack, codebuffer);
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
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;

    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "enum %s {\n", name);
    arg->indent_level++;
    arg->enum_element = idl_typeEnumNoElements(enumSpec);
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
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;

/*    idl_printIndent(arg->indent_level);
      idl_fileOutPrintf(idl_fileCur(), "EORB_FORCE_ENUM32(__%s)\n", name);*/
    arg->indent_level--;
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "};\n");
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
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;

    arg->enum_element--;
    idl_printIndent(arg->indent_level);
    if (arg->enum_element == 0) {
        idl_fileOutPrintf(idl_fileCur(), "%s\n", name);
    } else {
        idl_fileOutPrintf(idl_fileCur(), "%s,\n", name);
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
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;
    idl_typeSpec actualType;
    idl_typeSpec refType;
    char *dims;

    actualType = idl_typeDefActual(defSpec);
    refType = idl_typeDefRefered(defSpec);
    switch (idl_typeSpecType(actualType)) {
    case idl_tbasic:
        if (idl_typeBasicType(idl_typeBasic(actualType)) == idl_string) {
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"typedef ::DDS::String_mgr %s;\n", name);
            /* also generate code for _var and _out types */
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"typedef ::DDS::String_var %s_var;\n", name);
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"typedef ::DDS::String_out %s_out;\n", name);
        } else {
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"typedef %s %s;\n",
                idl_corbaCxxTypeFromTypeSpec(refType),
                name);
        }
    break;
    case idl_tenum:
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"typedef %s %s;\n",
        idl_corbaCxxTypeFromTypeSpec(refType),
            name);
    break;
    case idl_tstruct:
    case idl_tunion:
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"typedef %s %s;\n",
            idl_corbaCxxTypeFromTypeSpec(refType),
            name);
        /* generate _var and _out */
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"typedef %s_var %s_var;\n",
            idl_corbaCxxTypeFromTypeSpec(refType), name);
        idl_printIndent(arg->indent_level);
        idl_fileOutPrintf(idl_fileCur(),"typedef %s_out %s_out;\n",
            idl_corbaCxxTypeFromTypeSpec(refType), name);
    break;
    case idl_tseq:
        if (refType == actualType) {
            idl_seqTypeDefs(idl_typeSeq(refType), name, arg);
        } else {
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"typedef %s %s;\n",
                idl_corbaCxxTypeFromTypeSpec(refType),
                name);
            /* generate _var and _out */
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"typedef %s_var %s_var;\n",
                idl_corbaCxxTypeFromTypeSpec(refType), name);
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"typedef %s_out %s_out;\n",
                idl_corbaCxxTypeFromTypeSpec(refType), name);
        }
    break;
    case idl_tarray:
        if (refType == actualType) { /* ex. typedef long myArray[3][4]; */
            dims = idl_arrayDimensions(idl_typeArray(refType), arg);
            idl_arrayTypeDefs(idl_typeArray(refType), name, dims, arg);
            os_free(dims);
        } else {
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"typedef %s %s;\n",
                idl_corbaCxxTypeFromTypeSpec(refType), name);
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"typedef %s_slice %s_slice;\n",
               idl_corbaCxxTypeFromTypeSpec(refType), name);
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"typedef %s_var %s_var;\n",
                idl_corbaCxxTypeFromTypeSpec(refType), name);
            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"static %s_slice %s_alloc();\n", name, name);

            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"static void %s_free(%s_slice *);\n", name, name);

            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"static void %s_copy(%s_slice *to, const %s_slice *from);\n",
                name, name, name);

            idl_printIndent(arg->indent_level);
            idl_fileOutPrintf(idl_fileCur(),"static %s_slice %s_dup(const %s_slice *from);\n",
                name, name, name);
        }
    break;
    case idl_ttypedef: /* actual type will never be a typedef! */
    default:
        printf("idl_unionCaseOpenClose: Unsupported union member type (member name = %s, type name = %s)\n",
            name, idl_scopedTypeName(idl_typeSpec(defSpec)));
    }
}

static void
idl_constantOpenClose (
    idl_scope scope,
    idl_constSpec constantSpec,
    void *userData)
{
    struct idl_genSACPPData *arg = (struct idl_genSACPPData *)userData;
    idl_typeSpec t;

    t = idl_constSpecTypeGet(constantSpec);
    idl_printIndent(arg->indent_level);
    idl_fileOutPrintf(idl_fileCur(), "const %s %s = (%s)%s;\n",
        idl_corbaCxxTypeFromTypeSpec(t),
        idl_constSpecName(constantSpec),
        idl_corbaCxxTypeFromTypeSpec(t),
        idl_constSpecImage(constantSpec));
}

/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
*/
static idl_programControl idl_genSACPPLoadControl = {
    idl_prior
};

/** @brief return the program control structure for the splice type generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    return &idl_genSACPPLoadControl;
}

/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program
idl_genSacppType = {
    idl_getControl,
    idl_fileOpen,
    idl_fileClose,
    idl_moduleOpen,
    idl_moduleClose,
    idl_structureOpen,
    idl_structureClose,
    idl_structureMemberOpenClose,
    idl_enumerationOpen,
    idl_enumerationClose,
    idl_enumerationElementOpenClose,
    idl_unionOpen,
    idl_unionClose,
    idl_unionCaseOpenClose,
    NULL, /* idl_unionLabelsOpenClose */
    NULL, /* idl_unionLabelOpenClose */
    idl_typedefOpenClose,
    NULL, /* idl_boundedStringOpenClose */
    NULL, /* idl_sequenceOpenClose */
    idl_constantOpenClose,
    NULL, /* idl_artificialDefaultLabelOpenClose */
    &idl_genSACPPData  /* userData */
};

/** @brief return the callback table for the splice type generation functions.
 */
idl_program
idl_genSacppTypeProgram(
    void)
{
    return &idl_genSacppType;
}

/*
 * Implementation Considerations:
 *
 * - Mapping structures:
 * The members of a structure have a type, which can be arrays, sequences, strings
 * or unions. For these kind of types additional type definitions have to be generated.
 * These additional typedefs can be generated within the scope of the structure, but not
 * all compilers can handle this. Therefore we will generate these type-defs at the module
 * level of the structure. The structure definition it self will be built on heap and written
 * to the output file in the idl_structureClose() callback.
 *
 */
