
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
        v_publicFree(v_public(q));
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
