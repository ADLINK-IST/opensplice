static const char version[] = "$Id$";

/* 
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sd/hash.h>
#include <sd/malloc.h>

#define SD_HASH_FULLTAB	2	/* rehash when table gets this x full */
#define SD_HASH_GROWTAB 4	/* grow table by this factor */
#define SD_HASH_DEFAULT_SIZE 10 /* self explenatory */

struct __sd_hash {
    size_t			nelem;
    size_t			size;
    sd_hash_iter_t**		tab;
    const sd_hash_ops_t*	ops;
};

#define hindex(h, n)	((h)%(n))

/******************************************************************************/
static void rehash(sd_hash_t* a_this)
{
    size_t			i;
	int h, size;
    sd_hash_iter_t**	tab;
    sd_hash_iter_t*	p;
    sd_hash_iter_t*	q;
    
    size	= SD_HASH_GROWTAB * a_this->size;
    tab		= sd_calloc(size, sizeof(*tab));
    
    if (tab == 0) return;
    
    for (i = 0; i < a_this->size; i++) {
	for (p = a_this->tab[i]; p; p = q) {
	    q						= p->__next;
	    h						= hindex(p->__hkey,
								 size);
	    p->__next					= tab[h];
	    tab[h]					= p;
	    if (p->__next != 0) p->__next->__prev	= p;
	    p->__prev					= 0;
	}
    }
    free(a_this->tab);
    
    a_this->tab		= tab;
    a_this->size	= size;
}

/******************************************************************************/
extern sd_hash_t* sd_hash_new(size_t a_size, const sd_hash_ops_t* a_ops)
{
    const static sd_hash_ops_t default_ops = {
	(void*) &sd_hash_hash_string,
	(void*) &strcmp,
	0, 0, 0, 0
    };
    
    sd_hash_t*		hash;
    sd_hash_iter_t**	tab;
    
    if (a_size == 0) a_size = SD_HASH_DEFAULT_SIZE;
    
    hash	= sd_calloc(1, sizeof(*hash));
    tab		= sd_calloc(a_size, sizeof(*tab));
    
    if (hash == 0 || tab == 0) {
	free(hash);
	free(tab);
	return 0;
    }
    
    hash->nelem	= 0;
    hash->size	= a_size;
    hash->tab	= tab;
    hash->ops	= a_ops != 0 ? a_ops : &default_ops;
    
    return hash;
}

/******************************************************************************/
extern sd_hash_iter_t* sd_hash_lookup(sd_hash_t* a_this, const void* a_key)
{
    int			h;
    sd_hash_iter_t*	p;
    
    if (a_this == 0 || a_key == 0) return 0;
    
    h = hindex(a_this->ops->hash(a_key), a_this->size);
    for (p = a_this->tab[h]; p != 0; p = p->__next)
	if (a_this->ops->compare(a_key, p->key) == 0) {
	    return p;
	}
    
    return 0;
}

/******************************************************************************/
extern sd_hash_iter_t* sd_hash_lookadd(sd_hash_t* a_this, const void* a_key)
{
    int			h;
    sd_hash_iter_t*	p;
    
    if (a_this == 0 || a_key == 0)			return 0;
    if ((p = sd_hash_lookup(a_this, a_key)) != 0)	return p;
    if ((p = sd_calloc(1, sizeof(*p))) == 0)		return 0;
    
    if (a_this->ops->key_dup != 0)
	p->key = a_this->ops->key_dup(a_key);
    else
	p->key = (void*) a_key;
    
    p->hash					= a_this;
    p->__hkey					= a_this->ops->hash(a_key);
    
    if (a_this->nelem > SD_HASH_FULLTAB * a_this->size) rehash(a_this);
    
    h						= hindex(p->__hkey,
							 a_this->size);
    p->__next					= a_this->tab[h];
    a_this->tab[h]				= p;
    if (p->__next != 0) p->__next->__prev	= p;
    
    a_this->nelem++;
    
    return p;
}

/******************************************************************************/
extern sd_hash_iter_t* sd_hash_add(sd_hash_t* a_this, const void* a_key,
				   void* a_data)
{
    sd_hash_iter_t* p;
    
    if ((p = sd_hash_lookadd(a_this, a_key)) == 0) return 0;
    
    if (a_this->ops->data_free != 0) a_this->ops->data_free(p->data);
    
    if (a_this->ops->data_dup != 0)
	p->data = a_this->ops->data_dup(a_data);
    else
	p->data = a_data;
    
    return p;
}

/******************************************************************************/
extern void sd_hash_delete(sd_hash_t* a_this)
{
    sd_hash_clear(a_this);
    free(a_this->tab);
    free(a_this);
}

