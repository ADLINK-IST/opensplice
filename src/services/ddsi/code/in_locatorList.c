/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
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
    in_locator locator =
        Coll_List_popBack(_this);
    /* locator==NULL if empty list */
    while (locator!=NULL) {
        in_objectFree(in_object(locator));
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
    }
    return result;
}


/** */
in_locator
in_locatorListPopBack(in_locatorList *_this)
{
    in_locator result =
        Coll_List_popBack(_this);
    return result;
}
