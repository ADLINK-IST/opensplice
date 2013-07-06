static const char version[] = "$Id$";

/* 
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include <sd/list.h>
#include <sd/malloc.h>
#include <stdlib.h>

struct __sd_list {
    sd_list_iter_t*	head;
    sd_list_iter_t*	tail;
    size_t 		nelem;
};

/******************************************************************************/
extern sd_list_t* sd_list_new(size_t a_capacity)
{
    sd_list_t* this;
    
    this	= sd_calloc(1, sizeof(sd_list_t));
    this->head	= 0;
    this->tail	= 0;
    this->nelem	= 0;
    return this;
}

/******************************************************************************/
extern void sd_list_delete(sd_list_t* a_this)
{
    sd_list_iter_t *a_next;
    sd_list_iter_t *a_current;
    
     if (!a_this)
	return;

    /* Free the iterators */
    if (a_this->nelem > 0){
	a_current = a_this->head;
	do {
	    a_next = a_current->__next;
	    free(a_current);
	    a_current = a_next;
	} while (a_current);
    }

    free(a_this);
}

/******************************************************************************/
extern void sd_list_clear(sd_list_t* a_this)
{
    a_this->head    = 0;
    a_this->tail    = 0;
    a_this->nelem   = 0;
}

/******************************************************************************/
extern size_t sd_list_get_nelem(sd_list_t* a_this)
{
    return (a_this ? a_this->nelem : 0);
}

/******************************************************************************/
extern sd_list_iter_t* sd_list_begin(sd_list_t* a_this)
{
    return (a_this ? a_this->head : 0);
}

/******************************************************************************/
extern sd_list_iter_t* sd_list_end(sd_list_t* a_this)
{
    return 0;
}

/******************************************************************************/
extern sd_list_iter_t* sd_list_rbegin(sd_list_t* a_this)
{
    return (a_this ? a_this->tail : 0);
}

/******************************************************************************/
extern sd_list_iter_t* sd_list_rend(sd_list_t* a_this)
{
    return 0;
}

/******************************************************************************/
extern sd_list_iter_t* sd_list_lookup(sd_list_t* a_this, void* a_data)
{
    sd_list_iter_t* i;
    
    if (! a_this) return 0;
    
    for (i = a_this->head; i; i = i->__next)
	if (a_data == i->data)
	    return i;
    
    return 0;
}

/******************************************************************************/
extern sd_list_iter_t* sd_list_lookadd(sd_list_t* a_this, void* a_data)
{
    sd_list_iter_t* i;
    
    if (! a_this) return 0;
    
    if ((i = sd_list_lookup(a_this, a_data)) != 0)
        return i;

    return sd_list_add(a_this, a_data);
}

/******************************************************************************/
extern sd_list_iter_t* sd_list_add(sd_list_t* a_this, void* a_data)
{
    sd_list_iter_t* i;
    
    if (! a_this) return 0;
    
    
    if ((i = sd_calloc(1, sizeof(*i))) == 0)
	return 0;
    
    i->data		= a_data;
    i->list		= a_this;    
    i->__next		= a_this->head;
    i->__prev		= 0;
    a_this->head	= i;
    
    if (i->__next) i->__next->__prev	= i;
    if (!a_this->tail) a_this->tail	= i;
    
    a_this->nelem++;
    
    return i;
}

/******************************************************************************/
extern sd_list_iter_t* sd_list_sortadd(sd_list_t* a_this,
				       sd_list_func_t a_func, void* a_data)
{
    sd_list_iter_t* i;
    
    if (! a_this || ! a_func) return 0;
    
    for (i = a_this->head; i; i = i->__next)
	if ((*a_func)(i->data, a_data) > 0)
	    break;
    
    if (i)
	return sd_list_iter_insert(i, a_data);
    else
	return sd_list_append(a_this, a_data);
}

/******************************************************************************/
extern int sd_list_del(sd_list_t* a_this, void* a_data)
{
    sd_list_iter_t* i;
    
    if (!a_this)
	return -1;
    
    for (i = a_this->head; i; i = i->__next)
	if (a_data == i->data)
	    break;
    
    if (!i)
	return -1;
    
    sd_list_iter_del(i);
    return 0;
}

