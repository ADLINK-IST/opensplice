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

#include "idl_genSimulinkHelper.h"
#include "idl_keyDef.h"
#include "idl_typeSpecifier.h"
#include "os_stdlib.h"
#include "os_string.h"
#include "os_heap.h"

c_char
*simulinkScope(
        idl_scope scope)
{
    char *scopeStack = os_strdup ("");
    if (idl_scopeStackSize(scope) != 0) {
        char *sPrefix = "@Scope(";
        int si;
        scopeStack = os_realloc(scopeStack, strlen(sPrefix) + 1);
        os_strcat(scopeStack, sPrefix);
        for (si = 0; si < idl_scopeStackSize(scope); si++) {
            size_t slen;
            char *seName = idl_scopeElementName(idl_scopeIndexed(scope, si));

            /* 2 for scope separator, 1 for trailing parentheses */
            slen = strlen (scopeStack) + 2 + strlen (seName) + 1;
            scopeStack = os_realloc(scopeStack, slen + 1);
            if (strlen(scopeStack) != strlen(sPrefix)) {
                os_strcat(scopeStack, "::");
            }
            os_strcat (scopeStack, seName);
        }
        os_strcat (scopeStack, ")");
    }
    return scopeStack;
}

static c_char
*simulinkQualifiedElement(
        idl_scope scope,
        const char *name)
{
    size_t slen = NULL;
    idl_scopeType currentScopeType = NULL;
    char *scopeStack = os_strdup ("");
    if (idl_scopeStackSize(scope) != 0) {
        int si;
        for (si = 0; si < idl_scopeStackSize(scope); si++) {
            char *seName = idl_scopeElementName(idl_scopeIndexed(scope, si));

            /* 2 for scope separator, 1 for null byte */
            slen = strlen (scopeStack) + 2 + strlen (seName) +1;
            scopeStack = os_realloc(scopeStack, slen);
            if (strlen(scopeStack) != 0) {
                os_strcat(scopeStack, "::");
            }
            os_strcat (scopeStack, seName);
        }
        if(strlen(name) != 0) {
            /* 2 for scope separator, 1 for null byte */
            slen = strlen (scopeStack) + 2 + strlen (name) + 1;
            scopeStack = os_realloc(scopeStack, slen);
            if (strlen(scopeStack) != 0) {
                currentScopeType = idl_scopeElementType(idl_scopeCur(scope));
                if(currentScopeType == idl_tStruct) {
                    os_strcat(scopeStack, ".");
                } else {
                    os_strcat(scopeStack, "::");
                }
            }
            os_strcat (scopeStack, name);
        }
    } else {
    	os_free(scopeStack);
    	scopeStack = os_strdup(name);
    }
    return scopeStack;
}

static os_equality
table_cmpFunc(
        void *o1,
        void *o2,
        void *args)
{
    os_equality result;
    os_int cmp = strcmp((char*)o1,(char*)o2);

    OS_UNUSED_ARG(args);

    if (cmp < 0) {
        result = OS_LT;
    } else if (cmp > 0) {
        result = OS_GT;
    } else {
        result = OS_EQ;
    }
    return result;
}

static c_char *
simulink_getTableValue(
        ut_table table,
        const char *key)
{
    char *value = NULL;
    if(table != NULL)
    {
        value = (char *) ut_get (ut_collection(table), (void *) key);
        if(value != NULL) {
            return value;
        }
    }
    return NULL;
}

static void
freeTableKeys(
        void *e,
        void *arg)
{
    OS_UNUSED_ARG(arg);
    if(e != NULL) {
        os_free(e);
    }
}

static void
freeTableValues(
        void *e,
        void *arg)
{
    OS_UNUSED_ARG(arg);
    if(e != NULL) {
        os_free(e);
    }
}

ut_table
simulink_createTable()
{
    ut_table table = NULL;
    table = ut_tableNew(table_cmpFunc, NULL, freeTableKeys, NULL, freeTableValues, NULL);
    return table;
}

static void
simulink_insertTableValue(
        ut_table table,
        const char* key,
        const char* value)
{
    (void) ut_tableInsert(table, (void *) key, (void *) value);
}

