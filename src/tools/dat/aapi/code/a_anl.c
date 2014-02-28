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
 * For debugging purposes only.
 */
#define A_ANL_FORCE_COUNTER_DEC_OUTPUT  0



/**
 * \brief This file's context
 */
struct a_anlContext_s {
	FILE *out;               ///< File Pointer for standard output
	FILE *err;               ///< File Pointer for error messages
	a_lstList list;          ///< Copy of a pointer to the internal list data structure
	int showAnalyseOutput;   ///< Boolean value whether to show the analyse output (debug mode)
	int verboseOutput;       ///< Boolean value whether to show some progress information
	long scopeCounter;       ///< Used within a scopeWalkback

	c_address shmAddress;    ///< Shared Memory Start Address
	c_long shmSize;          ///< Shared Memory Size

	c_long dataSize;         ///< Computed total object size (header + data)
	
	a_stsContext stsContext; ///< Context holding and computing statistics
};





/********************************************************************
 Init, de-init
 ********************************************************************/


/**
 * \brief
 * Initialises this file's context and returns a new pointer to it.
 *
 * This operation reserves memory for the context, initialises this
 * context and returns a pointer to it. This operation must be
 * sucessfully performed before any other operation in this file.
 *
 * \param out
 * File pointer for 'normal' output, typically stdout
 *
 * \param err
 * File pointer for error messages, typically stderr
 *
 * \param list
 * Pointer to an a_lstList data structure. This list must already
 * have been created. If \a list is NULL, this operation will fail.
 *
 * \param showAnalyseOutput
 * Boolean value for specifying whether analyse output must be
 * displayed. If true (1), file pointer \a out (typically stdout)
 * will be used.
 *
 * \param verboseOutput
 * Boolean value specifying whether some brief information about the
 * analyse stage this module is in. If true (1), file pointer \a out
 * (typically stdout) will be used.
 *
 * \param stsContext
 * Pointer to a stsContext instance. If \a stsContext is NULL, the
 * operation will fail.
 *
 * \return
 * Pointer to the newly created context or NULL if anything failed.
 *
 * \note
 * Remember to de-initialise (\a a_anlDeInit) after use!
 *
 * \see
 * a_anlDeInit a_anlContext
 */
a_anlContext
a_anlInit(
	FILE *out,
	FILE *err,
	a_lstList list,
	int showAnalyseOutput,
	int verboseOutput,
	a_stsContext stsContext)
{
	a_anlContext context;
	if (list && stsContext) {
		context = a_memAlloc(sizeof(struct a_anlContext_s));
		if (context) {
			context->out               = out;
			context->err               = err;
			context->list              = list;
			context->showAnalyseOutput = showAnalyseOutput;
			context->verboseOutput     = verboseOutput;
			context->shmAddress        = 0;
			context->shmSize           = 0;
			context->dataSize          = 0;
			context->stsContext        = stsContext;
		}
	} else {
		context = NULL;
	}
	return context;
}



/**
 * \brief
 * De-initialises the context.
 *
 * This operation de-initialises the context and frees up memory.
 * 
 * \param context
 * The context to de-initialise.
 *
 * \note
 * Remember to call this function before the application terminates.
 *
 * \see a_anlInit, a_anlContext
 */
void
a_anlDeInit(
	a_anlContext context)
{
	if (context) {
		a_memFree(context);
	}
}



/********************************************************************
 
 ********************************************************************/


/**
 * \brief
 * Returns the total size of memory used by all database objects.
 *
 * This operation returns the total size of memory used by all
 * database objects.
 *
 * \param context
 * This file's context.
 *
 * \return
 * The total size of memory used by all database objects, or -1 if
 * context is NULL.
 *
 * \note
 * Be sure all database objects have been previously collected and
 * analysed.
 */
c_long
a_anlTotalDataSize(
	a_anlContext context)
{
	return context ? context->dataSize : -1;
}





/********************************************************************
 GENERAL AIDS (STATIC) - OUTPUT DEBUG INFO
 ********************************************************************/


/* Tells the user what's going on right now, but only if
 * 'verboseOutput' was requested.
 */
static void
a_anlVerboseOutString(
	a_anlContext context,
	char *msg)
{
	if (context->verboseOutput) {
		fprintf(context->out, "%s...\n", msg);
	}
}



/* Writes to output the result of updating a counter,
 * but only if verboseOutput was set.
 */
static void
a_anlVerboseOutCounterUpdateResult(
	a_anlContext context,
	char *counterTypeChar,
	c_address address,
	int updateResult)
{
	if (context->showAnalyseOutput && context->verboseOutput) {
		fprintf(context->out, ",{%s:%8.8X++%s}", counterTypeChar, (unsigned int)address, updateResult ? "" : "*FAIL*");
	}
}



static void
a_anlOutNewline(
	a_anlContext context)
{
	if (context->showAnalyseOutput) {
		fprintf(context->out, "\n");
	}
}


static void
a_anlOutObjectAddress(
	a_anlContext context,
	c_address address,
	char *aapiKind)
{
	if (context->showAnalyseOutput) {
		fprintf(context->out, "%8.8X,%1s", (unsigned int)address, aapiKind);
	}
}


static void
a_anlOutSize(
	a_anlContext context,
	c_long size)
{
	if (context->showAnalyseOutput) {
		fprintf(context->out, ",size:%2ld", (long)size);
	}
}


static void
a_anlOutLabel(
	a_anlContext context,
	char *label)
{
	if (context->showAnalyseOutput) {
		fprintf(context->out, ",%s", label);
	}
}



static void
a_anlOutBooleanvalue(
	a_anlContext context,
	char *label,
	int booleanValue)
{
	if (context->showAnalyseOutput) {
		a_anlOutLabel(context, label);
		fprintf(context->out, ":%s", booleanValue ? "TRUE" : "FALSE");
	}
}


#if 0

static void
a_anlOutCharvalue(
	a_anlContext context,
	char *label,
	char charValue)
{
	if (context->showAnalyseOutput) {
		fprintf(context->out, ",%s:\'%c\'", label, charValue);
	}
}

#endif


static void
a_anlOutStringvalue(
	a_anlContext context,
	char *label,
	char *stringValue)
{
	if (context->showAnalyseOutput) {
		fprintf(context->out, ",%s:\"%s\"", label, stringValue ? stringValue : "*NULL*");
	}
}


static void
a_anlOutPointervalue(
	a_anlContext context,
	char *label,
	void *ptrValue)
{
	if (context->showAnalyseOutput) {
		fprintf(context->out, ",%s:", label);
		if (ptrValue) {
			fprintf(context->out, "%8.8X", (unsigned int)ptrValue);
		} else {
			fprintf(context->out, "NULL");
		}
	}
}


static void
a_anlOutLongvalue(
	a_anlContext context,
	char *label,
	c_long clong)
{
	if (context->showAnalyseOutput) {
		fprintf(context->out, ",%s:%ld", label, (long)clong);
	}
}


static void
a_anlOutLongAndStringvalue(
	a_anlContext context,
	char *label,
	c_long clong,
	char *stringValue)
{
	if (context->showAnalyseOutput) {
		a_anlOutLongvalue(context, label, clong);
		fprintf(context->out, "(%s)", stringValue != NULL ? stringValue : "*NULL*");
	}
}


#if 0

static void
a_anlOutDoublevalue(
	a_anlContext context,
	char *label,
	double doubleValue)
{
	if (context->showAnalyseOutput) {
		fprintf(context->out, ",%s:%f", label, doubleValue);
	}
}

#endif


static void
a_anlOutIndexedElement(
	a_anlContext context,
	char *elementName,
	long index,
	c_address address)
{
	if (context->showAnalyseOutput) {
		fprintf(context->out, "\n  - %s#%2ld:%8.8X", elementName, index, (unsigned int)address);
	}
}




/********************************************************************
 GENERAL AIDS (STATIC) - 
 ********************************************************************/


static void
a_anlAddTypeRefs(
	a_anlContext context,
	c_address fromAddress,     // obj addr
	c_address toAddress,       // obj addr
	c_address rawFromAddress)  // hdr addr
{
	int result;
	a_lstAddRefAddrToEntrySublist(context->list, fromAddress, toAddress, L_REF_REFTOTYPE);
	result = a_lstAddRefAddrToEntrySublist(context->list, toAddress, fromAddress, L_REF_TYPEREF);
	a_anlVerboseOutCounterUpdateResult(context, "T", toAddress, result);
	if (toAddress) {
		result = a_lstRemoveOccurrence(context->list, rawFromAddress, toAddress);
		if (context->showAnalyseOutput) {
			if (!result || A_ANL_FORCE_COUNTER_DEC_OUTPUT) {
				fprintf(context->out, ",t[%8.8X->%8.8X--%s]", (unsigned int)rawFromAddress, (unsigned int)toAddress,
					result ? "" : "*FAIL*");
			}
		}
	}
}


static void
a_anlAddDataRefs(
	a_anlContext context,
	c_address fromAddress,     // obj addr
	c_address toAddress,       // obj addr
	c_address rawFromAddress)  // addr inside obj
{
	int result;
	a_lstAddRefAddrToEntrySublist(context->list, fromAddress, toAddress, L_REF_REFTODATA);
	result = a_lstAddRefAddrToEntrySublist(context->list, toAddress, fromAddress, L_REF_DATAREF);
	if (toAddress) {
		a_anlVerboseOutCounterUpdateResult(context, "D", toAddress, result);
		result = a_lstRemoveOccurrence(context->list, rawFromAddress, toAddress);
		if (context->showAnalyseOutput) {
			if (!result || A_ANL_FORCE_COUNTER_DEC_OUTPUT) {
				fprintf(context->out, ",d[%8.8X->%8.8X--%s]", (unsigned int)rawFromAddress, (unsigned int)toAddress,
					result ? "" : "*FAIL*");
			}
		}
	}
}



static void
a_anlAddUnknRef(
	a_anlContext context,
	c_address rawFromAddress,  // addr from scope (root or node)
	c_address toAddress)       // obj addr
{
	if (toAddress) {
		int result;
		result = a_lstAddRefAddrToEntrySublist(context->list, toAddress, rawFromAddress, L_REF_UNKNREF);
		a_anlVerboseOutCounterUpdateResult(context, "U", toAddress, result);
		result = a_lstRemoveOccurrence(context->list, rawFromAddress, toAddress);
		if (context->showAnalyseOutput) {
			if (!result || A_ANL_FORCE_COUNTER_DEC_OUTPUT) {
				fprintf(context->out, ",u[%8.8X->%8.8X--%s]", (unsigned int)rawFromAddress, (unsigned int)toAddress,
					result ? "" : "*FAIL*");
			}
		}
	}
}



static void
a_anlRmvUnknOcc(
	a_anlContext context,
	c_address rawFromAddress,  // addr from scope (root or node, like left, right, head, tail)
	c_address rawToAddress)    // addr to node
{
	if (rawToAddress) {
		int result;
		result = a_lstRemoveOccurrence(context->list, rawFromAddress, rawToAddress);
		if (context->showAnalyseOutput) {
			if (!result || A_ANL_FORCE_COUNTER_DEC_OUTPUT) {
				fprintf(context->out, ",u[%8.8X->%8.8X--%s]", (unsigned int)rawFromAddress, (unsigned int)rawToAddress,
					result ? "" : "*FAIL*");
			}
		}
	}
}





/********************************************************************
 GENERAL AIDS (STATIC) - OBJECT INTERROGATION
 ********************************************************************/


/**
 * \brief Checks whether \a objectName is derived from \a derivedFromName
 *
 * \param objectName Name of the (base)object to check
 * \param derivedFromName Name of the (base)object to check \a objectName
 *                        against to check if it is derived from
 *
 * \return Boolean value
 */
static int
a_anlCheckObject(
	char *objectName,
	char *derivedFromName)
{
	int result;
	if (objectName) {
		if (strcmp(objectName, derivedFromName) == 0) {
			result = 1;

		} else if ( (strcmp(objectName, "c_metaObject"    ) == 0)
		         || (strcmp(objectName, "c_specifier"     ) == 0)
	    	     || (strcmp(objectName, "c_operand"       ) == 0) ) {
			result = a_anlCheckObject("c_baseObject", derivedFromName);

		} else if ( (strcmp(objectName, "c_type"          ) == 0)
	    	     || (strcmp(objectName, "c_constant"      ) == 0)
	        	 || (strcmp(objectName, "c_module"        ) == 0)
		         || (strcmp(objectName, "c_property"      ) == 0)
		         || (strcmp(objectName, "c_operation"     ) == 0) ) {
			result = a_anlCheckObject("c_metaObject", derivedFromName);
			 
		} else if ( (strcmp(objectName, "c_structure"     ) == 0)
	    	     || (strcmp(objectName, "c_interface"     ) == 0)
	        	 || (strcmp(objectName, "c_collectionType") == 0)
		         || (strcmp(objectName, "c_primitive"     ) == 0)
		         || (strcmp(objectName, "c_enumeration"   ) == 0)
	    	     || (strcmp(objectName, "c_typeDef"       ) == 0)
	        	 || (strcmp(objectName, "c_union"         ) == 0)
		         || (strcmp(objectName, "c_fixed"         ) == 0) ) {
			result = a_anlCheckObject("c_type", derivedFromName);

		} else if ( (strcmp(objectName, "c_class"         ) == 0)
		         || (strcmp(objectName, "c_valuetype"     ) == 0) ) {
			result = a_anlCheckObject("c_interface", derivedFromName);

		} else if ( (strcmp(objectName, "c_expression"    ) == 0)
		         || (strcmp(objectName, "c_literal"       ) == 0)
	    	     || (strcmp(objectName, "c_constOperand"  ) == 0) ) {
			result = a_anlCheckObject("c_operand", derivedFromName);

		} else if ( (strcmp(objectName, "c_attribute"     ) == 0)
		         || (strcmp(objectName, "c_relation"      ) == 0) ) {
			result = a_anlCheckObject("c_property", derivedFromName);

		} else if ( (strcmp(objectName, "c_member"        ) == 0)
		         || (strcmp(objectName, "c_unionCase"     ) == 0)
	    	     || (strcmp(objectName, "c_parameter"     ) == 0) ) {
			result = a_anlCheckObject("c_specifier", derivedFromName);

		} else if ( (strcmp(objectName, "c_base"          ) == 0) ) {
			result = a_anlCheckObject("c_module", derivedFromName);

		} else if ( (strcmp(objectName, "c_exception"     ) == 0) ) {
			result = a_anlCheckObject("c_structure", derivedFromName);

		} else {
			result = 0;      // objectName not recognised
		}
	} else {
		result = 0;
	}
	return result;
}


/* Check if c_object o is derived from c_baseObject.
 * Assumption: c_getType(o) = c_class!
 */
