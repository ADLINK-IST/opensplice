/* $Id$
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 * Author: Marc Vertes <mvertes@meiosys.com>
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef __sd_hash_h
#define __sd_hash_h

/**
 * @file hash.h
 *
 * @brief Generic hash table. It is implemented as an array of doubly-linked
 * lists of iterators. The index within the array is computed by a efficient
 * hash function.
 */

#include <stddef.h>
#include <sd/defs.h>

__SD_BEGIN_DECLS

struct __sd_hash_ops {
    unsigned int	(*hash)		(const void*);
    int			(*compare)	(const void*, const void*);
    void*		(*key_dup)	(const void*);
    void		(*key_free)	(void*);
    void*		(*data_dup)	(const void*);
    void		(*data_free)	(void*);
};

/**
 * This structure holds hash table operations.
 */
typedef struct __sd_hash_ops sd_hash_ops_t;

/**
 * This is the hash table.
 */
typedef struct __sd_hash sd_hash_t;

struct __sd_hash_iter {
    void*			key;
    void*			data;
    struct __sd_hash*		hash;
    unsigned int		__hkey;
    struct __sd_hash_iter*	__next;
    struct __sd_hash_iter*	__prev;
    int				__foreach;
};

/**
 * This is the elementary container for storing data into the hash table.
 */
typedef struct __sd_hash_iter sd_hash_iter_t;

/**
 * Signature of a "foreach" function.
 */
typedef unsigned int (*sd_hash_func_t)(void* a_key, void* a_data,
				       void* a_userdata);

/**
 * Creates a new hash table. One can customize the memory (de)allocation
 * policy for keys and data stored in the hash table.
 * @param a_size the initial size of the array.
 * @param a_ops the hash operations. If NULL, then string keys are assumed and
 * no memory (de)allocation is performed for keys and data.
 * @return a dynamicaly allocated hash table.
 */
extern sd_hash_t* sd_hash_new(size_t a_size, const sd_hash_ops_t* a_ops);

/**
 * Destroys the hash table.
 */
extern void sd_hash_delete(sd_hash_t* a_this);

/**
 * clears the hash table.
 */
extern void sd_hash_clear(sd_hash_t* a_this);

/**
 * Looks for the iterator associated to the given key in the hash table.
 * @param a_key the key associated to the iterator.
 * @return a pointer to the found iterator or NULL.
 */
extern sd_hash_iter_t* sd_hash_lookup(sd_hash_t* a_this, const void* a_key);

/**
 * Looks for the iterator associated to the given key in the hash table and
 * creates it if doesn't exist.
 * @param a_key the key associated to the iterator.
 * @return a pointer to the found iterator or NULL.
 */
extern sd_hash_iter_t* sd_hash_lookadd(sd_hash_t* a_this, const void* a_key);

/**
 * Adds data associated with the given key into the hash table. If the
 * key already exists, the old iterator is freed according to the memory
 * management operations passed to sd_hash_new().
 * @param a_key the key associated to the iterator.
 * @return a pointer to the created or found iterator.
 */
extern sd_hash_iter_t* sd_hash_add(sd_hash_t* a_this, const void* a_key,
				 void* a_data);

/**
 * Removes an iterator from the hash table.
 * @param a_key the key associated to the iterator.
 */
extern void sd_hash_del(sd_hash_t* a_this, const void* a_key);

/**
 * Calls \a a_func for each element of the hash table, as long as \a a_func
 * returns 0.
 * @param a_func the "foreach" function.
 * @param a_data the user data passed to \a a_func.
 */
extern void sd_hash_foreach(sd_hash_t* a_this, sd_hash_func_t a_func,
			    void* a_data);

/**
 * Gets the number of iterators.
 */
extern unsigned int sd_hash_get_nelem(sd_hash_t* a_this);

/**
 * Gets the size of the array.
 */
extern unsigned int sd_hash_get_size(sd_hash_t* a_this);

/**
 * Gets the first iterator.
 */
extern sd_hash_iter_t* sd_hash_begin(sd_hash_t* a_this);

/**
 * Gets the last iterator.
 */
extern sd_hash_iter_t* sd_hash_end(sd_hash_t* a_this);

/**
 * Gets a pointer to the next iterator.
 */
extern sd_hash_iter_t* sd_hash_iter_next(sd_hash_iter_t* a_this);

/**
 * Gets a pointer to the previous iterator.
 */
extern sd_hash_iter_t* sd_hash_iter_prev(sd_hash_iter_t* a_this);

/**
 * Gets a pointer to the previous iterator.
 */
extern void sd_hash_iter_del(sd_hash_iter_t* a_this);

/**
 * Hashes strings.
 */
extern unsigned int sd_hash_hash_string(const char* a_string);

__SD_END_DECLS

#endif
