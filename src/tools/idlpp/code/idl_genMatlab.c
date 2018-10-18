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

#include "idl_genMatlab.h"
#include "idl_genMatlabHelper.h"

#include "idl_genMetaHelper.h"
#include "idl_keyDef.h"
#include "idl_tmplExp.h"

#include <os_stdlib.h>
#include <os_heap.h>

/* enumeration data */
static c_long enum_element = 0;

/* struct data */
static idl_typeStruct active_structSpec;
static idl_scope active_structScope;
static idl_streamOut structPublicFunctionsStream;
static idl_streamOut structConstructorStream;
static idl_streamOut structAsStructStream;
static idl_streamOut structOffsetsStream;

/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
*/
static idl_programControl idl_genMatlabLoadControl = {
    idl_prior
};

/** @brief return the program control structure for the splice type generation functions.
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    OS_UNUSED_ARG(userData);
    return &idl_genMatlabLoadControl;
}

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
	c_char *mlName;
	const c_char *keys;

	OS_UNUSED_ARG(name);
	OS_UNUSED_ARG(structSpec);
	OS_UNUSED_ARG(userData);

	keys = idl_keyResolve(idl_keyDefDefGet(), scope, name);

    /* Open file for used scope, if needed create the directories */
    mlName = idl_matlabId(name);
    idl_openMatlabPackage(scope, mlName);
    if (idl_fileCur() == NULL) {
        os_free(mlName);
        return idl_abort;
    }

    if(keys) {
    	idl_fileOutPrintf(idl_fileCur(), "classdef %s < Vortex.AbstractType\n", mlName);
    } else {
    	idl_fileOutPrintf(idl_fileCur(), "classdef %s\n", mlName);
    }
    idl_fileOutPrintf(idl_fileCur(), "  properties\n");
    os_free(mlName);

    /* save info for the close method */
    active_structSpec = structSpec;
    active_structScope = scope;

    /* set up stream for struct offsets */
    structOffsetsStream = idl_streamOutNew (0);
    structPublicFunctionsStream = idl_streamOutNew (0);
    structConstructorStream = idl_streamOutNew(0);
    structAsStructStream = idl_streamOutNew(0);

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
	const c_char *keys = "";
	c_char *descriptor;
	char *id = idl_matlabId(name);
	OS_UNUSED_ARG(name);
	OS_UNUSED_ARG(userData);

	keys = idl_keyResolve(idl_keyDefDefGet(), active_structScope, name);
    if (keys != NULL) {
        c_type type = idl_typeSpecDef(idl_typeSpec(active_structSpec));
        descriptor = idl_genXMLmeta(type, FALSE);
    } else {
        descriptor = os_strdup("");
    }


    idl_fileOutPrintf(idl_fileCur(), "  end\n"); /*properties*/
    idl_fileOutPrintf(idl_fileCur(), "\n");
    idl_fileOutPrintf(idl_fileCur(), "  methods\n");
    idl_fileOutPrintf(idl_fileCur(), "    function obj = %s(opt_struct)\n", id);
    idl_fileOutPrintf(idl_fileCur(), "      if nargin == 1\n");
    idl_fileOutPrintf(idl_fileCur(), "%s", idl_streamGet (idl_stream(structConstructorStream)));
    idl_fileOutPrintf(idl_fileCur(), "      end\n");
    idl_fileOutPrintf(idl_fileCur(), "    end\n\n");
    idl_fileOutPrintf(idl_fileCur(), "    function struct_copy = asStruct(obj)\n");
    idl_fileOutPrintf(idl_fileCur(), "%s", idl_streamGet (idl_stream(structAsStructStream)));
    idl_fileOutPrintf(idl_fileCur(), "    end\n\n");
    idl_fileOutPrintf(idl_fileCur(), "%s", idl_streamGet (idl_stream(structPublicFunctionsStream)));
    idl_fileOutPrintf(idl_fileCur(), "  end\n"); /*public methods*/
    idl_fileOutPrintf(idl_fileCur(), "\n");

    idl_fileOutPrintf(idl_fileCur(), "  methods (Static)\n");
    if(keys != NULL) {
            idl_fileOutPrintf(idl_fileCur(), "    function keys = getKey\n");
            idl_fileOutPrintf(idl_fileCur(), "      keys = '%s';\n", keys);
            idl_fileOutPrintf(idl_fileCur(), "    end\n");
            idl_fileOutPrintf(idl_fileCur(), "\n");
            idl_fileOutPrintf(idl_fileCur(), "    function xml = getTopicDescriptor\n");
            idl_fileOutPrintf(idl_fileCur(), "      xml = '%s';\n", descriptor);
            idl_fileOutPrintf(idl_fileCur(), "    end\n");
            idl_fileOutPrintf(idl_fileCur(), "\n");
    }
    /* TODO: code proper getTypeInfo */
    idl_fileOutPrintf(idl_fileCur(), "    function info = getTypeInfo(name)\n");
    idl_fileOutPrintf(idl_fileCur(), "      import Vortex.internal.ml_type_info_kind;\n");
    idl_fileOutPrintf(idl_fileCur(), "      switch(name)\n");
    idl_fileOutPrintf(idl_fileCur(), "%s", idl_streamGet (idl_stream(structOffsetsStream)));
    idl_fileOutPrintf(idl_fileCur(), "      end\n");
    idl_fileOutPrintf(idl_fileCur(), "    end\n");
    idl_fileOutPrintf(idl_fileCur(), "  end\n"); /* static functions */
    idl_fileOutPrintf(idl_fileCur(), "end\n"); /*classdef*/

    idl_streamOutFree(structOffsetsStream);
    idl_streamOutFree(structPublicFunctionsStream);
    idl_streamOutFree(structConstructorStream);
    idl_streamOutFree(structAsStructStream);
    structOffsetsStream = NULL;
    structPublicFunctionsStream = NULL;
    structConstructorStream = NULL;
    structAsStructStream = NULL;
    active_structSpec = NULL;
    active_structScope = NULL;

    os_free(id);
    os_free(descriptor);
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
	c_char *id;
	c_char *mlTypeName = idl_matlabTypeName(typeSpec);
	int shouldBeCell = (idl_matlabShouldBeCellArray(typeSpec) == 1);
	c_char *type = shouldBeCell ? "cell" : mlTypeName;
	c_char *initialization = idl_matlabFieldInitialization(typeSpec);
	c_char *dims_string = idl_matlabFormattedDimensions(typeSpec);

	OS_UNUSED_ARG(scope);
	OS_UNUSED_ARG(userData);

	id = idl_matlabId(name);

	idl_fileOutPrintf(idl_fileCur(), "    %s %s = %s\n", id,
			shouldBeCell ? "cell" : type,
					initialization);

	/* do setter functions */
	idl_streamOutPrintf(structPublicFunctionsStream,
			"    function obj = set.%s(obj,value)\n", id);
	if (idl_matlabIsArrayOfSequences(typeSpec)) {
			idl_streamOutPrintf(structPublicFunctionsStream,
						"      Vortex.check_seq_array(value,2);\n");
			idl_streamOutPrintf(structPublicFunctionsStream,
						"      for i = 1:numel(value)\n");
			/* TODO: check sequence */
			idl_streamOutPrintf(structPublicFunctionsStream,
						"        %%TODO: check sequence\n");
			idl_streamOutPrintf(structPublicFunctionsStream,
						"      end\n");
	} else if(idl_matlabIsSequence(typeSpec)) {
		/* TODO: check sequence */
		idl_streamOutPrintf(structPublicFunctionsStream,
					"      %%TODO: check sequence\n");
	} else if (idl_matlabIsString(typeSpec)) {
		unsigned long bound = idl_matlabStringBound(typeSpec);
		idl_streamOutPrintf(structPublicFunctionsStream,
					"      Vortex.check_string(value, %s, %lu);\n", dims_string, bound);
	} else {
		idl_streamOutPrintf(structPublicFunctionsStream,
					"      Vortex.check_dims(value, %s);\n", dims_string);
	}
	idl_streamOutPrintf(structPublicFunctionsStream,
			"      obj.%s = value;\n", id);
	idl_streamOutPrintf(structPublicFunctionsStream,
			"    end\n\n");

	/* Do constructor, and asStruct copying */
	if(idl_matlabIsStruct(typeSpec)) {
		if(idl_matlabIsNotMultiValued(typeSpec)) {
			/* Constructor */
			idl_streamOutPrintf(structConstructorStream, "        obj.%s = %s(opt_struct.%s);\n", id, mlTypeName, id);
			/* asStruct method */
			idl_streamOutPrintf(structAsStructStream, "      struct_copy.%s = obj.%s.asStruct;\n", id, id);
		} else if(idl_matlabIsArrayOfSequences(typeSpec)) {
			printf("ERROR: Unsupported - arrays of sequences of structures.\n");
			exit(1);
		} else if(idl_matlabIsSequence(typeSpec)) {
			/* Constructor */
			idl_streamOutPrintf(structConstructorStream, "        %s_local = %s.empty();\n", id, mlTypeName);
			idl_streamOutPrintf(structConstructorStream, "        for i = numel(opt_struct.%s):-1:1\n", id);
			idl_streamOutPrintf(structConstructorStream, "          %s_local(i) = %s(opt_struct.%s(i));\n", id, mlTypeName, id);
			idl_streamOutPrintf(structConstructorStream, "        end\n");
			idl_streamOutPrintf(structConstructorStream, "        obj.%s = %s_local;\n", id, id);
			/* asStruct method */
			idl_streamOutPrintf(structAsStructStream, "      %s_local = Vortex.init_field('%s',0);\n", id, mlTypeName);
			idl_streamOutPrintf(structAsStructStream, "      for i = numel(obj.%s):-1:1\n", id);
			idl_streamOutPrintf(structAsStructStream, "        %s_local(i) = obj.%s(i).asStruct;\n", id, id);
			idl_streamOutPrintf(structAsStructStream, "      end\n");
			idl_streamOutPrintf(structAsStructStream, "      struct_copy.%s = %s_local;\n", id, id);
		} else if(idl_matlabIsArray(typeSpec)) {
			/* Constructor */
			idl_streamOutPrintf(structConstructorStream, "        %s_local = %s;\n", id, initialization);
			idl_streamOutPrintf(structConstructorStream, "        for i = 1:numel(opt_struct.%s)\n", id);
			idl_streamOutPrintf(structConstructorStream, "          %s_local(i) = %s(opt_struct.%s(i));\n", id, mlTypeName, id);
			idl_streamOutPrintf(structConstructorStream, "        end\n");
			idl_streamOutPrintf(structConstructorStream, "        obj.%s = %s_local\n", id, id);
			/* asStruct method */
			idl_streamOutPrintf(structAsStructStream, "      struct_copy.%s = obj.%s.asStruct;\n", id, id);
			idl_streamOutPrintf(structAsStructStream, "      for i = numel(obj.%s):-1:1", id);
			idl_streamOutPrintf(structAsStructStream, "        a_struct_local(i) = obj.a_struct(i).asStruct();\n");
			idl_streamOutPrintf(structAsStructStream, "      end\n");
			idl_streamOutPrintf(structAsStructStream, "      struct_copy.%s = reshape(%s_local,size(obj.%s));\n", id, id, id);
		} else {
			/* Constructor */
			idl_streamOutPrintf(structConstructorStream, "        %%TODO: copy %s\n", id);
			/* asStruct method */
			idl_streamOutPrintf(structAsStructStream, "        %%TODO: copy %s\n", id);
		}
	} else if(idl_matlabIsEnum(typeSpec)) {
		/* Constructor */
		idl_streamOutPrintf(structConstructorStream, "       obj.%s = opt_struct.%s;\n", id, id);
		/* asStruct method */
		idl_streamOutPrintf(structAsStructStream, "     struct_copy.%s = int32(obj.%s);\n", id, id);
	} else {
		/* Constructor */
		idl_streamOutPrintf(structConstructorStream, "       obj.%s = opt_struct.%s;\n", id, id);
		/* asStruct method */
		idl_streamOutPrintf(structAsStructStream, "     struct_copy.%s = obj.%s;\n", id, id);
	}

	idl_streamOutTypeInfoEntries(structOffsetsStream, name, typeSpec);

	os_free((void*)mlTypeName);
	os_free((void*)initialization);
	os_free((void*)dims_string);
	os_free(id);
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
	c_char *mlName;
    OS_UNUSED_ARG(userData);
    OS_UNUSED_ARG(enumSpec);

    /* Open file for used scope, if needed create the directories */
    mlName = idl_matlabId(name);
    idl_openMatlabPackage(scope, mlName);
    if (idl_fileCur() == NULL) {
        os_free(mlName);
        return idl_abort;
    }
	/* Matlab Enum class template:
		classdef EnumName < int32
		  enumeration
			FIELD1 (0) % use ordinal value from IDLPP
			FIELD2 (1)
		  end
		end
	*/

	idl_fileOutPrintf(idl_fileCur(), "classdef %s < int32\n", mlName);
	idl_fileOutPrintf(idl_fileCur(), "  enumeration\n");
	os_free(mlName);
	enum_element = 0;

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
idl_enumerationClose(
    const char *name,
    void *userData)
{
	OS_UNUSED_ARG(name);
    OS_UNUSED_ARG(userData);

    idl_fileOutPrintf(idl_fileCur(), "  end\n");
    idl_fileOutPrintf(idl_fileCur(), "end\n");
    /* close file */
    idl_closeMatlabPackage();

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

    idl_fileOutPrintf(idl_fileCur(),"    %s (%d)\n", name, enum_element);

    enum_element++;
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
        /* TODO: Implement */
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
        /* TODO: Implement */
        OS_UNUSED_ARG(scope);
        OS_UNUSED_ARG(typeSeq);
        OS_UNUSED_ARG(userData);
}


/**
 * Specifies the callback table for the splice type generation functions.
 */
static struct idl_program
idl_genMatlabType = {
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
    NULL, /*idl_unionOpen */
    NULL, /* idl_unionClose */
    NULL, /* idl_unionCaseOpenClose */
    NULL, /* idl_unionLabelsOpenClose */
    NULL, /* idl_unionLabelOpenClose */
	idl_typedefOpenClose, /* idl_typedefOpenClose */
    NULL, /* idl_boundedStringOpenClose */
    idl_sequenceOpenClose,
    NULL, /* idl_constantOpenClose */
    NULL, /* idl_artificialDefaultLabelOpenClose */
    NULL /* userData */
};

/** @brief return the callback table for the splice type generation functions.
 */
idl_program
idl_genMatlabProgram (
		void)
{
	return &idl_genMatlabType;
}