static int
a_anlIsBaseObject(
	c_object object)
{
	return object ? a_anlCheckObject(c_metaObject(object)->name, "c_baseObject") : 0;
}


/**
 * \brief Counts the number of steps it takes to get from
 *        \a object to \a c_class, the bootstrap.
 *
 * \return The number of steps it took to get to the root
 *         class c_class. If the result is 0, \a object
 *         is the root class. Typically, result should
 *         not be greater than 3.
 */
static int
a_anlStepsToC_Class(
	c_object object)
{
	int result = 0;
	if (object) {
		c_type type = c_getType(object);
		if (type) {
			result = (object == (c_object)type) ? 0 : 1 + a_anlStepsToC_Class((c_object)type);
		}
	}
	return result;
}



/**
 * \brief Determines the kind of an object
 *
 * \note (Uses the L_* enum type from a_lst.h)
 *
 * \note
 *  We distinguish 4 types:
 *    - L_CLSS  "C"
 *    - L_BASE  "B"
 *    - L_META  "M"
 *    - L_DATA  "D"
 *  (and a fifth:
 *    - L_UNDF  "?")
 *  
 * These types can be defined as:
 *    - object = L_UNDF if its type = NULL
 *    - object = L_CLSS if object == type
 *    - object = L_BASE if type == L_CLSS and if object is derived from c_metaObject
 *    - object = L_META if type == L_CLSS and if object is not derived from c_metaObject
 *    - object = L_META if type == L_BASE
 *    - object = L_DATA if type == L_META
 *
 * \warn
 * This definition is not watertight. Theoretically, types could
 * point to each other, creating circulair relations, in which
 * an analysis algorithm could end up in an infinite loop.
 * Up till now though, this has not occurred, so we leave it
 * this way (for now).
 */
static a_lstObjectKind
a_anlGetObjectKind(
	c_object object)
{
	a_lstObjectKind objectKind;
	if (object) {
		c_type type = c_getType(object);
		if (type) {
			switch (a_anlStepsToC_Class(object)) {
				case 3:
					objectKind = L_DATA;
					break;

				case 2:
					objectKind = (a_anlGetObjectKind(c_object(type)) == L_META) ? L_DATA : L_META;
					break;

				case 1:
					objectKind = a_anlIsBaseObject(object) ? L_BASE : L_META;
					break;
					
				case 0:
					objectKind = L_CLSS;
					break;

				default:
					assert(FALSE);
					break;
			}
		} else {
			objectKind = L_UNDF;
		}
	} else {  // object == NULL
		objectKind = L_UNDEFINED;
	}
	return objectKind;
}



/**
 * \brief Assertion statement for an object's type to predict
 *        its name (and thus the type)
 *
 * This operation will print to the \a context->err file pointer
 * (typically stderr) if \a baseObject's name is different from
 * \a checkTypeName.
 *
 * \param context This file's context
 * \param object Object holding the pointer (reference) to the
 *               type object to be checked
 * \param baseObject Type object to be checked
 * \param type baseObject's type
 * \param checkTypeName Predicted name of baseObject
 *
 * \see a_anlAssertTypeDerived
 */
static void
a_anlAssertType(
	a_anlContext context,
	c_object object,           // object that the pointer originated from
	c_baseObject baseObject,   // referenced object
	c_type type,               // referenced object's type
	char *checkTypeName)       // name to check against type's name
{
	if (type) {
		char *typeName = c_metaObject(type)->name;
		if (!typeName) {
			typeName = "*NULL*";
		}
		if ( !(strcmp(checkTypeName, typeName) == 0) ) {
			fprintf(context->err, "*Assertion failure*: ");
			fprintf(context->err, "Object at %8.8X ", (unsigned int)object);
			fprintf(context->err, "has a reference to %8.8X ", (unsigned int)baseObject);
			fprintf(context->err, "with it's type's name \"%s\" ", typeName);
			fprintf(context->err, "but \"%s\" was expected.\n", checkTypeName);
		}
	}
}



/**
 * \brief Assertion statement for an object's type to be derived
 *        from some base object (specified by name).
 *
 * This operation will print a message to the \a context->err file
 * pointer (typically stderr) if \a baseObject does not seem to be
 * derived from an object with \a checkTypeName.
 *
 * \param context This file's context
 * \param object Object holding the pointer (reference) to the
 *               type object to be checked
 * \param baseObject Type object to be checked
 * \param type baseObject's type
 * \param checkTypeName Predicted name of baseObject
 *
 * \see a_anlAssertType
 */
static void
a_anlAssertTypeDerived(
	a_anlContext context,
	c_object object,           // object that the pointer originated from
	c_baseObject baseObject,   // referenced object
	c_type type,               // referenced object's type
	char *checkTypeName)       // name to check against type's name
{
	if (type) {
		char *typeName = c_metaObject(type)->name;
		if (!typeName) {
			typeName = "*NULL*";
		}
		if ( !(a_anlCheckObject(typeName, checkTypeName)) ) {
			fprintf(context->err, "*Assertion failure*: ");
			fprintf(context->err, "Object at %8.8X ", (unsigned int)object);
			fprintf(context->err, "has a reference to %8.8X ", (unsigned int)baseObject);
			fprintf(context->err, "with it's type's name \"%s\" ", typeName);
			fprintf(context->err, "which is not derived from \"%s\", as was expected.\n", checkTypeName);
		}
	}
}




/********************************************************************
 FILL (CREATE) INITIAL LIST
 ********************************************************************/


/**
 * \brief Call back function for filling the (internal AAPI-)list
 *        with all, to SPLICE known, database objects
 *
 * \param object Object to insert
 * \param context This file's context
 *
 * \see a_anlFillList
 */
static void
a_anlFillListCallBack(
	c_object object,
	a_anlContext context)
{
	a_lstObjectKind objectKind;
	c_type type;
	c_address addr = (c_address)object;
	c_long refCount;
	char *objectName;
	char *typeDesc;
	char *typeName;
	c_long alignment;
	c_long size;
	char *addNote = "";

	objectKind = a_anlGetObjectKind(object);
	type = c_getType(object);
	if (object) {
		refCount = c_refCount(object);
		if (type) {
			alignment = type->alignment;
			size = type->size;
			typeDesc = a_utlMetaKindStr(c_baseObject(type)->kind);
			typeName = c_metaObject(type)->name;
			
			switch (objectKind) {
				case L_DATA: {
					objectName = "";
					break;
				}
				
				case L_META: {
					if (c_checkType(object, "c_metaObject")) {
						objectName = c_metaObject(object)->name;
					} else if (c_checkType(object, "c_specifier")) {
						objectName = c_specifier(object)->name;
					} else {
						objectName = "";
					}
					break;
				}
				
				case L_BASE: {
					if (c_checkType(object, "c_metaObject")) {
						objectName = c_metaObject(object)->name;
					} else {
						if (c_checkType(object, "c_specifier")) {
							objectName = c_metaObject(c_specifier(object)->type)->name;
							if (!objectName) {
								objectName = "(c_specifier subtype)";
							}
						} else {
							objectName = "(c_operand subtype)";
						}
					}
					break;
				}

				case L_CLSS: {
					objectName = c_metaObject(object)->name;   // "(c_class)";
					break;
				}

				case L_UNDF: {
					objectName = "*AAPI error: unspecified aapi type encountered*";
					break;
				}

				case L_UNDEFINED: {
					objectName = "*AAPI error: NULL pointer object*";
					break;
				}

				default: {
					break;
				}
			}
			if (!objectName) {
				objectName = "*NULL*";
				addNote = "*objName==NULL*";
			}
		} else {
			objectName = "";
			typeDesc = "*NULL*";               // unexpected NULL value of c_getType()
			typeName = "";
			addNote = "*c_getType()==NULL*";
			alignment = -1;
			size = -1;
		}
	} else {
		refCount = -1;
		objectName = "*NULL*";
	}
	
	a_lstInsertNewEntry(context->list, addr, refCount, objectKind, objectName, typeDesc, typeName, alignment, size);
	if (strlen(addNote)) {
		a_lstAddEntryNote(context->list, addr, addNote);
	}
}




/**
 * \brief
 * Fills the (internal) list with all known database objects.
 *
 * This operation fills the (internal) list with all database
 * objects.
 *
 * \param context
 * This file's context.
 *
 * \param base
 * Database that has been successfully opened.
 *
 * \note
 * This operation uses \a c_baseObjectWalk (from c_base.h)
 * internally, which is only available in Splice's development
 * version.
 *
 * \todo
 * Get this function to return an int, to specify whether this
 * operation was successful and have calling functions check
 * this result value.
 */
void
a_anlFillList(
	a_anlContext context,
	c_base base)
{
	assert(base);
	assert(context->list);
	a_anlVerboseOutString(context, "Collecting database objects");
	c_baseObjectWalk(base, (c_baseWalkAction)a_anlFillListCallBack, context);

	// Needed for collecting statistics later:
	a_stsSetObjectsCount(context->stsContext, (c_long)a_lstCountAll(context->list));
}





/********************************************************************
 GENERAL AIDS (STATIC) - 
 ********************************************************************/


static void
a_anlScopeWalkback(
	c_metaObject metaObject,
	c_scopeWalkActionArg actionArg)
{
	c_object object = c_object(metaObject);
	a_lstObjectKind objectKind  = a_anlGetObjectKind(object);
	a_anlContext context = (a_anlContext)actionArg;
	a_anlOutIndexedElement(context, "Scp", context->scopeCounter++, (c_address)object);
	a_anlOutLabel(context, a_lstGetObjectKindChar(objectKind));
	if ( (objectKind == L_CLSS) || (objectKind == L_BASE) || (objectKind == L_META) ) {
		a_anlOutStringvalue(context, "->", metaObject->name);
	}
}




/********************************************************************
 ANALYSE META-META (BASE) OBJECTS  "B"
 ********************************************************************/


static void
a_anlBaseMetaobjectMembers(
	a_anlContext context,
	c_object object,
	c_metaObject metaObject)
{
	c_metaObject definedIn = metaObject->definedIn;
	a_anlOutPointervalue(context, "definedIn", (void *)definedIn);
	if (definedIn) {
		a_anlAddDataRefs(context, (c_address)object, (c_address)definedIn, (c_address)&(metaObject->definedIn));
		a_anlAssertTypeDerived(context, object, c_baseObject(definedIn), c_getType(definedIn), "c_metaObject");
	}
	// Although 'name' is a member of c_metaObject, it is omitted here
	// because it has already been outputted
}





static void
a_anlBaseTypeMembers(
	a_anlContext context,
	c_object object,
	c_type type)
{
	c_long alignment = type->alignment;
	c_base base = type->base;

	a_anlOutLongvalue(context, "alignment", (long)alignment);
	a_anlOutPointervalue(context, "base", (void *)base);
	if (base) {
		a_anlAddDataRefs(context, (c_address)object, (c_address)base, (c_address)&(type->base));
		a_anlAssertType(context, object, c_baseObject(base), c_getType(base), "c_base");
	}

	// Although 'size' is a member of c_type, it is omitted here
	// because it has already been outputted
	
	a_anlBaseMetaobjectMembers(context, object, c_metaObject(type));
}






static void
a_anlBaseInterfaceMembers(
	a_anlContext context,
	c_object object,
	c_interface interface)
{
	c_array inherits = interface->inherits;
	c_array references = interface->references;
	c_long inhSize = c_arraySize(inherits);
	c_long refSize = c_arraySize(references);
	c_bool abstract = interface->abstract;
	c_scope scope = interface->scope;
	c_long scopeCount = c_scopeCount(scope);
	c_scopeWalkAction scopeWalkAction = a_anlScopeWalkback;
	c_interface inherit, reference;
    c_long i;
		
	a_anlOutBooleanvalue(context, "abstract", (int)abstract);
	a_anlOutPointervalue(context, "scope", (void *)scope);
	a_anlAddDataRefs(context, (c_address)object, (c_address)scope, (c_address)&(interface->scope));
	a_anlOutLongvalue(context, "scopeCount", (long)scopeCount);
	a_anlOutLongvalue(context, "#Inherits", (long)inhSize);
	a_anlOutPointervalue(context, "references", (void *)references);
	a_anlAddDataRefs(context, (c_address)object, (c_address)references, (c_address)&(interface->references));
	a_anlOutLongvalue(context, "#References", (long)refSize);

	a_anlBaseTypeMembers(context, object, c_type(interface));
	
	context->scopeCounter = 0;
	c_scopeWalk(scope, scopeWalkAction, context);

	for (i = 0; i < inhSize; i++) {
		inherit = interface->inherits[i];
		if (inherit) {
			c_type inhType = c_type(inherit);
			a_anlOutIndexedElement(context, "Inh", (long)i, (c_address)inherit);
			a_anlAddDataRefs(context, (c_address)object, (c_address)inhType, (c_address)&(interface->inherits[i]));
			a_anlAssertType(context, object, c_baseObject(inherit), c_getType(inherit), "c_attribute");
		}
	}

	for (i = 0; i < refSize; i++) {
		reference = interface->references[i];
		if (reference) {
			a_anlOutIndexedElement(context, "Ref", (long)i, (c_address)reference);
			/* No AddDataRefs here! */
			a_anlAssertType(context, object, c_baseObject(reference), c_getType(reference), "c_attribute");
		}
	}
}



static void
a_anlBaseClass(
	a_anlContext context,
	c_object object,
	c_class class)
{
	a_anlOutLabel(context, "Class");

    c_long i, keysSize = c_arraySize(class->keys);
	c_class extends = class->extends;
	c_string key;

	a_anlOutPointervalue(context, "extends", (void *)extends);
	if (extends) {
		a_anlAddDataRefs(context, (c_address)object, (c_address)extends, (c_address)&(class->extends));
		a_anlAssertType(context, object, c_baseObject(extends), c_getType(extends), "c_class");
	}
	a_anlOutLongvalue(context, "#Keys", (long)keysSize);	

	a_anlBaseInterfaceMembers(context, object, c_interface(class));
	
	for (i = 0; i < keysSize; i++) {
		key = class->keys[i];
		if (key) {
			a_anlOutIndexedElement(context, "Key", (long)i, (c_address)key);
			a_anlAddDataRefs(context, (c_address)object, (c_address)key, (c_address)&(class->keys[i]));
			a_anlOutStringvalue(context, "Value", key);
			a_anlAssertTypeDerived(context, object, c_baseObject(key), c_getType(key), "c_type");
		}
	}

}


/**
 * \brief Analyses an object, of which the kind is L_BASE or L_CLSS
 *
 * \param object The (base)object to analyse
 * \param objectKind The object kind (in this case: L_BASE or L_CLSS)
 */
