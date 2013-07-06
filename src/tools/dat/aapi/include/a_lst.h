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
 * about all database objects. The entries within this list are
 * hidden.
 */


#ifndef A_LST_H
#define A_LST_H


#include "a_def.h"



/**
 * \brief
 * Object Kind (AAPI Defined)
 *
 * Different kinds of objects.
 */
typedef enum a_lstObjectKind {
	L_UNDEFINED,      ///< undefined kind (not object related)
	L_CLSS,           ///< "C" meta-meta-meta; c_class
	L_BASE,           ///< "B" meta-meta; c_baseObject or a child thereof
	L_META,           ///< "M" meta; object of which the type is meta-meta
	L_DATA,           ///< "D" data
	L_UNDF,           ///< "?" undefined object, could not determine category
	L_COUNT           ///< Number of defined kinds of object, used internally
} a_lstObjectKind;



/**
 * \brief
 * Kinds of References Lists
 *
 * Different kinds of references
 */
typedef enum a_lstRefsKind {
	L_REF_UNDEFINED,   ///< Undefined kind of reference (not to be used!)
	L_REF_TYPEREF,     ///< Referenced as Type
	L_REF_DATAREF,     ///< Referenced as Data
	L_REF_UNKNREF,     ///< Referenced as Unknown
	L_REF_REFTOTYPE,   ///< Type Reference
	L_REF_REFTODATA,   ///< Data Reference
	L_REF_REFTOUNKN,   ///< Unknown Reference
	L_REF_OCCURRENCES, ///< Occurrences List
	L_REF_COUNT        ///< Number of different kinds of references, used internally
} a_lstRefsKind;



/**
 * \brief
 * Kinds of properties
 *
 * Different kinds of properties. Use these to specify which entry
 * member to set or retrieve.
 */
typedef enum a_lstPropertyKind {
	L_PRP_UNDEFINED,   ///< Undefined property (not to be used!)
	L_PRP_REFCOUNT,    ///< Reference Count
	L_PRP_ALIGNMENT,   ///< Alignment
	L_PRP_SIZE,        ///< Object Size
	L_PRP_OURSIZE,     ///< Corrected Object Size (in case of c_string: strlen)
	L_PRP_COUNT        ///< Number of different properties, used internally
} a_lstPropertyKind;



/**
 * \brief
 * List Data Structure
 *
 * Pointer to a (hidden) data structure holding a list of all entries
 * and a list of all occurrences.
 */
typedef struct a_lstList_s *a_lstList;


/**
 * \brief
 * External Data Structure for presenting an object's values.
 *
 * Type Definition for a database object when it is presented through
 * a Walk or Query.
 *
 */
typedef struct a_lstObject {
	c_address       address;          ///< Object's Start Address in Shared Memory
	c_long          refCount;         ///< Reference Count, value from SPLICE
	c_long          alignment;        ///< Object's Alignment, value from SPLICE
	c_long          size;             ///< Object's Size, value from SPLICE
	c_long          ourSize;          ///< Object's corrected size (strlen() in case of c_string)
	char           *objectName;       ///< Object's name, if any
	char           *typeName;         ///< Type's name, if any
	char           *typeDesc;         ///< Type description, AAPI generated
	char           *value;            ///< Human readable representation of object's value
	char           *note;             ///< AAPI generated note
	a_counter       occurrencesCount; ///< Number of pointers to \a address were found in the Shared Memory
	a_counter       typeRefsCount;    ///< Number of times this object is Type Referenced
	a_counter       dataRefsCount;    ///< Number of times this object is Data Referenced
	a_counter       unknRefsCount;    ///< Number of times this object is Unknown Referenced
	a_counter       refsDifference;   ///< typeRefsCount + dataRefsCount - refCount
	a_counter       refsToTypeCount;  ///< Number of times this object has Type References to other objects
	a_counter       refsToDataCount;  ///< Number of times this object has Data References to other objects
	a_counter       refsToUnknCount;  ///< Number of times this object has Unknown References to other objects
	a_counter       occurrenceDiff;   ///< occurrencesCount - type/data/unknRefsCount (should always be 0!)
	a_lstObjectKind kind;             ///< Object Kind, AAPI generated
} *a_lstObject;



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
char *a_lstGetObjectKindChar(a_lstObjectKind objectKind);


/***********************************************************
 CREATION & DESTROY
 ***********************************************************/

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
a_lstList a_lstCreateList();


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
int a_lstSetNewOccurrencesArraySize(a_lstList list, c_long mmStateMaxUsed);


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
int a_lstDestroyList(a_lstList list);


/***********************************************************
 COUNTER OPERATIONS
 ***********************************************************/

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
a_counter a_lstCount(a_lstList list, a_lstObjectKind objectKind);


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
a_counter a_lstCountAll(a_lstList list);


