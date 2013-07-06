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
/**
 * @file
 * This module generates Splice meta data load functions
 * related to an IDL input file.
 */
#include "os_stdlib.h"

#include "idl_program.h"
#include "idl_scope.h"
#include "idl_genSpliceLoad.h"
#include "idl_genLanguageHelper.h"
#include "idl_genSplHelper.h"
#include "idl_tmplExp.h"

#include "c_typebase.h"
#include "c_iterator.h"

/* The following IDL to metadata type mapping applies to all routines.
 *
 *  IDL type			splice-type	meta data
 *                               
 *  char			c_char		c_primitive->kind P_CHAR 
 *  octet			c_octet		c_primitive->kind P_OCTET
 *  boolean			c_bool		c_primitive->kind P_BOOLEAN
 *  short			c_short		c_primitive->kind P_SHORT
 *  unsigned short		c_ushort	c_primitive->kind P_USHORT
 *  long			c_long		c_primitive->kind P_LONG
 *  unsigned long		c_ulong		c_primitive->kind P_ULONG
 *  long long			c_longlong	c_primitive->kind P_LONGLONG
 *  unsigned long long		c_ulonglong	c_primitive->kind P_ULONGLONG
 *  float			c_float		c_primitive->kind P_FLOAT
 *  double			c_double	c_primitive->kind P_DOUBLE
 *  string			c_string	c_collectionType->kind C_STRING, ->maxSize 0, ->subType "c_char"
 *  string<len>			c_string	c_collectionType->kind C_STRING, ->maxSize len, ->subType "c_char"
 *  sequence<type>		c_sequence	c_collectionType->kind C_SEQUENCE, ->maxSize 0, ->subType <type>
 *  sequence<type,len>		c_sequence		c_collectionType->kind C_SEQUENCE, ->maxSize len, ->subType <type>
 *  typedef <type-name> <name>	-		c_typeDef->alias <type-name>
 *  enum <name> {				c_enumeration->elements
 *	<element-name>				    elements[0..n] c_constant
 *  };
 *  struct <name> {				c_structure->members, ->scope (types defined within scope of struct)
 *      <member-type> <member-name>;		    members[0..n] c_specifier->name <member-name>, ->type <member-type>
 *  };
 *  union <name>				c_union->cases[0..n], ->scope, ->switchType <switch-type>
 *      switch (switch-type) {
 *	<case-label>:
 *          <case-type> <case-name>;		c_unionCase->name <case-name>, ->type <case-type>, ->labels[0..n]
 *	default:
 *          <case-type> <case-name>;		// 0 labels
 *  };
 *  <name>[<size-0>]..[<size-n>];		c_collectionType->kind C_ARRAY, ->maxSize <size-i>,
 *                                                      ->subType <element-type>
 *
 *  For modules, type definitions, structures, unions, enumerations and bounded strings,
 *  a load function is defined. Depending on the scope, the function is global (scope is
 *  global or a module) or local (scope is structure or union).
 *
 *  The general basic layout of a global load function is (except for the module load function):
 *
 *	c_metaObject __[<full-scope-name>_]<type-name>__load (
 *	    c_base base)		// The base of the meta data collection
 *	{
 *	    c_metaObject scope;		// The scope of the meta object
 *	    c_metaObject tscope;	// The scope of the subtype when applicable
 *	    c_metaObject o;		// The unbound meta object (nameless)
 *	    c_metaObject found;		// The bound meta object
 *
 *	    // Definition of the meta object
 *
 *	    c_metaFinalize(o);		// Finalize the meta object
 *	    found = c_metaBind(scope,"<type-name>",o); // Bind it to the specific type name
 *					// If found equals o, the definition is new.
 *					// If found equals NULL, the definition is not new and
 *					// inconsistent with the existing one.
 *					// Else the definition is not new but yet consistent.
 *	    c_free(o);			// Free the object (which when it is new is still
 *					// referenced by the meta data collection).
 *	    return found;		// Return the bound meta object reference
 *	}
 *
 *   The general basic layout of a static load function is:
 *
 *      static c_metaObject __<type-name>__load (
 *          c_metaObject scope)		// The scope of the type
 *      {
 *	    c_metaObject tscope;	// The scope of the subtype when applicable
 *	    c_metaObject o;		// The unbound meta object (nameless)
 *	    c_metaObject found;		// The bound meta object
 *
 *	    // Definition of the meta object
 *
 *	    c_metaFinalize(o);		// Finalize the meta object
 *	    found = c_metaBind(scope,"<type-name>",o); // Bind it to the specific type name
 *					// If found equals o, the definition is new.
 *					// If found equals NULL, the definition is not new and
 *					// inconsistent with the existing one.
 *					// Else the definition is not new but yet consistent.
 *	    c_free(o);			// Free the object (which when it is new is still
 *					// referenced by the meta data collection).
 *	    return found;		// Return the bound meta object reference
 *	}
 */

	/** Base variable for array dimension and sequence recursion level */
#define IDL_INDEX_VAR		'o'

/**
 * Enumerates scope selection options.
 * - sc_base:   use the "base" variable
 * - sc_scope:  use the "scope" variable
 * - sc_struct: for union or struct use the "o" variable
 */
typedef enum {
    sc_base,
    sc_scope,
    sc_struct 
} sc_scoping;

	/** Index for the member of a structure		*/
static c_long member_index;
	/** Index for the element of an enumeration	*/
static c_long element_index;
	/** Index for the union case of a union		*/
static c_long case_index;
	/** Index for the label of a union case		*/
static c_long label_index;

	/** Scope identifiers indexed by sc_scoping*/
static const char *scopeName[] = {
    "base",
    "scope",
    "o",
};

static void idl_arrayLoad(idl_scope scope, idl_typeSpec typeSpec, c_long indent);
static void idl_seqLoad(idl_scope scope, idl_typeSpec typeSpec, c_long indent);

/** @brief Generate a string representaion the literal value of a label
 * in metadata terms.
 *
 * @param labelVal Specifies the kind and the value of the label
 * @return String representing the image of \b labelVal
 */
static c_char *
idl_valueFromLabelVal(
    idl_labelVal labelVal)
{
    static c_char labelName[1000];

    /* QAC EXPECT 3416; No side effect here */
    if (idl_labelValType(idl_labelVal(labelVal)) == idl_lenum) {
        /* For an enumeration use the c_longValue of the enum value */
        snprintf(labelName, (size_t)sizeof(labelName), "c_longValue(_%s)",
	       idl_labelEnumVal(idl_labelEnum(labelVal)));
    } else {
        switch (idl_labelValueVal(idl_labelValue(labelVal)).kind) {
	    case V_CHAR:
		snprintf (labelName, (size_t)sizeof(labelName), "c_charValue (%u)",
		    idl_labelValueVal(idl_labelValue(labelVal)).is.Char);
		break;
	    case V_SHORT:
		snprintf (labelName, (size_t)sizeof(labelName), "c_shortValue (%d)", 
		    idl_labelValueVal(idl_labelValue(labelVal)).is.Short);
		break;
	    case V_USHORT:
		snprintf (labelName, (size_t)sizeof(labelName), "c_ushortValue (%u)", 
		    idl_labelValueVal(idl_labelValue(labelVal)).is.UShort);
		break;
	    case V_LONG:
		snprintf (labelName, (size_t)sizeof(labelName), "c_longValue (%d)", 
		    idl_labelValueVal(idl_labelValue(labelVal)).is.Long);
		break;
	    case V_ULONG:
		snprintf (labelName, (size_t)sizeof(labelName), "c_ulongValue (%u)", 
		    idl_labelValueVal(idl_labelValue(labelVal)).is.ULong);
		break;
	    case V_LONGLONG:
		snprintf (labelName, (size_t)sizeof(labelName), "c_longlongValue (%lld)", 
		    idl_labelValueVal(idl_labelValue(labelVal)).is.LongLong);
		break;
	    case V_ULONGLONG:
		snprintf (labelName, (size_t)sizeof(labelName), "c_ulonglongValue (%llu)", 
		    idl_labelValueVal(idl_labelValue(labelVal)).is.ULongLong);
		break;
	    case V_BOOLEAN:
		if ((int)idl_labelValueVal(idl_labelValue(labelVal)).is.Boolean == TRUE) {
		    snprintf (labelName, (size_t)sizeof(labelName), "c_boolValue (TRUE)");
		} else {
		    snprintf (labelName, (size_t)sizeof(labelName), "c_boolValue (FALSE)");
		}
		break;
	    default:
		break;
        }
    }

    return labelName;
}

