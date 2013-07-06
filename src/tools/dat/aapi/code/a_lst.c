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
 * This entry needs to be hidden from the outside, for it
 * uses trees (from a_tre) as some members.
 */
typedef struct a_lstEntry {
	c_address       address;          // object's start address
	c_long          refCount;         // value from SPLICE
	c_long          alignment;        // value from SPLICE
	c_long          size;             // value from SPLICE
	c_long          ourSize;          // object size as we compute it (differs from size with c_strings!)
	char           *objectName;       // name of the object if not L_DATA
	char           *typeDesc;         // description of this object's type
	char           *typeName;         // name of this object's type
	char           *value;            // mostly values from c_string types
	char           *note;             // error messages
	a_counter       occurrencesCount;
	a_counter       typeRefsCount;
	a_counter       dataRefsCount;
	a_counter       unknRefsCount;
	a_counter       refsToTypeCount;
	a_counter       refsToDataCount;
	a_counter       refsToUnknCount;
	a_treTree       occurrences;      // occurrences (raw!) throughout the whole shm for this entry
	a_treTree       typeRefs;         // which object(s) point to us as type?
	a_treTree       dataRefs;         // which object(s) point to us as data?
	a_treTree       unknRefs;
	a_treTree       refsToType;       // to which object do we point to as type? (only one!)
	a_treTree       refsToData;       // to which objects do we point to as data?
	a_treTree       refsToUnkn;
	a_lstObjectKind kind;
} *a_lstEntry;



/* Hidden data structure for the list; here we use a tree
 */
struct a_lstList_s {
	a_treTree tree;              // the actual structure holding all entries
	a_hshHashtable occurrences;
	c_long occurrencesArraySize; // inititial value for array size
	a_counter counters[L_COUNT]; // counters for all objectKinds
	a_lstEntry lastEntry;        // caching one entry
};



/***********************************************************
 UTILITY FUNCTIONS
 ***********************************************************/


/**
 * \brief
 * Returns a single character, specifying the objectKind
 *
 * \param objectKind
 * \return
 *
 * \see
 * a_lstObjectKind
 */
char *
a_lstGetObjectKindChar(
	a_lstObjectKind objectKind)
{
	switch (objectKind) {
		case L_DATA: return "D"; break;
		case L_META: return "M"; break;
		case L_BASE: return "B"; break;
		case L_CLSS: return "C"; break;
		case L_UNDF: return "?"; break;
		default:     return "."; break;  // should not occur!
	}
}



/***********************************************************
 COMPARE FUNCTIONS (for call back)
 ***********************************************************/


/* Compares the address values of two entries.
 * Returns -1 if entry1 <  entry 2
 *          0           ==
 *          1            >
 *        -99 if entry1 or entry2 is NULL
 * This function is needed by module a_tre, passed on to with
 * a pointer to function type.
 */
static int
a_lstCompareEntries(
	c_voidp entry1Ptr,
	c_voidp entry2Ptr)
{
	int result;
	a_lstEntry entry1 = (a_lstEntry)entry1Ptr;
	a_lstEntry entry2 = (a_lstEntry)entry2Ptr;
	if ( (!entry1) || (!entry2) ) {
		result = -99;
	} else if ((c_address)(entry1->address) < (c_address)(entry2->address)) {
		result = 1;
	} else if ((c_address)(entry1->address) > (c_address)(entry2->address)) {
		result = -1;
	} else {
		result = 0;
	}
	return result;
}



/* Compares two pointer values.
 * Returns -1 if ptr1 < ptr2
 *          0    ptr1 = ptr2
 *          1    ptr2 < ptr1
 * Returns -99 (error!) if ptr1 or ptr2 == NULL
 */
static int
a_lstComparePointerValues(
	c_voidp ptr1,
	c_voidp ptr2)
{
	int result;
	if (ptr1 && ptr2) {
		c_ulong ulong1 = (c_ulong)ptr1;
		c_ulong ulong2 = (c_ulong)ptr2;
		if (ulong1 < ulong2) {
			result = 1;
		} else if (ulong2 < ulong1) {
			result = -1;
		} else {
			result = 0;
		}
	} else {
		result = -99;
	}
	return result;
}




/***********************************************************
 CREATION & DESTROY
 ***********************************************************/


/* Creates a new entry and returns a pointer to that entry
 */
static a_lstEntry 
a_lstNewEntry(
	c_address address,
	c_long refCount,
	a_lstObjectKind kind,
	char *objectName,
	char *typeDesc,
	char *typeName,
	c_long alignment,
	c_long size)
{
	a_lstEntry entry = a_memAlloc(sizeof(struct a_lstEntry));
	if (entry) {
		entry->address          = address;
		entry->refCount         = refCount;
		entry->kind             = kind;
		entry->objectName       = a_memStrdup(objectName);
		entry->typeDesc         = a_memStrdup(typeDesc);
		entry->typeName         = a_memStrdup(typeName);
		entry->alignment        = alignment;
		entry->size             = size;
		entry->ourSize          = size; // initially
		entry->value            = NULL;
		entry->note             = NULL;
		entry->occurrencesCount = 0;
		entry->typeRefsCount    = 0;
		entry->dataRefsCount    = 0;
		entry->unknRefsCount    = 0;
		entry->refsToTypeCount  = 0;
		entry->refsToDataCount  = 0;
		entry->refsToUnknCount  = 0;
		entry->occurrences      = NULL;  // subtree('s) will be created
		entry->typeRefs         = NULL;  //         at first insertion!
		entry->dataRefs         = NULL;
		entry->unknRefs         = NULL;
		entry->refsToType       = NULL;
		entry->refsToData       = NULL;
		entry->refsToUnkn       = NULL;
	}
	return entry;
}



/* Destroys (frees) an entry.
 * If entry == NULL, this function will do nothing (fail safe).
 */
static void
a_lstDestroyEntry(
	a_lstEntry *entry)
{
	if (*entry) {
		a_memFree((*entry)->objectName);
		a_memFree((*entry)->typeDesc);
		a_memFree((*entry)->typeName);
		a_memFree((*entry)->value);
		a_memFree((*entry)->note);
		a_treDestroyTree((*entry)->occurrences, NULL);
		a_treDestroyTree((*entry)->typeRefs, NULL);
		a_treDestroyTree((*entry)->dataRefs, NULL);
		a_treDestroyTree((*entry)->unknRefs, NULL);
		a_treDestroyTree((*entry)->refsToType, NULL);
		a_treDestroyTree((*entry)->refsToData, NULL);
		a_treDestroyTree((*entry)->refsToUnkn, NULL);
		a_memFree(*entry);
	}
}