static void
a_anlBaseObject(
	a_anlContext context,
	c_object object,
	a_lstObjectKind objectKind)
{
	char *name = c_metaObject(object)->name;
	c_type type = c_getType(object);
	a_anlAssertType(context, object, c_baseObject(type), c_getType(type), "c_class");
	a_anlAssertType(context, object, c_baseObject(name), c_getType(name), "c_string");

/*
	At this point, we already know that the typeName is "c_class".
	There's no need to mention this here again.
*/
//	char *typeName = type ? c_metaObject(type)->name : NULL;
//	c_metaKind metaKind = c_baseObject(object)->kind;

	a_anlOutStringvalue(context, "name", name);
	a_anlAddDataRefs(context, (c_address)object, (c_address)name, (c_address)&(c_metaObject(object)->name));
//	a_anlOutStringvalue(context, "Typename", typeName);
//	a_anlOutLongAndStringvalue(context, "metaKind", metaKind, a_utlMetaKindStr(metaKind));   // is always M_CLASS
	a_anlBaseClass(context, object, c_class(object));
}






/********************************************************************
 ANALYSE META OBJECTS  "M"
 ********************************************************************/


/**
 * \brief
 * Calculates and returns the maximum string size for a primitive type
 *
 * \param sizeOfValue
 * The value of sizeof(primitive_type)
 *
 * \param isSigned
 * Boolean value specifying whether this is a signed type, i.e. if an
 * extra character space must be counted for a sign char (-).
 *
 * \return
 * The number of characters needed in a string if the value would
 * be converted
 */
int
a_anlMetaLiteralMaxStrSize(
	size_t sizeOfValue,
	int isSigned)
{
	int nrBytes = (int)sizeOfValue;
	int nrBits = nrBytes * 8;
	int nrValueChars = (int)ceil(log(nrBits));
	int nrSignChars = isSigned ? 1 : 0;
	int nrNullChars = 1;
	// return 1 + ((int)(ceil(log((int)sizeOfValue)))) + isSigned ? 1 : 0;
	return nrSignChars + nrValueChars + nrNullChars;
}



/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_literal
 *
 * \param context
 * This file's context
 * \param literal
 * Instance of a Meta Object (kind: L_META) of c_literal
 */
static void
a_anlMetaLiteral(
	a_anlContext context,
	c_literal literal)
{
	c_value value = literal->value;
	c_valueKind valueKind = value.kind;
	a_anlOutLongAndStringvalue(context, "valueKind", (long)valueKind, a_utlValueKindStr(valueKind));
	
	char *strValue;
	switch(valueKind) {
			
		case V_CHAR: {
			char *localStrValue = a_memAlloc(4);
			sprintf(localStrValue, "\'%c\'", (char)value.is.Char);
			strValue = a_memStrdup(localStrValue);
			a_memFree(localStrValue);
			break;
		}

		case V_WCHAR: {
			char *localStrValue = a_memAlloc(4);
			sprintf(localStrValue, "\'%c\'", (char)value.is.WChar);
			strValue = a_memStrdup(localStrValue);
			a_memFree(localStrValue);
			break;
		}
			
		case V_STRING: {
			char *localStrValue = a_memAlloc(strlen((char *)value.is.String) + 3);
			sprintf(localStrValue, "\"%s\"", (char *)value.is.String);
			strValue = a_memStrdup(localStrValue);
			a_memFree(localStrValue);
			a_anlAddDataRefs(context, (c_address)literal, (c_address)value.is.String, (c_address)&(literal->value.is));
			break;
		}
			
		case V_WSTRING: {
			char *localStrValue = a_memAlloc(strlen((char *)value.is.WString) + 3);
			sprintf(localStrValue, "\"%s\"", (char *)value.is.WString);
			strValue = a_memStrdup(localStrValue);
			a_memFree(localStrValue);
			a_anlAddDataRefs(context, (c_address)literal, (c_address)value.is.String, (c_address)&(literal->value.is));
			break;
		}
			
		case V_FIXED: {
			char *localStrValue = a_memAlloc(strlen((char *)value.is.Fixed) + 3);
			sprintf(localStrValue, "\"%s\"", (char *)value.is.Fixed);
			strValue = a_memStrdup(localStrValue);
			a_memFree(localStrValue);
			a_anlAddDataRefs(context, (c_address)literal, (c_address)value.is.String, (c_address)&(literal->value.is));
			break;
		}
			 
		case V_OBJECT: {
			char *localStrValue = a_memAlloc(11);
			sprintf(localStrValue, "0x%8.8X", (unsigned int)value.is.Object);
			strValue = a_memStrdup(localStrValue);
			a_memFree(localStrValue);
			a_anlAddDataRefs(context, (c_address)literal, (c_address)value.is.String, (c_address)&(literal->value.is));
			break;
		}

		case V_BOOLEAN: {
			int blValue = (int)value.is.Boolean;
			strValue = a_memStrdup(blValue ? "TRUE" : "FALSE");
			break;
		}

		case V_OCTET: {
			unsigned int oValue = (unsigned int)value.is.Octet;
			strValue = a_memAlloc(a_anlMetaLiteralMaxStrSize(sizeof(unsigned int), 0));
			sprintf(strValue, "%o", oValue);
			break;
		}

		case V_LONG: {
			long llValue = (long)value.is.Long;
			strValue = a_memAlloc(a_anlMetaLiteralMaxStrSize(sizeof(long), 1));
			sprintf(strValue, "%ld", llValue);
			break;
		}

		case V_LONGLONG: {
			long long llValue = (long long)value.is.LongLong;
			strValue = a_memAlloc(a_anlMetaLiteralMaxStrSize(sizeof(long long), 1));
			sprintf(strValue, "%ld", (long)llValue);          // [gaat dit goed met deze cast?]
			break;
		}

		case V_SHORT: {
			short shValue = (short)value.is.Short;
			strValue = a_memAlloc(a_anlMetaLiteralMaxStrSize(sizeof(short), 1));
			sprintf(strValue, "%d", shValue);
			break;
		}

		case V_USHORT: {
			unsigned short shValue = (unsigned short)value.is.UShort;
			strValue = a_memAlloc(a_anlMetaLiteralMaxStrSize(sizeof(unsigned short), 0));
			sprintf(strValue, "%d", shValue);
			break;
		}

		case V_ULONG: {
			unsigned long ulValue = (unsigned long)value.is.ULong;
			strValue = a_memAlloc(a_anlMetaLiteralMaxStrSize(sizeof(unsigned long), 0));
			sprintf(strValue, "%ld", ulValue);
			break;
		}

		case V_ULONGLONG: {
			unsigned long long ullValue = (unsigned long long)value.is.ULongLong;
			strValue = a_memAlloc(a_anlMetaLiteralMaxStrSize(sizeof(unsigned long long), 0));
			sprintf(strValue, "%ld", (long int)ullValue);   // [gaat deze cast goed?]
			break;
		}

		case V_FLOAT: {
			double fValue = (double)value.is.Float;
			strValue = a_memAlloc(a_anlMetaLiteralMaxStrSize(sizeof(double), 1));
			sprintf(strValue, "%f", fValue);
			break;
		}

		case V_DOUBLE: {
			double dValue = (double)value.is.Double;
			strValue = a_memAlloc(a_anlMetaLiteralMaxStrSize(sizeof(double), 1));
			sprintf(strValue, "%f", dValue);
			break;
		}

		case V_UNDEFINED: {
			strValue = a_memStrdup("(V_UNDEFINED encountered)");
			break;
		}
			
		default: {
			strValue = a_memStrdup("a_anlMetaLiteral *TODO*");
			break;
		}
	}
	a_lstAddEntryValue(context->list, (c_address)literal, strValue);
	a_anlOutStringvalue(context, "Value", strValue);
	a_memFree(strValue);
}



/**
 * \brief
 * Analyses the members of a Meta Object (L_META) of type c_metaObject
 *
 * \param context
 * This file's context
 * \param metaObject
 * Instance of a Meta Object (kind: L_META) of c_metaObject
 *
 * \see
 * a_anlMetaAttribute a_anlMetaRelation
 * a_anlMetaModuleMembers a_anlMetaConstant a_anlMetaOperation
 * a_anlMetaTypeMembers
 */
static void
a_anlMetaMetaobjectMembers(
	a_anlContext context,
	c_metaObject metaObject)
{
	c_metaObject definedIn = metaObject->definedIn;
	a_anlOutPointervalue(context, "definedIn", (void *)definedIn);
	if (definedIn) {
		a_anlAddDataRefs(context, (c_address)metaObject, (c_address)definedIn, (c_address)&(metaObject->definedIn));
	}

	// We already have displayed the name and counted its pointervalue
//	char *name = metaObject->name;
//	a_anlOutStringvalue(context, "Name", name);
//	a_anlAddDataRefs(context, (c_address)metaObject, (c_address)name, (c_address)&(metaObject->name));
}



/**
 * \brief
 * Analyses the members of a Meta Object (L_META) of type c_property
 *
 * \param context
 * This file's context
 * \param property
 * Instance of a Meta Object (kind: L_META) of c_property
 *
 * \see
 * a_anlMetaMetaobjectMembers a_anlMetaAttribute a_anlMetaRelation
 */
static void
a_anlMetaPropertyMembers(
	a_anlContext context,
	c_property property)
{
	c_long offset = property->offset;
	c_type type = property->type;
	a_anlOutLongvalue(context, "offset", (long)offset);
	a_anlOutPointervalue(context, "type", (void *)type);
	a_anlAddDataRefs(context, (c_address)property, (c_address)type, (c_address)&(property->type));
	a_anlMetaMetaobjectMembers(context, c_metaObject(property));
}



/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_attribute
 *
 * \param context
 * This file's context
 * \param attribute
 * Instance of a Meta Object (kind: L_META) of c_attribute
 *
 * \see
 * a_anlMetaPropertyMembers
 */
static void
a_anlMetaAttribute(
	a_anlContext context,
	c_attribute attribute)
{
	c_bool isReadOnly = attribute->isReadOnly;
	a_anlOutLabel(context, "Attribute");
	a_anlOutBooleanvalue(context, "isReadOnly", (int)isReadOnly);
	a_anlMetaPropertyMembers(context, c_property(attribute));
}



/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_relation
 *
 * \param context
 * This file's context
 * \param relation
 * Instance of a Meta Object (kind: L_META) of c_relation
 *
 * \see
 * a_anlMetaPropertyMembers
 */
static void
a_anlMetaRelation(
	a_anlContext context,
	c_relation relation)
{
	char *inverse = relation->inverse;
	a_anlOutLabel(context, "Relation");
	a_anlOutStringvalue(context, "inverse", inverse);
	a_anlAddDataRefs(context, (c_address)relation, (c_address)inverse, (c_address)&(relation->inverse));
	a_anlMetaPropertyMembers(context, c_property(relation));
}



/**
 * \brief
 * Analyses the members of a Meta Object (L_META) of type c_module
 *
 * \param context
 * This file's context
 * \param module
 * Instance of a Meta Object (kind: L_META) of c_module
 *
 * \see
 * a_anlMetaMetaobjectMembers a_anlMetaModule a_anlMetaBase
 */
static void
a_anlMetaModuleMembers(
	a_anlContext context,
	c_module module)
{
	c_scope scope = module->scope;
	c_long scopeCount = c_scopeCount(scope);
	c_scopeWalkAction scopeWalkAction = a_anlScopeWalkback;

	a_anlOutPointervalue(context, "scope", (void *)scope);
	if (scope) {
		a_anlAddDataRefs(context, (c_address)module, (c_address)scope, (c_address)&(module->scope));
	}
	a_anlOutLongvalue(context, "scopeCount", (long)scopeCount);
	a_anlMetaMetaobjectMembers(context, c_metaObject(module));
	context->scopeCounter = 0;
	c_scopeWalk(scope, scopeWalkAction, context);
}




/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_base
 *
 * \param context
 * This file's context
 * \param baseValue
 * Instance of a Meta Object (kind: L_META) of c_base
 *
 * \note
 * \a c_base is derived from c_module, internally (hidden
 * information). Therefore, a cast to a private defined
 * clone of c_base is used internally.
 *
 * \see
 * a_anlMetaModuleMembers
 */
static void
a_anlMetaBase(
	a_anlContext context,
	c_base baseValue)
{
	a_base base = (a_base)baseValue;
	int i;
	c_type meta;
	a_anlOutLabel(context, "Base");
	
	a_anlOutPointervalue(context, "mm", (void *)base->mm);
	a_anlAddDataRefs(context, (c_address)base, (c_address)base->mm, (c_address)&(base->mm));
	
	a_anlOutPointervalue(context, "confidence", (void *)base->confidence);

	a_anlOutPointervalue(context, "bindings", (void *)base->bindings);
//	a_anlAddUnknRef(context, (c_address)base, (c_address)base->bindings, (c_address)&(base->bindings));
	a_anlOutLongvalue(context, "bindings->offset", (long)base->bindings->offset);
	a_anlOutLongvalue(context, "bindings->size", (long)base->bindings->size);

	a_anlOutPointervalue(context, "bindings->root->left", (void *)base->bindings->root->left);
//	a_anlAddUnknRef(context, (c_address)base, (c_address)base->bindings->root->left, (c_address)&(base->bindings->root->left));
	
	a_anlOutPointervalue(context, "bindings->root->right", (void *)base->bindings->root->right);
//	a_anlAddUnknRef(context, (c_address)base, (c_address)base->bindings->root->right, (c_address)&(base->bindings->root->right));
	
	// a_anlOutPointervalue(context, "bindLock", (void *)base->bindLock);
	// a_anlOutPointervalue(context, "schemaLock", (void *)base->schemaLock);

	a_anlOutPointervalue(context, "string_type", (void *)base->string_type);
	a_anlAddDataRefs(context, (c_address)base, (c_address)base->string_type, (c_address)&(base->string_type));
	
	a_anlOutPointervalue(context, "firstObject", (void *)base->firstObject);
	a_anlAddDataRefs(context, (c_address)base, (c_address)base->firstObject, (c_address)&(base->firstObject));
	
	a_anlOutPointervalue(context, "lastObject", (void *)base->lastObject);
	a_anlAddDataRefs(context, (c_address)base, (c_address)base->lastObject, (c_address)&(base->lastObject));

	a_anlMetaModuleMembers(context, c_module(baseValue));

	for (i = 0; i < M_COUNT; i++) {
		meta = base->metaType[i];
		a_anlOutIndexedElement(context, "Mta", (long)i, (c_address)meta);
		if (meta) {
			a_lstObjectKind objectKind = a_anlGetObjectKind(c_object(meta));
			a_anlOutLabel(context, a_lstGetObjectKindChar(objectKind));
			if ( (objectKind == L_CLSS) || (objectKind == L_BASE) || (objectKind == L_META) ) {
				char *typeName = c_metaObject(meta)->name;
				if (typeName) {
					a_anlOutStringvalue(context, "->", typeName);
				}
			}
			a_anlAddDataRefs(context, (c_address)base, (c_address)meta, (c_address)&(base->metaType[i]));
		}
	}
}