/******************************************************************************/
extern void sd_list_foreach(sd_list_t* a_this, sd_list_func_t a_func,
			    void* a_userdata)
{
    sd_list_iter_t* i;
    sd_list_iter_t* j;
    
    if (!a_this || !a_func)
	return;

    for (i = a_this->head; i; i = j) {
	int ret;
	
	i->__foreach	= 1;
	ret		= (*a_func)(i->data, a_userdata);
	j		= i->__next;
	
	if (i->__foreach == 0)
	    sd_list_iter_del(i);
	else
	    i->__foreach = 0;
	
	if (ret) return;
    }
}

/******************************************************************************/
extern void sd_list_rforeach(sd_list_t* a_this, sd_list_func_t a_func,
			     void* a_userdata)
{
    sd_list_iter_t* i;
    sd_list_iter_t* j;
    
    if (!a_this || !a_func)
	return;

    for (i = a_this->tail; i; i = j) {
	int ret;
	
	i->__foreach	= 1;
	ret		= (*a_func)(i->data, a_userdata);
	j		= i->__prev;
	
	if (i->__foreach == 0)
	    sd_list_iter_del(i);
	else
	    i->__foreach = 0;
	
	if (ret) return;
    }
}

/******************************************************************************/
extern sd_list_iter_t* sd_list_iter_next(sd_list_iter_t* a_this)
{
    return (a_this ? a_this->__next : 0);
}

/******************************************************************************/
extern sd_list_iter_t* sd_list_iter_prev(sd_list_iter_t* a_this)
{
    return (a_this ? a_this->__prev : 0);
}

/******************************************************************************/
extern void sd_list_iter_del(sd_list_iter_t* a_this)
{
    if (!a_this)
	return;
    
    if (a_this->__foreach == 1) {
	a_this->__foreach = 0;
	return;
    }
    
    if (a_this->__next)
	a_this->__next->__prev = a_this->__prev;
    else
	a_this->list->tail = a_this->__prev;

    if (a_this->__prev)
	a_this->__prev->__next = a_this->__next;
    else
	a_this->list->head = a_this->__next;
    
    a_this->list->nelem--;
    
    free(a_this);
}

/******************************************************************************/
extern sd_list_iter_t* sd_list_iter_insert(sd_list_iter_t* a_this,
					   void* a_data)
{
    sd_list_iter_t* i;
    
    if (! a_this) return 0;
    
    if (a_this->list->head == a_this)
	return sd_list_prepend(a_this->list, a_data);
    
    if ((i = sd_calloc(1, sizeof(*i))) == 0)
	return 0;
    
    i->data			= a_data;
    i->list			= a_this->list;
    i->__prev			= a_this->__prev;
    i->__next			= a_this;
    
    /* CAUTION: always exists since a_this is not the head */
    a_this->__prev->__next	= i;    
    a_this->__prev		= i;
    
    a_this->list->nelem++;
    
    return i;
}

/******************************************************************************/
extern sd_list_iter_t* sd_list_prepend(sd_list_t* a_this, void* a_data)
{
    sd_list_iter_t* i;
    
    if (! a_this) return 0;
    
    if ((i = sd_calloc(1, sizeof(*i))) == 0)
	return 0;
    
    i->list		= a_this;
    i->data		= a_data;
    i->__prev		= 0;
    i->__next		= a_this->head;
    a_this->head	= i;
    
    if (i->__next)
	i->__next->__prev	= i;
    else
	a_this->tail		= i;
    
    a_this->nelem++;
    
    return i;
}

/******************************************************************************/
extern sd_list_iter_t* sd_list_append(sd_list_t* a_this, void* a_data)
{
    sd_list_iter_t* i;
    
    if (! a_this) return 0;
    
    if ((i = sd_calloc(1, sizeof(*i))) == 0)
	return 0;
    
    i->list		= a_this;
    i->data		= a_data;
    i->__prev		= a_this->tail;
    i->__next		= 0;
    a_this->tail	= i;
    
    if (i->__prev)
	i->__prev->__next	= i;
    else
	a_this->head		= i;
    
    a_this->nelem++;
    
    return i;
}