/**
 * \brief
 * Creates a new list
 *
 * This operation creates a list (on heap) and returns a pointer
 * to that list. This operation will fail if the memory can not
 * be allocated.
 *
 * \return
 * Pointer to the newly created list, or NULL if the operation
 * fails.
 *
 * \remark
 * This operation uses os_malloc() internally, intending to be
 * platform independent.
 *
 * \see
 * a_lstList a_lstDestroyList
 */
a_lstList
a_lstCreateList(
	c_long occurrencesArraySize)
{
	a_lstList list = a_memAlloc(sizeof(struct a_lstList_s));
	if (list) {
		c_long i;
		for (i = 0; i < L_COUNT; i++) {
			list->counters[i] = 0;
		}
		list->tree = a_treCreateTree();
		list->occurrencesArraySize = occurrencesArraySize;
		list->occurrences = NULL;  // will be created later
		list->lastEntry = NULL;
	}
	return list;
}



/**
 * \brief
 * Sets a new (preferable) array size for holding the
 * occurrences.
 *
 * This operation sets a new array size for holding the
 * occurrences list. The occurrences list is a sublist of \a list
 * and is to held all pointer values that a scan through the raw
 * data can find. The implementation of this occurrences list is
 * a \a hash \a table, that uses an array. For performance
 * reasons, it is desired to set the array size to a specific
 * value.\n
 * If this function is not used, a predefined, default array size
 * will be used.\n
 * Once the occurrences (sub)list is created, this function has no
 * meaning. The occurrences (sub)list will be created at first
 * insertion into that list.
 *
 * \param list
 * The list in which the array size for the Occurrences (sub)list
 * must be altered. If \a list is NULL, this operation will fail.
 *
 * \param mmStateMaxUsed
 * Value of \a mmState->maxUsed. It uses a macro in \a a_def.h for
 * determining the optimal array size.
 *
 * \return
 * True (1) if the new value was set, false (0) if it was not.
 *
 * \see
 * a_lstInsertNewOccurrence a_def.h
 */
int
a_lstSetNewOccurrencesArraySize(
	a_lstList list,
	c_long mmStateMaxUsed)
{
	int result;
	if (list) {
		list->occurrencesArraySize = A_OCCARRSIZEFROMMAXUSED(mmStateMaxUsed);   /* see a_def.h */
		result = 1;
	} else {
		result = 0;
	}
	return result;
}




/**
 * \brief
 * Destroys all entries in a list and the list itself.
 *
 * This operation destroys all entries in a list, including its
 * occurrences sublist and afterwards the list itself, freeing up
 * memory.
 *
 * \param list
 * The list to destroy. If \a list is NULL, this operation will fail.
 *
 * \return
 * True (1) if the list was successfully destroyed, false (0) if
 * anything failed.
 *
 * \see
 * a_lstList a_lstCreateList
 */
int
a_lstDestroyList(
	a_lstList list)
{
	int result = 0;
	if (list) {
		if (list->tree) {
			a_treDestroyTree(list->tree, (a_treFreeAction)a_lstDestroyEntry);
		}
		if (list->occurrences) {
			a_hshDestroyHashtable(list->occurrences);
		}
		a_memFree(list);
		result++;
	}
	return result;
}





/***********************************************************
 LIST COUNTER OPERATIONS
 ***********************************************************/


/* Increases an objectKind counter.
 * Preconditions: objectKind is within its bounds and list != NULL.
 */
static void
a_lstIncCounter(
	a_lstList list,
	a_lstObjectKind objectKind)
{
	if (list) {
		if ( (L_UNDEFINED < objectKind) && (objectKind < L_COUNT) ) {
			list->counters[objectKind]++;
		}
	}
}



/**
 * \brief
 * Returns the number of objects in a list, specified by its
 * \a objectKind.
 *
 * This operation returns the number of objects of a specific
 * kind in the list.
 *
 * \param list
 * The list from which the number of objects is requested.
 * If \a list is NULL, this operation will fail.
 *
 * \param objectKind
 * The kind of object that needs to be counted.
 *
 * \return
 * The number of objects counted in \a list, of specified type, or
 * -1 if the operation fails.
 *
 * \see
 * a_lstList a_lstCountAll a_lstObjectKind
 */
a_counter
a_lstCount(
	a_lstList list,
	a_lstObjectKind objectKind)
{
	a_counter result;
	if (list) {
		if ( (L_UNDEFINED <= objectKind) && (objectKind < L_COUNT) ) {
			result = list->counters[objectKind];
		}
	} else {
		result = -1;
	}
	return result;
}



/**
 * \brief
 * Returns the total number of objects in a list.
 *
 * This operation returns the total number of objects in a list.
 *
 * \param list
 * The list from which the number of objects is requested. If \a list
 * is NULL, this operation will fail.
 *  
 * \return
 * The number of objects counted in \a list, of specified type, or -1 
 * if the operation fails.
 *
 * \see
 * a_lstCount
 */
a_counter
a_lstCountAll(
	a_lstList list)
{
	a_counter result;
	assert(list);
	if (list) {
		int i;
		a_counter subResult;
		result = 0;
		for (i = 0; (i < L_COUNT) && (0 <= result); i++) {
			subResult = a_lstCount(list, i);
			if (0 <= subResult) {
				result += subResult;
			} else {
				result = -1;
			}
		}
	} else {
		result = -1;
	}
	return result;
}




/***********************************************************
 ENTRY MANIPULATION (Insert, Find)
 ***********************************************************/


/* Inserts an entry into the list
 * If the entry can not be inserted (because list == NULL),
 * the entry will be destroyed(!)
 * Returns 0 if operation failed
 */
static int
a_lstInsertEntry(
	a_lstList list,
	a_lstEntry entry,
	int dupesAllowed)
{
	int result;
	if (list && entry) {
		a_treSortAction sortAction = (a_treSortAction)a_lstCompareEntries;
		result = a_treInsertNode(list->tree, (c_voidp)entry, sortAction, dupesAllowed);
	} else {
		result = 0;
	}
	return result;
}