/** @brief Generate code to determine the scope of a module or the global scope.
 *
 * @param scope Defines the current scope
 */
static void
idl_scopePrint (
    idl_scope scope)
{
    /* QAC EXPECT 3416; No side effect here */
    if ((idl_scopeStackSize(scope) == 0) || (idl_scopeElementType(idl_scopeCur(scope)) == idl_tFile)) {
        /* if the scope stack is empty, base is the database object
           thus translate base to a metaObject which is the scope
	     */
        idl_fileOutPrintf (idl_fileCur(), "    scope = c_metaObject(base);\n");
    } else {
	/* use the load function of the top scope element, this function will
	   return the scope object
	*/
        idl_fileOutPrintf (idl_fileCur(), "    scope = __%s_%s__load (base);\n",
	       idl_scopeBasename(scope),
	       idl_scopeStack (scope, "_", NULL));
    }
}

/** @brief callback function called on opening the IDL input file.
 *
 * Generate standard macro's used within meta data load functions:
 * - Resolve
 * - ResolveType
 *
 * @param scope Current scope (not used)
 * @param name Name of the IDL input file (not used)
 * @return Next action for this file (idl_explore)
 */
static idl_action
idl_fileOpen(
    idl_scope scope,
    const char *name,
    void *userData)
{
    idl_fileOutPrintf(idl_fileCur(), "#define Resolve(s,o) c_metaResolve(c_metaObject(s),o)\n");
    idl_fileOutPrintf(idl_fileCur(), "#define ResolveType(s,t) c_type(c_metaResolve(c_metaObject(s),t))\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");

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
 * The load function is to retun the meta object specifying the module
 * and its contents.
 *
 * The name of the function is:
 * @verbatim
        __<scope-basename>_<scope-elements>_<module-name>__load
   @endverbatim
 *
 * The function passes the database base object.
 *
 * The function first determines the scope object of the module itself,
 * which is either the database or another module.
 *
 * Then it tries to resolve the module within that scope, if found
 * the module was already defined and the function leaves returning
 * the modules meta object. Otherwise it declares the module before
 * returning the modules meta object.
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
    idl_fileOutPrintf(idl_fileCur(), "c_metaObject\n");
    idl_fileOutPrintf(
        idl_fileCur(),
        "__%s_%s__load (\n",
        idl_scopeBasename(scope),
        idl_scopeStack(scope, "_", name));
    idl_fileOutPrintf(idl_fileCur(), "    c_base base)\n");
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_fileOutPrintf(idl_fileCur(), "    c_metaObject scope;\n");
    idl_fileOutPrintf(idl_fileCur(), "    c_metaObject o;\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
    /* find the scope meta object of the module */
    idl_scopePrint(scope);
    /* try to find the module within the specified scope */
    idl_fileOutPrintf(idl_fileCur(), "    o = c_metaResolve(scope,\"%s\");\n", name);
    idl_fileOutPrintf(idl_fileCur(), "    if (o == NULL) {\n");
    /* the module is not found, thus declare it */
    idl_fileOutPrintf(idl_fileCur(), "        o = c_metaDeclare(scope,\"%s\",M_MODULE);\n", name);
    idl_fileOutPrintf(idl_fileCur(), "    }\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
    /* return the modules meta object  */
    idl_fileOutPrintf(idl_fileCur(), "    return o;\n");
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");

    /* return idl_explore to indicate that the rest of the module needs to be processed */
    return idl_explore;
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
 * The load function is to retun the meta object specifying the structure
 * and its contents.
 *
 * The name of the function is:
 *      __<scope-elements>_<structure-name>__load
 *
 * If the structure is defined on global scope or within a module,
 * the function passes the database base object. Otherwise it passes
 * the meta object it is defined in (structure or union). The database
 * base object is then determined from the scope object.
 *
 * The function then defines a structure and allocates
 * space (array of c_object) for the structure members.
 *
 * After that it specifies the structures scope.
 *
 * @param scope Current scope (and scope of the structure definition)
 * @param name Name of the structure
 * @param structSpec Specification of the struct holding the amount of members
 * @return Next action for this struct (idl_explore)
 */
static idl_action
idl_structureOpen(
    idl_scope scope,
    const char *name,
    idl_typeStruct structSpec,
    void *userData)
{
    /* QAC EXPECT 3416; No side effect here */
    if ((idl_scopeStackSize(scope) == 0) || (idl_scopeElementType (idl_scopeCur(scope)) == idl_tModule)) {
        idl_fileOutPrintf(idl_fileCur(), "c_metaObject\n");
        idl_fileOutPrintf(idl_fileCur(), "__%s__load (\n", idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "    c_base base)\n");
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject scope;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject tscope;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_array members;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject o;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject found;\n");
        idl_fileOutPrintf(idl_fileCur(), "\n");
        /* find the scope meta object of the structure */
        idl_scopePrint(scope);
    } else {
        /* The structure is defined within the scope of another structure
           or union. Define the function to be static.
        */
        idl_fileOutPrintf(idl_fileCur(), "static c_metaObject\n");
        idl_fileOutPrintf(idl_fileCur(), "__%s__load (\n", idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject scope)\n");
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject tscope;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_array members;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject o;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject found;\n");
	/* get the database base object from the scope object */
        idl_fileOutPrintf(idl_fileCur(), "    c_base base = c_getBase(scope);\n");
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
    /* define the structure */
    idl_fileOutPrintf(idl_fileCur(), "    o = c_metaObject(c_metaDefine(scope,M_STRUCTURE));\n");
    /* allocate space for the structure members */
    idl_fileOutPrintf(idl_fileCur(), "    members = c_arrayNew(ResolveType(base,\"c_object\"),%d);\n",
	idl_typeStructNoMembers(structSpec));
    /* define the structures scope */
    idl_fileOutPrintf(idl_fileCur(), "    c_metaObject(o)->definedIn = scope;\n");
    /* The first member maps on members[0] */
    member_index = 0;

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
 * The function finalizes the meta data definition of the structure 
 * by assigning the members, then finalizing the structure.
 *
 * The function then again finds the structure in the meta data.
 * If found equals o, the structure was not yet defined.
 * If found does not equal o, it was already defined and has the
 * same definition.
 * If found = NULL, it was already defined with a different definition.
 *
 * @param name Name of the structure (not used)
 */
static void
idl_structureClose (
    const char *name,
    void *userData)
{
    idl_fileOutPrintf(idl_fileCur(), "    c_structure(o)->members = members;\n");
    idl_fileOutPrintf(idl_fileCur(), "    c_metaFinalize(o);\n");
    idl_fileOutPrintf(idl_fileCur(), "    found = c_metaBind(scope,\"%s\",o);\n",name);
    idl_fileOutPrintf(idl_fileCur(), "    c_free(o);\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
    idl_fileOutPrintf(idl_fileCur(), "    return found;\n");
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
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
 * First the generated function defines a structure member meta object for the
 * members indexed by member_index. After that the name of the member is set.
 *
 * Generated code then depends on the type of the structure member:
 * - If the type is idl_tbasic and the maximumLength is > 0, it is a bounded
 *   string for which a load function is available which must be called to
 *   determine the type scope.
 * - If the type is idl_tbasic and the maximumLength is 0, it is a basic
 *   type for which the scope is global. "scope" can be used to resolve
 *   the type.
 * - If the type is idl_tarray, the routine to load array types is called.
 * - If the type is idl_tseq, the routine to load sequence types is called.
 * - For all other types the type specific load function is called and
 *   their scope is used as the type scope.
 *
 *  Finally the type is resolved by its type name and the specified type scope.
 *  The type is assigned to the members type reference.
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
    /* Define an M_MEMBER meta object ans assign it to members[member_index] */
    idl_fileOutPrintf(
        idl_fileCur(),
        "    members[%d] = (c_voidp)c_metaDefine(scope,M_MEMBER);\n",
        member_index);
    /* Assign the members name to the members meta object name attribute */
    idl_fileOutPrintf(
        idl_fileCur(),
        "    c_specifier(members[%d])->name = c_stringNew(base,\"%s\");\n",
        member_index,
        name);
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
	   /* The member is of basic type definition */
       /* QAC EXPECT 3416; No side effect here */
        if (idl_typeBasicMaxlen(idl_typeBasic(typeSpec)) > 0) {
	    /* Only bounded string has maximum length > 0.
	       Load the bounded string meta data to determine it's scope
	    */
            idl_fileOutPrintf(
                idl_fileCur(),
                "    tscope = __%s_%d__load (o)->definedIn;\n",
                idl_scopeStack (scope, "_", NULL),
                idl_typeBasicMaxlen(idl_typeBasic(typeSpec)));
        } else {
	        /* Take the scope of the member */
            idl_fileOutPrintf (idl_fileCur(), "    tscope = scope;\n");
        }
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
	/* The member is of array type (inline) */
        idl_fileOutPrintf(idl_fileCur(), "    {\n");
        idl_fileOutPrintf(idl_fileCur(), "        c_metaObject p;\n");
        idl_fileOutPrintf(idl_fileCur(), "\n");
	/* Generate code for loading array meta data */
        idl_arrayLoad(scope, typeSpec, 1);
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
	/* The member is of sequence type (inline) */
        idl_fileOutPrintf(idl_fileCur(), "    {\n");
        idl_fileOutPrintf(idl_fileCur(), "        c_metaObject %c;\n", IDL_INDEX_VAR+1 /* 'p' */);
        idl_fileOutPrintf(idl_fileCur(), "\n");
	/* Generate code for loading sequence meta data */
        idl_seqLoad(scope, typeSpec, 1);
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
    } else {
        /* The member is of other user type */
        /* QAC EXPECT 3416; No side effect here */
        if (idl_scopeEqual(scope, idl_typeUserScope(idl_typeUser(typeSpec))) == FALSE) {
	    /* If the member type is defined outside the structure, then load the metadata of that type */
            idl_fileOutPrintf(
                idl_fileCur(),
                "    tscope = __%s__load (base)->definedIn;\n",
                idl_scopedTypeName(typeSpec));
        } else {
	    /* If the member type is defined inside the structure, then load the metadata of that type */
            idl_fileOutPrintf(
                idl_fileCur(),
                "    tscope = __%s__load (o)->definedIn;\n",
                idl_scopedTypeName(typeSpec));
        }
    }
    /* Resolve the type of the member within the type scope and assign it to the
       members meta object type attribute.
    */
    idl_fileOutPrintf(idl_fileCur(), "    c_specifier(members[%d])->type = ResolveType(tscope,\"%s\");\n",
	member_index,
	idl_typeFromTypeSpec(typeSpec));
    /* Next member maps on the next members array element */
    member_index++;
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
 * The load function is to retun the meta object specifying the union
 * and its contents.
 *
 * The name of the function is:
 * @verbatim
        __<scope-elements>_<union-name>__load
   @endverbatim
 *
 * If the union is defined on global scope or within a module,
 * the function passes the database base object. Otherwise it passes
 * the meta object it is defined in (structure or union). The database
 * base object is then determined from the scope object.
 *
 * The generated function then defines a union and specifies the unions scope.
 *
 * After that the type scope of the switchtype is determined, after which
 * the type is resolved and assigned to the unions switchType.
 *
 * Finally the function allocates space (array of c_object) for the
 * union cases.
 *
 * @param scope Current scope
 * @param name Name of the union
 * @param unionSpec Specifies the number of union cases and the union switch type
 * @return Next action for this union (idl_explore)
 */
static idl_action
idl_unionOpen(
    idl_scope scope,
    const char *name,
    idl_typeUnion unionSpec,
    void *userData)
{
    /* QAC EXPECT 3416; No side effect here */
    if ((idl_scopeStackSize(scope) == 0) || (idl_scopeElementType (idl_scopeCur(scope)) == idl_tModule)) {
	/* Union is defined within the global scope or a module */
        idl_fileOutPrintf(idl_fileCur(), "c_metaObject\n");
        idl_fileOutPrintf(idl_fileCur(), "__%s__load (\n", idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "    c_base base)\n");
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject scope;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject tscope;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_array cases;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_array labels;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject o;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject found;\n");
        idl_fileOutPrintf(idl_fileCur(), "\n");
	/* Assign the correct scope to the scope variable */
        idl_scopePrint(scope);
    } else {
	/* Union is defined within a struct or a union, the load function
	   is defined static because it is only locally used.
	*/
        idl_fileOutPrintf(idl_fileCur(), "static c_metaObject\n");
        idl_fileOutPrintf(idl_fileCur(), "__%s__load (\n", idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject scope)\n");
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject tscope;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_array cases;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_array labels;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject o;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject found;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_base base = c_getBase(scope);\n");
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
    /* Define an M_UNION meta object */
    idl_fileOutPrintf(idl_fileCur(), "    o = c_metaObject(c_metaDefine(scope,M_UNION));\n");
    /* Specify the scope of the union */
    idl_fileOutPrintf(idl_fileCur(), "    c_metaObject(o)->definedIn = scope;\n");
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeUnionSwitchKind(unionSpec)) == idl_tbasic) {
	/* The switch kind is of basic type, therefor the switch type scope
	   may equal the union scope.
	*/
        idl_fileOutPrintf(idl_fileCur(), "    tscope = scope;\n");
    } else {
        /* Push the union identifier on the scope stack */
        idl_scopePush(scope, idl_scopeElementNew(name, idl_tUnion));
        /* QAC EXPECT 3416; No side effect here */
        if (idl_scopeEqual(scope, idl_typeUserScope(idl_typeUser(idl_typeUnionSwitchKind(unionSpec)))) == FALSE) {
            /* if the switch kind is defined outside the union, then load the metadata of that type */
            idl_fileOutPrintf(
                idl_fileCur(),
                "    tscope = __%s__load (base)->definedIn;\n",
                idl_scopedTypeName(idl_typeUnionSwitchKind(unionSpec)));
        } else {
	    /* if the switch kind is defined inside the union, then load the metadata of that type */
            idl_fileOutPrintf(
                idl_fileCur(),
                "    tscope = __%s__load (o)->definedIn;\n",
                idl_scopedTypeName(idl_typeUnionSwitchKind(unionSpec)));
        }
	/* Remove the union identifier from the scope stack */
        idl_scopePopFree(scope);
    }
    /* Assign the switch type specifier to the unions switch type attribute */
    idl_fileOutPrintf(idl_fileCur(), "    c_union(o)->switchType=ResolveType(tscope,\"%s\");\n",
	idl_typeFromTypeSpec(idl_typeUnionSwitchKind(unionSpec)));
    /* Create an array for all the union cases */
    idl_fileOutPrintf(
        idl_fileCur(),
        "    cases = c_arrayNew(ResolveType(base,\"c_object\"),%d);\n",
        idl_typeUnionNoCases(unionSpec));
    /* The first union case maps on cases[0] */
    case_index = 0;
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
 * The function finalizes the meta data definition of the union 
 * by assigning the union cases, then finalizing the union.
 *
 * The function then again finds the union in the meta data.
 * If found equals o, the union was not yet defined.
 * If found does not equal o, it was already defined and has the
 * same definition.
 * If found = NULL, it was already defined with a different definition.
 *
 * @param name Name of the union
 */
static void
idl_unionClose(
    const char *name,
    void *userData)
{
    /* Assign the union cases to the unions cases attribute */
    idl_fileOutPrintf(idl_fileCur(), "    c_union(o)->cases = cases;\n");
    /* Finalize the meta object (union) */
    idl_fileOutPrintf(idl_fileCur(), "    c_metaFinalize(o);\n");
    /* Find the union meta object again */
    idl_fileOutPrintf(idl_fileCur(), "    found = c_metaBind(scope,\"%s\",o);\n",name);
    /* Free the original union meta object */
    idl_fileOutPrintf(idl_fileCur(), "    c_free(o);\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
    idl_fileOutPrintf(idl_fileCur(), "    return found;\n");
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
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
 * First the generated function defines a union case meta object for the
 * union cases indexed by case_index. After that the name of the union
 * case is set.
 *
 * Generated code then depends on the type of the structure member:
 * - If the type is idl_tbasic and the maximumLength is > 0, it is a bounded
 *   string for which a load function is available which must be called to
 *   determine the type scope.
 * - If the type is idl_tbasic and the maximumLength is 0, it is a basic
 *   type for which the scope is global. "scope" can be used to resolve
 *   the type.
 * - If the type is idl_tarray, the routine to load array types is called.
 * - If the type is idl_tseq, the routine to load sequence types is called.
 * - For all other types the type specific load function is called and
 *   their scope is used as the type scope.
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
    /* Define an M_UNIONCASE meta object and assign it to cases[case_index] */
    idl_fileOutPrintf(
        idl_fileCur(),
        "    cases[%d] = (c_voidp)c_metaDefine(scope,M_UNIONCASE);\n",
        case_index);
    /* Specify the cases[case_index] name attribute */
    idl_fileOutPrintf(
        idl_fileCur(),
        "    c_specifier(cases[%d])->name = c_stringNew(base,\"%s\");\n",
        case_index,
        name);
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        /* The union case is of basic type definition */
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeBasicMaxlen(idl_typeBasic(typeSpec)) > 0) {
	    /* Only bounded string has maximum length > 0.
	       Load the bounded string meta data to determine it's scope
	    */
            idl_fileOutPrintf(
                idl_fileCur(),
                "    tscope = __%s_%d__load (o)->definedIn;\n",
                idl_scopeStack(scope, "_", NULL),
                idl_typeBasicMaxlen(idl_typeBasic(typeSpec)));
        } else {
	       /* Take the scope of the member */
            idl_fileOutPrintf(idl_fileCur(), "    tscope = scope;\n");
        }
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tarray) {
	/* The union case is of array type (inline) */
        idl_fileOutPrintf(idl_fileCur(), "    {\n");
        idl_fileOutPrintf(idl_fileCur(), "        c_metaObject p;\n");
        idl_fileOutPrintf(idl_fileCur(), "\n");
	/* Generate code for loading array meta data */
        idl_arrayLoad(scope, typeSpec, 1);
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(typeSpec) == idl_tseq) {
	/* The union case is of array type (inline) */
        idl_fileOutPrintf(idl_fileCur(), "    {\n");
        idl_fileOutPrintf(idl_fileCur(), "        c_metaObject %c;\n", IDL_INDEX_VAR+1 /* 'p' */);
        idl_fileOutPrintf(idl_fileCur(), "\n");
	/* Generate code for loading sequence meta data */
        idl_seqLoad(scope, typeSpec, 1);
        idl_fileOutPrintf(idl_fileCur(), "    }\n");
    } else {
        /* The union case is of other user type */
        /* QAC EXPECT 3416; No side effect here */
        if (idl_scopeEqual(scope, idl_typeUserScope(idl_typeUser(typeSpec))) == FALSE) {
	    /* If the union case type is defined outside the union, then load the metadata of that type */
            idl_fileOutPrintf(
                idl_fileCur(),
                "    tscope = __%s__load (base)->definedIn;\n",
                idl_scopedTypeName(typeSpec));
        } else {
            /* If the union case type is defined inside the union, then load the metadata of that type */
            idl_fileOutPrintf(
                idl_fileCur(),
                "    tscope = __%s__load (o)->definedIn;\n",
                idl_scopedTypeName(typeSpec));
        }
    }
    /* Resolve the type of the union case within the type scope and assign it to the
       cases[case_index] meta object type attribute.
    */
    idl_fileOutPrintf(
        idl_fileCur(),
        "    c_specifier(cases[%d])->type = ResolveType(tscope,\"%s\");\n",
        case_index,
        idl_typeFromTypeSpec(typeSpec));
    /* Assign the labels meta data specification to the labels cases[case_index] attribute */
    idl_fileOutPrintf(
        idl_fileCur(),
        "    c_unionCase(cases[%d])->labels = labels;\n",
        case_index);
    /* Next union case maps on the next cases array element */
    case_index++;
}

/** @brief callback function called on definition of the union case labels in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
   =>       case label1.1; .. case label1.n;
                <union-case-1>;
   =>       case label2.1; .. case label2.n;
                ...        ...
   =>       case labeln.1; .. case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
        };
   @endverbatim
 *
 * The function generates code to assign the labels variable an
 * array with the required length to hold all union case labels.
 *
 * @param scope Current scope (the union the labels are defined in)
 * @param labelSpec Specifies the number of labels of the union case
 */
static void
idl_unionLabelsOpenClose(
    idl_scope scope,
    idl_labelSpec labelSpec,
    void *userData)
{
    /* QAC EXPECT 3416; No side effect here */
    if (idl_labelSpecNoLabels(labelSpec)) {
        /* If more than 0 labels, the create labels array */
        idl_fileOutPrintf(
            idl_fileCur(),
            "    labels = c_arrayNew(ResolveType(base,\"c_object\"),%d);\n",
            idl_labelSpecNoLabels(labelSpec));
    } else {
        /* No labels will be specified (default union case) */
        idl_fileOutPrintf(idl_fileCur(), "    labels = NULL;\n");
    }
    /* The first label maps on labels[0] */
    label_index = 0;
}

/** @brief callback function called on definition of a union case label in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
        union <union-name> switch(<switch-type>) {
   =>       case label1.1;
   =>       ..         ..
   =>       case label1.n;
                <union-case-1>;
   =>       case label2.1;
   =>       ..         ..
   =>       case label2.n;
                ...        ...
   =>       case labeln.1;
   =>       ..         ..
   =>       case labeln.n;
                <union-case-n>;
            default:
                <union-case-m>;
        };
   @endverbatim
 *
 * The generated code creates a literal meta object for the labels
 * indexed by label_index. After that the value of the label is set.
 *
 * @param scope Current scope (the union the label is defined in)
 * @param labelVal Specifies the value of the label
 */
static void
idl_unionLabelOpenClose(
    idl_scope scope,
    idl_labelVal labelVal,
    void *userData)
{
    if (idl_labelValType(labelVal) != idl_ldefault) {
        /* For the label create an M_LITERAL meta object and assign it to labels[label_index] */
        idl_fileOutPrintf(idl_fileCur(), "    labels[%d] = (c_voidp)c_metaDefine(scope,M_LITERAL);\n",
            label_index);
	/* Assign the label its value */
        idl_fileOutPrintf(idl_fileCur(), "    c_literal(labels[%d])->value = %s;\n",
            label_index, idl_valueFromLabelVal(labelVal));
    }
    /* Next label maps on the next labels array element */
    label_index++;
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
 * The function first generates a function to load the enumeration meta data.
 * Depending of the scope of the enumeration it will be a global or a static function.
 *
 * The generated function first declares a meta data enumeration definition for the
 * enumeration, its scope is set and an array is allocated to hold the enumeration
 * elements.
 *
 * An array is allocated to hold the the of elements of the enumeration.
 *
 * @param scope Current scope
 * @param name Name of the enumeration
 * @param enumSpec Specifies the number of elements in the enumeration
 * @return Next action for this enumeration (idl_explore)
 */
static idl_action
idl_enumerationOpen(
    idl_scope scope,
    const char *name,
    idl_typeEnum enumSpec,
    void *userData)
{
    /* QAC EXPECT 3416; No side effect here */
    if ((idl_scopeStackSize(scope) == 0) || (idl_scopeElementType (idl_scopeCur(scope)) == idl_tModule)) {
	/* Enumeration is defined within the global scope or a module */
        idl_fileOutPrintf(idl_fileCur(), "c_metaObject\n");
        idl_fileOutPrintf(idl_fileCur(), "__%s__load (\n", idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "    c_base base)\n");
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject scope;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_array elements;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject o;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject found;\n");
        idl_fileOutPrintf(idl_fileCur(), "\n");
        /* Assign the correct scope to the scope variable */
        idl_scopePrint(scope);
    } else {
        /* Enumeration is defined within a struct or a union, the load function
           is defined static because it is only locally used.
        */
        idl_fileOutPrintf(idl_fileCur(), "static c_metaObject\n");
        idl_fileOutPrintf(idl_fileCur(), "__%s__load (\n", idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject scope)\n");
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_array elements;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject o;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject found;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_base base = c_getBase(scope);\n");
        idl_fileOutPrintf(idl_fileCur(), "\n");
    }
    /* Define an M_ENUMERATION meta object */
    idl_fileOutPrintf(idl_fileCur(), "    o = c_metaObject(c_metaDefine(scope,M_ENUMERATION));\n");
    /* Specify the scope of the enumeration */
    idl_fileOutPrintf(idl_fileCur(), "    c_metaObject(o)->definedIn = scope;\n");
    /* Create an array to hold the enumeration elements */
    idl_fileOutPrintf(idl_fileCur(), "    elements = c_arrayNew(ResolveType(base,\"c_object\"),%d);\n",
	idl_typeEnumNoElements(enumSpec));
    /* The first enumeration element maps on elements[0] */
    element_index = 0;

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
 * The function finalizes the meta data definition of the enumeration 
 * by assigning the elements, then finalizing the enumeration.
 *
 * The function then again finds the enumeration in the meta data.
 * If found equals o, the enumeration was not yet defined.
 * If found does not equal o, it was already defined and has the
 * same definition.
 * If found = NULL, it was already defined with a different definition.
 *
 * @param name Name of the enumeration
 */
static void
idl_enumerationClose(
    const char *name,
    void *userData)
{
    /* Assign the enumeration elements to the enumerations elements attribute */
    idl_fileOutPrintf(idl_fileCur(), "    c_enumeration(o)->elements = elements;\n");
    /* Finalize the meta object (enumeration) */
    idl_fileOutPrintf(idl_fileCur(), "    if (c_metaFinalize(o) == S_ACCEPTED) {\n");
    /* Find the enumeration meta object again */
    idl_fileOutPrintf(idl_fileCur(), "        found = c_metaBind(scope,\"%s\",o);\n",name);
    /* If the enumeration was not accepted then initiate rollback procedure. */
    idl_fileOutPrintf(idl_fileCur(), "    } else {\n");
    idl_fileOutPrintf(idl_fileCur(), "        found = NULL;\n");
    idl_fileOutPrintf(idl_fileCur(), "    }\n");
    /* Free the original enumeration meta object */
    idl_fileOutPrintf(idl_fileCur(), "    c_free(o);\n");
    idl_fileOutPrintf(idl_fileCur(), "    return found;\n");
    idl_fileOutPrintf(idl_fileCur(), "}\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
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
 * The function declares a meta data constant definition for the
 * enumeration element and assigns it to the elements indexed by
 * element_index.
 *
 * @param scope Current scope
 * @param name Name of the enumeration element
 */
static void
idl_enumerationElementOpenClose(
    idl_scope scope,
    const char *name,
    void *userData)
{
    /* Define an M_CONSTANT meta object and assign its reference to elements[element_index] */
    idl_fileOutPrintf(idl_fileCur(), "    elements[%d] = (c_voidp)c_metaDeclare(scope,\"%s\",M_CONSTANT);\n",
        element_index, idl_languageId(name));
    /* Next enumeration element maps on the next elements array element */
    element_index++;
}

/** @brief this function handles definition of arrays in the IDL input file.
 *
 * @verbatim
   Generate code for the following IDL construct:
   =>   <type> <name>[<size-0>]..[<size-n>];
   @endverbatim
 *
 * The function generates code to load meta data describing an "n+1" dimensional array.
 * An array is always defined in the context of a typedef, a struct or a union.
 * The typedef, struct or union prepares the context for the generated code.
 * This implies that the typedef, struct or union must define an "c_metaObject var"
 * within a limited scope, where "var" = 'o'+"indent".
 * "indent" functions as an index for the actual dimension.
 *
 * The first task of this function is to generate code to define a collection meta
 * object of type array and define the scope of it.
 * After that the generated code determines the scope of the subtype of the array.
 * Based on the scope, the subtype meta description is resolved.
 * Finally the array size is set and the array is finalized.
 *
 * @param scope Current scope
 * @param typeSpec Specifies the type of the array
 * @param indent Specifies the index for the dimension of the array
 */
static void
idl_arrayLoad(
    idl_scope scope,
    idl_typeSpec typeSpec,
    c_long indent)
{
    idl_scopeType sType;
    idl_typeSpec arrayType = idl_typeArrayType(idl_typeArray(typeSpec));

    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(typeSpec) == idl_tarray) {
	/* The array is of another array type (next dimension) */
        idl_arrayLoad(scope, arrayType, indent);
        idl_printIndent(indent);
	/* Define an M_COLLECTION meta object to hold the array and assign it to var 'p'+<indent> */
        idl_fileOutPrintf(idl_fileCur(), "    %c = c_metaObject(c_metaDefine(scope,M_COLLECTION));\n",
            IDL_INDEX_VAR+indent);
        idl_printIndent(indent);
	/* Define the scope of the array */
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject(%c)->definedIn = scope;\n",
            IDL_INDEX_VAR+indent);
        idl_printIndent(indent);
	/* Set the collection kind attribute to C_ARRAY */
        idl_fileOutPrintf(idl_fileCur(), "    c_collectionType(%c)->kind = C_ARRAY;\n",
            IDL_INDEX_VAR+indent);
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeSpecType(idl_typeSpec(arrayType)) == idl_tarray) {
	       /* If the array is of an array type again, set tscope to scope */
            idl_printIndent(indent);
            idl_fileOutPrintf(idl_fileCur(), "    tscope = scope;\n");
            /* QAC EXPECT 3416; No side effect here */
        } else if (idl_typeSpecType(idl_typeSpec(arrayType)) == idl_tbasic) {
            /* If the array is of a basic type */
            idl_printIndent(indent);
            /* QAC EXPECT 3416; No side effect here */
            if (idl_typeBasicMaxlen(idl_typeBasic(arrayType)) > 0) {
                /* This is a bounded string */
                /* QAC EXPECT 3416; No side effect here */
                if (idl_scopeStackSize(idl_typeUserScope(idl_typeUser(typeSpec))) == 0) {
#if 0
                    /* If the bounded string is defined on global scope then set tscope scope */
                    idl_fileOutPrintf(idl_fileCur(), "    tscope = scope;\n");
#else
    	            idl_fileOutPrintf(idl_fileCur(), "    tscope = __%s_%d__load (o)->definedIn;\n",
	       	        idl_scopeStack(scope, "_", NULL),
                    idl_typeBasicMaxlen(idl_typeBasic(arrayType)));
#endif
                } else {
                    sType = idl_scopeElementType(idl_scopeCur(idl_typeUserScope(idl_typeUser(arrayType))));
                    /* QAC EXPECT 3416; No side effect here */
                    if ((sType == idl_tStruct) || (sType == idl_tUnion)) {
            /* If the bounded string is defined within a structure or union then load
			   the structure or union metadata. The structure or union is the scope of
			   the bounded string */
                        idl_fileOutPrintf(
                            idl_fileCur(),
                            "    tscope = __%s_%d__load (o)->definedIn;\n",
                            idl_scopeStack(scope, "_", NULL),
                            idl_typeBasicMaxlen(idl_typeBasic(idl_typeArrayType(idl_typeArray(typeSpec)))));
          		    } else {
			/* If the bounded string is defined within an other user type then load
			   the user type metadata. The user type scope is the scope of the bounded string */
                        idl_fileOutPrintf(
                            idl_fileCur(),
                            "    tscope = __%s_%d__load (scope)->definedIn;\n",
                            idl_scopeStack (scope, "_", NULL),
                            idl_typeBasicMaxlen(idl_typeBasic(idl_typeArrayType(idl_typeArray(typeSpec)))));
                    }
                }
            } else {
		/* For other basic types, set tscope to scope */
                idl_fileOutPrintf(idl_fileCur(), "    tscope = scope;\n");
            }
            /* QAC EXPECT 3416; No side effect here */
        } else if (idl_typeSpecType(idl_typeSpec(arrayType)) == idl_tseq) {
	    /* If the array is of an sequence type, prepare a sequence context */
            indent++;
            idl_fileOutPrintf(idl_fileCur(), "        {\n");
            idl_fileOutPrintf(idl_fileCur(), "            c_metaObject %c;\n", IDL_INDEX_VAR+indent);
            idl_fileOutPrintf(idl_fileCur(), "\n");
	    /* Handle the sequence */
            idl_seqLoad(scope, idl_typeSpec(arrayType), indent);
            idl_fileOutPrintf(idl_fileCur(), "        }\n");
            indent--;
        } else {
            /* The array is of another user type, load the meta data of the user type and set tscope accordingly */
            idl_printIndent(indent);
            /* QAC EXPECT 3416; No side effect here */
            if (idl_scopeStackSize(idl_typeUserScope(idl_typeUser(arrayType))) == 0) {
                /* The user type is defined on global scope */
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "    tscope = __%s__load (base)->definedIn;\n",
                    idl_scopedTypeName(idl_typeArrayType(idl_typeArray(typeSpec))));
            } else {
                sType = idl_scopeElementType(idl_scopeCur(idl_typeUserScope(idl_typeUser(arrayType))));
                if ((sType == idl_tStruct) || (sType == idl_tUnion)) {
                    /* The user type is defined within a structure or a union */
                    idl_fileOutPrintf(
                        idl_fileCur(),
                        "    tscope = __%s__load (o)->definedIn;\n",
                        idl_scopedTypeName(idl_typeArrayType(idl_typeArray(typeSpec))));
                } else {
                    idl_fileOutPrintf(
                        idl_fileCur(),
                        "    tscope = __%s__load (base)->definedIn;\n",
                        idl_scopedTypeName(idl_typeArrayType(idl_typeArray(typeSpec))));
                }
            }
        }
        idl_printIndent(indent);
	/* Resolve the subtype of the array based upon the determined scope of the subtype and set the
	   arrays subType attribute
	*/
        idl_fileOutPrintf(idl_fileCur(), "    c_collectionType(%c)->subType = ResolveType(tscope,\"%s\");\n",
    	    IDL_INDEX_VAR+indent,
	        idl_typeSpecName(arrayType));
        idl_printIndent(indent);
	/* Set the size of the array */
        idl_fileOutPrintf(idl_fileCur(), "    c_collectionType(%c)->maxSize = %d;\n",
	        IDL_INDEX_VAR+indent,
	        idl_typeArraySize(idl_typeArray(typeSpec)));
        idl_printIndent(indent);
	/* Finalize the array */
        idl_fileOutPrintf(idl_fileCur(), "    c_metaFinalize(%c);\n",
            IDL_INDEX_VAR+indent);
            idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    c_free(c_metaBind(tscope,\"%s\",%c));\n",
            idl_typeSpecName(typeSpec),
    	    IDL_INDEX_VAR+indent);
        idl_printIndent(indent);
        idl_fileOutPrintf(idl_fileCur(), "    c_free(%c);\n\n",
    	    IDL_INDEX_VAR+indent);
    }
}

/** @brief this function handles definition of sequences in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   sequence <<type>[,<size>]> <name>;
   @endverbatim
 *
 * The function generates code to load meta data describing a sequence.
 * A sequence is always defined in the context of a typedef, a struct or a union.
 * The typedef, struct or union prepares the context for the generated code.
 * This implies that the typedef, struct or union must define an "c_metaObject var"
 * within a limited scope, where "var" = 'o'+"indent".\n
 * Sequences can be defined anonymous:
 * @verbatim
        sequence<sequence<<type>[,<size>]>[,<size>]> <name>.
   @endverbatim
 * Such a construct can be seen as a multi dimensional or recursive sequence where "indent"
 * functions as an index for the actual dimension or recursion level.
 *
 * The first task of this function is to generate code to define a collection meta
 * object of type sequence and define the scope of it.
 * After that the generated code determines the scope of the subtype of the sequence.
 * Based on the scope, the subtype meta description is resolved.
 * Finally the sequence size is set (0 specifies an unbounded sequence) and the
 * sequence is finalized.
 *
 * @param scope Current scope
 * @param typeSpec Specifies the type and the size of the sequence
 * @param indent Specifies the index for the recursion level of the sequence
 */
static void
idl_seqLoad(
    idl_scope scope,
    idl_typeSpec typeSpec,
    c_long indent)
{
    sc_scoping scoping;
    idl_scopeType sType;
    idl_typeSpec seqType = idl_typeSeqType(idl_typeSeq(typeSpec));
    c_long scope_index;
    c_long subType_defined;
    c_long i;

    sType = idl_scopeElementType(idl_scopeCur(scope));
    if ((sType == idl_tStruct) || (sType == idl_tUnion)) {
	/* The sequence is defined within a structure or union, therefor the
           scope is the structure or union.
	*/
        scoping = sc_struct;
    } else {
	/* The sequence is not defined within a structure or union, therefor the
           scope is defined by the "scope" variable.
	*/
        scoping = sc_scope;
    }
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(typeSpec) == idl_tseq) {
        /* The sequence type is of another sequence (next dimension) */
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeSpecType(seqType) == idl_tseq) {
	    /* If the subtype of that sequence is again a sequence */
            indent++;
            idl_fileOutPrintf(idl_fileCur(), "    {\n");
            idl_fileOutPrintf(idl_fileCur(), "        c_metaObject %c;\n", IDL_INDEX_VAR+indent);
            idl_fileOutPrintf(idl_fileCur(), "\n");
            idl_seqLoad(scope, seqType, indent);
            idl_fileOutPrintf(idl_fileCur(), "    }\n");
            indent--;
        }
        idl_printIndent(indent);
	/* Define a collection meta data object for the sequence */
        idl_fileOutPrintf(idl_fileCur(), "    %c = c_metaObject(c_metaDefine(%s,M_COLLECTION));\n",
    	    IDL_INDEX_VAR+indent,
    	    scopeName[scoping]);
        idl_printIndent(indent);
	/* Set the scope of the sequence */
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject(%c)->definedIn = %s;\n",
    	    IDL_INDEX_VAR+indent,
    	    scopeName[scoping]);
        idl_printIndent(indent);
	/* Set the collection kind attribute to C_SEQUENCE */
        idl_fileOutPrintf (idl_fileCur(), "    c_collectionType(%c)->kind = C_SEQUENCE;\n",
    	    IDL_INDEX_VAR+indent);
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeSpecType(idl_typeSpec(seqType)) == idl_tseq) {
	    /* If the subtype of the sequence is a sequence then determine the scope of
	       the subtype and resolve the type to set the subtype of the sequence
	    */
            idl_printIndent(indent);
    	    idl_fileOutPrintf(idl_fileCur(), "    tscope = %s;\n", scopeName[scoping]);
            idl_printIndent(indent);
            idl_fileOutPrintf (idl_fileCur(), "    c_collectionType(%c)->subType = ResolveType(tscope,\"%s\");\n",
                IDL_INDEX_VAR+indent,
	        idl_typeSpecName(idl_typeSeqType(idl_typeSeq(typeSpec))));
            /* QAC EXPECT 3416; No side effect here */
        } else if (idl_typeSpecType(idl_typeSpec(seqType)) == idl_tbasic) {
            /* The sequence subtype is of basic type */
            idl_printIndent(indent);
            /* QAC EXPECT 3416; No side effect here */
            if (idl_typeBasicMaxlen(idl_typeBasic(idl_typeSpec(seqType))) > 0) {
            /* The subtype is a bounded sequence which is loaded to determine its scope */
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "    tscope = __%s_%d__load (%s)->definedIn;\n",
                    idl_scopeStack(scope, "_", NULL),
                    idl_typeBasicMaxlen(idl_typeBasic(idl_typeSpec(seqType))),
                    scopeName[scoping]);
            } else {
                /* The subtype scope is defined by "scope" variable */
    	        idl_fileOutPrintf(idl_fileCur(), "    tscope = scope;\n");
    	    }
            idl_printIndent(indent);
	    /* Resolve the subtype based upon the determined subtype scope and set the collections subType attribute */
            idl_fileOutPrintf(
                idl_fileCur(),
                "    c_collectionType(%c)->subType = ResolveType(tscope,\"%s\");\n",
                IDL_INDEX_VAR+indent,
                idl_typeSpecName(idl_typeSpec(seqType)));
        } else {
            /* The sequence is of another user type */

            /* check for recursive single or multi level definition */
            scope_index = idl_scopeStackSize(scope) - 1;
            subType_defined = 0;
            while ((scope_index >= 0) && 
                   ((idl_scopeElementType(idl_scopeIndexed(scope,scope_index)) == idl_tStruct) ||
                   (idl_scopeElementType(idl_scopeIndexed(scope,scope_index)) == idl_tUnion))) {
                /* QAC EXPECT 3416; No side effect here */
                if (strcmp(idl_typeSpecName(idl_typeSpec(seqType)),
                           idl_scopeElementName(idl_scopeIndexed(scope, scope_index))) == 0) {
                    subType_defined = 1;
                    /* QAC EXPECT 3416; No side effect here */
                    if (scope_index == (idl_scopeStackSize(scope) - 1)) {
                        /* single level recursive definition */
                        idl_printIndent(indent);
                        idl_fileOutPrintf(
                            idl_fileCur(),
                            "    c_collectionType(%c)->subType = c_type(c_keep(c_object(o)));\n",
                            IDL_INDEX_VAR+indent);
                    } else {
                        /* multi level recursive definition */
                        idl_printIndent(indent);
                        idl_fileOutPrintf(
                            idl_fileCur(),
                            "    c_collectionType(%c)->subType = c_type(c_keep(c_object(o",
                            IDL_INDEX_VAR+indent);
                        for (i = 0; i < ((idl_scopeStackSize(scope) - 1) - scope_index); i++) {
                            idl_fileOutPrintf (idl_fileCur(), "->definedIn");
                        }
                        idl_fileOutPrintf (idl_fileCur(), ")));\n");
                    }
                }
                scope_index--;
            }
            if (subType_defined == 0) {
                idl_printIndent(indent);
                if (((sType == idl_tStruct) || (sType == idl_tUnion)) && 
                    idl_scopeEqual(scope, idl_typeUserScope(idl_typeUser(seqType))) == TRUE) {
                    idl_fileOutPrintf(
                        idl_fileCur(),
                        "    tscope = __%s__load (o)->definedIn;\n",
                        idl_scopedTypeName (seqType));
                } else {
                    idl_fileOutPrintf(
                        idl_fileCur(),
                        "    tscope = __%s__load (base)->definedIn;\n",
                        idl_scopedTypeName (seqType));
                }
                idl_printIndent(indent);
                idl_fileOutPrintf(
                    idl_fileCur(),
                    "    c_collectionType(%c)->subType = ResolveType(tscope,\"%s\");\n",
                    IDL_INDEX_VAR+indent,
                    idl_typeSpecName(idl_typeSeqType(idl_typeSeq(typeSpec))));
            }
        }
        idl_printIndent(indent);
	/* Set the size of the sequence */
        idl_fileOutPrintf(
            idl_fileCur(),
            "    c_collectionType(%c)->maxSize = %d;\n",
            IDL_INDEX_VAR+indent,
            idl_typeSeqMaxSize(idl_typeSeq(typeSpec)));
        idl_printIndent(indent);
	/* Finalize the sequence */
        idl_fileOutPrintf(
            idl_fileCur(),
            "    c_metaFinalize(%c);\n\n",
            IDL_INDEX_VAR+indent);
        idl_printIndent(indent);
        idl_fileOutPrintf(
            idl_fileCur(),
            "    c_free(c_metaBind(%s,\"%s\",%c));\n",
            scopeName[scoping],
            idl_typeSpecName(typeSpec),
            IDL_INDEX_VAR+indent);
        idl_printIndent(indent);
        idl_fileOutPrintf(
            idl_fileCur(),
            "    c_free(%c);\n\n",
            IDL_INDEX_VAR+indent);
        idl_printIndent(indent);
        idl_fileOutPrintf (idl_fileCur(), "    tscope = %s;\n", scopeName[scoping]);
    }
}

/** @brief callback function called on definition of a named type in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   typedef <type-name> <name>;
   @endverbatim
 *
 * The function of typedefOpenClose is to generate code to download the
 * meta data of an type definiton.
 *
 * For the type definition, a load function is first defined. Depending on
 * the scope of the type definition the function will be global or static.
 * For an array or sequence type definition, the specific array or sequence
 * generate function is called and complemented with the type definition
 * meta data. For other types the code is is fully generated.
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
    /* QAC EXPECT 3416; No side effect here */
    if ((idl_scopeStackSize(scope) == 0) || (idl_scopeElementType(idl_scopeCur(scope)) == idl_tModule)) {
	/* The type is defined within the global scope or within a module,
	   therfore define a global function.
	*/
        idl_fileOutPrintf(idl_fileCur(), "c_metaObject\n");
        idl_fileOutPrintf(idl_fileCur(), "__%s__load (\n", idl_scopeStack(scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "    c_base base)\n");
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject scope;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject tscope;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject o;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject found;\n");
        idl_fileOutPrintf(idl_fileCur(), "\n");
        idl_scopePrint(scope);
    } else {
	/* The type is defined within another type, therfore define a static function */
        idl_fileOutPrintf(idl_fileCur(), "static c_metaObject\n");
        idl_fileOutPrintf(idl_fileCur(), "__%s__load (\n", idl_scopeStack (scope, "_", name));
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject scope)\n");
        idl_fileOutPrintf(idl_fileCur(), "{\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject tscope;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject o;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject found;\n");
        idl_fileOutPrintf(idl_fileCur(), "    c_base base = c_getBase(scope);\n");
        idl_fileOutPrintf(idl_fileCur(), "\n");
        idl_scopePrint(scope);
    }
    /* QAC EXPECT 3416; No side effect here */
    if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tarray) {
        /* If the typedef is an array definition */
        idl_arrayLoad (scope, idl_typeDefRefered(defSpec), 0);
        /* Define an M_TYPEDEF meta object */
        idl_fileOutPrintf(idl_fileCur(), "    o = c_metaObject(c_metaDefine(scope,M_TYPEDEF));\n");
        /* Set the scope of the type definition */
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject(o)->definedIn = scope;\n");
        idl_fileOutPrintf (idl_fileCur(), "    tscope = scope;\n");
	/* Resolve the subtype of the type definition based on its scope and set the alias attribute
	   of the type definition
	*/
        idl_fileOutPrintf(idl_fileCur(), "    c_typeDef(o)->alias = ResolveType(tscope,\"%s\");\n",
	    idl_typeSpecName(idl_typeDefRefered(defSpec)));
        /* QAC EXPECT 3416; No side effect here */
    } else if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tseq) {
        /* If the typedef is an sequence definition */
        idl_seqLoad(scope, idl_typeDefRefered(defSpec), 0);
        /* Define an M_TYPEDEF meta object */
        idl_fileOutPrintf(idl_fileCur(), "    o = c_metaObject(c_metaDefine(scope,M_TYPEDEF));\n");
        /* Set the scope of the type definition */
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject(o)->definedIn = scope;\n");
        idl_fileOutPrintf(idl_fileCur(), "    tscope = scope;\n");
	/* Resolve the subtype of the type definition based on its scope and set the alias attribute
	   of the type definition
	*/
        idl_fileOutPrintf(
            idl_fileCur(),
            "    c_typeDef(o)->alias = ResolveType(tscope,\"%s\");\n",
            idl_typeSpecName(idl_typeDefRefered(defSpec)));
    } else {
        /* If the typedef is of a basic type or another user type */
        /* Define an M_TYPEDEF meta object */
        idl_fileOutPrintf(idl_fileCur(), "    o = c_metaObject(c_metaDefine(scope,M_TYPEDEF));\n");
        /* Set the scope of the type definition */
        idl_fileOutPrintf(idl_fileCur(), "    c_metaObject(o)->definedIn = scope;\n");
        /* QAC EXPECT 3416; No side effect here */
        if (idl_typeSpecType(idl_typeDefRefered(defSpec)) == idl_tbasic) {
	    /* If the subtype is of a basic type */
            /* QAC EXPECT 3416; No side effect here */
            if (idl_typeBasicMaxlen(idl_typeBasic(idl_typeDefRefered(defSpec))) > 0) {
                /* This is a bounded string which must be loaded in order to determine its scope */
    	        idl_fileOutPrintf(
                    idl_fileCur(),
                    "    tscope = __%s_%d__load (scope)->definedIn;\n",
                    idl_scopeStack(scope, "_", NULL),
                    idl_typeBasicMaxlen(idl_typeBasic(idl_typeDefRefered(defSpec))));
            } else {
                idl_fileOutPrintf(idl_fileCur(), "    tscope = scope;\n");
            }
        } else {
            /* This is a user type which must be loaded in order to determine its scope */
            idl_fileOutPrintf(
                idl_fileCur(),
                "    tscope = __%s__load (base)->definedIn;\n",
                idl_scopedTypeName (idl_typeSpec(idl_typeDefRefered(defSpec))));
        }
        /* Resolve the subtype of the type definition based on its scope and set the alias attribute
           of the type definition
        */
        idl_fileOutPrintf(idl_fileCur(), "    c_typeDef(o)->alias = ResolveType(tscope,\"%s\");\n",
	    idl_typeFromTypeSpec(idl_typeSpec(idl_typeDefRefered(defSpec))));
    } 
    /* Finalize the type definition */
    idl_fileOutPrintf(idl_fileCur(), "    c_metaFinalize(o);\n");
    idl_fileOutPrintf(idl_fileCur(), "    found = c_metaBind(scope,\"%s\",o);\n",name);
    idl_fileOutPrintf(idl_fileCur(), "    c_free(o);\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
    idl_fileOutPrintf(idl_fileCur(), "    return found;\n");
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}