static char**
getTokens(
        char *line)
{
    static char *tokens[2];
    const char *token0 = NULL;
    char* value = strchr(line,'=');

    if(value == NULL)
    {
        return NULL;
    }

    token0 = os_strndup(line, (os_size_t)(value - line)); /* string before the '=' sign */
    tokens[0] = os_str_trim(token0, NULL);  /* removes white spaces from beginning and end of the string */
    if( tokens[0] == token0) {
        tokens[0] = os_strdup(tokens[0]);
    }
    os_free((void *)token0);

    tokens[1] = os_str_trim(value + 1, NULL); /* removes white spaces from beginning and end of the second token */
    if( tokens[1] == value + 1) {
        tokens[1] = os_strdup(tokens[1]);
    }

    return tokens;
}

static void
readPropertiesFile(
        FILE *file,
        ut_table table)
{
    char line[1024];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        char **value = getTokens(line);
        if(value != NULL && strlen(value[0]) > 0 && strlen(value[1]) > 0)
        {
            if(value[0][0]=='#')
            {
                //free tokens that don't get insert to table
                os_free((void *)value[0]);
                os_free((void *)value[1]);
                continue;
            }
            simulink_insertTableValue(table, value[0], value[1]);
        }
    }
}

void
simulink_readProperties(
        char *fileName,
        ut_table table)
{
    FILE *file;
    if (fileName != 0 && (file = fopen(fileName, "r")) != 0)
    {
        readPropertiesFile(file, table);
        (void) fclose(file);
    }
}

static os_int32
tc_tableWalkAction (
		void *k,
		void *v,
		void *arg)
{
    fprintf((FILE *)arg, "%s = %s\n", (char*)k, (char*)v);
    return 1;
}

void
simulink_writeProperties(
		ut_table table,
		const os_char *fileName)
{
    os_int32 elementCount = ut_count((ut_collection)table);

    FILE *file;
    if(elementCount > 0)
    {
        if (fileName != 0 && (file = fopen(fileName, "w")) != 0)
        {
            fprintf(file, "# Each line in this file defines a key-value pair that determines how Simulink names are\n"
                    "# generated or how IDL unbounded String fields have bounds set in Simulink.\n"
                    "#\n"
                    "# Names: Because the Simulink namespace is 'flat', multiple IDL elements may map to the\n"
                    "# same Simulink name. The IDL import operation will report an error in such cases.\n"
                    "#\n"
                    "# String bounds: Simulink cannot fully support IDL unbounded strings. Because every\n"
                    "# Simulink bus must fixed size, IDLPP specifies a maximum size for unbounded strings. The\n"
                    "# default is 256 characters. This file allows you change the maximum for an unbound string.\n"
                    "#\n"
                    "# The file format is as follows:\n"
                    "#\n"
                    "# 1) lines are of the format: <key> = <value>. Spaces at the beginning or end of the line,\n"
                    "# or around the equals sign are ignored.\n"
                    "# 2) for lines with key's of the form <idl-qualified-name>#name, the value is the Simulink\n"
                    "# name assigned to the generated Simulink element.\n"
                    "# 3) for lines with key's of the form <idl-qualified-name>#stringMax, the value is the\n"
                    "# Simulink an integer specifying the maximum size for an unbounded string.\n"
                    "#\n"
                    "# To change the generated Simulink element, change one or more values in this file, and then\n"
                    "# re-run the IDL import.\n\n");
            (void) ut_tableKeyValueWalk(table, tc_tableWalkAction, file);
            (void) fclose(file);
        }
    }
}


#define KEY_LIST_ANNOTATION "@Key(%s)"
const c_char *
simulinkKeyListAnnotation(
		idl_scope scope,
		const char *name)
{
	const c_char *key_list = idl_keyResolve(idl_keyDefDefGet(), scope, name);
	c_char *result = NULL;
	if(key_list != NULL) {
		result = (c_char *)os_malloc(sizeof(KEY_LIST_ANNOTATION) + strlen(key_list));
		os_sprintf(result, KEY_LIST_ANNOTATION, key_list);
	} else {
		result = os_strdup("");
	}
	return (const c_char*)result;
}

