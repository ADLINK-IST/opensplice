/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "v__collection.h"
#include "v__entity.h"
#include "v__query.h"
#include "v_public.h"

void
v_collectionInit(
    _Inout_ v_collection c,
    _In_opt_z_ const c_char *name)
{
    c_type type;

    assert(C_TYPECHECK(c,v_collection));

    v_entityInit(v_entity(c), name);
    type = c_resolve(c_getBase(c_object(c)),"kernelModuleI::v_query");
    assert(type);
    c->queries = c_setNew(type);
    c_free(type);
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
        v_queryDetachWaitsets(q);
        v_publicFree(v_public(q));
        c_free(q);
        q = v_query(c_take(c->queries));
    }
    v_entityFree(v_entity(c));
}

void
v_collectionDeinit(
    v_collection c)
{
    assert(C_TYPECHECK(c,v_collection));

    v_entityDeinit(v_entity(c));
    c_free(c->queries);
    c->queries = NULL;
}
