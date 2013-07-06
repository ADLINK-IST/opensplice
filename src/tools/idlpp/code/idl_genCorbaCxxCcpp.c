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
#include "idl_keyDef.h"
#include "idl_genCorbaCxxCcpp.h"
#include "idl_genCxxHelper.h"
#include "idl_tmplExp.h"

#include "os_heap.h"
#include "os_stdlib.h"
#include "c_typebase.h"
#include <ctype.h>
#include <fcntl.h>

static idl_macroAttrib idlpp_macroAttrib;
static idl_streamIn idlpp_inStream;
static c_char *idlpp_template;
static idl_macroSet idlpp_macroSet;
static os_char* clientheader = NULL;

/* QAC EXPECT 0285; Need dollar here, this is specified */
#define IDL_TOKEN_START         '$'
#define IDL_TOKEN_OPEN          '('
#define IDL_TOKEN_CLOSE         ')'

/* idl_null is an empty function, used to bypass QAC errors */
static void
idl_null(void)
{
}

void
idl_genCorbaCxxCcpp_setClientHeader(
    os_char* newClientHeader)
{
    assert(!clientheader);
    if(newClientHeader)
    {
        clientheader = os_strdup(newClientHeader);
    }
}

/* fileOpen callback

   return idl_explore to state that the rest of the file needs to be processed
*/
static idl_action
idl_fileOpen (
    idl_scope scope,
    const char *name,
    void *userData)
{
    idl_tmplExp te;
    c_char tmplFileName [1024];
    c_char *tmplPath;
    c_char *orbPath;
    int tmplFile;
    struct os_stat tmplStat;
    unsigned int nRead;
    os_char* tmpName;
    os_uint32 i;

    tmplPath = os_getenv ("OSPL_TMPL_PATH");
    orbPath = os_getenv ("OSPL_ORB_PATH");
    if (tmplPath == NULL) {
        printf ("OSPL_TMPL_PATH not defined\n");
        return (idl_abort);
    }
    if (orbPath == NULL) {
        printf ("OSPL_ORB_PATH not defined\n");
        return (idl_abort);
    }

    /* Prepare file header template */
    snprintf(tmplFileName, (size_t)sizeof(tmplFileName), "%s%c%s%ccorbaCxxMainInclude", tmplPath, OS_FILESEPCHAR, orbPath, OS_FILESEPCHAR);
    /* QAC EXPECT 3416; No unexpected side effects here */
    if ((os_stat(tmplFileName, &tmplStat) != os_resultSuccess) ||
        (os_access(tmplFileName, OS_ROK) != os_resultSuccess)) {
        printf ("No template found or protection violation (%s)\n", tmplFileName);
        return (idl_abort);
    }
    /* QAC EXPECT 5007; will not use wrapper */
    idlpp_template = os_malloc((size_t)((int)tmplStat.stat_size+1));
    tmplFile = open(tmplFileName, O_RDONLY);
    nRead = (unsigned int)read(tmplFile, idlpp_template, (size_t)tmplStat.stat_size);
    memset(&idlpp_template[nRead], 0, (size_t)((int)tmplStat.stat_size+1-nRead));
    close(tmplFile);
    idlpp_macroAttrib = idl_macroAttribNew(IDL_TOKEN_START, IDL_TOKEN_OPEN, IDL_TOKEN_CLOSE);
    idlpp_macroSet = idl_macroSetNew();
    idlpp_inStream = idl_streamInNew(idlpp_template, idlpp_macroAttrib);
    idl_macroSetAdd(idlpp_macroSet, idl_macroNew("basename", name));
    if(clientheader != NULL)
    {
        tmpName = os_strdup(name);
        for(i = 0; i < strlen(tmpName); i++)
        {
            tmpName[i] = toupper (tmpName[i]);
        }
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("basename_upper", tmpName));
        os_free(tmpName);
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("clientheaderdefine", "#define CCPP_USE_CUSTOM_SUFFIX_"));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("clientheaderundef", "#undef CCPP_USE_CUSTOM_SUFFIX_"));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("clientheader", clientheader));
    } else
    {
        /* dds2071: Define some default macro values.. for the client header .h
         * was chosen as an arbitrary default
         */
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("basename_upper", ""));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("clientheaderdefine", ""));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("clientheaderundef", ""));
        idl_macroSetAdd(idlpp_macroSet, idl_macroNew("clientheader", ".h"));
    }
    te = idl_tmplExpNew(idlpp_macroSet);
    idl_tmplExpProcessTmpl(te, idlpp_inStream, idl_fileCur());
    idl_streamInFree(idlpp_inStream);
    idl_tmplExpFree(te);

    return idl_abort;
}

/* idl_genCorbaCxxCcpp specifies the local
   callback routines
*/
static struct idl_program
idl_genCorbaCxxCcpp = {
    NULL,
    idl_fileOpen,
    NULL, /* idl_fileClose */
    NULL, /* idl_moduleOpen */
    NULL, /* idl_moduleClose */
    NULL, /* idl_structureOpen */
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

/* genCorbaCxxHelperProgram returns the local
   table of callback routines.
*/
idl_program
idl_genCorbaCxxCcppProgram (
    void)
{
    return &idl_genCorbaCxxCcpp;
}