#define IDL_FILE_ANNOTATION "@IdlFile(%s.idl)"
const c_char *
simulinkIdlFileAnnotation(
		idl_scope scope) {
	c_char *baseName = idl_scopeBasename(scope);
	c_char *result = (c_char *)os_malloc(sizeof(IDL_FILE_ANNOTATION) + strlen(baseName));
	os_sprintf(result, IDL_FILE_ANNOTATION, baseName);
	os_free((void*)baseName);
	return (const c_char *)result;
}

const c_char
*simulink_getClassNameFromName(
        idl_scope scope,
        const char *name,
        c_ulong showWarning)
{
    const char *className;
    char *tableValue;
    char *qualifiedName = simulinkQualifiedElement(scope, name);
    /*sizeof(“#name”) includes the null-byte */
    qualifiedName = (char *)os_realloc(qualifiedName, strlen(qualifiedName) + sizeof("#name"));
    os_strcat(qualifiedName, "#name");

    className = simulink_getTableValue(simulink_propertiesTable, qualifiedName);
    if(className == NULL)
    {
        className = name;
        simulink_insertTableValue(simulink_propertiesTable, os_strdup(qualifiedName), os_strdup(className));
    }
    if(showWarning) {
        tableValue = simulink_getTableValue(simulink_nameTable, className);
        if(tableValue != NULL) {
            fprintf(stderr, "Error: IDL element %.*s translates to the Simulink artifact named %s,\n"
                    "which already corresponds to another IDL element %.*s.\n"
                    "Edit the %s file to assign unique Simulink names to each IDL element.\n",
                    (int)(strlen(qualifiedName)-strlen("#name")), qualifiedName, className,
                    (int)(strlen(tableValue)-strlen("#name")), tableValue, simulink_propertiesFileName);
        } else {
            simulink_insertTableValue(simulink_nameTable, os_strdup(className), os_strdup(qualifiedName));
        }
    }
    os_free((void*)qualifiedName);
    return className;
}

c_char
*simulinkGetDataType(
        idl_typeSpec typeSpec)
{
    char *idlSpecName = NULL;
    char *dataType = NULL;
    size_t len;

    idl_typeUser typeUser = idl_typeUser(typeSpec);
    idl_scope type_scope = idl_typeUserScope(typeUser);
    char *qualifiedTypeSpecName = simulinkQualifiedElement(type_scope, (char *)idl_typeSpecName(typeSpec));
    qualifiedTypeSpecName = os_realloc(qualifiedTypeSpecName, strlen(qualifiedTypeSpecName) + sizeof("#name"));
    os_strcat(qualifiedTypeSpecName, "#name");

    idlSpecName = simulink_getTableValue(simulink_propertiesTable, qualifiedTypeSpecName);
    if(idlSpecName == NULL) {
        idlSpecName = idl_typeSpecName(typeSpec);
        simulink_insertTableValue(simulink_propertiesTable, os_strdup(qualifiedTypeSpecName), os_strdup(idlSpecName));
    }

    len = strlen(idlSpecName);
    if (idl_typeSpecType(typeSpec) == idl_tstruct) {
        dataType = os_malloc(sizeof("Bus: ") + len); /*sizeof("Bus: ") includes null byte*/
        os_sprintf(dataType, "Bus: %s", idlSpecName);
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
        dataType = os_malloc(sizeof("Enum: ") + len);/*sizeof("Enum: ") includes null byte*/
        os_sprintf(dataType, "Enum: %s", idlSpecName);
    } else {
        printf("idl_genSimulinkHelper.c:simulinkClassName: Unexpected type %d\n",
            idl_typeSpecType(typeSpec));
    }

    os_free(qualifiedTypeSpecName);

    return dataType;
}