/******************************************************************************/
extern void sd_hash_clear(sd_hash_t* a_this)
{
    size_t		h;
    sd_hash_iter_t*	p;
    sd_hash_iter_t*	q;
    
    if (a_this == 0) return;
    
    for (h = 0; h < a_this->size; h++) {
	for (p = a_this->tab[h]; p; p = q) {
	    q = p->__next;
	    if (a_this->ops->key_free) a_this->ops->key_free(p->key);
	    if (a_this->ops->data_free) a_this->ops->data_free(p->data);
	    free(p);
	}
	a_this->tab[h] = 0;
    }
    a_this->nelem = 0;
}

/******************************************************************************/
extern void sd_hash_del(sd_hash_t* a_this, const void* a_key)
{
    int			h;
    sd_hash_iter_t*	p;
    
    h = hindex(a_this->ops->hash(a_key), a_this->size);
    for (p = a_this->tab[h]; p != 0; p = p->__next)
	if (a_this->ops->compare(a_key, p->key) == 0) break;
    if (p == 0) return;
    
    sd_hash_iter_del(p);
}

/******************************************************************************/
extern void sd_hash_foreach(sd_hash_t* a_this, sd_hash_func_t a_func,
			    void* a_data)
{
    size_t			h, ret;
    sd_hash_iter_t*	p;
    sd_hash_iter_t*	q;
    
    if (a_this == 0 || a_func == 0) return;
    
    for (h = 0; h < a_this->size; h++)
	for (p = a_this->tab[h]; p != 0; p = q) {
	    p->__foreach		= 1;
	    ret			= (*a_func)(p->key, p->data, a_data);
	    q			= p->__next;
	    
	    if (p->__foreach == 0)
		sd_hash_iter_del(p);
	    else
		p->__foreach	= 0;
	    
	    if (ret != 0) return;
	}
}

/******************************************************************************/
extern unsigned int sd_hash_get_nelem(sd_hash_t* a_this)
{
    if (a_this == 0) return 0;
    return a_this->nelem;
}

/******************************************************************************/
extern unsigned int sd_hash_get_size(sd_hash_t* a_this)
{
    if (a_this == 0) return 0;
    return a_this->size;
}

/******************************************************************************/
extern sd_hash_iter_t* sd_hash_begin(sd_hash_t* a_this)
{
    size_t h;
    
    if (a_this == 0) return 0;
    for (h = 0; h < a_this->size; h++)
	if (a_this->tab[h])
	    return a_this->tab[h];
    
    return 0;
}

/******************************************************************************/
extern sd_hash_iter_t* sd_hash_end(sd_hash_t* a_this)
{
    return 0;
}

/******************************************************************************/
extern sd_hash_iter_t* sd_hash_iter_next(sd_hash_iter_t* a_this)
{
    int h;
	size_t i;
    
    if (a_this == 0)		return 0;
    if (a_this->__next != 0)	return a_this->__next;
    
    h = hindex(a_this->__hkey, a_this->hash->size);
    
    for (i = h + 1; i < a_this->hash->size; i++)
	if (a_this->hash->tab[i])
	    return a_this->hash->tab[i];
    
    return 0;
}

/******************************************************************************/
extern sd_hash_iter_t* sd_hash_iter_prev(sd_hash_iter_t* a_this)
{
    int			h, i;
    sd_hash_iter_t*	p;
    
    if (a_this == 0)		return 0;
    if (a_this->__prev != 0)	return a_this->__prev;
    
    h = hindex(a_this->__hkey, a_this->hash->size);
    
    for (i = h - 1; i > 0; i--)
	for (p = a_this->hash->tab[i]; p; p = p->__next)
	    if (p->__next == 0)
		return p;
    
    return 0;
}

/******************************************************************************/
extern void sd_hash_iter_del(sd_hash_iter_t* a_this)
{
    if (a_this == 0) return;
    
    if (a_this->hash->ops->data_free != 0)
	a_this->hash->ops->data_free(a_this->data);
    a_this->data	= 0;
    
    if (a_this->hash->ops->key_free != 0)
	a_this->hash->ops->key_free(a_this->key);
    a_this->key		= 0;
    
    if (a_this->__foreach == 1) {
	a_this->__foreach = 0;
        return;
    }
    
    if (a_this->__next != 0) a_this->__next->__prev = a_this->__prev;
    if (a_this->__prev != 0)
	a_this->__prev->__next = a_this->__next;
    else
	a_this->hash->tab[hindex(a_this->__hkey, a_this->hash->size)] =
	    a_this->__next;
    
    a_this->hash->nelem--;
    
    free(a_this);
}

/******************************************************************************/
extern unsigned int sd_hash_hash_string(const char* a_string)
{
    register unsigned int h;
    
    for (h = 0; *a_string != '\0'; a_string++)
	h = *a_string + 31 * h;
    
    return h;
}