/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_module
 *
 * \param context
 * This file's context
 * \param module
 * Instance of a Meta Object (kind: L_META) of c_module
 *
 * \see
 * a_anlMetaModuleMembers
 */
static void
a_anlMetaModule(
	a_anlContext context,
	c_module module)
{
	char *typeName = c_metaObject(c_getType(module))->name;
	char *checkName = "c_base";
	if (typeName && (strcmp(typeName, checkName) == 0)) {
		a_anlMetaBase(context, c_base(module));
	} else {
		a_anlOutLabel(context, "Module");
		a_anlMetaModuleMembers(context, module);
	}
}




/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_constant
 *
 * \param context
 * This file's context
 * \param constant
 * Instance of a Meta Object (kind: L_META) of c_constant
 *
 * \see
 * a_anlMetaMetaobjectMembers
 */
static void
a_anlMetaConstant(
	a_anlContext context,
	c_constant constant)
{
	c_operand operand = constant->operand;
	c_type type = constant->type;

	a_anlOutLabel(context, "Constant");
	a_anlOutPointervalue(context, "operand", (void *)operand);
	if (operand) {
		a_anlAddDataRefs(context, (c_address)constant, (c_address)operand, (c_address)&(constant->operand));
	}
	a_anlOutPointervalue(context, "type", (void *)type);
	if (type) {
		a_anlAddDataRefs(context, (c_address)constant, (c_address)type, (c_address)&(constant->type));
	}
	a_anlMetaMetaobjectMembers(context, c_metaObject(constant));
}



/**
 * \brief
 * Analyses the members of a Meta Object (L_META) of type c_type
 *
 * \param context
 * This file's context
 * \param type
 * Instance of a Meta Object (kind: L_META) of c_type
 *
 * \see
 * a_anlMetaInterfaceMembers a_anlMetaCollectiontype
 * a_anlMetaEnumeration a_anlMetaPrimitive a_anlMetaTypedef
 * a_anlMetaStructure a_anlMetaUnion
 */
static void
a_anlMetaTypeMembers(
	a_anlContext context,
	c_type type)
{
	c_long alignment = type->alignment;
	c_base base = type->base;
	c_long size = type->size;
	a_anlOutLongvalue(context, "alignment", (long)alignment);
	a_anlOutPointervalue(context, "base", (void *)base);
	a_anlAddDataRefs(context, (c_address)type, (c_address)base, (c_address)&(type->base));
	a_anlOutLongvalue(context, "size", (long)size);
	a_anlMetaMetaobjectMembers(context, c_metaObject(type));
	a_anlAssertType(context, type, c_baseObject(base), c_getType(base), "c_base");
}



/**
 * \brief
 * Analyses the members of a Meta Object (L_META) of type c_interface
 *
 * \param context
 * This file's context
 * \param interface
 * Instance of a Meta Object (kind: L_META) of c_interface
 *
 * \see
 * a_anlMetaInterface a_anlMetaClass a_anlMetaTypeMembers
 */
static void
a_anlMetaInterfaceMembers(
	a_anlContext context,
	c_interface interface)
{
	c_bool abstract = interface->abstract;
	c_array inherits = interface->inherits;
	c_array references = interface->references;
	c_long inhSize = c_arraySize(inherits);
	c_long refSize = c_arraySize(references);
	c_scope scope = interface->scope;
	c_scopeWalkAction scopeWalkAction = a_anlScopeWalkback;
    c_long i;
	c_interface inherit, reference;

	a_anlOutBooleanvalue(context, "abstract", (int)abstract);

	if (scope) {
		c_long scopeCount = c_scopeCount(scope);
		a_anlOutPointervalue(context, "scope", (void *)scope);
		a_anlAddDataRefs(context, (c_address)interface, (c_address)scope, (c_address)&(interface->scope));
		a_anlOutLongvalue(context, "scopeCount", (long)scopeCount);
	}
	
	if (inherits) {
		a_anlOutPointervalue(context, "inherits", (void *)inherits);
		a_anlAddDataRefs(context, (c_address)interface, (c_address)inherits, (c_address)&(interface->inherits));
	}
	a_anlOutLongvalue(context, "#Inherits", (long)inhSize);

	if (references) {
		a_anlOutPointervalue(context, "references", (void *)references);
		a_anlAddDataRefs(context, (c_address)interface, (c_address)references, (c_address)&(interface->references));
	}
	a_anlOutLongvalue(context, "#References", (long)refSize);

	a_anlMetaTypeMembers(context, c_type(interface));

	if (scope) {
		context->scopeCounter = 0;
		c_scopeWalk(scope, scopeWalkAction, context);
	}

	for (i = 0; i < inhSize; i++) {
		inherit = interface->inherits[i];
		if (inherit) {
			a_anlOutIndexedElement(context, "Inh", (long)i, (c_address)inherit);
			a_anlAddDataRefs(context, (c_address)interface, (c_address)inherit, (c_address)&(interface->inherits[i]));
			a_anlAssertType(context, interface, c_baseObject(inherit), c_getType(inherit), "c_attribute");
		}
	}

	for (i = 0; i < refSize; i++) {
		reference = interface->references[i];
		if (reference) {
			a_anlOutIndexedElement(context, "Ref", (long)i, (c_address)reference);
			/* No AddDataRefs here! */
			a_anlAssertType(context, interface, c_baseObject(reference), c_getType(reference), "c_attribute");
		}
	}
}



/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_interface
 *
 * \param context
 * This file's context
 * \param interface
 * Instance of a Meta Object (kind: L_META) of c_interface
 *
 * \see
 * a_anlMetaInterfaceMembers
 */
static void
a_anlMetaInterface(
	a_anlContext context,
	c_interface interface)
{
	a_anlOutLabel(context, "Interface");
	a_anlMetaInterfaceMembers(context, interface);
}



/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_class
 *
 * \param context
 * This file's context
 * \param class
 * Instance of a Meta Object (kind: L_META) of c_class
 *
 * \see
 * a_anlMetaInterfaceMembers
 */
static void
a_anlMetaClass(
	a_anlContext context,
	c_class class)
{
	c_long i, arraySize = c_arraySize(class->keys);
	c_class extends = class->extends;
	c_string key;

	a_anlOutLabel(context, "Class");

	a_anlOutPointervalue(context, "extends", (void *)extends);

	/* Gaat vaak mis: (extends krijgt wel eens waarde -1 (???) )   nog uitzoeken */
	if (extends && ((c_address)extends != 0xFFFFFFFF)) {
		a_anlAddDataRefs(context, (c_address)class, (c_address)extends, (c_address)&(class->extends));
		a_anlOutStringvalue(context, "extendsName", c_metaObject(extends)->name);
		a_anlAssertType(context, class, c_baseObject(extends), c_getType(extends), "c_class");
	}
	a_anlOutLongvalue(context, "#Keys", (long)arraySize);

	a_anlMetaInterfaceMembers(context, c_interface(class));
	
	for (i = 0; i < arraySize; i++) {
		key = class->keys[i];
		if (key) {
			a_anlOutIndexedElement(context, "Key", (long)i, (c_address)key);
			a_anlOutStringvalue(context, "Value", key);
			a_anlAddDataRefs(context, (c_address)class, (c_address)key, (c_address)&(class->keys[i]));
		}
	}
}



/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_collectionType
 *
 * \param context
 * This file's context
 * \param collectionType
 * Instance of a Meta Object (kind: L_META) of c_collectionType
 *
 * \see
 * a_anlMetaTypeMembers
 */
static void
a_anlMetaCollectionType(
	a_anlContext context,
	c_collectionType collectionType)
{
	c_collKind collKind = collectionType->kind;
	c_long maxSize = collectionType->maxSize;
	c_type subType = collectionType->subType;

	a_anlOutLabel(context, "CollectionType");
	a_anlOutLongvalue(context, "maxSize", (long)maxSize);
	a_anlOutPointervalue(context, "subType", (void *)subType);
	if (subType) {
		a_anlAddDataRefs(context, (c_address)collectionType, (c_address)subType, (c_address)&(collectionType->subType));
	} else {
		a_anlOutLabel(context, "*subType=NULL*");
		a_lstAddEntryNote(context->list, (c_address)collectionType, "*subType=NULL*");
	}

	a_anlOutLongAndStringvalue(context, "collKind", collKind, a_utlCollKindStr(collKind));
	a_anlMetaTypeMembers(context, c_type(collectionType));
}



/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_enumeration
 *
 * \param context
 * This file's context
 * \param enumeration
 * Instance of a Meta Object (kind: L_META) of c_enumeration
 *
 * \see
 * a_anlMetaTypeMembers
 */
static void
a_anlMetaEnumeration(
	a_anlContext context,
	c_enumeration enumeration)
{
    c_array elements = enumeration->elements;
	c_long i, arraySize = c_arraySize(elements);
	c_type element;

	a_anlOutLabel(context, "Enumeration");
	a_anlOutPointervalue(context, "elements", (void *)elements);
	a_anlAddDataRefs(context, (c_address)enumeration, (c_address)elements, (c_address)&(enumeration->elements));
	a_anlOutLongvalue(context, "#Elements", (long)arraySize);

	a_anlMetaTypeMembers(context, c_type(enumeration));
	
	for (i = 0; i < arraySize; i++) {
		element = elements[i];
		if (element) {
			a_anlAssertType(context, enumeration, c_baseObject(element), c_getType(element), "c_constant");
			a_anlOutIndexedElement(context, "Elm", (long)i, (c_address)element);
			/* No AddDataRefs here; element is a data object of its own, thus counted later */
			a_anlOutStringvalue(context, "", c_metaObject(element)->name);
		}
	}
}



/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_primitive
 *
 * \param context
 * This file's context
 * \param primitive
 * Instance of a Meta Object (kind: L_META) of c_primitive
 *
 * \see
 * a_anlMetaTypeMembers
 */
static void
a_anlMetaPrimitive(
	a_anlContext context,
	c_primitive primitive)
{
	c_primKind primKind = primitive->kind;

	a_anlOutLabel(context, "Primitive");
	a_anlOutLongAndStringvalue(context, "primKind", primKind, a_utlPrimKindStr(primKind));

	switch (primKind) {
		case P_UNDEFINED:
		case P_BOOLEAN:
		case P_CHAR:
		case P_WCHAR:
		case P_OCTET:
		case P_SHORT:
		case P_USHORT:
		case P_LONG:
		case P_ULONG:
		case P_LONGLONG:
		case P_ULONGLONG:
		case P_FLOAT:
		case P_DOUBLE:
		case P_VOIDP:
		case P_MUTEX:
		case P_LOCK:
		case P_COND:
//			break;
		default:
			a_anlOutLabel(context, "(Primitive Value)");
			break;
	}
	a_anlMetaTypeMembers(context, c_type(primitive));
}




/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_typeDef
 *
 * \param context
 * This file's context
 * \param typeDefValue
 * Instance of a Meta Object (kind: L_META) of c_typeDef
 *
 * \see
 * a_anlMetaTypeMembers
 */
static void
a_anlMetaTypeDef(
	a_anlContext context,
	c_typeDef typeDefValue)
{
	c_type alias = typeDefValue->alias;
	a_anlOutLabel(context, "typeDef");
	a_anlOutPointervalue(context, "alias", (void *)alias);
	a_anlAddDataRefs(context, (c_address)typeDefValue, (c_address)alias, (c_address)&(typeDefValue->alias));
	a_anlMetaTypeMembers(context, c_type(typeDefValue));
}



/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_structure
 *
 * \param context
 * This file's context
 * \param structure
 * Instance of a Meta Object (kind: L_META) of c_structure
 *
 * \see
 * a_anlMetaTypeMembers
 */
static void
a_anlMetaStructure(
	a_anlContext context,
	c_structure structure)
{
	c_array members = structure->members;
	c_array references = structure->references;
	c_scope scope = structure->scope;
	c_scopeWalkAction scopeWalkAction = a_anlScopeWalkback;
	
	c_long memSize = c_arraySize(members);
	c_long refSize = c_arraySize(references);
    c_long i;
	c_member member, reference;

	c_long offset;
	c_string name;
	c_type type;
	c_string typeName;

	c_object referenceObject, *referenceObjectPtr, referencedObject;

	a_anlOutLabel(context, "Structure");	

	if (scope) {
		c_long scopeCount = c_scopeCount(scope);
		a_anlOutPointervalue(context, "scope", (void *)scope);
		a_anlAddDataRefs(context, (c_address)structure, (c_address)scope, (c_address)&(structure->scope));
		a_anlOutLongvalue(context, "scopeCount", (long)scopeCount);
	}
	
	if (members) {
		a_anlOutPointervalue(context, "members", (void *)members);
		a_anlAddDataRefs(context, (c_address)structure, (c_address)members, (c_address)&(structure->members));
	}
	a_anlOutLongvalue(context, "#Members", (long)memSize);

	if (references) {
		a_anlOutPointervalue(context, "references", (void *)references);
		a_anlAddDataRefs(context, (c_address)structure, (c_address)references, (c_address)&(structure->references));
	}
	a_anlOutLongvalue(context, "#References", (long)refSize);
	
	a_anlMetaTypeMembers(context, c_type(structure));

	if (scope) {
		context->scopeCounter = 0;
		c_scopeWalk(scope, scopeWalkAction, context);
	}

	for (i = 0; i < memSize; i++) {
		member = structure->members[i];
		if (member) {
			offset = member->offset;
			name = c_specifier(member)->name;
			type = c_specifier(member)->type;
			a_anlAssertType(context, structure, c_baseObject(member), c_getType(member), "c_member");
			a_anlOutIndexedElement(context, "Mem", (long)i, (c_address)member);
			a_anlOutLongvalue(context, "offset", (long)offset);
			a_anlOutPointervalue(context, "memberType", (void *)type);
			if (type) {
				typeName = c_metaObject(type)->name;
				if (typeName) {
					a_anlOutStringvalue(context, "TypeName", typeName);
				}
			}
			// Do not count, just display info:
			a_anlOutStringvalue(context, "memberName", name);
		}
	}

	for (i = 0; i < refSize; i++) {
		reference = structure->references[i];
		if (reference) {
			offset = reference->offset;
			name = c_specifier(reference)->name;
			type = c_specifier(reference)->type;
			a_anlAssertType(context, structure, c_baseObject(reference), c_getType(reference), "c_member");
			referenceObject = (c_object)((c_address)structure + (c_address)offset);
			referenceObjectPtr = referenceObject;
			referencedObject = *referenceObjectPtr;
			a_anlOutIndexedElement(context, "Ref", (long)i, (c_address)reference);
			a_anlOutLongvalue(context, "offset", (long)offset);
			a_anlOutPointervalue(context, "->", (void *)referenceObject);
			a_anlOutPointervalue(context, "referenceType", (void *)type);
			if (type) {
				typeName = c_metaObject(type)->name;
				if (typeName) {
					a_anlOutStringvalue(context, "TypeName", typeName);
				}
			}
			// Do not count, just display info:
			a_anlOutStringvalue(context, "referenceName", name);
		}
	}
}