c_char
*simulinkClassName(
        idl_typeSpec typeSpec)
{
	const char *idlSpecName = idl_typeSpecName(typeSpec);
    size_t len = strlen(idlSpecName);
    char *dataType = NULL;
    if (idl_typeSpecType(typeSpec) == idl_tstruct) {
        dataType = os_malloc(sizeof("Bus: ") + len); /*sizeof("Bus: ") includes null byte*/
        os_sprintf(dataType, "Bus: %s", idlSpecName);
    } else if (idl_typeSpecType(typeSpec) == idl_tenum) {
    	dataType = os_malloc(sizeof("Enum: ") + len);/*sizeof("Enum: ") includes null byte*/
        os_sprintf(dataType, "Enum: %s", idlSpecName);
    } else {
        printf("idl_genSimulinkHelper.c:simulinkClassName: Unexpected type %d\n",
            idl_typeSpecType(typeSpec));
    }
    return dataType;
}

c_ulong
simulink_containSeqType(
        idl_typeSpec typeSpec) {
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        /* if the specified type is a basic type */
        return OS_FALSE;
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        return simulink_containSeqType(idl_typeDefActual(idl_typeDef(typeSpec)));
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        return simulink_containSeqType (idl_typeArrayActual(idl_typeArray(typeSpec)));
    } else if(idl_typeSpecType(typeSpec) == idl_tseq) {
        return OS_TRUE;
    }

    return OS_FALSE;
}

static idl_typeBasic
simulink_typeBasicFromTypeSpec(
        idl_typeSpec typeSpec)
{
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        /* if the specified type is a basic type */
        return idl_typeBasic(typeSpec);
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        return simulink_typeBasicFromTypeSpec(idl_typeDefActual(idl_typeDef(typeSpec)));
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        return simulink_typeBasicFromTypeSpec (idl_typeArrayActual(idl_typeArray(typeSpec)));
    } else if(idl_typeSpecType(typeSpec) == idl_tseq) {
        return simulink_typeBasicFromTypeSpec (idl_typeSeqActual(idl_typeSeq(typeSpec)));
    }
    return NULL;
}

c_char
*idl_SimulinkTypeFromTypeSpec(
        idl_typeSpec typeSpec)
{
    c_char *typeName = NULL;

    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        /* if the specified type is a basic type */
    switch (idl_typeBasicType(idl_typeBasic(typeSpec))) {
    case idl_short:
        typeName = os_strdup("int16");
        break;
    case idl_ushort:
        typeName = os_strdup("uint16");
        break;
    case idl_long:
        typeName = os_strdup("int32");
        break;
    case idl_ulong:
        typeName = os_strdup("uint32");
        break;
    case idl_longlong:
    case idl_ulonglong:
        typeName = os_strdup("double");
        break;
    case idl_float:
        typeName = os_strdup("single");
        break;
    case idl_double:
        typeName = os_strdup("double");
        break;
    case idl_char:
        typeName = os_strdup("int8");
        break;
    case idl_string:
        typeName = os_strdup("int8");
        break;
    case idl_boolean:
        typeName = os_strdup("boolean");
        break;
    case idl_octet:
        typeName = os_strdup("uint8");
        break;
    default:
        /* No processing required, empty statement to satisfy QAC */
        break;
    /* QAC EXPECT 2016; Default case must be empty here */
    }
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
        return idl_SimulinkTypeFromTypeSpec (idl_typeArrayActual(idl_typeArray (typeSpec)));
    } else if (idl_typeSpecType(typeSpec) == idl_tstruct ||
            idl_typeSpecType(typeSpec) == idl_tenum) {
        return simulinkClassName(typeSpec);
    } else if (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        return idl_SimulinkTypeFromTypeSpec (idl_typeDefActual(idl_typeDef (typeSpec)));
    } else if(idl_typeSpecType(typeSpec) == idl_tseq) {
        c_ulong showWarning = OS_FALSE;
    	return simulinkSeqClassRef(idl_typeSeq(typeSpec), showWarning);
    } else {
        printf("idl_genSimulinkHelper.c:idl_SimulinkTypeFromTypeSpec: Unexpected type %d\n",
            idl_typeSpecType(typeSpec));
    }
    return typeName;
}

