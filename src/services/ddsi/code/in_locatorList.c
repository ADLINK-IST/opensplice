/*
 * in_locatorList.c
 *
 *  Created on: Mar 9, 2009
 *      Author: frehberg
 */

/* **** interface headers **** */
#include "in_locatorList.h"

/* **** implementation headers **** */
#include "in__object.h"

/* **** private functions **** */

/* **** public functions **** */


/** */
os_boolean
in_locatorListInit(in_locatorList *_this)
{
    Coll_List_init(_this);
    return OS_TRUE;
}

/** */
void
in_locatorListDeinit(in_locatorList *_this)
{
    /* delete elements from back */
    in_locator locator = Coll_List_popBack(_this);
    /* locator==NULL if empty list */
    while (locator) {
        in_locatorFree(locator);
        /* get next element of list (from back) */
        locator = Coll_List_popBack(_this);
    }
}

/** */
os_boolean
in_locatorListPushBack(in_locatorList *_this,
        in_locator newElem)
{
    os_boolean result = OS_TRUE;
    if (Coll_List_pushBack(_this, newElem)!= COLL_OK) {
        /* out or memory */
        result = OS_FALSE;
    } else
    {
    	in_locatorKeep(newElem);
    }
    return result;
}


/** */
in_locator
in_locatorListPopBack(in_locatorList *_this)
{
    in_locator result = Coll_List_popBack(_this);

    return result;
}


/** */
os_size_t
in_locatorListLength(in_locatorList *_this)
{
    os_size_t result =
        Coll_List_getNrOfElements(_this);
    return result;
}