/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_union
 *
 * \param context
 * This file's context
 * \param unionValue
 * Instance of a Meta Object (kind: L_META) of c_union
 *
 * \see
 * a_anlMetaTypeMembers
 */
static void
a_anlMetaUnion(
	a_anlContext context,
	c_union unionValue)
{
	c_array cases = unionValue->cases;
	c_array references = unionValue->references;
	c_long cseSize = c_arraySize(cases);
	c_long refSize = c_arraySize(references);
	c_type switchType = unionValue->switchType;
	c_scope scope = unionValue->scope;
	c_scopeWalkAction scopeWalkAction = a_anlScopeWalkback;
	c_long i;
	c_unionCase unionCase;
	
	a_anlOutLabel(context, "Union");
	
	a_anlOutPointervalue(context, "switchType", (void *)switchType);
	if (switchType) {
		a_anlAddDataRefs(context, (c_address)unionValue, (c_address)switchType, (c_address)&(unionValue->switchType));
	}

	a_anlOutPointervalue(context, "cases", (void *)cases);
	if (cases) {
		a_anlAddDataRefs(context, (c_address)unionValue, (c_address)cases, (c_address)&(unionValue->cases));
	}
	a_anlOutLongvalue(context, "#Cases", (long)cseSize);

	a_anlOutPointervalue(context, "references", (void *)references);
	if (references) {
		a_anlAddDataRefs(context, (c_address)unionValue, (c_address)references, (c_address)&(unionValue->references));
	}
	a_anlOutLongvalue(context, "#References", (long)refSize);

	if (scope) {
		c_long scopeCount = c_scopeCount(scope);
		a_anlOutPointervalue(context, "scope", (void *)scope);
		a_anlAddDataRefs(context, (c_address)unionValue, (c_address)scope, (c_address)&(unionValue->scope));
		a_anlOutLongvalue(context, "scopeCount", (long)scopeCount);
	}
	
	a_anlMetaTypeMembers(context, c_type(unionValue));

	if (scope) {
		context->scopeCounter = 0;
		c_scopeWalk(scope, scopeWalkAction, context);
	}

	for (i = 0; i < cseSize; i++) {
		unionCase = unionValue->cases[i];
		if (unionCase) {
			a_anlAssertType(context, unionValue, c_baseObject(unionCase), c_getType(unionCase), "c_unionCase");
			a_anlOutIndexedElement(context, "Cse", (long)i, (c_address)unionCase);
			/* No AddDataRefs here! */
			a_anlOutStringvalue(context, "->", c_specifier(unionCase)->name);
		}
	}
	for (i = 0; i < refSize; i++) {
		unionCase = unionValue->references[i];
		if (unionCase) {
			a_anlAssertType(context, unionValue, c_baseObject(unionCase), c_getType(unionCase), "c_unionCase");
			a_anlOutIndexedElement(context, "Ref", (long)i, (c_address)unionCase);
			/* No AddDataRefs here! */
			a_anlOutStringvalue(context, "->", c_specifier(unionCase)->name);
		}
	}
}



/**
 * \brief
 * Analyses the members of a Meta Object (L_META) of type c_specifier
 *
 * \param context
 * This file's context
 * \param specifier
 * Instance of a Meta Object (kind: L_META) of c_specifier
 *
 * \see
 * a_anlMetaMember a_anlUnionCase
 */
static void
a_anlMetaSpecifierMembers(
	a_anlContext context,
	c_specifier specifier)
{
	c_type type = specifier->type;
	c_string name = specifier->name;
	/* Skip printing (and [de-]counting) specifier->name, for that has already been done */
	a_anlOutPointervalue(context, "specifier->type", (void *)type);
	a_anlAddDataRefs(context, (c_address)specifier, (c_address)type, (c_address)&(specifier->type));
	a_anlAssertType(context, specifier, c_baseObject(name), c_getType(name), "c_string");
	a_anlAssertTypeDerived(context, specifier, c_baseObject(type), c_getType(type), "c_type");
}



/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_member
 *
 * \param context
 * This file's context
 * \param member
 * Instance of a Meta Object (kind: L_META) of c_member
 *
 * \see
 * a_anlMetaSpecifierMembers
 */
static void
a_anlMetaMember(
	a_anlContext context,
	c_member member)
{
	c_long offset = member->offset;
	a_anlOutLongvalue(context, "offset", (long)offset);
	a_anlMetaSpecifierMembers(context, c_specifier(member));
}



/**
 * \brief
 * Analyses a Meta Object (L_META) of type c_unionCase
 *
 * \param context
 * This file's context
 * \param unionCase
 * Instance of a Meta Object (kind: L_META) of c_unionCase
 *
 * \see
 * a_anlMetaSpecifierMembers
 */
static void
a_anlMetaUnionCase(
	a_anlContext context,
	c_unionCase unionCase)
{
	int i;
	c_array labels = unionCase->labels;
	c_long labelsSize = c_arraySize(labels);
	a_anlOutPointervalue(context, "labelsPtr", (void *)labels);
	a_anlAddDataRefs(context, (c_address)unionCase, (c_address)labels, (c_address)&(unionCase->labels));
	a_anlOutLongvalue(context, "#Labels", labelsSize);
	a_anlMetaSpecifierMembers(context, c_specifier(unionCase));

	for (i = 0; i < labelsSize; i++) {
		c_literal label = unionCase->labels[i];
		if (label) {
			a_anlAssertType(context, unionCase, c_baseObject(label), c_getType(label), "c_literal");
			a_anlOutIndexedElement(context, "Lbl", (long)i, (c_address)label);
		}
	}
}



/**
 * \brief
 * Analyses an object, of which the kind is L_META
 *
 * \param context
 * This file's context
 * \param o
 * The (meta)object to analyse
 * \param objectKind
 * The object kind (in this case: L_META)
 */
static void
a_anlMetaObject(
	a_anlContext context,
	c_object o,
	a_lstObjectKind objectKind)
{
	c_type type = c_getType(o);
	char *name;
	char *typeName = type ? c_metaObject(type)->name : NULL;
	
	if (type) {
		a_anlAssertTypeDerived(context, o, c_baseObject(type), c_getType(type), "c_type");
	}
	if (c_checkType(o, "c_metaObject")) {
		name = c_metaObject(o)->name;
		if (name) {
			a_anlAssertType(context, o, c_baseObject(name), c_getType(name), "c_string");
		}
		a_anlAddDataRefs(context, (c_address)o, (c_address)name, (c_address)&(c_metaObject(o)->name));
	} else if (c_checkType(o, "c_specifier")) {
		name = c_specifier(o)->name;
		if (name) {
			a_anlAssertType(context, o, c_baseObject(name), c_getType(name), "c_string");
		}
		a_anlAddDataRefs(context, (c_address)o, (c_address)name, (c_address)&(c_specifier(o)->name));
	} else {
		name = "(anonymous c_operand subtype)";
	}
	a_anlOutStringvalue(context, "name", name);

	c_metaKind metaKind = c_baseObject(o)->kind;
	a_anlOutStringvalue(context, "Typename", typeName);

	a_anlOutLongAndStringvalue(context, "metaKind", metaKind, a_utlMetaKindStr(metaKind));

	switch (metaKind) {
		case M_ATTRIBUTE:    a_anlMetaAttribute(context, c_attribute(o));           break;
		case M_RELATION:     a_anlMetaRelation(context, c_relation(o));             break;

		case M_BASE:
		case M_MODULE:       a_anlMetaModule(context, c_module(o));                 break;
		case M_CONSTANT:     a_anlMetaConstant(context, c_constant(o));             break;

		case M_INTERFACE:    a_anlMetaInterface(context, c_interface(o));           break;
		case M_CLASS:        a_anlMetaClass(context, c_class(o));                   break;
		case M_COLLECTION:   a_anlMetaCollectionType(context, c_collectionType(o)); break;
		case M_ENUMERATION:  a_anlMetaEnumeration(context, c_enumeration(o));       break;
		case M_PRIMITIVE:    a_anlMetaPrimitive(context, c_primitive(o));           break;
		case M_TYPEDEF:      a_anlMetaTypeDef(context, c_typeDef(o));               break;
		case M_EXCEPTION:
		case M_STRUCTURE:    a_anlMetaStructure(context, c_structure(o));           break;
		case M_UNION:        a_anlMetaUnion(context, c_union(o));                   break;

		case M_MEMBER:       a_anlMetaMember(context, c_member(o));                 break;
		case M_UNIONCASE:    a_anlMetaUnionCase(context, c_unionCase(o));           break;

		case M_LITERAL:      a_anlMetaLiteral(context, c_literal(o));               break;

		default:
			a_anlOutLongAndStringvalue(context, "*Meta-TODO*", metaKind, a_utlMetaKindStr(metaKind));
			break;
	}
}







/********************************************************************
 ANALYSE DATA OBJECTS  "D"
 ********************************************************************/


struct a_anlScopeContext {
	a_anlContext anlContext;
	int scopeCounter;
	c_object object;
};



static void
a_anlDataClassScopeCallback(
	c_attribute attribute,
	struct a_anlScopeContext *scopeContext)
{
	a_anlContext context = scopeContext->anlContext;
	c_object object = scopeContext->object;
	c_long offset = c_property(attribute)->offset;
	c_type type = c_property(attribute)->type;
	c_metaObject definedIn = c_metaObject(attribute)->definedIn;
	c_string name = c_metaObject(attribute)->name;
	c_string typeName = type ? c_metaObject(type)->name : NULL;
	
	a_anlOutIndexedElement(context, "SCP", (long)(scopeContext->scopeCounter++), (c_address)attribute);
	a_anlOutBooleanvalue(context, "isReadOnly", (int)attribute->isReadOnly);
	a_anlOutLongvalue(context, "Offset", (long)offset);
	a_anlOutPointervalue(context, "type", (void *)type);
	a_anlOutStringvalue(context, "name", name);
	a_anlOutPointervalue(context, "definedIn", (void *)definedIn);
	a_anlOutStringvalue(context, "typeName", typeName);

	if (c_typeIsRef(type)) {
		c_address rawAddress = (c_address)((c_address)object + (c_address)offset);
		c_object *refObjPtr = refObjPtr = (c_object *)rawAddress;
	    c_address refAddress = (c_address)*refObjPtr;
		a_anlAddDataRefs(context, (c_address)object, (c_address)refAddress, (c_address)rawAddress);
	}
	a_anlAssertType(context, object, c_baseObject(attribute), c_getType(attribute), "c_attribute");
	a_anlAssertTypeDerived(context, object, c_baseObject(type), c_getType(type), "c_type");
}



static void
a_anlDataInterfaceMembers(
	a_anlContext context,
	c_object object,
	c_interface interface)
{
	c_long inhSize = c_arraySize(interface->inherits);
	c_long refSize = c_arraySize(interface->references);
	c_interface inherit, reference;
    c_long i;

	a_anlOutLongvalue(context, "#Inherits", (long)inhSize);
	a_anlOutLongvalue(context, "#References", (long)refSize);
	
	c_scope scope = interface->scope;
	if (scope) {
		struct a_anlScopeContext scopeContext;
		scopeContext.anlContext = context;
		scopeContext.scopeCounter = 0;
		scopeContext.object = object;
		context->scopeCounter = 0;
		c_scopeWalk(scope, a_anlDataClassScopeCallback, &scopeContext);
	}

	for (i = 0; i < inhSize; i++) {
		inherit = interface->inherits[i];
		if (inherit) {
			a_anlOutIndexedElement(context, "Inh", (long)i, (c_address)inherit);
			a_anlAssertType(context, object, c_baseObject(inherit), c_getType(inherit), "c_interface");
		}
	}

	for (i = 0; i < refSize; i++) {
		reference = interface->references[i];
		if (reference) {
			a_anlOutIndexedElement(context, "Ref", (long)i, (c_address)reference);
			a_anlAssertTypeDerived(context, object, c_baseObject(reference), c_getType(reference), "c_metaObject");
		}
	}
}



static void
a_anlDataClass(
	a_anlContext context,
	c_object object,
	c_class class)
{
	a_anlOutLabel(context, "Class");
	a_anlDataInterfaceMembers(context, object, c_interface(class));
}




static void
a_anlDataCollectionArrayElements(
	a_anlContext context,
	c_object object,
	c_collectionType array)
{
	c_long i, size;

	c_object baseObject;
	c_object currentObject, *currentObjectPtr, referenceObject;
	c_type subType = array->subType;
	c_collKind collKind = c_collectionType(array)->kind;
	
	switch (collKind) {
		case C_ARRAY:
			a_anlOutLabel(context, "Array");
			if (array->maxSize == 0) {
				size = c_arraySize(*((c_array *)object));
				baseObject = *((c_object *)object);
			} else {
				size = array->maxSize;
				baseObject = object;
			}
			break;
		case C_SEQUENCE:
			a_anlOutLabel(context, "Sequence");
			size = c_arraySize(*((c_array *)object));
			baseObject = *((c_object *)object);
			break;
		default:
			assert(0);   /* collKind must be C_ARRAY or C_SEQUENCE! */
			break;
	}

	a_anlOutLongvalue(context, "maxSize", (long)array->maxSize);
	a_anlOutLongvalue(context, "arrayLength", (long)size);
	a_anlOutPointervalue(context, "subType", (void *)subType);
	a_anlAssertTypeDerived(context, object, c_baseObject(subType), c_getType(subType), "c_type");

	currentObject = baseObject;
	for (i = 0; i < size; i++) {
		currentObjectPtr = currentObject;
		referenceObject = (c_object)*currentObjectPtr;
		a_anlOutIndexedElement(context, "Elm", (long)i, (c_address)currentObject);
		a_anlOutPointervalue(context, "", (void *)referenceObject);
		if (referenceObject) {
			a_anlAddDataRefs(context, (c_address)object, (c_address)referenceObject,
				(c_address)&(*currentObjectPtr));
		}
		currentObject = (c_object)((c_address)currentObject + (c_address)sizeof(c_voidp));
	}
}




typedef struct a_anlCollectionContext {
	c_collectionType collectionType;
	c_type           subType;
	c_action         action;
	long             counter;
	a_anlContext     anlContext;
} *a_anlCollectionContext;




static int
a_anlDataComplexCollectionElementsCallback(
	c_object object,
	a_anlCollectionContext anlCollectionContext)
{
	a_anlContext anlContext = anlCollectionContext->anlContext;
	long counter = anlCollectionContext->counter++;
	a_anlOutIndexedElement(anlContext, "Cll", counter, (c_address)object);
	/* No AddDaraRefs here! */
	return 1;
}