c_char *
simulinkSeqTypeName(
		idl_typeSpec typeSpec)
{
	idl_type type = idl_typeSpecType(typeSpec);
	switch(type) {
	case idl_tbasic:
		switch(idl_typeBasicType(idl_typeBasic(typeSpec))) {
		case idl_boolean:
			return ("boolean");
		case idl_char:
			return ("int8");
		case idl_octet:
			return ("uint8");
		case idl_short:
			return ("int16");
		case idl_ushort:
			return ("uint16");
		case idl_long:
			return ("int32");
		case idl_ulong:
			return ("uint32");
		case idl_longlong:
			return ("double"); /*int64*/
		case idl_ulonglong:
			return ("double"); /*uint64*/
		case idl_float:
			return ("single");
		case idl_double:
			return ("double");
		case idl_string:
			{
				static char buffer[30];
				idl_typeBasic basicSpec = idl_typeBasic(typeSpec);
				c_ulong strMax = idl_typeBasicMaxlen(basicSpec);
				if(strMax) {
				    sprintf(buffer, "string%u", strMax);
				} else {
				    sprintf(buffer, "string");
				}
				return buffer;
			}
		}
		break;
	case idl_tenum:
	case idl_tstruct:
		return idl_typeSpecName(typeSpec);
	case idl_ttypedef:
		{
			idl_typeSpec actualSpec = idl_typeDefActual(idl_typeDef(typeSpec));
			return simulinkSeqTypeName(actualSpec);
		}
	case idl_tarray:
		{
			static c_char formatted_name[256]; /* TODO: Dynamically allocate this, and all return values */
			idl_typeArray typeArray = idl_typeArray(typeSpec);
			idl_typeSpec actualSpec = idl_typeArrayActual(typeArray);
			c_ulong n = simulink_arrayNDimensions(typeArray);
			c_ulong *dims = (c_ulong *)os_malloc(n * sizeof(c_ulong));
			c_char *formatted_dims = (c_char *)os_malloc(n * (20 + 1) + 1);
			c_ulong i;

			idl_typeBasic typeBasic = simulink_typeBasicFromTypeSpec(typeSpec);
			if(typeBasic != NULL && idl_typeBasicType(typeBasic) == idl_string && !simulink_containSeqType(typeSpec)) {
			    n--;
			}

			simulink_arrayGetDimensions(typeArray, dims);
			*formatted_dims = '\0';
			for(i = 0; i < n; i++) {
				c_char formatted_number[21];
				os_strcat(formatted_dims, "_");
				sprintf(formatted_number, "%u", dims[i]);
				os_strcat(formatted_dims, formatted_number);
			}
			sprintf(formatted_name, "%s%s", simulinkSeqTypeName(actualSpec), formatted_dims);
			os_free((void*)formatted_dims);
			os_free((void*)dims);
			return formatted_name;
		}
	case idl_tseq:
	{
	    idl_typeSeq typeSeq = idl_typeSeq(typeSpec);
	    return simulinkSeqClassName(typeSeq);
	}
	case idl_tunion:
        printf("idl_genSimulinkHelper.c:simulinkSeqTypeName: Unsupported type in sequence: %d\n",
            idl_typeSpecType(typeSpec));
        exit(1);
        return "";
	}
	return "";
}

c_char *
simulinkSeqWrappedTypeNameRef(
		idl_typeSpec typeSpec)
{
	idl_type type = idl_typeSpecType(typeSpec);
	switch(type) {
	case idl_tenum:
		{
			c_char *wrappedTypeName = simulinkSeqTypeName(typeSpec);
			c_char *result = (c_char *)os_malloc(strlen(wrappedTypeName) + sizeof("Enum: "));
			sprintf(result, "Enum: %s", wrappedTypeName);
			return result;
		}
		break;
	case idl_tstruct:
		{
			c_char *wrappedTypeName = simulinkSeqTypeName(typeSpec);
			c_char *result = (c_char *)os_malloc(strlen(wrappedTypeName) + sizeof("Bus: "));
			sprintf(result, "Bus: %s", wrappedTypeName);
			return result;
		}
		break;
	case idl_tseq:
		{
		    c_ulong showWarning = OS_FALSE;
	    	idl_typeSeq seqSpec = idl_typeSeq(typeSpec);
	    	c_char *simulinkType = simulinkSeqClassRef(seqSpec, showWarning);
	    	return simulinkType;
		}
		break;
	case idl_tbasic:
	    return idl_SimulinkTypeFromTypeSpec(typeSpec);
	case idl_ttypedef:
		{
		    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
		        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
		    }
		    return simulinkSeqWrappedTypeNameRef(typeSpec);
		}
	case idl_tarray:
		{
			idl_typeArray typeArray = idl_typeArray(typeSpec);
			return simulinkSeqWrappedTypeNameRef(idl_typeArrayActual(typeArray));
		}
	default:
		printf("*** ERROR *** Should not get here. idl_type = %d\n", type);
		exit(1);
		break;
	}

	return NULL;
}

