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
   This module generates include file for the C++ API
*/

#include "idl_program.h"
#include "idl_scope.h"
#include "idl_genCorbaCxxCcpp.h"
#include "idl_genISOCxxHeader.h"
#include "idl_genCxxHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_tmplExp.h"
/* TBD */
#include "idl_keyDef.h"
#include "idl_typeSpecifier.h"
#include "idl_genLanguageHelper.h"

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
    os_char* origName;
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
        tmpName[i] = (os_char) toupper (tmpName[i]);
    }
    idl_fileOutPrintf(idl_fileCur(), "\n#ifdef %s_DCPS_TYPESUPPORT_DEFINED\n", tmpName);
    idl_fileOutPrintf(idl_fileCur(), "#ifndef %s_DCPS_HPP_\n", tmpName);
    idl_fileOutPrintf(idl_fileCur(), "#define %s_DCPS_HPP_\n", tmpName);
    os_free(tmpName);
    idl_fileOutPrintf(idl_fileCur(), "#include \"dds/dds.hpp\"\n\n");
    /* If the filename ends in Streams include isocpp streams for REGISTER_STREAMS_TOPIC_TRAITS */
    if(idl_getIsISOCppStreams())
    {
        origName = os_strdup(name);
        origName[strlen(origName)-7] = '\0';
        idl_fileOutPrintf(idl_fileCur(), "#include \"%s_DCPS.hpp\"\n", origName);
        idl_fileOutPrintf(idl_fileCur(), "#include \"dds/streams/streams.hpp\"\n\n");
        os_free(origName);
    }
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
        /* Remove "StreamSample" for REGISTER_STREAM_TOPIC_TRAITS */
        if(strlen(name) > 12 && strcmp(name+strlen(name)-12, "StreamSample")==0)
        {
            sampleType[strlen(sampleType)-12] = '\0';
            idl_fileOutPrintf(idl_fileCur(),
                        "REGISTER_STREAM_TOPIC_TRAITS(%s)\n",
                            sampleType);

        }
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
