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
   This module generates Helper functions for the C API
*/

#include "idl_program.h"
#include "idl_scope.h"
#include "idl_keyDef.h"
#include "idl_genCorbaCHelper.h"
#include "idl_genCHelper.h"
#include "idl_genLanguageHelper.h"
#include "idl_tmplExp.h"

#include "c_typebase.h"

/* idl_null is an empty function, used to bypass QAC errors */
static void
idl_null(
    void)
{
}

/* fileOpen callback

   return idl_explore to state that the rest of the file needs to be processed
*/
static idl_action
idl_fileOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    return idl_explore;
}

/* moduleOpen callback

   return idl_explore to state that the rest of the module needs to be processed
*/
static idl_action
idl_moduleOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    return idl_explore;
}

/* structureOpen callback

   A structure is a type that can be communicated via the DCPS API.
   A structure can have a key list defining all key fields.
   For the structure the following helper functions are defined:
   - a function to query the structures scoped type name
   - a function to query the structures key list
   The name of the functions are:
       __<scope-elements>_<struct-name>__name
   and
       __<scope-elements>_<struct-name>__keys
   respectively.

   return idl_abort to state that the rest of the structure does not need to be processed
*/
static idl_action
idl_structureOpen (
    idl_scope scope,
    const char *name,
    idl_typeStruct structSpec,
    void *userData)
{
    const c_char *key_list;

    idl_fileOutPrintf(idl_fileCur(), "const char *\n");
    idl_fileOutPrintf(
        idl_fileCur(),
        "__%s__name(void)\n",
        idl_scopeStack(scope, "_", name));
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    idl_fileOutPrintf(
        idl_fileCur(),
        "    return (const char*)\"%s\";\n",
        idl_scopeStack(scope, "::", name));

    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
    key_list = idl_keyResolve(idl_keyDefDefGet(), scope, name);
    idl_fileOutPrintf(idl_fileCur(), "const char *\n");
    idl_fileOutPrintf(
        idl_fileCur(),
        "__%s__keys(void)\n",
        idl_scopeStack (scope, "_", name));
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    if (key_list) {
        idl_fileOutPrintf(idl_fileCur(), "    return (const char*)\"%s\";\n", key_list);
    } else {
        idl_fileOutPrintf(idl_fileCur(), "    return (const char*)\"\";\n");
    }
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");

    return idl_abort;
}

/* unionOpen callback

   A union is a type that can be communicated via the DCPS API.
   A union can have a key list defining all key fields (practically
   only the switch).
   For the union the following helper functions are defined:
   - a function to query the union scoped type name
   - a function to query the union key list
   The name of the functions are:
       __<scope-elements>_<union-name>__name
   and
       __<scope-elements>_<union-name>__keys
   respectively.

   return idl_abort to state that the rest of the union does not need to be processed
*/
static idl_action
idl_unionOpen(
    idl_scope scope,
    const char *name,
    idl_typeUnion unionSpec,
    void *userData)
{
    const c_char *key_list;

    idl_fileOutPrintf(idl_fileCur(), "const char *\n");
    idl_fileOutPrintf(idl_fileCur(), "__%s__name(void)\n",
	idl_scopeStack(scope, "_", name));
    idl_fileOutPrintf(idl_fileCur(), "{\n");

    idl_fileOutPrintf(
        idl_fileCur(),
        "    return (const char*)\"%s\";\n",
        idl_scopeStack(scope, "::", name));

    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
    key_list = idl_keyResolve(idl_keyDefDefGet(), scope, name);
    idl_fileOutPrintf(idl_fileCur(), "const char *\n");
    idl_fileOutPrintf(
        idl_fileCur(),
        "__%s__keys(void)\n",
        idl_scopeStack(scope, "_", name));
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    if (key_list) {
        idl_fileOutPrintf(
            idl_fileCur(),
            "    return (const char *)\"%s\";\n",
            key_list);
    } else {
        idl_fileOutPrintf(idl_fileCur(), "    return (const char *)\"\";\n");
    }
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");

    return idl_abort;
}

/* typedefOpen callback

   A typedef of a structure or union is a type that can be
   communicated via the DCPS API.
   For the typedef the following helper functions are defined:
   - a function to query the typedef scoped type name
   - a function to query the typedef key list
   The name of the functions are:
       __<scope-elements>_<typedef-name>__name
   and
       __<scope-elements>_<typedef-name>__keys
   respectively.
*/
static void
idl_typedefOpenClose (
    idl_scope scope,
    const char *name,
    idl_typeDef defSpec,
    void *userData)
{
    switch (idl_typeSpecType(idl_typeDefActual(defSpec))) {
    case idl_tstruct:
        idl_structureOpen(scope, name, idl_typeStruct(idl_typeDefActual(defSpec)), userData);
	break;
    case idl_tunion:
        idl_unionOpen(scope, name, idl_typeUnion(idl_typeDefActual(defSpec)), userData);
	break;
    default:
        /* Empty statement to satisfy QAC */
        idl_null();
	break;
    }
}

/* idl_genCorbaCHelper specifies the local
   callback routines
*/
static struct idl_program
idl_genCorbaCHelper = {
    NULL,
    idl_fileOpen,
    NULL, /* idl_fileClose */
    idl_moduleOpen,
    NULL, /* idl_moduleClose */
    idl_structureOpen,
    NULL, /* idl_structureClose */
    NULL, /* idl_structureMemberOpenClose */
    NULL, /* idl_enumerationOpen */
    NULL, /* idl_enumerationClose */
    NULL, /* idl_enumerationElementOpenClose */
    idl_unionOpen,
    NULL, /* idl_unionClose */
    NULL, /* idl_unionCaseOpenClose */
    NULL, /* idl_unionLabelsOpenClose */
    NULL, /* idl_unionLabelOpenClose */
    idl_typedefOpenClose,
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
idl_genCorbaCHelperProgram(
    void)
{
    return &idl_genCorbaCHelper;
}