c_ulong
simulinkSequenceMax(
		idl_typeSeq typeSeq)
{
	c_ulong seqMax = idl_typeSeqMaxSize(typeSeq);
	return seqMax;
}

c_ulong
simulinkStringMax(
		idl_typeBasic typeString)
{
	c_ulong seqMax = idl_typeBasicMaxlen(typeString);

	if(!seqMax) {
		/* TODO: figure out if there is a user provided override for seqMax */
	    seqMax = simulink_getStrMaxDimension();
	}
	return seqMax;
}

#define SEQ_CLASS_NAME_FMT "seq%s_%s"
c_char *
simulinkSeqClassName(
		idl_typeSeq typeSeq)
{
	c_char* wrappedTypeName = simulinkSeqTypeName(idl_typeSeqType(typeSeq));
	c_char *className;
	c_ulong seqMax = simulinkSequenceMax(typeSeq);
	c_char max_bound[20] = "";
	if(seqMax) {
	    sprintf(max_bound, "%u", seqMax);
	}
	className = os_malloc(sizeof(SEQ_CLASS_NAME_FMT) + strlen(max_bound) + strlen(wrappedTypeName));
	sprintf(className, SEQ_CLASS_NAME_FMT, max_bound, wrappedTypeName);

	return className;
}

#define SEQ_CLASS_REF_FMT "Bus: %s"
c_char *
simulinkSeqClassRef(
		idl_typeSeq typeSeq,
		c_ulong showWarning)
{
	c_char* className = simulinkSeqClassName(typeSeq);
	idl_typeUser typeUser = idl_typeUser(typeSeq);
    idl_scope scope = idl_typeUserScope(typeUser);
    const char *busName = simulink_getClassNameFromName(scope, className, showWarning);

	c_char *classRef = (c_char *)os_malloc(sizeof(SEQ_CLASS_REF_FMT) + strlen(busName));
	sprintf(classRef, SEQ_CLASS_REF_FMT, busName);
	os_free((void*)className);

	return classRef;
}

c_char *
simulinkSeqDimensions(
		idl_typeSeq typeSeq)
{
	static c_char buffer[40];
	c_ulong seqMax = simulinkSequenceMax(typeSeq);
	idl_typeSpec wrappedType = idl_typeSeqType(typeSeq);
	idl_type type = idl_typeSpecType(wrappedType);
	if(type == idl_tbasic
			&& idl_typeBasicType(idl_typeBasic(wrappedType)) == idl_string) {
		sprintf(buffer, "[%u, %u]", seqMax, simulinkStringMax(idl_typeBasic(wrappedType)));
	} else if(type == idl_tarray) {
		idl_typeArraySize(idl_typeArray(wrappedType));
	} else {
		sprintf(buffer, "%u", seqMax);
	}

	return buffer;

}

c_char *
simulinkTypeAnnotation(
		idl_typeSpec typeSpec)
{
	c_char *annotation = "";
	idl_type type = idl_typeSpecType(typeSpec);
	if(type == idl_tbasic) {
		idl_typeBasic basicSpec = idl_typeBasic(typeSpec);
		idl_basicType basicType = idl_typeBasicType(basicSpec);
		if(basicType == idl_string) {
			annotation = idl_typeBasicMaxlen(basicSpec) ? "@BString" : "@String";
		}
	} else if(type == idl_tseq) {
		annotation = idl_typeSeqMaxSize(idl_typeSeq(typeSpec)) ? "@BSequence" : "@Sequence";
	} else if(type == idl_tarray) {
		annotation = simulinkTypeAnnotation(idl_typeArrayActual(idl_typeArray (typeSpec)));
	}

	return annotation;
}