/**
 * \brief
 * Creates a new entry and inserts that entry into the list.
 *
 * This operation will create an entry holding the basic information
 * about a SPLICE database object and inserts that entry into the
 * list. Internally, the list will be ordered at all times, sorted
 * by the address's value.
 *
 * \param list
 * The list in which a new entry must be created and inserted. If
 * list is NULL, the operation will fail.
 *
 * \param address
 * The memory address of the database object.
 *
 * \param refCount
 * The value of the object's Reference Count, as it is returned by
 * c_refCount() (see \a c_base.h ).
 *
 * \param objectKind
 * The object's kind as it is categorised by AAPI.
 * 
 * \param objectName
 * The object's name. Objects of kind L_DATA have no name.
 *
 * \param typeDesc
 * A description of the object's type (AAPI generated)
 *
 * \param typeName
 * The name of the object's type. In technical terms, the value of
 * \a c_metaObject(c_getType())->name.
 *
 * \param alignment
 * The object's alignment, as it is held by its type. In technical
 * terms, the value of \a c_getType()->alignment.
 *
 * \param size
 * The object's size, as it is held by its type. In technical terms,
 * the value of \a c_getType()->size.
 *
 * \return
 * True (>0) if operation was successful
 *
 * \see
 * a_lstList a_lstObjectKind
 */
int
a_lstInsertNewEntry(
	a_lstList list,
	c_address address,
	c_long refCount,
	a_lstObjectKind objectKind,
	char *objectName,
	char *typeDesc,
	char *typeName,
	c_long alignment,
	c_long size)
{
	int result;
	if (address) {
		a_lstEntry entry = a_lstNewEntry(address, refCount, objectKind, objectName, typeDesc, typeName, alignment, size);
		result = a_lstInsertEntry(list, entry, A_LST_NODUPESALLOWED);
	} else {
		result = 0;
	}
	if (result) {
		a_lstIncCounter(list, objectKind);
	}
	return result;
}



/**
 * \brief
 * Returns the first entry of the (sorted) list
 *
 * This operation returns the first entry of \a list. Internally,
 * the list is kept in a tree structure. The first entry is the
 * far most left leaf node.
 *
 * \param list
 * the list of which the first entry must be returned.
 *
 * \return
 * Address value of list's first entry, or -1 if the list is empty.
 *
 * \see
 * a_lstList a_lstFindLast
 */
c_address
a_lstFindFirst(
	a_lstList list)
{
	a_lstEntry entry = (a_lstEntry)a_treFindFirstValue(list->tree);
	return entry ? entry->address : 0;
}



/**
 * \brief
 * Returns the last entry of the (sorted) list
 *
 * This operation returns the last entry of \a list. Internally,
 * the list is kept in a tree structure. The last entry is the
 * far most right leaf node.
 *
 * \param list
 * The list of which the last entry must be returned.
 *
 * \return
 * Address value of list's last entry, or -1 if the list is empty.
 *
 * \see
 * a_lstList a_lstFindFirst
 */
c_address
a_lstFindLast(
	a_lstList list)
{
	a_lstEntry entry = (a_lstEntry)a_treFindLastValue(list->tree);
	return entry ? entry->address : 0;
}



/* Finds an entry in the list, specified by its address and
 * returns a pointer to that entry, or NULL if not found.
 * Internally, a cache-entry is checked first, otherwise
 * a full tree search will be performed.
 */
static a_lstEntry
a_lstFindEntry(
	a_lstList list,
	c_address address)
{
	a_lstEntry result = NULL;
	if (list->lastEntry) {
		if (list->lastEntry->address == address) {
			result = list->lastEntry;
		}
	}
	if (!result) {
		a_treSortAction compareAction = (a_treSortAction)a_lstCompareEntries;
		a_lstEntry searchEntry = a_lstNewEntry(address, -1, L_UNDEFINED, "", "", "", -1, -1);
		result = (a_lstEntry)a_treFindValue(list->tree, searchEntry, compareAction);
		if (result) {
			list->lastEntry = result;
		}
		a_lstDestroyEntry(&searchEntry);
	}
	return result;
}



/**
 * \brief
 * Checks whether an entry with specified address exists in the list.
 *
 * This operation checks the list to see if an entry exists with
 * specified address.
 *
 * \param list
 * The list to search in
 *
 * \param address
 * The entry's address to search for
 *
 * \return
 * Boolean value specifying whether an entry with the specified address
 * is found.
 *
 * \see a_lstList
 */
int
a_lstInList(
	a_lstList list,
	c_address address)
{
	return a_lstFindEntry(list, address) ? 1 : 0;
}



/**
 * \brief
 * Checks whether an address lays within an object.
 *
 * This operation checks whether for a specific object, of which an
 * entry holding this object's address is searched for in the list,
 * a certain address is being part of.\n
 * Internally, an AAPI computed object-size is used for determining
 * the object's end.
 *
 * \param list
 * The list to search in
 *
 * \param objAddress
 * The object's address to check. Internally, the list is searched
 * for an entry holding this address.
 *
 * \param checkAddress
 * The address to check
 *
 * \return
 * Boolean value specifying whether \a checkAddress is considered to
 * be part of the object with Start Address \a objAddress. If the
 * list is empty or no entry with specified address is found, the
 * result is also false.
 *
 * \note
 * The Object's header is not considered to be a part of the object
 * itself.
 *
 * \see
 * a_lstList a_lstInObjHeader
 */
int
a_lstInObject(
	a_lstList list,
	c_address objAddress,
	c_address checkAddress)
{
	int result;
	c_long size = a_lstEntryProperty(list, objAddress, L_PRP_OURSIZE);
	if (-1 < size) {
		c_address objEndAddress = objAddress + size - 1;
		result = ( (objAddress <= checkAddress) && (checkAddress <= objEndAddress) ) ? 1 : 0;
	} else {
		result = 0;
	}
	return result;
}



/**
 * \brief
 * Checks whether an address lays within an object's header.
 *
 * This operation checks whether for a specific object address
 * a certain address is being part of its header.\n
 *
 * \param objAddress
 * The object's address to check.
 *
 * \param checkAddress
 * The address to check
 *
 * \return
 * Boolean value specifying whether \a checkAddress is considered to
 * be part of the object's header with Start Address \a objAddress.
 *
 * \see
 * a_lstInObject
 *
 * \note
 * There will be no check performed if \a objAddress really exists as
 * the start of an object in the list.
 *
 * \todo
 * Move this function from a_lst to a_anl.
 */
int
a_lstInObjHeader(
	c_address objAddress,
	c_address checkAddress)
{
	int result;
	c_address headerAddress = A_HEADERADDR(objAddress);
	if (0 <= headerAddress) {
		result = ( (headerAddress <= checkAddress) && (checkAddress <= objAddress) ) ? 1 : 0;
	} else {
		result = 0;
	}
	return result;
}



