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

#include "v__collection.h"
#include "v__observer.h"
#include "v_query.h"
#include "v_public.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/

/**************************************************************
 * Protected functions
 **************************************************************/
void
v_collectionInit(
    v_collection c,
    const c_char *name,
    v_statistics s,
    c_bool enable)
{
    c_base base;

    assert(C_TYPECHECK(c,v_collection));

    v_observerInit(v_observer(c), name, s, enable);
    base =  c_getBase(c_object(c));
    c->queries = c_setNew(c_resolve(base,"kernelModule::v_query"));
}

void
v_collectionFree(
    v_collection c)
{
    v_query q;

    assert(C_TYPECHECK(c,v_collection));

    q = v_query(c_take(c->queries));
    while (q != NULL) {
    	/* The v_publicFree shouldn't be here, because it should be the
    	 * responsibility of the 'creator' of the query to have this
    	 * knowledge and perform the v_publicFree. The only thing that should be
		 * required here is freeing the reference to the query by doing a
		 * c_free.
		 *
		 * Because it is not exactly clear where the v_publicFree should be
		 * performed, it is performed here for now.
		 */
        v_publicFree(v_public(q));
        c_free(q);
        q = v_query(c_take(c->queries));
    }
    v_observerFree(v_observer(c));
}

void
v_collectionDeinit(
    v_collection c)
{
    assert(C_TYPECHECK(c,v_collection));

    v_observerDeinit(v_observer(c));
    c_free(c->queries);
    c->queries = NULL;
}

/**************************************************************
 * Public functions
 **************************************************************/
