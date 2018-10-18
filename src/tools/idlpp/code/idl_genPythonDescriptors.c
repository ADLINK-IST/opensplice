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

#include "idl_genPythonDescriptors.h"

#include "idl_genMetaHelper.h"
#include "idl_keyDef.h"

#include <os_heap.h>

/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
*/
static idl_programControl idl_genPythonDescriptorsLoadControl = {
    idl_prior
};

/** @brief return the program control structure for the splice type generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    OS_UNUSED_ARG(userData);
    return &idl_genPythonDescriptorsLoadControl;
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
	OS_UNUSED_ARG(scope);
	OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    printf("<topics>\n");

    /* return idl_explore to indicate that the rest of the file needs to be processed */
    return idl_explore;
}

static void
idl_fileClose(
    void *userData)
{
    OS_UNUSED_ARG(userData);

    printf("</topics>\n");
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

/* Matlab class template:
	classdef struct_name < Vortex.AbstractType
	  properties
		FIELD1 FIELD1Type = initDimensions
		FIELD2 (1)
	  end

	  methods (Static)
	     function keys = getKey
	        keys = 'STRUCT_KEY_STRING';
	     end
	  end
	end
*/

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
	c_char *id;
	const c_char *keys;
	c_char *descriptor;

	OS_UNUSED_ARG(userData);

	keys = idl_keyResolve(idl_keyDefDefGet(), scope, name);
    if (keys != NULL) {
        c_type type = idl_typeSpecDef(idl_typeSpec(structSpec));
        descriptor = idl_genXMLmeta(type, FALSE);
        id = idl_scopeStack (scope, "::", name);

        printf( "  <topictype>\n"
        		"    <id>%s</id>\n"
        		"    <keys>%s</keys>\n"
        		"    <descriptor><![CDATA[%s]]></descriptor>\n"
        		"  </topictype>\n",
				id, keys, descriptor);

        os_free(id);
        os_free(descriptor);
    }

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
/*static void
idl_structureClose(
    const char *name,
    void *userData)
{
	OS_UNUSED_ARG(name);
	OS_UNUSED_ARG(userData);
}*/

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
/*static void
idl_structureMemberOpenClose(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec,
    void *userData)
{
	OS_UNUSED_ARG(scope);
	OS_UNUSED_ARG(name);
	OS_UNUSED_ARG(typeSpec);
	OS_UNUSED_ARG(userData);
}*/

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
/*static idl_action
idl_enumerationOpen(
    idl_scope scope,
    const char *name,
    idl_typeEnum enumSpec,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(enumSpec);
    OS_UNUSED_ARG(userData);

    return idl_explore;
}*/

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
/*static void
idl_enumerationClose(
    const char *name,
    void *userData)
{
	OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);
}*/

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
/*static void
idl_enumerationElementOpenClose (
    idl_scope scope,
    const char *name,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);
}*/

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
/*static void
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
}*/

/*static void
idl_sequenceOpenClose (
    idl_scope scope,
    idl_typeSeq typeSeq,
    void *userData)
{
    OS_UNUSED_ARG(scope);
    OS_UNUSED_ARG(typeSeq);
    OS_UNUSED_ARG(userData);
}*/


/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program
idl_genPythonDescriptorsType = {
    idl_getControl,
    idl_fileOpen,
    idl_fileClose,
    idl_moduleOpen,
    NULL, /* idl_moduleClose */
    idl_structureOpen,
    NULL, /* idl_structureClose */
    NULL, /* idl_structureMemberOpenClose */
    NULL, /* idl_enumerationOpen */
    NULL, /* idl_enumerationClose */
    NULL, /* idl_enumerationElementOpenClose */
    NULL, /*idl_unionOpen */
    NULL, /* idl_unionClose */
    NULL, /* idl_unionCaseOpenClose */
    NULL, /* idl_unionLabelsOpenClose */
    NULL, /* idl_unionLabelOpenClose */
	NULL, /* idl_typedefOpenClose */
    NULL, /* idl_boundedStringOpenClose */
    NULL, /* idl_sequenceOpenClose */
    NULL, /* idl_constantOpenClose */
    NULL, /* idl_artificialDefaultLabelOpenClose */
    NULL /* userData */
};

/** @brief return the callback table for the splice type generation functions.
 */
idl_program
idl_genPythonDescriptorsProgram (
		void)
{
	return &idl_genPythonDescriptorsType;
}