/**
 * \brief
 * Returns the \a highest \a lower address in the list, if any.
 *
 * This operation returns the highest object address that is
 * \b lower than specified address, in other words: the nearest
 * lower.
 *
 * \param list
 * The list to search in
 *
 * \param currentAddr
 * The address that a \a highest \a lower address must be searched
 * from
 *
 * \return
 * Address of the highest lower entry found, 0 if the \a list is
 * NULL or empty, or when \a currentAddr is already the lowest
 * address in the list.
 *
 * \see
 * a_lstList a_lstFindLowestHigherObjAddr
 */
c_address
a_lstFindHighestLowerObjAddr(
	a_lstList list,
	c_address currentAddr)
{
	c_address result;
	if (list) {
		a_treSortAction sortAction = (a_treSortAction)a_lstCompareEntries;
		a_lstEntry searchEntry = a_lstNewEntry(currentAddr, -1, -1, NULL, NULL, NULL, -1, -1);
		a_lstEntry resultEntry = a_treFindHighestLowerValue(list->tree, (void *)searchEntry, sortAction);
		result = resultEntry ? resultEntry->address : 0;
		a_lstDestroyEntry(&searchEntry);
	} else {
		result = 0;
	}
	return result;
}



/**
 * \brief
 * Returns the \a lowest \a higher address in the list, if any.
 *
 * This operation returns the lowest object address that is
 * \b higher than specified address, in other words: the nearest
 * higher.
 *
 * \param list
 * The list to search in
 *
 * \param currentAddr
 * The address that a \a lowest \a higher address must be searched
 * from
 *
 * \return
 * Address of the lowest higher entry found, 0 if the \a list is
 * NULL or empty, or when \a currentAddr is already the highest
 * address in the list.
 *
 * \see
 * a_lstList a_lstFindHighestLowerObjAddr
 */
c_address
a_lstFindLowestHigherObjAddr(
	a_lstList list,
	c_address currentAddr)
{
	c_address result;
	if (list) {
		a_treSortAction sortAction = (a_treSortAction)a_lstCompareEntries;
		a_lstEntry searchEntry = a_lstNewEntry(currentAddr, -1, -1, NULL, NULL, NULL, -1, -1);
		a_lstEntry resultEntry = a_treFindLowestHigherValue(list->tree, (void *)searchEntry, sortAction);
		result = resultEntry ? resultEntry->address : 0;
		a_lstDestroyEntry(&searchEntry);
	} else {
		result = 0;
	}
	return result;
}




/***********************************************************
 ENTRY INFORMATION GATHERING
 ***********************************************************/


/**
 * \brief
 * Returns the object kind of the specified address
 *
 * This operation searches in the list for an entry holding
 * specified address and returns the objectKind thereof.
 *
 * \param list
 * The list to search in
 *
 * \param address
 * The address to search for in the list
 *
 * \return
 * The objectKind of the entry found, or L_UNDEFINED if list is NULL
 * or an entry with specified address is not found.
 *
 * \see
 * a_lstList a_lstObjectKind
 */
a_lstObjectKind
a_lstGetObjectKind(
	a_lstList list,
	c_address address)
{
	a_lstObjectKind result;
	a_lstEntry entry = a_lstFindEntry(list, address);
	if (entry) {
		result = entry->kind;
	} else {
		result = L_UNDEFINED;
	}
	return result;
}



/**
 * \brief
 * Returns the value of an entry's specific property in the list
 *
 * This operation searches in the list for an entry with specified
 * address value and returns the value of one of the members
 * (=property).
 *
 * \param list
 * The list to search in
 *
 * \param address
 * The address of the entry to search for
 *
 * \param propertyKind
 * Which property's value to return
 *
 * \return
 * The entry's property value, corresponding to address in the list.
 * If list is NULL, or an entry with \a address can not be found, -1
 * will be returned.
 *
 * \see
 * a_lstList a_lstPropertyKind
 */
c_long
a_lstEntryProperty(
	a_lstList list,
	c_address address,
	a_lstPropertyKind propertyKind)
{
	c_long result;
	a_lstEntry entry;
	entry = address ? a_lstFindEntry(list, address) : NULL;
	if (entry) {
		switch (propertyKind) {
			case L_PRP_REFCOUNT:  result = entry->refCount;  break;
			case L_PRP_ALIGNMENT: result = entry->alignment; break;
			case L_PRP_SIZE:      result = entry->size;      break;
			case L_PRP_OURSIZE:   result = entry->ourSize;   break;
			default:              result = -1;               break;
		}
	} else {
		result = -1;
	}
	return result;
}



/**
 * \brief
 * Returns the 'count' of an entry's sublist
 *
 * This operation searches for an entry in the list, specified by
 * \a address, and returns the number of elements of a specific
 * sublist.
 *
 * \param list
 * The list to search in
 *
 * \param address
 * The address to search for in the list
 *
 * \param refsKind
 * Which sublist to return the count of
 *
 * \return
 * The number of elements in the specified sublist of the entry in
 * the list that was specified by \a address. If address can not be
 * found in the list or list is NULL, -1 is returned.
 *
 * \see
 * a_lstList a_lstRefsKind
 */
a_counter
a_lstEntrySublistCount(
	a_lstList list,
	c_address address,
	a_lstRefsKind refsKind)
{
	a_counter result;
	a_lstEntry entry = a_lstFindEntry(list, address);
	if (entry) {
		switch (refsKind) {
			case L_REF_TYPEREF:     result = entry->typeRefsCount;    break;
			case L_REF_DATAREF:     result = entry->dataRefsCount;    break;
			case L_REF_UNKNREF:     result = entry->unknRefsCount;    break;
			case L_REF_REFTOTYPE:   result = entry->refsToTypeCount;  break;
			case L_REF_REFTODATA:   result = entry->refsToDataCount;  break;
			case L_REF_REFTOUNKN:   result = entry->refsToUnknCount;  break;
			case L_REF_OCCURRENCES: result = entry->occurrencesCount; break;
			default:                result = -1;                      break;
		}
	} else {
		result = -1;
	}
	return result;
}



/***********************************************************
 ENTRY MANIPULATION (Add Values to SubLists, Notes, etc.)
 ***********************************************************/


/**
 * \brief
 * Adds an address value to one of the sublists of an entry in the
 * list
 *
 * This operation searches for an entry in the list, specified by
 * \a address, and adds an address value of \a refAddr to one of
 * the sublists, specified by refsKind.
 *
 * \param list
 * The list to search in
 *
 * \param address
 * The address of the entry to search for
 *
 * \param refAddr
 * The reference address to add in one of the sublists
 *
 * \param refsKind
 * The kind of sublist \a refAddr must be added to
 *
 * \return
 * Boolean value specifying whether the operation was successful. The
 * operation will fail if \a list is NULL, if no entry with
 * \a address can be found or if \a refsKind is out of bounds.
 *
 * \see
 * a_lstList a_lstRefsKind
 */