/***********************************************************
 ENTRY MANIPULATION (Insert, Find)
 ***********************************************************/

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
int a_lstInsertNewEntry(a_lstList list, c_address address,
	c_long refCount, a_lstObjectKind objectKind,
	char *objectName, char *typeDesc, char *typeName,
	c_long alignment, c_long size);


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
c_address a_lstFindFirst(a_lstList list);


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
c_address a_lstFindLast(a_lstList list);


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
int a_lstInList(a_lstList list, c_address address);


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
int a_lstInObject(a_lstList list, c_address objAddress, c_address checkAddress);


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
int a_lstInObjHeader(c_address objAddress, c_address checkAddress);


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
c_address a_lstFindHighestLowerObjAddr(a_lstList list, c_address currentAddr);


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
c_address a_lstFindLowestHigherObjAddr(a_lstList list, c_address currentAddr);


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
a_lstObjectKind a_lstGetObjectKind(a_lstList list, c_address address);


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
c_long a_lstEntryProperty(a_lstList list, c_address address, a_lstPropertyKind propertyKind);


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
a_counter a_lstEntrySublistCount(a_lstList list, c_address address, a_lstRefsKind refsKind);


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
int a_lstAddRefAddrToEntrySublist(a_lstList list, c_address address, c_address refAddr, a_lstRefsKind refsKind);


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
void a_lstAddEntryValue(a_lstList list, c_address address, char *value);


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
void a_lstAddEntryNote(a_lstList list, c_address address, char *note);


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
int a_lstSetObjectName(a_lstList list, c_address address, char *name);


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
int a_lstSetOurSize(a_lstList list, c_address address, c_long ourSize);


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
int a_lstInsertNewOccurrence(a_lstList list, c_address address, c_address referenceAddress);


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
int a_lstRemoveOccurrence(a_lstList list, c_address address, c_address referenceAddress);


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
a_counter a_lstOccurrencesCount(a_lstList list);


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
a_counter a_lstDiff(c_long refC, a_counter tRef, a_counter dRef, a_counter uRef);


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
void a_lstCheckConsistencies(a_lstList list);


/***********************************************************
 TREE DUMP (For test Purposes Only)
 ***********************************************************/

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
void a_lstTreeDump(a_lstList list);


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
void a_lstOccurrencesDump(a_lstList list);


/***********************************************************
 WALK FUNCTIONS
 ***********************************************************/

/**
 * \brief
 * Type definition for a user defined function for a
 * callback action, intiated by the Walk and Query functions.
 *
 * \param lstObject
 * Instance of a_lstObject that will be passed along to the
 * call back function.
 *
 * \param actionArg
 * Pointer to a user defined context that will be passed along
 *
 * \return
 * The user defined function must return true (1) in order to
 * indicate to the walk function to continue. If false (0) is
 * returned, the walk will abort.
 *
 * \see
 * a_lstWalk a_lstEntryQuery
 */
typedef int (*a_lstWalkAction)(a_lstObject lstObject, void *actionArg);


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
int a_lstListWalk(a_lstList list, a_lstWalkAction walkAction, void *listWalkArg);


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
int a_lstEntryQuery(
	a_lstList list, c_address address, a_lstWalkAction walkAction,
	void *walkArg);


/**
 * \brief
 * Type definition for a user defined function for a
 * call back action, initiated by the ReferencesWalk function.
 *
 * \param address
 * The reference address that will be passed along to the call back
 * function
 *
 * \param actionArg
 * Pointer to a user defined context that will be passed along
 *
 * \param refsKind
 * Kind of sublist that a reference is passed along from
 *
 * \return
 * Boolean value that the user defined function must return to
 * indicate if the call back function was successful. If false (0) is
 * returned, the walk function will abort.
 *
 * \see
 * a_lstReferencesWalk
 */
typedef int (*a_lstEntryRefsWalkAction)
	(c_address address, void *actionArg, a_lstRefsKind refsKind);


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
int a_lstEntryReferencesWalk(
	a_lstList list, c_address address, a_lstEntryRefsWalkAction walkAction,
	void *walkActionArg, a_lstRefsKind refsKind);


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
int a_lstListUpdateEntryCounters(a_lstList list);


/*----------------------------------------------------------
 WALK THE OCCURRENCES LIST
 ----------------------------------------------------------*/

/**
 * \brief
 * Type definition for a user defined function for a
 * call back action, initiated by the OccurrencesWalk function.
 *
 * \param address
 * The memory address of the \a occurrence that will be passed along
 * to the call back function
 *
 * \param referenceAddress
 * The reference address of the \a occurrence that will be passed
 * along to the call back function
 *
 * \param actionArg
 * Pointer to a user defined context that will be passed along
 *
 * \return
 * Boolean value that the user defined function must return to
 * indicate if the call back function was successful. If false (0) is
 * returned, the walk function will abort.
 *
 * \see
 * a_lstOccurrencesWalk
 */
typedef int (*a_lstOccurrencesWalkAction)
	(c_address address, c_address referenceAddress, void *actionArg);


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
int a_lstOccurrencesWalk(
	a_lstList list, a_lstOccurrencesWalkAction walkAction, void *actionArg);



#endif  /* A_LST_H */

//END a_lst.h
