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

#include "os.h"

#include "cf__nodeList.h"
#include "cf_node.h"

#define LIST_BLOCKSIZE 10

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
cf_nodeList
cf_nodeListNew()
{
    cf_nodeList list;

    list = (cf_nodeList)os_malloc((os_uint32)C_SIZEOF(cf_nodeList));

    list->maxNrNodes = 0;
    list->nrNodes = 0;
    list->theList = NULL;

    return list;
}

void
cf_nodeListFree(
    cf_nodeList list)
{
    assert(list != NULL);

    if (list->theList != NULL) {
        os_free(list->theList);
        list->theList = NULL;
    }
    list->nrNodes = 0;
    list->maxNrNodes = 0;
    os_free(list);
}


/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
void
cf_nodeListClear(
    cf_nodeList list)
{
    c_long i;

    assert(list != NULL);
    for (i=0; i<list->nrNodes; i++) {
      cf_nodeFree(list->theList[i]);
    }
    list->nrNodes = 0;
}

c_object
cf_nodeListInsert(
    cf_nodeList list,
    cf_node o)
{
    cf_node *newList;
   
    assert(list != NULL);

    if (list->nrNodes == list->maxNrNodes) {
      list->maxNrNodes += LIST_BLOCKSIZE;
      newList = (cf_node *)os_malloc((os_uint32)(list->maxNrNodes * (int)sizeof(cf_node)));
      memcpy(newList, list->theList, 
             (size_t)((size_t)(list->maxNrNodes - LIST_BLOCKSIZE) * sizeof(cf_node)));
      if (list->theList != NULL) {
          os_free(list->theList);
      }
      list->theList = newList;
    }

    list->nrNodes++;
    list->theList[list->nrNodes - 1] = o;

    return NULL;
}

c_bool
cf_nodeListWalk(
    cf_nodeList list,
    cf_nodeWalkAction action,
    cf_nodeWalkActionArg arg)
{
    c_bool result;
    c_long i;
    unsigned int actionResult;

    result = TRUE;
    actionResult = 1;
    for (i = 0; (i < list->nrNodes) && ((int)actionResult > 0); i++) {
        actionResult = action(list->theList[i], arg);
        if ((int)actionResult == 0) {
            result = FALSE;
        }
    }
    return result;
}