static void
a_anlDataComplexCollectionElements(
	a_anlContext context,
	c_object object,
	c_collectionType collectionType)
{
	c_collection collection = (c_collection)object;
	if (collection) {
		c_action walkAction = (c_action)a_anlDataComplexCollectionElementsCallback;
		struct a_anlCollectionContext collectionContext;
		collectionContext.collectionType = collectionType;
		collectionContext.subType        = c_subType(collection);
		collectionContext.counter        = 0;
		collectionContext.anlContext     = context;
		c_walk(collection, walkAction, &collectionContext);
	}
}




/* Because we're unable to locate and use the internal data structures
 * within the database, we need to clone them:
 */


C_CLASS(a_avlNode);

C_STRUCT(a_avlNode) {
	a_avlNode left;
	a_avlNode right;
	c_short height;
	c_short padding;   // ?
	c_object object;   // data
	a_avlNode next;    // InsOrder chain
//	a_avlNode unknown; // padding?
};


C_CLASS(a_scope);

C_STRUCT(a_scope) {
	a_avlNode root;
	c_ulong offset;
	c_ulong size;
	c_ulong dummy; // c_mm
	a_avlNode head;
	a_avlNode tail;
};



static void
a_anlDataCollectionScope(
	a_anlContext context,
	c_object object)
{
	a_scope scope = (a_scope)object;
	if (scope) {
		long counter = 0;
		a_avlNode root = scope->root;
		a_avlNode head = scope->head;
		a_avlNode tail = scope->tail;
		a_avlNode node;
		
		a_anlOutPointervalue(context, "scope->root", (void *)root);
		a_anlRmvUnknOcc(context, (c_address)&(scope->root), (c_address)root);
		a_anlOutPointervalue(context, "scope->head", (void *)head);
		a_anlRmvUnknOcc(context, (c_address)&(scope->head), (c_address)head);
		a_anlOutPointervalue(context, "scope->tail", (void *)tail);
		a_anlRmvUnknOcc(context, (c_address)&(scope->tail), (c_address)tail);
//		a_stsInsertNewBlock(context->stsContext, A_STS_BLOCK_AVLNODE, (c_address)scope, sizeof(C_STRUCT(a_scope)));

		node = head;
		while (node) {
			a_avlNode left = node->left;
			a_avlNode right = node->right;
			c_object data = node->object;
			a_avlNode next = node->next;
			a_anlOutIndexedElement(context, "Nde", counter++, (c_address)node);
			a_anlOutPointervalue(context, "left", (void *)left);
			a_anlRmvUnknOcc(context, (c_address)&(node->left), (c_address)left);
			a_anlOutPointervalue(context, "right", (void *)right);
			a_anlRmvUnknOcc(context, (c_address)&(node->right), (c_address)right);
			a_anlOutPointervalue(context, "data", (void *)data);
			a_anlAddUnknRef(context, (c_address)&(node->object), (c_address)data);
			a_anlOutPointervalue(context, "next", (void *)next);
			a_anlRmvUnknOcc(context, (c_address)&(node->next), (c_address)next);
			assert(context->stsContext);
			a_stsInsertNewBlock(context->stsContext, A_STS_BLOCK_AVLNODE, (c_address)node, sizeof(C_STRUCT(a_avlNode)));
			node = node->next;
		}

//		a_anlAssertType(context, object, c_baseObject(object), c_getType(object), "c_scope");  // zinloos!
//		a_anlAddDataRefs(context, (c_address)object, (c_address)scope, (c_address)&(*((c_scope *)object)));
//		c_scopeWalk(c_scope(object), scopeWalkAction, &scopewalkContext);
	}
}



static void
a_anlDataCollection(
	a_anlContext context,
	c_object object,
	c_collectionType collectionType)
{
	c_long count = c_count((c_collection)collectionType);
	c_collKind collKind = collectionType->kind;
	c_type subType = collectionType->subType;

	a_anlOutLongAndStringvalue(context, "TypeCollKind", collKind, a_utlCollKindStr(collKind));
	a_anlOutLongvalue(context, "TypeCount", (long)count);
	a_anlOutPointervalue(context, "TypeSubType", (void *)subType);
	if (subType) {
		char *subTypeName = c_metaObject(subType)->name;
		a_anlOutStringvalue(context, "TypeSubTypeName", subTypeName);
		a_anlAssertTypeDerived(context, object, c_baseObject(subType), c_getType(subType), "c_type");
	} else {
		a_anlOutLabel(context, "*TypeSubType=NULL*");
		a_lstAddEntryNote(context->list, (c_address)object, "*TypeSubType=NULL*");
	}


	switch (collKind) {
		case C_ARRAY:
		case C_SEQUENCE:
			a_anlDataCollectionArrayElements(context, object, collectionType);
			break;

		case C_STRING: {
			long ourSize = strlen((char *)object);
			a_lstSetOurSize(context->list, (c_address)object, ourSize);
			a_anlOutStringvalue(context, "Value", (char *)object);
//			a_anlOutPointervalue(context, "valuePtr", (void *)object);
			a_lstAddEntryValue(context->list, (c_address)object, (char *)object);
//			a_anlAssertType(context, object, c_baseObject(object), c_getType(object), "c_string");  // zinloos, we weten dit al!
			break;
		}
			
		case C_SET:
		case C_LIST:
		case C_BAG:
		case C_DICTIONARY:
		case C_QUERY:
			a_anlDataComplexCollectionElements(context, object, collectionType);  // todo
			break;

		case C_SCOPE:
		{
			a_anlOutPointervalue(context, "scope", (void *)object);
			a_anlDataCollectionScope(context, object);
			break;
		}

//		case C_MAP:
//		case C_WSTRING:
//			a_anlOutLabel(context, "*huh?*");
//			break;

//		case C_COUNT:
		case C_UNDEFINED:
			a_anlOutLabel(context, "**?**");
			break;

		default:
			a_anlOutLabel(context, "**DataCollection-TODO**");
			break;
	}
}




#if 0
static void
a_anlDataEnumeration(
	a_anlContext context,
	c_object object,
	c_enumeration enumeration)
{
    c_array elements = enumeration->elements;
	c_type element;
	c_long i, size;
	size = c_arraySize(elements);
	a_anlOutLabel(context, "Enumeration");
	a_anlOutLongvalue(context, "#Elements", (long)size);
	for (i = 0; i < size; i++) {
		element = elements[i];
		if (element) {
			c_object *currentObjectPtr, referenceObject;
			currentObjectPtr = (c_object *)element;
			referenceObject = (c_object)(c_address)*currentObjectPtr;
			a_anlOutIndexedElement(context, "Elm", (long)i, (unsigned int)element);
			printf("=%X", (unsigned int)referenceObject);
			a_anlAddDataRefs(context, (unsigned int)object, (unsigned int)element);
		}
	}
}
#endif


#if 0
static void
a_anlDataPrimitive(
	a_anlContext context,
	c_object object,
	c_primitive primitive)
{
    c_primKind primKind = primitive->kind;
	a_anlOutLongAndStringvalue(context, "primKind", primKind, a_utlPrimKindStr(primKind));
}
#endif


static void
a_anlDataTypeDef(
	a_anlContext context,
	c_object object,
	c_typeDef typeDef_v)
{
	c_type alias = typeDef_v->alias;
	c_string typeName = c_metaObject(typeDef_v)->name;
	c_string aliasName = alias ? c_metaObject(alias)->name : NULL;
	a_anlOutLabel(context, "typeDef");
	a_anlOutPointervalue(context, "type", (void *)typeDef_v);
	a_anlOutStringvalue(context, "typeName", typeName);
	a_anlOutPointervalue(context, "typeAlias", (void *)alias);
	if (aliasName) {
		a_anlOutStringvalue(context, "aliasName", aliasName);
	}
}


static void
a_anlDataStructure(
	a_anlContext context,
	c_object object,
	c_structure structure)
{
	c_long memSize = c_arraySize(structure->members);
	c_long refSize = c_arraySize(structure->references);
    c_long i;
	c_member member, reference;

	c_long offset;
	c_string name, typeName;
	c_metaKind kind;
	c_type type;
	c_object referenceObject, *referenceObjectPtr, referencedObject;

	a_anlOutLabel(context, "Structure");	

	a_anlOutLongvalue(context, "#Members", (long)memSize);
	a_anlOutLongvalue(context, "#References", (long)refSize);

	for (i = 0; i < memSize; i++) {
		member = structure->members[i];
		if (member) {
			offset = member->offset;
			name = c_specifier(member)->name;
			kind = c_baseObject(member)->kind;
			type = c_specifier(member)->type;
			typeName = c_metaObject(type)->name;
			a_anlOutIndexedElement(context, "Mem", (long)i, (c_address)member);
			a_anlOutStringvalue(context, "name", name);
			a_anlOutPointervalue(context, "type", (void *)type);
			a_anlOutStringvalue(context, "typeName", typeName);
			a_anlOutLongAndStringvalue(context, "kind", (long)kind, a_utlMetaKindStr(kind));
			a_anlOutLongvalue(context, "offset", offset);
			
			a_anlAssertType(context, object, c_baseObject(name), c_getType(name), "c_string");
			a_anlAssertType(context, object, c_baseObject(member), c_getType(member), "c_member");

			if (c_typeIsRef(type)) {
				c_address rawAddress = (c_address)((c_address)object + (c_address)offset);
				c_object *refObjPtr = refObjPtr = (c_object *)rawAddress;
			    c_address refAddress = (c_address)*refObjPtr;
				a_anlAddDataRefs(context, (c_address)object, (c_address)refAddress, (c_address)rawAddress);
			}
		}
	}

//	a_anlAssertType(context, object, c_baseObject(attribute), c_getType(attribute), "c_attribute");
//	a_anlAssertTypeDerived(context, object, c_baseObject(type), c_getType(type), "c_type");


	for (i = 0; i < refSize; i++) {
		reference = structure->references[i];
		if (reference) {
			offset = reference->offset;
			name = c_specifier(reference)->name;
			referenceObject = (c_object)((c_address)object + (c_address)offset);
			referenceObjectPtr = referenceObject;
			referencedObject = *referenceObjectPtr;
			
			a_anlOutIndexedElement(context, "Ref", (long)i, (c_address)referenceObject);
			a_anlOutPointervalue(context, "", (void *)referencedObject);
//			a_anlAssertType(context, object, c_baseObject(name), c_getType(name), "c_string");            // zinloos
			a_anlAssertType(context, object, c_baseObject(reference), c_getType(reference), "c_member");
		}
	}
}
			



#if 0
static void
a_anlDataUnionCase(
	a_anlContext context,
	c_object object,
	c_unionCase unionCase)
{
	char *name = c_specifier(object)->name;
	c_type type = c_specifier(object)->type;
	a_anlOutStringvalue(context, "name", name);
	a_anlAddDataRefs(context, (c_address)object, (c_address)name, (c_address)&(c_specifier(object)->name));
	a_anlOutPointervalue(context, "type", (void *)type);
	a_anlAddDataRefs(context, (c_address)object, (c_address)type, (c_address)&(c_specifier(object)->type));
	a_anlAssertType(context, object, c_baseObject(name), c_getType(name), "c_string");
	a_anlAssertTypeDerived(context, object, c_baseObject(type), c_getType(type), "c_type");
}
#endif




/* [Private, part of a call back construction]
 * Compute references of a data-object.
 * In this case, we only need to increase the TypeReference
 * of its type (result og c_getType()). Although not according to
 * specifications, we must be prepared for type == NULL.
 */
static void
a_anlDataObject(
	a_anlContext context,
	c_object o,
	a_lstObjectKind objectKind)
{
	c_type type = c_getType(o);
	char *typeName = type ? c_metaObject(type)->name : NULL;
	c_type actualType = c_typeActualType(type);
	c_metaKind typeMetaKind = c_baseObject(type)->kind;
	a_anlOutStringvalue(context, "Typename", typeName);
	a_anlOutLongAndStringvalue(context, "TypeMetaKind", typeMetaKind, a_utlMetaKindStr(typeMetaKind));

	switch (typeMetaKind) {
		case M_CLASS:        a_anlDataClass(context, o, c_class(actualType));             break;
		case M_COLLECTION:   a_anlDataCollection(context, o, c_collectionType(actualType)); break;
//		case M_INTERFACE:    a_anlDataInterface(context, o, c_interface(actualType));     break;
//		case M_ENUMERATION:  a_anlDataEnumeration(context, o, c_enumeration(actualType)); break;
//		case M_PRIMITIVE:    a_anlDataPrimitive(context, o, c_primitive(actualType));     break;
		case M_TYPEDEF:      a_anlDataTypeDef(context, o, c_typeDef(actualType));         break;
//		case M_EXCEPTION:
		case M_STRUCTURE:    a_anlDataStructure(context, o, c_structure(actualType));     break;
//		case M_UNIONCASE:    a_anlDataUnionCase(context, o, c_unionCase(actualType));     break;
		default:
			a_anlOutLongAndStringvalue(context, "**Data-TODO**", typeMetaKind, a_utlMetaKindStr(typeMetaKind));
			break;
	}
	a_anlAssertTypeDerived(context, o, c_baseObject(type), c_getType(type), "c_type");    // zinloze actie
}








/********************************************************************
 WALK & CALL BACK FUNCTIONS FOR COMPUTING ALL REFERENCES
 ********************************************************************/


typedef enum a_anlAddrKind {
	A_ANL_NOT_IN_OBJECT,
	A_ANL_BEFORE_FIRST_OBJECT,
	A_ANL_AFTER_LAST_OBJECT,
	A_ANL_BETWEEN_OBJECTS,
	A_ANL_IS_OBJECT_ADDR,
	A_ANL_IN_OBJECT,
	A_ANL_IN_HEADER_IS_TYPEREF,
	A_ANL_IN_HEADER
} a_anlAddrKind;



/* Determines the kind of pointer at 'address'.
 * If 'address' is part of an object (inside object data
 * or header), *objAddress will be filled with the object's
 * start address.
 * Warning: exhaustive function! Use with care.
 */
static a_anlAddrKind
a_anlDetermineAddrKind(
	a_anlContext context,
	c_address address,
	c_address *objAddress)
{
	a_anlAddrKind result;
	if (a_lstInList(context->list, address)) {
		result = A_ANL_IS_OBJECT_ADDR;
		*objAddress = address;
	} else {
		*objAddress = a_lstFindHighestLowerObjAddr(context->list, address);
		if (0 == *objAddress) {
			result = A_ANL_BEFORE_FIRST_OBJECT;
		} else if (a_lstInObject(context->list, *objAddress, address)) {
			result = A_ANL_IN_OBJECT;
		} else {
			*objAddress = a_lstFindLowestHigherObjAddr(context->list, address);
			if (0 == *objAddress) {
				result = A_ANL_AFTER_LAST_OBJECT;
			} else if (a_lstInObjHeader(*objAddress, address)){
				if ( address == (c_address)c_getType(c_object((c_voidp)*objAddress)) ) {
					result = A_ANL_IN_HEADER_IS_TYPEREF;
				} else {
					result = A_ANL_IN_HEADER;
				}
			} else {
				result = A_ANL_BETWEEN_OBJECTS;
				*objAddress = -1;
			}
		}
	}
	return result;
}
	