a_counter
a_lstAddRefAddrToEntrySublist(
	a_lstList list,
	c_address address,
	c_address refAddr,
	a_lstRefsKind refsKind)
{
	a_counter result;
	a_lstEntry entry = address ? a_lstFindEntry(list, address) : NULL;
	if (entry) {
		a_treSortAction sortAction = (a_treSortAction)a_lstComparePointerValues;
		switch (refsKind) {
			case L_REF_TYPEREF:
				if (!entry->typeRefs) {
					entry->typeRefs = a_treCreateTree();
				}
				result = a_treInsertNode(entry->typeRefs, (c_voidp)refAddr, sortAction, A_LST_DUPESALLOWED);
				break;

			case L_REF_DATAREF:
				if (!entry->dataRefs) {
					entry->dataRefs = a_treCreateTree();
				}
				result = a_treInsertNode(entry->dataRefs, (c_voidp)refAddr, sortAction, A_LST_DUPESALLOWED);
				break;

			case L_REF_UNKNREF:
				if (!entry->unknRefs) {
					entry->unknRefs = a_treCreateTree();
				}
				result = a_treInsertNode(entry->unknRefs, (c_voidp)refAddr, sortAction, A_LST_DUPESALLOWED);
				break;

			case L_REF_REFTOTYPE:
				if (!entry->refsToType) {
					entry->refsToType = a_treCreateTree();
				}
				result = a_treInsertNode(entry->refsToType, (c_voidp)refAddr, sortAction, A_LST_DUPESALLOWED);
				break;

			case L_REF_REFTODATA:
				if (!entry->refsToData) {
					entry->refsToData = a_treCreateTree();
				}
				result = a_treInsertNode(entry->refsToData, (c_voidp)refAddr, sortAction, A_LST_DUPESALLOWED);
				break;

			case L_REF_REFTOUNKN:
				if (!entry->refsToUnkn) {
					entry->refsToUnkn = a_treCreateTree();
				}
				result = a_treInsertNode(entry->refsToUnkn, (c_voidp)refAddr, sortAction, A_LST_DUPESALLOWED);
				break;

			case L_REF_OCCURRENCES:
				if (!entry->occurrences) {
					entry->occurrences = a_treCreateTree();
				}
				result = a_treInsertNode(entry->occurrences, (c_voidp)refAddr, sortAction, A_LST_DUPESALLOWED);
				break;

			default:
				result = 0;
				break;
		}
	} else {
		result = 0;
	}
	return result;
}



/* Adds (appends) a (string)value to an entry.
 * If value == NULL, the string "*NULL*" will be appended.
 * This function will do nothing if entry == NULL.
 */
static void
a_lstAddEntryValueDo(
	a_lstEntry entry,
	char *value)
{
	if (entry) {
		if (!entry->value) {
			entry->value = a_memStrdup(value);
		} else {
			const char *fill = ", ";
			char *newValue;
			if (!value) {
				value = "*NULL*";
			}
			newValue = a_memAlloc(strlen(entry->value) + strlen(fill) + strlen(value) + 1);
			if (newValue) {
				strcpy(newValue, entry->value);
				strcat(newValue, fill);
				strcat(newValue, value);
				a_memFree(entry->value);
				entry->value = newValue;
			}
		}
	}
}	



/**
 * \brief
 * Adds a (string)value to the \a value member of an entry
 *
 * This operation searches for an entry in the list, specified by
 * its address and adds the string \a value. If there already is
 * a string \a value, this one will be appended, comma seperated.
 *
 * \param list
 * The list to search in
 *
 * \param address
 * The address value of the entry to search for
 *
 * \param value
 * String value to append
 *
 * \see
 * a_lstList
 */
void
a_lstAddEntryValue(
	a_lstList list,
	c_address address,
	char *value)
{
	a_lstEntry entry = a_lstFindEntry(list, address);
	a_lstAddEntryValueDo(entry, value);
}

		

/* Adds (appends) a note to an entry's note field.
 * If note == NULL, the string "*NULL*" will be appended.
 * This function will do nothing if entry == NULL.
 */
static void
a_lstAddEntryNoteDo(
	a_lstEntry entry,
	char *note)
{
	if (entry) {
		if (!entry->note) {
			entry->note = a_memStrdup(note);
		} else {
			const char *fill = ", ";
			char *newNote;
			if (!note) {
				note = "*NULL*";
			}
			newNote = a_memAlloc(strlen(entry->note) + strlen(fill) + strlen(note) + 1);
			if (newNote) {
				strcpy(newNote, entry->note);
				strcat(newNote, fill);
				strcat(newNote, note);
				a_memFree(entry->note);
				entry->note = newNote;
			}
		}
	}
}	



/**
 * \brief
 * Adds a (string)value to the \a note member of an entry
 *
 * This operation searches for an entry in the list, specified by
 * its address and adds the string \a note. If there already is
 * a string \a note, this one will be appended, comma seperated.
 *
 * \param list
 * The list to search in
 *
 * \param address
 * The address value of the entry to search for
 *
 * \param note
 * String to append
 *
 * \note
 * Internally, the string will be duplicated.
 *
 * \see
 * a_lstList
 */
void
a_lstAddEntryNote(
	a_lstList list,
	c_address address,
	char *note)
{
	a_lstEntry entry = a_lstFindEntry(list, address);
	a_lstAddEntryNoteDo(entry, note);
}




/**
 * \brief
 * Sets a (new) name for an entry
 *
 * This operation sets a (new) name for an entry in the list. If
 * there already is a name, it will be overwritten.
 *
 * \param list
 * The list to search in
 *
 * \param address
 * The address value of an entry to search for
 *
 * \param name
 * String to set the new entry name to
 *
 * \return
 * Boolean value specifying whether the operation was successful
 *
 * \note
 * Internally, the string will be duplicated.
 *
 * \see
 * a_lstList
 */
int
a_lstSetObjectName(
	a_lstList list,
	c_address address,
	char *name)
{
	int result;
	a_lstEntry entry = a_lstFindEntry(list, address);
	if (entry) {
		if (entry->objectName) {
			a_memFree(entry->objectName);
		}
		entry->objectName = a_memStrdup(name);
		result = 1;
	} else {
		result = 0;
	}
	return result;
}



