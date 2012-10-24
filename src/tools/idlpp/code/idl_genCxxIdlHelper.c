/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "idl_program.h"
#include "idl_scope.h"
#include "idl_genCxxIdlHelper.h"
#include "idl_keyDef.h"

/* The functions in this file create a key list that apply only to the
 * file being handled.  This is in contrast to the key list that is created
 * when the meta model is created within the idl_base.l/y code (when the
 * preprocessed idl is handled, which will also include the keylists of idl
 * files in the include chain).
 *
 * We need this list of keys that apply only to the idl file, so we can determine
 * whether a module contains a keylist (indicating that it should be generated
 * in the Dcps.idl file).  If a module does not contain a keylist it should
 * not appear in the Dcps.idl file : see idl_genCxxIdl.c for this generation.
 */

static idl_action
idl_fileOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    idl_idlScopeKeyList = NULL;

    return idl_explore;
}

static idl_action
idl_moduleOpen (
    idl_scope scope,
    const char *name,
    void *userData)
{
    return idl_explore;
}

static idl_action
idl_structureOpen(
    idl_scope scope,
    const char *name,
    idl_typeStruct structSpec,
    void *userData)
{
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        /* keylist defined for this struct */
       
        /* Add this key to the list for this idl file */
        idl_idlScopeKeyList = os_iterAppend(idl_idlScopeKeyList, idl_scopeDup(scope));
    }

    return idl_abort;
}

static idl_action
idl_unionOpen (
    idl_scope scope,
    const char *name,
    idl_typeUnion unionSpec,
    void *userData)
{
    if (idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        /* keylist defined for this union */

        /* Add this key to the list for this idl file */
        idl_idlScopeKeyList = os_iterAppend(idl_idlScopeKeyList, idl_scopeDup(scope));
    }

    return idl_abort;
}

static void
idl_typedefOpenClose(
    idl_scope scope,
    const char *name,
    idl_typeDef defSpec,
    void *userData)
{
    if ((idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tstruct ||
        idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tunion) &&
        idl_keyResolve(idl_keyDefDefGet(), scope, name) != NULL) {
        /* keylist defined for this typedef of struct or union */

        /* Add this key to the list for this idl file */
        idl_idlScopeKeyList = os_iterAppend(idl_idlScopeKeyList, idl_scopeDup(scope));
    }
}

static struct idl_program
idl_genCxxIdlHelper = {
    NULL,
    idl_fileOpen,
    NULL, /* idl_fileClose */
    idl_moduleOpen,
    NULL, /*idl_moduleClose */
    idl_structureOpen,
    NULL, /* idl_structureClose */
    NULL, /* idl_structureMemberOpenClose */
    NULL, /* idl_enumerationOpen */
    NULL, /* idl_enumerationClose */
    NULL, /* idl_enumerationElementOpenClose */
    idl_unionOpen,
    NULL, /* idl_unionOpen */
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

idl_program
idl_genCxxIdlHelperProgram(
    void)
{
    return &idl_genCxxIdlHelper;
}