/* Report Kind
 *   - What to report (in debug mode)?
 */
typedef enum a_anlReportKind {
	A_ANL_REP_UNDEFINED,
	A_ANL_REP_TO_HDR,
	A_ANL_REP_HDR_TO_INSIDE_OBJ,
	A_ANL_REP_HDR_TO_OUTSIDE_OBJ,
	A_ANL_REP_OUTSIDE_OBJ_TO_INSIDE_OBJ,
	A_ANL_REP_OBJ_TO_HDR,
	A_ANL_REP_OBJ_TO_OLD_OBJ,
	A_ANL_REP_HDR_TO_UNKNOWN
} a_anlReportKind;



/* Reports unexpected pointervalues, with comments
 */
static void
a_anlOutReport(
	a_anlContext context,
	c_address rawFromAddress,
	c_address objFromAddress,
	c_address rawToAddress,
	c_address objToAddress,
	long refCount,
	a_anlReportKind reportKind)
{
	if (-1 == objFromAddress) {
		a_anlOutObjectAddress(context, rawFromAddress, "*");
	} else {
		a_anlOutObjectAddress(context, objFromAddress, "*");
	}
	a_anlOutLabel(context, "*Unexpected*:");
	switch (reportKind) {
		case A_ANL_REP_TO_HDR:
			a_anlOutPointervalue(context, "Pointer at", (void *)rawFromAddress);
			a_anlOutPointervalue(context, "points to object's header at", (void *)rawToAddress);
			a_anlOutPointervalue(context, "(Object Start Address)", (void *)objToAddress);
			break;
		case A_ANL_REP_HDR_TO_INSIDE_OBJ:
			a_anlOutPointervalue(context, "Pointer in header at", (void *)rawFromAddress);
			a_anlOutPointervalue(context, "points to inside object", (void *)objToAddress);
			a_anlOutPointervalue(context, "at", (void *)rawToAddress);
			break;
		case A_ANL_REP_OUTSIDE_OBJ_TO_INSIDE_OBJ:
			a_anlOutPointervalue(context, "Pointer (outside object) at", (void *)rawFromAddress);
			a_anlOutPointervalue(context, "points to inside object", (void *)objToAddress);
			a_anlOutPointervalue(context, "at", (void *)rawToAddress);
			break;
		case A_ANL_REP_HDR_TO_OUTSIDE_OBJ:
			a_anlOutPointervalue(context, "Pointer in header at", (void *)rawFromAddress);
			a_anlOutPointervalue(context, "points to outside object at", (void *)rawToAddress);
			break;
		case A_ANL_REP_OBJ_TO_HDR:
			a_anlOutPointervalue(context, "Pointer at", (void *)rawFromAddress);
			a_anlOutPointervalue(context, "points to header address", (void *)rawToAddress);
			a_anlOutPointervalue(context, "which is part of object", (void *)objToAddress);
			break;
		case A_ANL_REP_OBJ_TO_OLD_OBJ:
			a_anlOutLabel(context, "*Dangling Pointer*:");
			a_anlOutPointervalue(context, "Pointer at", (void *)rawFromAddress);
			a_anlOutPointervalue(context, "points to address", (void *)rawToAddress);
			a_anlOutLongvalue(context, "what seems to be the remains of an old, unknown object, with RefCount", refCount);
			break;
		case A_ANL_REP_HDR_TO_UNKNOWN:
			a_anlOutPointervalue(context, "Pointer in header at", (void *)rawFromAddress);
			a_anlOutPointervalue(context, "points to something not being an object start at", (void *)rawToAddress);
			break;
		default:
			break;
	}
	a_anlOutNewline(context);
}



static int
a_anlAddressIsInShm(
	a_anlContext context,
	c_address address)
{
	return (context->shmAddress <= address) && (address < context->shmAddress + context->shmSize) ? 1 : 0;
}

	




/* Checks 'address' to see if it could be the start of a database object,
 * by checking the existence of the confidence value, sugesting it (still)
 * has a header. If it does, fill the reference count.
 */
static int
a_anlCheckAddressForOldObject(
	a_anlContext context,
	c_address address,
	long *refCount)
{
	int result = 0;
	c_address hdrAddress = A_HEADERADDR(address);
	c_address *hdrAddrPtr =  (c_address *)hdrAddress;
	if (a_anlAddressIsInShm(context, hdrAddress)) {
		if (*hdrAddrPtr == A_CONFIDENCE) {
			c_address refCountAddr = A_REFCADDR(address);
			c_address *refCountAddrPtr = (c_address *)refCountAddr;
			*refCount = *refCountAddrPtr;
			if (0 < *refCount) {
				result = 1;
			}
		}
	}
	return result;
}








static int
a_anlAnalyseRemainingOccurrencesCallback(
	c_address address,
	c_address referenceAddr,
	a_anlContext context)
{
	int result = 1;
	c_address fromObjAddr;
	c_address toObjAddr;
	a_anlAddrKind fromAddrKind = a_anlDetermineAddrKind(context, address, &fromObjAddr);
	a_anlAddrKind toAddrKind = a_anlDetermineAddrKind(context, referenceAddr, &toObjAddr);
	switch (fromAddrKind)
	{
		case A_ANL_BEFORE_FIRST_OBJECT:
		case A_ANL_BETWEEN_OBJECTS:
		case A_ANL_AFTER_LAST_OBJECT:
			{
				switch (toAddrKind)
				{
					case A_ANL_IS_OBJECT_ADDR:
						a_lstAddRefAddrToEntrySublist(context->list, toObjAddr, address, L_REF_UNKNREF);
						break;

					case A_ANL_IN_OBJECT:
						a_anlOutReport(context, address, fromObjAddr, referenceAddr, toObjAddr, 0,
							A_ANL_REP_OUTSIDE_OBJ_TO_INSIDE_OBJ);
						break;

					case A_ANL_IN_HEADER:
					case A_ANL_IN_HEADER_IS_TYPEREF:
						a_anlOutReport(context, address, fromObjAddr, referenceAddr, toObjAddr, 0, A_ANL_REP_TO_HDR);
						break;

					default:   // ignore all other cases
						a_anlOutObjectAddress(context, (c_address)address,
							"Ignoring address outside of object pointing to other address, also outside of object");
						a_anlOutNewline(context);
						break;
				}
			}
			break;

		case A_ANL_IN_HEADER_IS_TYPEREF:
			{
				switch (toAddrKind)
				{
					case A_ANL_BEFORE_FIRST_OBJECT:
					case A_ANL_AFTER_LAST_OBJECT:
					case A_ANL_BETWEEN_OBJECTS:
						// report database error(?) pointer in object header does not point to object
						a_anlOutReport(context, address, fromObjAddr, referenceAddr, toObjAddr, 0, A_ANL_REP_HDR_TO_OUTSIDE_OBJ);
						break;
					case A_ANL_IN_HEADER:
					case A_ANL_IN_HEADER_IS_TYPEREF:
						// report database error(?) pointer in object header point to (other?) objectheader
						a_anlOutReport(context, address, fromObjAddr, referenceAddr, toObjAddr, 0, A_ANL_REP_TO_HDR);
						break;
					case A_ANL_IN_OBJECT:
						a_anlOutReport(context, address, fromObjAddr, referenceAddr, toObjAddr, 0, A_ANL_REP_HDR_TO_INSIDE_OBJ);
						break;
					case A_ANL_IS_OBJECT_ADDR:
						// add fromObjAddr to toObjAddr->typeRefs (?)  maybe already counted? -> mark for deletion!
						// add toObjAddr to fromObjAddr->refsToType (?)   dito
						a_lstAddRefAddrToEntrySublist(context->list, toObjAddr, fromObjAddr, L_REF_TYPEREF);
						a_lstAddRefAddrToEntrySublist(context->list, fromObjAddr, toObjAddr, L_REF_REFTOTYPE);
						a_anlOutObjectAddress(context, (c_address)address, "seems to be a typeRef");
						a_anlOutPointervalue(context, "pointing to", (void *)referenceAddr);
						a_anlOutLabel(context, "*TypeRef should already have been registered!*");
						a_anlOutNewline(context);
						break;
					default:
						a_anlOutObjectAddress(context, (c_address)address, "*TODO* Ignoring typeref header addr");
						a_anlOutNewline(context);
						break;
				}
			}
			break;
		
		case A_ANL_IN_HEADER:
			{
				switch (toAddrKind)
				{
					case A_ANL_IS_OBJECT_ADDR:
						a_lstAddRefAddrToEntrySublist(context->list, toObjAddr, address, L_REF_UNKNREF);
						break;

					default:
						a_anlOutReport(context, address, fromObjAddr, referenceAddr, toObjAddr, 0, A_ANL_REP_HDR_TO_UNKNOWN);
						break;
				}
			}
			break;
		
		
		case A_ANL_IS_OBJECT_ADDR:
		case A_ANL_IN_OBJECT:
			{
				switch (toAddrKind)
				{
					case A_ANL_BEFORE_FIRST_OBJECT:
					case A_ANL_BETWEEN_OBJECTS:
					case A_ANL_AFTER_LAST_OBJECT: {
						long refCount;
						int isOldObject = a_anlCheckAddressForOldObject(context, referenceAddr, &refCount);
						if (isOldObject) {
							a_anlOutReport(context, address, fromObjAddr, referenceAddr, toObjAddr, refCount,
								A_ANL_REP_OBJ_TO_OLD_OBJ);
						} else {
							a_anlOutObjectAddress(context, (c_address)fromObjAddr, "!");
							a_anlOutPointervalue(context, "Pointer at", (void *)address);
							a_anlOutPointervalue(context, "points to", (void *)referenceAddr);
							a_anlOutLabel(context, "which is not inside an(other) object");
							if (a_lstAddRefAddrToEntrySublist(context->list, fromObjAddr, referenceAddr, L_REF_REFTOUNKN)) {
								a_anlOutStringvalue(context, "Update", "ok");
							} else {
								a_anlOutStringvalue(context, "Update", "*FAIL*");
							}
							a_anlOutNewline(context);
						}
						break;
					}
					case A_ANL_IN_HEADER:
					case A_ANL_IN_HEADER_IS_TYPEREF:
						// report database error (?) pointer in object points to inside (other) object's header
						a_anlOutReport(context, address, fromObjAddr, referenceAddr, toObjAddr, 0, A_ANL_REP_TO_HDR);
						break;
					case A_ANL_IS_OBJECT_ADDR:
					case A_ANL_IN_OBJECT:
						// report *TODO* !
						a_anlOutObjectAddress(context, (c_address)fromObjAddr, "!");
						a_anlOutLabel(context, "*TODO:DataRef*");
						a_anlOutPointervalue(context, "Pointer at", (void *)address);
						a_anlOutPointervalue(context, "points to", (void *)referenceAddr);
						if (toObjAddr != referenceAddr) {
							a_anlOutPointervalue(context, "(Object's Start Address)", (void *)toObjAddr);
						}
						if (toObjAddr == fromObjAddr) {
							a_anlOutLabel(context, "(Points to its own object!)");
						}
						a_lstAddRefAddrToEntrySublist(context->list, toObjAddr, fromObjAddr, L_REF_UNKNREF);
						a_lstAddRefAddrToEntrySublist(context->list, fromObjAddr, toObjAddr, L_REF_REFTOUNKN);
						a_anlOutNewline(context);
						break;
					default:
						a_anlOutLabel(context, "*TODO* From: IN or IS_OBJ");
						a_anlOutNewline(context);
						break;
				}
			}
			break;
		
		
		default:
			a_anlOutObjectAddress(context, (c_address)address, "*TODO* Uncategorised pointer address");
			a_anlOutNewline(context);
			break;
	}
	if (address == referenceAddr) {
		a_anlOutObjectAddress(context, address, "*Har?!*");
		a_anlOutPointervalue(context, "Points to itself", (void *)referenceAddr);
		a_anlOutNewline(context);
	}
	return result;
}






/* At the stage of calling this function, it is assumed that:
 *   - All "occurrences" have previously been collected and
 *     reside in list->occurrences
 *   - All database objects have been collected (by
 *     c_baseObjectWalk) and analysed; all data references
 *     have been registered with the corresponding entries.
 */
static int
a_anlAnalyseRemainingOccurrences(
	a_anlContext context)
{
	int result;
	a_lstOccurrencesWalkAction walkAction = (a_lstOccurrencesWalkAction)a_anlAnalyseRemainingOccurrencesCallback;
	a_anlOutLongvalue(context, "#Remaining occurrences to analyse", a_lstOccurrencesCount(context->list));
	a_anlOutNewline(context);
	result = a_lstOccurrencesWalk(context->list, walkAction, context);
	a_anlOutLongvalue(context, "#Remaining occurrences after analysis", a_lstOccurrencesCount(context->list));
	a_anlOutNewline(context);
	return result;
}



static int
a_anlSpaceLeftForObject(
	c_address objAddr,
	c_address nextObjAddr,
	c_address shmEndAddr)
{
	const long minObjSize = sizeof(c_voidp);
	return ( (0 == nextObjAddr) || (objAddr + minObjSize < nextObjAddr) ) && (objAddr + minObjSize < shmEndAddr) ? 1 : 0;
}



/* Scans the whole shared memory, at every interval of
 * sizeof(c_voidp), for confidence checks.
 */