/**
 * \brief
 * Sets a (new) value for the \a ourSize member of an entry
 *
 * This operation searches for an entry with \a address in the list
 * and sets the (internal) value of \a ourSize. This member
 * \a ourSize can hold a different value from the value that was
 * returned by SPLICE (i.e. c_getType()->size), specifying the
 * real size an object occupies in memory.
 *
 * \param list
 * The list to search in
 *
 * \param address
 * The address value of an entry to search for
 *
 * \param ourSize
 * New value of \a ourSize to set
 *
 * \return
 * Boolean value specifying whether the operation was successful
 *
 * \see
 * a_lstList
 */
int
a_lstSetOurSize(
	a_lstList list,
	c_address address,
	c_long ourSize)
{
	int result;
	a_lstEntry entry = a_lstFindEntry(list, address);
	if (entry) {
		entry->ourSize = ourSize;
		result = 1;
	} else {
		result = 0;
	}
	return result;
}

			

/***********************************************************
 OCCURRENCES-LIST MANIPULATION
 ***********************************************************/


/**
 * \brief
 * Inserts a new \a occurrence
 *
 * This operation inserts a new occurrence into the list. An
 * occurrence consists of a memory address and the address it refers
 * to.
 *
 * \note
 * Internally, the occurrences list is a seperate list from the one
 * holding all object entries.
 *
 * \param list
 * The list to store the occurrence in
 *
 * \param address
 * The memory address of the reference to store
 *
 * \param referenceAddress
 * The value of the reference, i.e. the address it refers to
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_lstList
 */
int
a_lstInsertNewOccurrence(
	a_lstList list,
	c_address address,
	c_address referenceAddress)
{
	int result;
	if (list) {
		if (!list->occurrences) {
			list->occurrences = a_hshCreateHashtable(list->occurrencesArraySize, NULL);
		}
		result = a_hshInsertEntry(list->occurrences, (a_hshKey)address, (a_hshValue)referenceAddress);
	} else {
		result = 0;
	}
	return result;
}



/**
 * \brief
 * Removes an occurrence from the list
 *
 * This operation removes an occurrence from the list. An occurrence
 * consists of a memory address and the address it refers to.
 *
 * \note
 * Internally, the occurrences list is a seperate list from the one
 * holding all object entries.
 *
 * \param list
 * The list to remove the occurrence from
 *
 * \param address
 * The memory address of the reference to remove
 *
 * \param referenceAddress
 * The value of the reference, i.e. the address it refers to.
 * Although this value is not needed to search the occurrence, it is
 * used as a check. If this value does not match the one holded in
 * the list, the operation will fail.
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_lstList
 */
int
a_lstRemoveOccurrence(
	a_lstList list,
	c_address address,
	c_address referenceAddress)
{
	return a_hshTake(list->occurrences, (a_hshKey)address) ? 1 : 0;
}



/**
 * \brief
 * Returns the number of occurrences held by \a list
 *
 * This operation counts the number of occurrences that are held in
 * the list.
 *
 * \param list
 * The list in which the occurrences must be counted
 *
 * \return
 * The number of occurrences found in the list, or -1 if the
 * operation failed, e.g. if list is NULL.
 *
 * \see
 * a_lstList
 */
a_counter
a_lstOccurrencesCount(
	a_lstList list)
{
	return a_hshCount(list->occurrences);
}




/***********************************************************
 INCONSISTENCY CHECK (will move to a_anl in the future)
 ***********************************************************/


/**
 * \brief
 * Calculates the difference in Reference Counters of an object
 * (obsolete)
 *
 * \param refC
 * Reference Count as it is held by the SPLICE database
 *
 * \param tRef
 * The number of type references an object has
 *
 * \param dRef
 * The number of data references an object has
 *
 * \param uRef
 * The number of uncategorised refrences an object has
 *
 * \return
 * Calculated Difference value
 */
a_counter
a_lstDiff(
	c_long refC,
	a_counter tRef,
	a_counter dRef,
	a_counter uRef)
{
	return tRef + dRef + uRef - (a_counter)refC;
}



/* Look for inconsistencies within one entry and report
 * them in the entry's note field.
 * This function will do noting if entry == NULL.
 */
static void
a_lstCheckConsistenciesCallback(
	a_lstEntry entry,
	void *dummyPtr)
{
	if (entry) {
		const a_lstObjectKind objectKind = entry->kind;
		const c_long refC     = entry->refCount;
		const a_counter tRef  = entry->typeRefsCount;
		const a_counter dRef  = entry->dataRefsCount;
		const a_counter uRef  = entry->unknRefsCount;
		const a_counter diff  = refC - (tRef + dRef);
		const c_long algn     = entry->alignment;
		const c_long size     = entry->size;
		const a_counter occrs = entry->occurrencesCount;
		
		if (refC <= 0) {
			a_lstAddEntryNoteDo(entry, "*RefC<=0*");
		}
		
		if (0 < algn) {
			if ((algn < size) && (size % algn)) {
				a_lstAddEntryNoteDo(entry, "*Size%Algn!=0*");
			}
		} else if (!algn) {
			a_lstAddEntryNoteDo(entry, "*Algn==0*");
		}

		if (!size) {
			a_lstAddEntryNoteDo(entry, "*Size==0*");
		}

		if (0 < diff) {
			a_lstAddEntryNoteDo(entry, "*0<Diff*");
		}

		if ((objectKind == L_DATA) && (0 < tRef)) {
			a_lstAddEntryNoteDo(entry, "*Data Object has Type Reference(s)*");
		}
		
		if (1 < entry->refsToTypeCount) {
			a_lstAddEntryNoteDo(entry, "**AAPI-Error: Multiple Type References counted**");
		}
		
		if (occrs < tRef + dRef + uRef) {
			a_lstAddEntryNoteDo(entry, "**AAPI-Error: Double Count(s)**");
//			a_lstAddEntryNoteDo(entry, "**!**");
		}
	}
}



/**
 * \brief
 * Checks the consistency of each entry in the list
 *
 * This operation walks over all entries in the list and checks for
 * deviant values. If anything deviant is found, a note will be
 * added for that entry. All checks will be done within the entry
 * itself; no references will be checked.
 *
 * \param list
 * The list to check all entries' consistencies of.
 *
 * \see
 * a_lstList
 *
 * \todo
 * Move this function to a_anl. (a_lst is not meant for analysing
 * \b any data!)
 */
void
a_lstCheckConsistencies(
	a_lstList list)
{
	a_treWalk(list->tree, (a_treWalkAction)a_lstCheckConsistenciesCallback, NULL);
}