c_ulong
simulink_arrayNDimensions(
		idl_typeArray typeArray)
{
	idl_typeSpec wrappedTypeSpec = idl_typeArrayType(typeArray);
	idl_type wrappedType = idl_typeSpecType(wrappedTypeSpec);
	if(wrappedType == idl_tarray) {
		return 1 + simulink_arrayNDimensions(idl_typeArray(wrappedTypeSpec));
	} else if(wrappedType == idl_tbasic) {
		idl_typeBasic typeBasic = idl_typeBasic(wrappedTypeSpec);
		return idl_typeBasicType(typeBasic) == idl_string ? 2 : 1;
	} else {
		return 1;
	}
}

static c_ulong str_max_dimension = 256;
c_ulong
simulink_getStrMaxDimension()
{
    return str_max_dimension;
}

void
simulink_setStrMaxDimension(
        c_ulong stringMax)
{
    str_max_dimension = stringMax;
}

#define MAX_UNBOUNDED_STR_DEFAULT 256
void
simulink_recordStringBound(
        idl_typeSpec typeSpec,
        idl_scope scope,
        const char *name)
{
    idl_typeBasic typeBasic = simulink_typeBasicFromTypeSpec(typeSpec);
    if(typeBasic != NULL && idl_typeBasicType(typeBasic) == idl_string) {
        if(idl_typeBasicMaxlen(typeBasic) == NULL) {
            c_char *stringDims;
            c_char *qualifiedElement = simulinkQualifiedElement(scope, name);
            qualifiedElement = (char *)os_realloc(qualifiedElement, strlen(qualifiedElement) + sizeof("#stringMax"));
            os_strcat(qualifiedElement, "#stringMax");

            stringDims = simulink_getTableValue(simulink_propertiesTable, qualifiedElement);
            if(stringDims == NULL)
            {
                stringDims = (char *) os_malloc(sizeof(MAX_UNBOUNDED_STR_DEFAULT));
                os_sprintf(stringDims, "%d", MAX_UNBOUNDED_STR_DEFAULT);
                simulink_insertTableValue(simulink_propertiesTable, os_strdup(qualifiedElement), stringDims);
            } else if (os_strtoull(stringDims, (char **)NULL, 10) == 0) {
                fprintf(stderr, "Error: Entered value for %s is not a valid integer to set the maximum size for an unbounded string.\n"
                "Edit the %s file to assign an integer to specify the maximum size for %.*s.\n",
                qualifiedElement, simulink_propertiesFileName, (int)(strlen(qualifiedElement) - strlen("#stringMax")), qualifiedElement);
            }
            simulink_setStrMaxDimension((c_ulong) os_strtoull(stringDims, (char **)NULL, 10));
            os_free((void *) qualifiedElement);
        }
    }
}

#define MAX_UNBOUNDED_SEQ_DEFAULT 16
c_ulong
simulink_seqTypeBound(
        idl_typeSeq typeSeq,
        idl_scope scope,
        const char *name)
{
    c_ulong seqMax = idl_typeSeqMaxSize(typeSeq);
    if(seqMax == NULL)
    {
        c_char *seqDims;
        c_char *qualifiedSeqName = simulinkQualifiedElement(scope, name);
        qualifiedSeqName = (char *)os_realloc(qualifiedSeqName, strlen(qualifiedSeqName) + sizeof("#seqMax"));
        os_strcat(qualifiedSeqName, "#seqMax");

        seqDims = simulink_getTableValue(simulink_propertiesTable, qualifiedSeqName);
        if(seqDims == NULL) {
            seqDims = (char *) os_malloc(sizeof(MAX_UNBOUNDED_SEQ_DEFAULT));
            os_sprintf(seqDims, "%d", MAX_UNBOUNDED_SEQ_DEFAULT);
            simulink_insertTableValue(simulink_propertiesTable, os_strdup(qualifiedSeqName), seqDims);
        } else if (os_strtoull(seqDims, (char **)NULL, 10) == 0) {
            fprintf(stderr, "Error: Entered value for %s is not a valid integer to set the maximum dimension for an unbounded sequence.\n"
            "Edit the %s file to assign an integer to specify the maximum dimension for %.*s.\n",
            qualifiedSeqName, simulink_propertiesFileName, (int)(strlen(qualifiedSeqName) - strlen("#seqMax")), qualifiedSeqName);
        }
        os_free((void *)qualifiedSeqName);
        seqMax = (c_ulong) os_strtoull(seqDims, (char **)NULL, 10);
    }
    return seqMax;
}