static void
a_anlScanForRemainingConfChecks(
	a_anlContext context)
{
	const c_address startAddress = (c_address)context->shmAddress;
	if (startAddress) {
		c_voidp addressPtr = (c_voidp)startAddress;
		const size_t stepSize = sizeof(c_voidp);
		const long shmSize = context->shmSize;
		const c_address endAddress = startAddress + (c_address)shmSize;
		c_address addressValue;
		while ((c_address)addressPtr < endAddress) {
			c_address *addrPtrPtr = addressPtr;
			addressValue = (c_address)*addrPtrPtr;
			if (addressValue == A_CONFIDENCE) {
				c_address objAddr = A_OBJECTADDR((c_address)addressPtr);
				if (a_lstInList(context->list, objAddr)) {
					// obj is in list; skip the remaining of the object
					long objSize = a_lstEntryProperty(context->list, objAddr, L_PRP_OURSIZE);
					addressPtr = (c_voidp)(objAddr + objSize);
					while ((c_address)addressPtr % sizeof(c_voidp)) {     // get back into alignment
						addressPtr++;
					}
				} else {
					a_anlOutObjectAddress(context, (c_address)addressPtr, "*Unexpected*");
					a_anlOutLabel(context, "Orphaned Confidence Check");
					c_address nextObj = a_lstFindLowestHigherObjAddr(context->list, (c_address)addressPtr);
					if (a_anlSpaceLeftForObject(objAddr, nextObj, endAddress)) {
						a_anlOutLabel(context, "With enough space left for an object");
						c_address refCountAddr = A_REFCADDR(objAddr);
						c_address *refCountAddrPtr = (c_address *)refCountAddr;
						long refCount = (long)*refCountAddrPtr;
						a_anlOutLongvalue(context, "If this was an object, its ReferenceCount would have been", refCount);
					} else {
						a_anlOutLabel(context, "but there does not seem te be enough space for an object left");
					}
					if (nextObj) {
						a_anlOutPointervalue(context, "Next object starts at", (void *)nextObj);
					}
					a_anlOutNewline(context);
					addressPtr += stepSize;
				}
			} else {
				addressPtr += stepSize;
			}
		}
	}
}



/* Scans the whole shared memory, at every interval of
 * sizeof(c_voidp), for pointer adrresses. If that value
 * points to an address within the shared memory segment, it
 * _could_ be a pointer and is recorded as such.
 * Note that there's no way telling if it really is a
 * pointer at this point, as we only scan the "raw data".
 * All collected pointer addresses and their values should
 * be removed from the list as they are analysed.
 * The remaining addresses should be examined manually.
 */
static void
a_anlCollectOccurrences(
	a_anlContext context)
{
	int addResult;
	const c_address startAddress = (c_address)context->shmAddress;
	if (startAddress) {
		c_voidp addressPtr = (c_voidp)startAddress;
		const size_t stepSize = sizeof(c_voidp);
		const long shmSize = context->shmSize;
		const c_address endAddress = startAddress + (c_address)shmSize;
		c_address addressValue;
		while ((c_address)addressPtr < endAddress) {
			c_address *addrPtrPtr = addressPtr;
			addressValue = (c_address)*addrPtrPtr;
			if ( (startAddress <= addressValue) && (addressValue <= endAddress) ) {
				if ( (addressValue - context->shmAddress) % stepSize == 0) {
					addResult = a_lstAddRefAddrToEntrySublist(context->list,
							(c_address)addressValue, (c_address)addressPtr, L_REF_OCCURRENCES);
					addResult = a_lstInsertNewOccurrence(context->list, (c_address)addressPtr, addressValue);
				}
			}
			addressPtr += stepSize;
		}
		a_anlOutLongvalue(context, "#Occurrences collected", a_lstOccurrencesCount(context->list));
		a_anlOutNewline(context);
	}
}



#if 0
struct a_anlComputeRefsEntryCrossReferenceContext {
	a_anlContext anlContext;
	c_address entryAddress;
	c_address referenceAddress;
};


static int
a_anlComputeRefsEntryCrossReferenceCallbackCallback(
	c_address address,
	struct a_anlComputeRefsEntryCrossReferenceContext *context,
	a_lstRefsKind refsKind)
{
	int result = 1;
	if (address == context->referenceAddress) {
		switch (refsKind) {
			case L_REF_REFTOTYPE: printf("T->:"); break;
			case L_REF_REFTODATA: printf("D->:"); break;
			case L_REF_REFTOUNKN: printf("U->:"); break;
			default: printf("huh?"); break;
		}
		printf("%8.8X", (unsigned int)address);
	}
	return result;
}


static int
a_anlComputeRefsEntryCrossReferenceCallback(
	c_address address,
	struct a_anlComputeRefsEntryCrossReferenceContext *context,
	a_lstRefsKind refsKind)
{
	int result = 1;
	a_anlContext anlContext = context->anlContext;
	c_address entryAddress = context->entryAddress;
	c_address objAddress;
	// a_anlAddrKind addrKind = 
	a_anlDetermineAddrKind(anlContext, address, &objAddress);
	if (objAddress != -1) {
		printf("Entry: %8.8X: O:%8.8X ObjectAddress:%8.8X ",
			(unsigned int)entryAddress, (unsigned int)address, (unsigned int)objAddress);
	} else {
		printf("Entry: %8.8X: O:%8.8X Not part of an object  ", (unsigned int)entryAddress, (unsigned int)address);
	}
	printf("\n");
	return result;
}

static void
a_anlComputeRefsEntryCrossReference(
	a_anlContext anlContext,
	c_address address)
{
	struct a_anlComputeRefsEntryCrossReferenceContext context;
	context.anlContext = anlContext;
	context.entryAddress = address;
	a_lstEntryRefsWalkAction walkAction = (a_lstEntryRefsWalkAction)a_anlComputeRefsEntryCrossReferenceCallback;
	a_lstEntryReferencesWalk(anlContext->list, address, walkAction, &context, L_REF_OCCURRENCES);
}
#endif



/* Call back function for computing all
 * references of one object.
 * From here, analyse the specified objectKind.
 */
static void
a_anlComputeRefsCallback(
	c_object o,
	a_anlContext context)
{
	c_object *oPtr = &o;
	c_type type = c_getType(o);
	c_long size = type ? type->size : -1;
	a_lstObjectKind objectKind = a_anlGetObjectKind(o);
	a_anlOutObjectAddress(context, (c_address)o, a_lstGetObjectKindChar(objectKind));
	a_anlOutSize(context, size);
	a_anlOutPointervalue(context, "Type", (void *)type);
	if (type) {
//		c_address objAddr = (c_address)oPtr;
		c_address tpeAddr = A_TYPEADDR((c_address)*oPtr);
		a_anlAddTypeRefs(context, (c_address)o, (c_address)c_getType(o), (c_address)tpeAddr);
	}
	c_address nxtAddr = A_NEXTADDR((c_address)*oPtr);
	c_address prvAddr = A_PREVADDR((c_address)*oPtr);
	
	a_lstAddRefAddrToEntrySublist(context->list, (c_address)o, nxtAddr, L_REF_REFTOUNKN);
	a_lstAddRefAddrToEntrySublist(context->list, (c_address)o, prvAddr, L_REF_REFTOUNKN);

	a_lstAddRefAddrToEntrySublist(context->list, nxtAddr, (c_address)o, L_REF_UNKNREF);
	a_lstAddRefAddrToEntrySublist(context->list, prvAddr, (c_address)o, L_REF_UNKNREF);

//	a_lstRemoveOccurrence(context->list, a_nextAddr((c_address)*oPtr), nxtAddr);
//	a_lstRemoveOccurrence(context->list, a_prevAddr((c_address)*oPtr), prvAddr);

	switch (objectKind) {
		case L_DATA: a_anlDataObject(context, o, objectKind); break;
		case L_META: a_anlMetaObject(context, o, objectKind); break;
		case L_BASE: a_anlBaseObject(context, o, objectKind); break;
		case L_CLSS: a_anlBaseObject(context, o, objectKind); break;
		default: break;
	}
	a_anlOutNewline(context);
	context->dataSize += a_lstEntryProperty(context->list, (c_address)o, L_PRP_OURSIZE) + A_HEADERSIZE;
}



/********************************************************************
 CREATE STATISTICS
 ********************************************************************/


typedef struct a_anlStatsContext {
	a_anlContext anlContext;
} *a_anlStatsContext;



static int
a_anlCollectObjectsForStatisticsCallback(
	a_lstObject lstObject,
	a_anlStatsContext statsContext)
{
	int result;
	if (statsContext) {
		a_anlContext context = statsContext->anlContext;
		if (context) {
			if (lstObject) {
				c_address objectAddress = lstObject->address;
				c_address headerAddress = A_HEADERADDR(objectAddress);
				c_long objectSize = lstObject->ourSize + 1;
				while (objectSize % A_PTRSIZE) {
					objectSize++;
				}
				assert(context->stsContext);
				result = a_stsInsertNewBlock(context->stsContext, A_STS_BLOCK_OBJECTDATA, objectAddress, objectSize);
				if (result) {
					result = a_stsInsertNewBlock(context->stsContext, A_STS_BLOCK_OBJECTHEADER, headerAddress, A_HEADERSIZE);
				}
			} else {
				result = 0;
			}
		} else {
			result = 0;
		}
	} else {
		result = 0;
	}
	return result;
}



static int
a_anlScanForMemoryMarkers(
	a_anlContext context)
{
	int result;
	c_address firstObjAddr = a_lstFindFirst(context->list);
	if (0 < firstObjAddr) {
		c_address shmEndAddress = (c_address)(context->shmAddress + context->shmSize);
		c_address markerAddress;
		c_long markerSize = 4 * A_PTRSIZE;
		
		markerAddress = (c_address)(A_HEADERADDR(firstObjAddr) - markerSize);
		if (context->shmAddress <= markerAddress) {
			result = 1;
			c_address markerValueAddress;
			long *markerValuePtr;
			assert(context->stsContext);
			while ((markerAddress < shmEndAddress) && result) {
				result = a_stsInsertNewBlock(context->stsContext, A_STS_BLOCK_MARKER, markerAddress, markerSize);
				markerValueAddress = (c_address)(markerAddress + 3 * A_PTRSIZE);
				markerValuePtr = (long *)markerValueAddress;
				if (*markerValuePtr) {
					markerAddress += (markerSize + *markerValuePtr);
				} else {
					markerAddress = shmEndAddress;
				}
			}
		} else {
			result = 0;
		}
	} else {
		result = 0;
	}
	return result;
}



static int
a_anlCollectObjectsForStatistics(
	a_anlContext context)
{
	int result;
	if (context) {
		if (context->list) {
//			printf("MemoryBlocksCount: %d\n", a_stsBlocksCount(context->stsContext));
			a_lstWalkAction walkAction = (a_lstWalkAction)a_anlCollectObjectsForStatisticsCallback;
			struct a_anlStatsContext statsContext;
			statsContext.anlContext = context;
			result = a_lstListWalk(context->list, walkAction, &statsContext);
			if (result) {
//				printf("MemoryBlocksCount: %d\n", a_stsBlocksCount(context->stsContext));
				result = a_anlScanForMemoryMarkers(context);
			}
//			printf("MemoryBlocksCount: %d\n", a_stsBlocksCount(context->stsContext));
		} else {
			result = 0;
		}
	} else {
		result = 0;
	}
	return result;
}




/**
 * \brief
 * Walks over all objects and determine and analyse its references.
 *
 * This operation walks over all database objects (again) and
 * analyses all references (pointers) it might have. These
 * references will be counted, as well the counters of the referenced
 * object.
 *
 * \param context
 * This file's context.
 *
 * \param base
 * Database that previously has been successfully opened.
 *
 * \param address
 * Shared Memory Start Address
 *
 * \param size
 * Shared Memory Size
 *
 * \note
 * Make sure the list has been filled (using \a a_anlFillList) before
 * calling this function
 *
 * \todo
 * Provide a return value
 *
 * \todo
 * Remove the need for specifying address and size, for those are
 * already known within the context!
 */
void
a_anlComputeRefs(
	a_anlContext context,
	c_base base,
	c_address address,
	c_long size)
{
	c_baseWalkAction walkAction = (c_baseWalkAction)a_anlComputeRefsCallback;
	assert(base);
	assert(context->list);
	context->shmAddress = address;
	context->shmSize = size;

	a_anlVerboseOutString(context, "Scanning for all pointers");
	a_anlCollectOccurrences(context);
//	a_lstOccurrencesDump(context->list);
	
	a_anlVerboseOutString(context, "Analysing all database objects and cross-referencing with collected pointers");
	c_baseObjectWalk(base, walkAction, context);   // c_baseObjectWalk in c_base.h (in debug version only!)

	a_anlVerboseOutString(context, "Analysing uncategorised, remaining pointers");
	a_anlAnalyseRemainingOccurrences(context);
//	a_lstDeleteOccurrences(context->list);   // there's no need to do this here

	a_anlVerboseOutString(context, "Scanning for (old) Confidence Checks");
	a_anlScanForRemainingConfChecks(context);

	a_anlVerboseOutString(context, "Recalculating internal counters");
	a_lstListUpdateEntryCounters(context->list);

	a_anlVerboseOutString(context, "Scanning for inconsistencies");
	a_lstCheckConsistencies(context->list);


//	a_anlComputeRefsEntryCrossReference(context, (c_address)0xA09CE2C0);

	a_anlVerboseOutString(context, "Collecting statistical data");
	a_anlCollectObjectsForStatistics(context);

	a_anlVerboseOutString(context, "Searching for unused memory blocks");
	a_stsCalculateFreeSpaces(context->stsContext, context->shmAddress, context->shmSize);


//	a_lstTreeDump(context->list);   // for test purposes only
}




/********************************************************************
 WIPE OBJECTS FROM SHARED MEMORY
 ********************************************************************/


static int
a_anlWipeObjectsCallback(
	a_lstObject lstObject,
	void *actionArg)
{
	int result = 1;  // always continue the walk
	if (lstObject) {
		c_address address = lstObject->address;
		long ourSize = lstObject->ourSize;
		if (address && (0 < ourSize)) {
			c_address startAddress = (c_address)(address - A_HEADERSIZE);
			c_address endAddress = (c_address)(address + ourSize - 1);
			c_address walkAddress;
			c_address *walkAddressPtr;
			for (walkAddress = startAddress; walkAddress < endAddress; walkAddress += sizeof(c_voidp)) {
				walkAddressPtr = (c_address *)walkAddress;
				*walkAddressPtr = (c_address)A_WIPEVALUE;
			}
		}
	}
	return result;
}



/**
 * \brief
 * Wipes (overwrites) all known database objects in Shared Memory.
 *
 * This operation wipes (overwrites) all known database objects in
 * the Shared Memory. Before calling this function, it is assumed
 * that a Shared Memory with a Splice database is currently attached
 * and all database objects are collected.\n
 * After a successful operation of this function, you might want to
 * print a hex dump of the memory.
 *
 * \param context
 * The context of this file (a_anl) that holds all information to
 * perform the wipe.
 *
 * \return
 * This operation returns true (1) if all objects were successfully
 * overwriten, false (0) if anything failed or if the (internal) list
 * of database objects is empty.
 *
 * \note
 * This is (currently) the only AAPI operation that alters any data
 * in the Shared Memory.
 *
 * \warning
 * Use with care! After this operation, no Splice operations can be
 * performed, for the database is no longer valid.
 */
int
a_anlWipeObjects(
	a_anlContext context)
{
	int result;
	a_lstWalkAction walkAction = (a_lstWalkAction)a_anlWipeObjectsCallback;
	result = a_lstListWalk(context->list, walkAction, NULL);
	return result;
}



//END a_anl.c
