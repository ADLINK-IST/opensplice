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
   This module generates include file for the C++ API
*/

#include "idl_program.h"
#include "idl_scope.h"
#include "idl_genCorbaCxxCcpp.h"
#include "idl_genISOCxxHeader.h"
#include "idl_genCxxHelper.h"
#include "idl_tmplExp.h"
/* TBD */
#include "idl_keyDef.h"
#include "idl_typeSpecifier.h"

#include "os_heap.h"
#include "os_stdlib.h"
#include <ctype.h>

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
    os_char* tmpName;
    size_t i;
    (void) scope; /* Unused */
    (void) userData; /* Unused */
    idl_fileOutPrintf(idl_fileCur(), "#ifndef OPENSPLICE_ISOCXX_PSM\n");
    idl_fileOutPrintf(idl_fileCur(), "#define OPENSPLICE_ISOCXX_PSM\n");
    idl_fileOutPrintf(idl_fileCur(), "#endif\n");
    /* below always returns idl_abort regardless of success or failure */
    (void) idl_genCorbaCxxCcppProgram()->fileOpen(scope, name, userData);
    tmpName = os_strdup(name);
    for(i = 0; i < strlen(tmpName); i++)
    {
        tmpName[i] = toupper (tmpName[i]);
    }
    idl_fileOutPrintf(idl_fileCur(), "\n#ifdef %s_DCPS_TYPESUPPORT_DEFINED\n", tmpName);
    idl_fileOutPrintf(idl_fileCur(), "#ifndef %s_DCPS_HPP_\n", tmpName);
    idl_fileOutPrintf(idl_fileCur(), "#define %s_DCPS_HPP_\n", tmpName);
    os_free(tmpName);
    idl_fileOutPrintf(idl_fileCur(), "#include \"dds/dds.hpp\"\n\n");
    return idl_explore;
}

/**
 * Just close out the file with an endif.
 */
static void
idl_fileClose(
    void *userData)
{
    (void) userData;
    idl_fileOutPrintf(idl_fileCur(), "\n#endif\n");
    idl_fileOutPrintf(idl_fileCur(), "#endif\n");
}

/**
 * Not yet required
 * @return idl_explore - Apparently this means process the rest of the file.
 */
static idl_action
idl_structureOpen(
    idl_scope scope,
    const char *name,
    idl_typeStruct structSpec,
    void *userData)
{
    char* sampleType;
    (void) scope;
    (void) userData;
    if (idl_keyResolve(idl_keyDefDefGet(), idl_typeUserScope(idl_typeUser(structSpec)), name) != NULL)
    {
        sampleType = idl_corbaCxxTypeFromTypeSpec(idl_typeSpec(structSpec));
        idl_fileOutPrintf(idl_fileCur(),
                            "REGISTER_TOPIC_TRAITS(%s)\n",
                                sampleType);
        os_free(sampleType);
    }
    return idl_explore;
}

static struct idl_program
idl_genMyActions = {
    NULL,
    idl_fileOpen,
    idl_fileClose,
    NULL, /* idl_moduleOpen */
    NULL, /* idl_moduleClose */
    idl_structureOpen,
    NULL, /* idl_structureClose */
    NULL, /* idl_structureMemberOpenClose */
    NULL, /* idl_enumerationOpen */
    NULL, /* idl_enumerationClose */
    NULL, /* idl_enumerationElementOpenClose */
    NULL, /* idl_unionOpen */
    NULL, /* idl_unionClose */
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

/**
 * Actions to generate a Foo_DCPS.hpp for the ISO C++ PSM from idl Foo.idl
 */
idl_program
idl_genISOCxxHeaderProgram (
    void)
{
    return &idl_genMyActions;
}
