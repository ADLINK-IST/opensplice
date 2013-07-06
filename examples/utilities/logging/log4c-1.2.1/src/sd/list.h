/* $Id$
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 * 
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef __sd_list_h
#define __sd_list_h

/**
 * @file list.h @ingroup sd
 *
 * @brief Generic list object. It is implemented as an array of doubly-linked
 * lists of iterators. The index within the array is computed by a efficient
 * list function.
 */

#include <stddef.h>
#include <sd/defs.h>

__SD_BEGIN_DECLS

/**
 * This is the list object.
 */
typedef struct __sd_list sd_list_t;

struct __sd_list_iter {
    void*			data;
    struct __sd_list*		list;
    struct __sd_list_iter*	__next;
    struct __sd_list_iter*	__prev;
    int				__foreach;
};

/**
 * This is the elementary container for storing data into the list object.
 */
typedef struct __sd_list_iter sd_list_iter_t;

/**
 * Signature of a "foreach" function.
 */
typedef unsigned int (*sd_list_func_t)(void* a_data, void* a_userdata);

/**
 * Creates a list.
 * @param a_capacity initial number of preallocated iterators
 * @return the list object.
 */
extern sd_list_t* sd_list_new(size_t a_capacity);

/**
 * Destroys the list object.
 * @todo need a function parameter to destroy list elements.
 */
extern void sd_list_delete(sd_list_t* a_this);

/**
 * Adds the given_data at the head of the list.
 */
extern sd_list_iter_t* sd_list_prepend(sd_list_t* a_this, void* a_data);

/**
 * Adds the given data at the tail of the list.
 */
extern sd_list_iter_t* sd_list_append(sd_list_t* a_this, void* a_data);

/**
 * Looks for the iterator associated to the given data in the list object.
 * @param a_data the data to find
 * @return a pointer to the found iterator or NULL.
 */
extern sd_list_iter_t* sd_list_lookup(sd_list_t* a_this, void* a_data);

/**
 * Looks for the iterator associated to the given data in the list object and
 * creates it if doesn't exist, using @c sd_list_add().
 * @param a_data the data to find/add
 * @return a pointer to the found iterator or NULL.
 */
extern sd_list_iter_t* sd_list_lookadd(sd_list_t* a_this, void* a_data);

/**
 * Adds the given data into the list object. If the data already exists,
 * the associated iterator is returned.
 * @warning the element is added at the begining of the list.
 * @param a_data the data to add
 * @return a pointer to the created or found iterator.
 */
extern sd_list_iter_t* sd_list_add(sd_list_t* a_this, void* a_data);

/**
 * Applies the given function to all list elements, starting from the
 * first one. As soon as the function returns a non-null value, the
 * given data is inserted in the list (before the element).
 * @param a_func the "sort" function.
 * @param a_data the data to add
 * @return a pointer to the created iterator.
 */
extern sd_list_iter_t* sd_list_sortadd(sd_list_t* a_this,
				       sd_list_func_t a_func,
				       void* a_data);

/**
 * Removes an iterator from the list object.
 * @param a_data the data associated to the iterator.
 */
extern int sd_list_del(sd_list_t* a_this, void* a_data);

/**
 * clears the list object.
 */
extern void sd_list_clear(sd_list_t* a_this);

/**
 * Calls \a a_func for each element of the list object, as long as \a a_func
 * returns 0.
 * @param a_func the "foreach" function.
 * @param a_data the user data passed to \a a_func.
 */
extern void sd_list_foreach(sd_list_t* a_this, sd_list_func_t a_func,
			    void* a_userdata);

/**
 * Calls \a a_func for each element of the list object, as long as \a a_func
 * returns 0.
 * Same as sd_list_foreach but from tail to head of list.
 * @param a_func the "foreach" function.
 * @param a_data the user data passed to \a a_func.
 */
extern void sd_list_rforeach(sd_list_t* a_this, sd_list_func_t a_func,
			     void* a_userdata);

/**
 * Gets the number of iterators.
 */
extern size_t sd_list_get_nelem(sd_list_t* a_this);

/**
 * Gets the iterator pointing to the first element of the list.
 */
extern sd_list_iter_t* sd_list_begin(sd_list_t* a_this);

/**
 * Gets the past-the-last-element iterator of the list.
 */
extern sd_list_iter_t* sd_list_end(sd_list_t* a_this);

/**
 * Gets the iterator pointing to the last element of the list.
 */
extern sd_list_iter_t* sd_list_rbegin(sd_list_t* a_this);

/**
 * Gets the before-the-first-element iterator of the list.
 */
extern sd_list_iter_t* sd_list_rend(sd_list_t* a_this);

/**
 * Gets a pointer to the next iterator.
 */
extern sd_list_iter_t* sd_list_iter_next(sd_list_iter_t* a_this);

/**
 * Gets a pointer to the previous iterator.
 */
extern sd_list_iter_t* sd_list_iter_prev(sd_list_iter_t* a_this);

/**
 * Deletes the iterator from the list.
 */
extern void sd_list_iter_del(sd_list_iter_t* a_this);

/**
 * Deletes the iterator from the list.
 */
extern void sd_list_iter_del(sd_list_iter_t* a_this);

/**
 * Creates a new iterator and inserts it before @a a_this.
 * @param a_data the data associated to the iterator.
 */
extern sd_list_iter_t* sd_list_iter_insert(sd_list_iter_t* a_this,
					   void* a_data);

__SD_END_DECLS

#endif