/***********************************************************
 TREE DUMP (For test Purposes Only)
 ***********************************************************/


static void
a_lstTreeDumpCallback(
	a_lstEntry entry,
	void *dummyPtr,
	int depth)
{
	int i;
	printf("%2d  ", depth);
	for (i = 0; i < depth; i++) {
		printf("   ");
	}
	printf("%8.8X\n", (unsigned int)entry->address);
}


/**
 * \brief
 * Displays all entries from the list in tree format
 *
 * This operation displays a list of all entries to \a stdout,
 * tree formatted.
 *
 * \param list
 * The list to display
 *
 * \note
 * For test purposes only
 */
void
a_lstTreeDump(
	a_lstList list)
{
	a_treWalkReverse(list->tree, (a_treWalkReverseAction)a_lstTreeDumpCallback, NULL);
}



static int
a_lstOccurrencesDumpCallback(
	a_hshKey key,
	a_hshValue value,
	void *dummyPtr)
{
	printf("Dump: %8.8X->%8.8X\n", (unsigned int)key, (unsigned int)value);
	return 1;
}



/**
 * brief
 * Displays all occurrences
 *
 * This operation displays all occurrences to \a stdout.
 *
 * \param list
 * The list holding all occurrences to display
 *
 * \note
 * For test purposes only
 */
void
a_lstOccurrencesDump(
	a_lstList list)
{
	a_hshWalk(list->occurrences, (a_hshWalkAction)a_lstOccurrencesDumpCallback, NULL);
	printf("Count: %ld\n", (long)a_hshCount(list->occurrences));
}



/***********************************************************
 WALK FUNCTIONS
 ***********************************************************/


/* Structure aid for the ListWalk Callbacks
 */
typedef struct a_lstCallbackContext {
	a_lstWalkAction userAction;
	void *userArg;
} *a_lstCallbackContext;



/* Creates an lstObject and copies data from the lstEntry
 * into it. Returns a pointer to this new object, or NULL
 * if the malloc failed.
 * This copy construction is required for keeping specific
 * members of lstEntry hidden to the outside, like some
 * tree members (from a_tre).
 * Remember to free (using a_memfree()) the object after
 * use.
 */
static a_lstObject
a_lstEntryToObject(
	a_lstEntry entry)
{
	a_lstObject o = a_memAlloc(sizeof(struct a_lstObject));
	if (o) {
		o->address          = entry->address;
		o->refCount         = entry->refCount;
		o->alignment        = entry->alignment;
		o->size             = entry->size;
		o->ourSize          = entry->ourSize;
		o->objectName       = a_memStrdup(entry->objectName);
		o->typeName         = a_memStrdup(entry->typeName);
		o->typeDesc         = a_memStrdup(entry->typeDesc);
		o->value            = a_memStrdup(entry->value);
		o->note             = a_memStrdup(entry->note);
		o->occurrencesCount = entry->occurrencesCount;
		o->typeRefsCount    = entry->typeRefsCount;
		o->dataRefsCount    = entry->dataRefsCount;
		o->unknRefsCount    = entry->unknRefsCount;
		o->refsToTypeCount  = entry->refsToTypeCount;
		o->refsToDataCount  = entry->refsToDataCount;
		o->refsToUnknCount  = entry->refsToUnknCount;
		o->kind             = entry->kind;
		o->refsDifference   = o->refCount - (o->typeRefsCount + o->dataRefsCount);
		o->occurrenceDiff   = o->occurrencesCount - o->typeRefsCount - o->dataRefsCount - o->unknRefsCount;
	}
	return o;
}


/* General Callback function for both the ListWalk and
 * EntryQuery. The function to eventually call back to
 * is encapsulated in the callbackContext.
 */
static int
a_lstListWalkCallback(
	void *valuePtr,
	struct a_lstCallbackContext *callbackContext)
{
	int result;
	a_lstEntry entry = (a_lstEntry)valuePtr;
	a_lstObject object = a_lstEntryToObject(entry);  // creates a new object!
	if (object) {
		result = (callbackContext->userAction)(object, callbackContext->userArg);
		a_memFree(object);
	} else {
		result = 0;
	}
	return result;
}
	


/**
 * \brief
 * Walks over all entries in the list, calling a user defined function
 * for every entry.
 *
 * This operation walks over all entries in the list, calling a user
 * defined function for every entry in the list. The user defined
 * function must be of a type that of \a a_lstWalkAction and must
 * return true (1) to indicate that function operated succesfully,
 * otherwise the walk will abort.
 *
 * \param list
 * The list to walk over. If list is NULL, the operation will fail
 *
 * \param walkAction
 * The user defined function that must be executed for every entry
 * in the list
 *
 * \param listWalkArg
 * Pointer to a user defined context that will be passed along
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 * If the walk is aborted, the operation will fail and \a false (0)
 * is returned.
 *
 * \see
 * a_lstList a_lstWalkAction a_lstEntryQuery
 */
int
a_lstListWalk(
	a_lstList list,
	a_lstWalkAction walkAction,
	void *listWalkArg)
{
	struct a_lstCallbackContext callbackContext;
	callbackContext.userAction = walkAction;
	callbackContext.userArg = listWalkArg;
	return a_treWalk(list->tree, (a_treWalkAction)a_lstListWalkCallback, &callbackContext);
}



/**
 * \brief
 * Searches for a specific entry in the list and calls a user defined
 * function for that entry.
 *
 * This operation searches for a specific entry in the list,
 * specified by \a address, and calls a user defined function for
 * that entry. The user defined function must be of a type that of
 * \a a_lstWalkAction and must return true (1) to indicate that
 * the function operated succesfully.
 *
 * \param list
 * The list to walk over. If list is NULL, the operation will fail
 *
 * \param address
 * The address to search for. If no entry with \a address is found,
 * the operation will fail.
 *
 * \param walkAction
 * The user defined function that must be executed for the entry
 * in the list
 *
 * \param walkArg
 * Pointer to a user defined context that will be passed along
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_lstList a_lstWalkAction a_lstListWalk
 */
int
a_lstEntryQuery(
	a_lstList list,
	c_address address,
	a_lstWalkAction walkAction,
	void *walkArg)
{
	struct a_lstCallbackContext callbackContext;
	a_lstEntry entry;

	callbackContext.userAction = walkAction;
	callbackContext.userArg = walkArg;
	entry = a_lstFindEntry(list, address);
	return entry ? a_lstListWalkCallback((void *)entry, &callbackContext) : 0;
}	



/*----------------------------------------------------------
 (Even more) WALK FUNCTIONS
 ----------------------------------------------------------*/