/** @brief callback function called on definition of a bounded string in the IDL input file.
 *
 * Generate code for the following IDL construct:
 * @verbatim
   =>   string<string-len>;
   @endverbatim
 *
 *  This function generates the meta data load function for a bounded string.
 *  In order the determine a unique function name the following convention
 *  is applied:
 *      __<full-scope-name>_<string-length>__load
 *  The full scope name is used because bounded string are user defined and
 *  equal definitions within different scopes are not assignable.
 *
 * @param scope Current scope
 * @param typeBasic Specifies the size of the bounded string
 */
void
idl_boundedStringOpenClose(
    idl_scope scope,
    idl_typeBasic typeBasic,
    void *userData)
{
    idl_fileOutPrintf(idl_fileCur(), "static c_metaObject\n");
    idl_fileOutPrintf(idl_fileCur(), "__%s_%d__load (\n",
	idl_scopeStack(scope, "_", NULL),
	idl_typeBasicMaxlen(typeBasic));
    idl_fileOutPrintf(idl_fileCur(), "    c_metaObject scope)\n");
    idl_fileOutPrintf(idl_fileCur(), "{\n");
    idl_fileOutPrintf(idl_fileCur(), "    c_metaObject o;\n");
    idl_fileOutPrintf(idl_fileCur(), "    c_metaObject found;\n");
    idl_fileOutPrintf(idl_fileCur(), "\n");
    /* Define an M_COLLECTION meta object */
    idl_fileOutPrintf(idl_fileCur(), "    o = c_metaObject(c_metaDefine(scope,M_COLLECTION));\n");
    /* Set the collection kind attribute to C_STRING */
    idl_fileOutPrintf(idl_fileCur(), "    c_collectionType(o)->kind = C_STRING;\n");
    /* Resolve the "c_char" type as being the subtype of the collection */
    idl_fileOutPrintf(idl_fileCur(), "    c_collectionType(o)->subType = ResolveType(scope,\"c_char\");\n");
    /* Set the size of the collection based on the bound of the string */
    idl_fileOutPrintf(idl_fileCur(), "    c_collectionType(o)->maxSize = %d;\n",
	idl_typeBasicMaxlen(typeBasic));
    /* Set the scope of the bounded string */
    idl_fileOutPrintf(idl_fileCur(), "    c_metaObject(o)->definedIn = scope;\n");
    /* Finalize the bounded string */
    idl_fileOutPrintf(idl_fileCur(), "    c_metaFinalize(o);\n");
    idl_fileOutPrintf(idl_fileCur(), "    found = c_metaBind(scope,\"C_STRING<%d>\",o);\n",
	idl_typeBasicMaxlen(typeBasic));
    idl_fileOutPrintf(idl_fileCur(), "    c_free(o);\n\n");
    idl_fileOutPrintf(idl_fileCur(), "    return found;\n");
    idl_fileOutPrintf(idl_fileCur(), "}\n\n");
}

/**
 * Standard control structure to specify that inline
 * type definitions are to be processed prior to the
 * type itself in contrast with inline.
 */
static idl_programControl idl_genSpliceLoadControl = {
    idl_prior
};

/** @brief return the program control structure for the metadata generation functions.
 * @return program control structure for the metadata generation functions
 */
static idl_programControl *
idl_getControl(
    void *userData)
{
    return &idl_genSpliceLoadControl;
}

/** 
 * Specifies the callback table for the metadata generation functions.
 */
static struct idl_program
idl_genSpliceLoad = {
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
    idl_unionOpen,
    idl_unionClose,
    idl_unionCaseOpenClose,
    idl_unionLabelsOpenClose,
    idl_unionLabelOpenClose,
    idl_typedefOpenClose,
    idl_boundedStringOpenClose,
    NULL, /* idl_sequenceOpenClose */
    NULL, /* idl_constantOpenClose */
    NULL, /* idl_artificialDefaultLabelOpenClose */
    NULL  /* userData */
};

/** @brief return the callback table for the metadata generation functions.
 * @return the callback table for the metadata generation functions
 */
idl_program
idl_genSpliceLoadProgram(
    void)
{
    return &idl_genSpliceLoad;
}