void
simulink_arrayGetDimensions(
        idl_typeArray typeArray,
        c_ulong dimensions[])
{
    idl_typeSpec wrappedTypeSpec = idl_typeArrayType(typeArray);
    idl_type wrappedType = idl_typeSpecType(wrappedTypeSpec);

    dimensions[0] = idl_typeArraySize(typeArray);
    if(wrappedType == idl_tarray) {
        simulink_arrayGetDimensions(idl_typeArray(wrappedTypeSpec), &dimensions[1]);
    } else if(wrappedType == idl_tbasic) {
        idl_typeBasic typeBasic = idl_typeBasic(wrappedTypeSpec);
        if(idl_typeBasicType(typeBasic) == idl_string) {
            c_ulong strMax = idl_typeBasicMaxlen(typeBasic);
            dimensions[1] = strMax ? strMax : simulink_getStrMaxDimension();
        }
    }
}

c_char *
simulink_formatDimensions(
		c_ulong dimensions[],
		c_ulong nDimensions)
{
	/* Format is: n
	 *    or
	 *      [n1 n2 n3 n4]
	 *
	 * Worst case size of a dimension is 20 characters (ULONG_MAX is 20 digits).
	 * Worst case string to alloc is: nDimensions * (20 + 1) + 1
	 */
	c_char *formatted = (c_char *)os_malloc(nDimensions * (20 + 1) + 1 + 1);

	if(nDimensions == 1) {
		sprintf(formatted, "%u", dimensions[0]);
	} else {
		c_char formatted_number[21]; /* ULONG_MAX is 20 digits */
		c_ulong i;
		sprintf(formatted, "[");
		for(i = 0; i < nDimensions; i++) {
			sprintf(formatted_number, "%u", dimensions[i]);
			if(i > 0) {
				os_strcat(formatted, " ");
			}
			os_strcat(formatted, formatted_number);
		}
		os_strcat(formatted, "]");
	}
	return formatted;
}

c_char *simulink_sequenceTypeDimensions(
		idl_typeSpec typeSpec,
		c_ulong seqMax) {
	c_char formatted_number[20 + 1];

    while (idl_typeSpecType(typeSpec) == idl_ttypedef) {
        typeSpec = idl_typeDefRefered(idl_typeDef(typeSpec));
    }

    if(idl_typeSpecType(typeSpec) == idl_tarray) {
        idl_typeArray typeArray = idl_typeArray(typeSpec);
        /* allocation must include the space for seqMax */
        c_ulong n = simulink_arrayNDimensions(typeArray) + 1;
        c_ulong *dims = (c_ulong *)malloc(n * sizeof(c_ulong));
        c_char *formatted = NULL;
        dims[0] = seqMax;
        simulink_arrayGetDimensions(typeArray, &dims[1]);
        formatted = simulink_formatDimensions(dims, n);
        free((void *)dims);
        return formatted;
	}
    if(idl_typeSpecType(typeSpec) == idl_tbasic) {
		idl_typeBasic typeBasic = idl_typeBasic(typeSpec);
		if(idl_typeBasicType(typeBasic) == idl_string) {
			c_char formatted[2 * 20 + 3 + 1];
			c_ulong strMax = idl_typeBasicMaxlen(typeBasic);
			sprintf(formatted, "[%u %u]", seqMax, strMax ? strMax : simulink_getStrMaxDimension());
			return os_strdup(formatted);
		}
    }

	sprintf(formatted_number, "%u", seqMax);
	return os_strdup(formatted_number);
}