/* Structure aid for the EntryRefsWalk Callbacks
 */
typedef struct a_lstEntryRefsCallbackContext {
	a_lstEntryRefsWalkAction userAction;
	void *userArg;
	a_lstRefsKind refsKind;
} *a_lstEntryRefsCallbackContext;



/* General Callback function for all Walk Functions that return
 * only one address.
 * The function to eventually call back to,
 * is encapsulated in the EntryRefsCallbackContext.
 */
static int
a_lstEntryRefsWalkCallback(
	void *valuePtr,
	struct a_lstEntryRefsCallbackContext *callbackContext)
{
	return (callbackContext->userAction)((c_address)valuePtr, callbackContext->userArg, callbackContext->refsKind);
}



/**
 * \brief
 * Walks over all entries in an entry's sublist (list of references)
 * and calls a user defined function for every reference found.
 *
 * This operation searches for an entry in the list, specified by
 * \a address, and walks over all entries in its sublist, specified
 * by its \a refsKind, calling a user defined function for every
 * entry (reference) found in that sublist.
 *
 * \param list
 * The list to search in
 *
 * \param address
 * Entry's address value to search for in the list
 *
 * \param walkAction
 * The user defined function that must be executed for every
 * reference in the sublist
 *
 * \param walkActionArg
 * Pointer to a user defined context that will be passed along
 *
 * \param refsKind
 * Kind of sublist that must be walked over
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 * If \a address is not found in \a list, the operation will fail.
 * The operation will also fail if the user defined function
 * returns false, indicating an aborted walk.
 *
 * \see
 * a_lstList a_lstEntryRefsWalkAction a_lstRefsKind
 */
int
a_lstEntryReferencesWalk(
	a_lstList list,
	c_address address,
	a_lstEntryRefsWalkAction walkAction,
	void *walkActionArg,
	a_lstRefsKind refsKind)
{
	int result;
	a_lstEntry entry = a_lstFindEntry(list, address);
	if (entry) {
		a_treTree tree;
		switch (refsKind) {
			case L_REF_TYPEREF:     tree = entry->typeRefs;    break;
			case L_REF_DATAREF:     tree = entry->dataRefs;    break;
			case L_REF_UNKNREF:     tree = entry->unknRefs;    break;
			case L_REF_REFTOTYPE:   tree = entry->refsToType;  break;
			case L_REF_REFTODATA:   tree = entry->refsToData;  break;
			case L_REF_REFTOUNKN:   tree = entry->refsToUnkn;  break;
			case L_REF_OCCURRENCES: tree = entry->occurrences; break;
			default:                tree = NULL;               break;
		}
		if (tree) {
			a_treWalkAction treeWalkAction = (a_treWalkAction)a_lstEntryRefsWalkCallback;
			struct a_lstEntryRefsCallbackContext callbackContext;
			callbackContext.userAction = walkAction;
			callbackContext.userArg = walkActionArg;
			callbackContext.refsKind = refsKind;
			result = a_treWalk(tree, treeWalkAction, &callbackContext);
		}
	} else {
		result = 0;
	}
	return result;
}




/* Callback function for (re)calculating an entry's tree counters.
 * Returns 0 if entry == NULL, otherwise >0.
 */
static int
a_lstListUpdateEntryCountersCallback(
	a_lstEntry entry,
	void *dummyContext)
{
	int result = entry ? 1 : 0;
	if (entry) {
		entry->typeRefsCount    = entry->typeRefs ? (a_counter)a_treCount(entry->typeRefs) : 0;
		entry->dataRefsCount    = entry->dataRefs ? (a_counter)a_treCount(entry->dataRefs) : 0;
		entry->unknRefsCount    = entry->unknRefs ? (a_counter)a_treCount(entry->unknRefs) : 0;
		entry->refsToTypeCount  = entry->refsToType ? (a_counter)a_treCount(entry->refsToType) : 0;
		entry->refsToDataCount  = entry->refsToData ? (a_counter)a_treCount(entry->refsToData) : 0;
		entry->refsToUnknCount  = entry->refsToUnkn ? (a_counter)a_treCount(entry->refsToUnkn) : 0;
		entry->occurrencesCount = entry->occurrences ? (a_counter)a_treCount(entry->occurrences) : 0;
	}
	return result;
}



/**
 * \brief
 * (Re)calculates all entry's sublist counters
 *
 * This operation (re)calculates all counters of all entries in the
 * list.
 *
 * \param list
 * The list to recalculate all entry's sublist counters. If list is
 * NULL, the operation will fail.
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_lstList
 */
int
a_lstListUpdateEntryCounters(
	a_lstList list)
{
	return a_treWalk(list->tree, (a_treWalkAction)a_lstListUpdateEntryCountersCallback, NULL);
}



/*----------------------------------------------------------
 WALK THE OCCURRENCES LIST
 ----------------------------------------------------------*/


typedef struct a_lstOccurrencesWalkContext {
	a_lstOccurrencesWalkAction userAction;
	void *userContext;
} *a_lstOccurrencesWalkContext;



static int
a_lstOccurrencesWalkCallback(
	a_hshKey key,
	a_hshValue value,
	a_lstOccurrencesWalkContext walkContext)
{
	return (walkContext->userAction)((c_address)key, (c_address)value, walkContext->userContext);
}


/**
 * \brief
 * Walks over all occurrences in the list, calling a user defined
 * function for every occurrence.
 *
 * This operation walks over all occurrences in the list, calling a
 * user defined function for every occurrence found in the list. The
 * user defined function must be of a type that of
 * \a a_lstOccurrencesWalkAction and must return true (1) to indicate
 * that function operated succesfully, otherwise the walk will abort.
 *
 * \param list
 * The list to walk over. If list is NULL, the operation will fail
 *
 * \param walkAction
 * The user defined function that must be executed for every entry
 * in the list
 *
 * \param actionArg
 * Pointer to a user defined context that will be passed along
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 * If the walk is aborted, the operation will fail and \a false (0)
 * is returned.
 *
 * \see
 * a_lstList a_lstOccurrencesWalkAction
 */
int
a_lstOccurrencesWalk(
	a_lstList list,
	a_lstOccurrencesWalkAction walkAction,
	void *actionArg)
{
	int result;
	struct a_lstOccurrencesWalkContext walkContext;
	walkContext.userAction = walkAction;
	walkContext.userContext = actionArg;
	result = a_hshWalk(list->occurrences, (a_hshWalkAction)a_lstOccurrencesWalkCallback, &walkContext);
	return result;
}



//END a_lst.c
