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
#include "v__entry.h"
#include "v__observer.h"
#include "v__observable.h"
#include "v__reader.h"
#include "v__group.h"
#include "v_entity.h"
#include "v_partition.h"
#include "v_topic.h"
#include "v_writer.h"
#include "v_kernel.h"
#include "v_handle.h"
#include "v_dataReaderEntry.h"
#include "v_networkReaderEntry.h"
#include "v_deliveryServiceEntry.h"
#include "v_writerInstance.h"
#include "v_public.h"
#include "v_proxy.h"
#include "os_report.h"

/* The entry maintains a set of v_group proxy's for each associated group.
 * The proxy's userData field is used as bit mask that consists a complete flag and a durability flag.
 * The complete flag set means that the associaded group is complete.
 * The durability flag set means that the group maintains non-volatile data.
 * The following macros define the flags and the required code to test and set the flags.
 */
#define COMPLETE_FLAG (0x0001)
#define DURABILITY_FLAG (0x0002)

#define SET_COMPLETE(userData, complete) \
        { c_address data = (c_address)userData; \
          if ((complete) == V_ALIGNSTATE_COMPLETE) {data |= COMPLETE_FLAG;} else {data &= (c_address)~COMPLETE_FLAG;} \
          userData = (c_voidp)data; \
        }
#define SET_DURABILITY(userData, durability) \
        { c_address data = (c_address)userData; \
          if (durability) {data |= DURABILITY_FLAG;} else {data &= (c_address)~DURABILITY_FLAG;} \
          userData = (c_voidp)data; \
        }
#define TEST_COMPLETE(userData) (((c_address)userData) & (c_address)COMPLETE_FLAG)
#define TEST_DURABILITY(userData) (((c_address)userData) & (c_address)DURABILITY_FLAG)

/* protected */
void
v_entryInit(
    v_entry _this,
    v_reader reader)
{
    v_kernel kernel;

    assert(C_TYPECHECK(_this,v_entry));
    assert(C_TYPECHECK(reader,v_reader));

    kernel = v_objectKernel(reader);
    _this->reader = reader;
    _this->alignState = V_ALIGNSTATE_INCOMPLETE;
    _this->nvgCount = 0; /* non volatile group count */
    _this->groups = c_tableNew(v_kernelType(kernel,K_PROXY), "source.index,source.server");
    assert(_this->groups != NULL);
}

void
v_entryFree(
    v_entry _this)
{
    c_iter proxies;
    v_proxy proxy;
    v_group group;

    assert(C_TYPECHECK(_this,v_entry));

    proxies = ospl_c_select(_this->groups, 0);
    proxy = c_iterTakeFirst(proxies);
    while (proxy != NULL) {
        group = v_group(v_proxyClaim(proxy));
        if (group) {
            v_groupRemoveEntry(group, _this);
            v_entryRemoveGroup(_this, group);
            v_proxyRelease(proxy);
        }
        c_free(proxy);
        proxy = c_iterTakeFirst(proxies);
    }
    c_iterFree(proxies);
}


v_writeResult
v_entryWrite(
    v_entry _this,
    v_message o,
    v_networkId writingNetworkId,
    c_bool groupRoutingEnabled,
    v_instance *instance,
    v_messageContext context)
{
    v_writeResult writeResult;

    assert(C_TYPECHECK(_this,v_entry));
    assert(C_TYPECHECK(o,v_message));

    switch(v_objectKind(_this->reader)) {
    case K_DATAREADER:
        writeResult = v_dataReaderEntryWrite(v_dataReaderEntry(_this),
                                             o, instance, context);
    break;
    case K_NETWORKREADER:
        writeResult = v_networkReaderEntryWrite(v_networkReaderEntry(_this),
                                                o, writingNetworkId,
                                                groupRoutingEnabled);
    break;
    case K_DELIVERYSERVICE:
        writeResult = v_deliveryServiceEntryWrite(v_deliveryServiceEntry(_this),o,instance);
    break;
    default:
        writeResult = V_WRITE_UNDEFINED;
        OS_REPORT(OS_CRITICAL,
                  "v_entryWrite failed",writeResult,
                  "illegal reader kind (%d) specified",
                  v_objectKind(_this->reader));
        assert(FALSE);
    }
    return writeResult;
}


v_writeResult
v_entryResend(
    v_entry _this,
    v_message o)
{
    v_writeResult writeResult;

    OS_UNUSED_ARG(o);
    assert(C_TYPECHECK(_this,v_entry));
    assert(C_TYPECHECK(o,v_message));

    switch(v_objectKind(_this->reader)) {
    case K_NETWORKREADER:
        writeResult = V_WRITE_SUCCESS;
    break;
    case K_DATAREADER:
    default:
        writeResult = V_WRITE_UNDEFINED;
        OS_REPORT(OS_CRITICAL,
                  "v_entryWrite failed",writeResult,
                  "illegal reader kind (%d) specified",
                  v_objectKind(_this->reader));
        assert(FALSE);
    }
    return writeResult;
}

static c_bool
get_completeness(c_object o, c_voidp arg)
{
    v_alignState *alignState = (v_alignState *)arg;
    if (!TEST_COMPLETE(v_proxyUserData(o))) {
        *alignState = V_ALIGNSTATE_INCOMPLETE;
    }
    return *alignState == V_ALIGNSTATE_COMPLETE;
}

void
v_entryAddGroup(
    v_entry _this,
    v_group group)
{
    v_proxy proxy;
    v_proxy found;
    v_alignState alignState;
    c_bool durability;
    c_voidp userData = NULL;
    c_bool stateChanged = FALSE;

    assert(C_TYPECHECK(_this,v_entry));
    assert(C_TYPECHECK(group,v_group));

    alignState = v_groupCompleteGet(group);
    durability = v_groupIsDurable(group);
    SET_COMPLETE(userData, alignState);
    SET_DURABILITY(userData, durability);

    /* Create group proxy and store completeness (and in the future also durability)
     * of the group as user data in the proxy.
     */
    proxy = v_proxyNew(v_objectKernel(group),
                       v_publicHandle(v_public(group)),
                       (c_voidp)userData);

    OSPL_LOCK(_this->reader);
    found = ospl_c_insert(_this->groups, proxy);
    if(found != proxy){
        /* The group was already available in the groupset. This can happen if
         * the reader gets notified of the group it has just created. In that
         * case the administration should not be updated.
         */
    } else {
        if (_this->alignState != alignState) {
            if (_this->alignState == V_ALIGNSTATE_COMPLETE) {
                /* reader becomes incomplete because an incomplete group is added. */
                _this->alignState = V_ALIGNSTATE_INCOMPLETE;
            } else {
                /* reader was incomplete and group is complete so recheck total completeness. */
                _this->alignState = V_ALIGNSTATE_COMPLETE;
                (void)c_tableWalk(_this->groups, get_completeness, &_this->alignState);
            }
            assert(_this->alignState == V_ALIGNSTATE_COMPLETE || _this->alignState == V_ALIGNSTATE_INCOMPLETE);
            if (_this->alignState == alignState) {
                /* reader completeness has changed so notify state change. */
                stateChanged = TRUE;
            }
        }
        if (durability == TRUE) {
            _this->nvgCount++;
        }
        if (stateChanged) {
            v__readerNotifyStateChange_nl(_this->reader, _this->alignState == V_ALIGNSTATE_COMPLETE);
        }
    }
    OSPL_UNLOCK(_this->reader);
    c_free(proxy);
}

void
v_entryRemoveGroup(
    v_entry _this,
    v_group group)
{
    c_query query;
    q_expr qExpr;
    c_value params[2];
    c_iter groups;
    v_proxy proxy, proxy2;
    v_handle handle;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_entry));
    assert(group != NULL);
    assert(C_TYPECHECK(group,v_group));

    handle = v_publicHandle(v_public(group));
    qExpr = (q_expr)q_parse("source.index = %0 and source.server = %1");
    params[0] = c_ulongValue(handle.index);
    params[1] = c_addressValue(handle.server);

    OSPL_LOCK(_this->reader);
    query = c_queryNew(_this->groups, qExpr, params);
    q_dispose(qExpr);
    groups = ospl_c_select(query, 0);
    c_free(query);
    assert(c_iterLength(groups) <= 1);
    proxy = v_proxy(c_iterTakeFirst(groups));
    proxy2 = c_remove(_this->groups, proxy, NULL, NULL);
    c_free(proxy);
    if (proxy2) {
        if (_this->alignState == V_ALIGNSTATE_INCOMPLETE) {
            _this->alignState = V_ALIGNSTATE_COMPLETE;
            (void)c_tableWalk(_this->groups, get_completeness, &_this->alignState);
            if (_this->alignState == V_ALIGNSTATE_COMPLETE) {
                /* reader became complete because the only incomplete group is removed. */
                v__readerNotifyStateChange_nl(_this->reader, _this->alignState == V_ALIGNSTATE_COMPLETE);
            }
        }
        if (TEST_DURABILITY(v_proxyUserData(proxy2))) {
            _this->nvgCount--;
        }
        c_free(proxy2);
    }
    OSPL_UNLOCK(_this->reader);
    c_iterFree(groups);
}

struct set_completeness_arg {
    v_handle groupHandle;
    v_alignState alignState;
};

static c_bool
set_completeness(c_object o, c_voidp arg)
{
    v_proxy p = v_proxy(o);
    c_bool found = FALSE;
    struct set_completeness_arg *a = (struct set_completeness_arg *)arg;

    if (v_handleIsEqual(p->source, a->groupHandle)) {
        SET_COMPLETE(p->userData, a->alignState);
        found = TRUE;
    }
    return !found;
}

v_alignState
v_entryNotifyGroupStateChange(
    v_entry _this,
    v_handle groupHandle,
    v_alignState alignState)
{
    struct set_completeness_arg arg;
    c_bool changed = FALSE;
    v_reader reader = v_reader(_this->reader);

    OSPL_LOCK(reader);

    arg.groupHandle = groupHandle;
    arg.alignState = alignState;
    (void)c_tableWalk(_this->groups, set_completeness, &arg);
    if (alignState != _this->alignState) {
        if (_this->alignState == V_ALIGNSTATE_COMPLETE) {
            _this->alignState = V_ALIGNSTATE_INCOMPLETE;
        } else {
            _this->alignState = V_ALIGNSTATE_COMPLETE;
            (void)c_tableWalk(_this->groups, get_completeness, &_this->alignState);
        }
        alignState = _this->alignState;
        changed = TRUE;
    }
    if (changed) {
        v__readerNotifyStateChange_nl(reader, alignState == V_ALIGNSTATE_COMPLETE);
        if (alignState == V_ALIGNSTATE_COMPLETE) {
            /* At this point it all goups have become complete so the reader can get
             * the historical data (changes).
             * For now the data is still pushed by the group so it is already available
             * in the reader but this should be replaced by the reader getting it as
             * soon as the groups are complete.
             * Just getting the data from the group at this point isn't possible without
             * solving locking issues, this operation is called by the group and requires
             * locking of the reader and groups to get the data, need to decide if this
             * can be performed synchonuous or asychronuous by a reader/subscriber/participant
             * or application thread.
             */
        }
    }
    OSPL_UNLOCK(reader);

    return alignState;
}

struct groupExistsArg {
    v_proxy proxy;
    c_bool exists;
};

static c_bool groupExists(c_object o, c_voidp arg)
{
    v_proxy p1 = v_proxy(o);
    struct groupExistsArg *a = (struct groupExistsArg *)arg;

    a->exists = v_handleIsEqual(p1->source, a->proxy->source);
    return !(a->exists); /* as long as a->equal==FALSE, continue walk */
}

c_bool
v_entryGroupExists(
    v_entry _this,
    v_group group)
{
    struct groupExistsArg arg;
    c_bool exists;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_entry));
    assert(group != NULL);
    assert(C_TYPECHECK(group,v_group));

    arg.exists = FALSE;
    arg.proxy = v_proxyNew(v_objectKernel(group), v_publicHandle(v_public(group)), NULL);
    exists = c_tableWalk(_this->groups, groupExists, &arg);
    assert(exists != (arg.exists));
    OS_UNUSED_ARG(exists);
    c_free(arg.proxy);
    return arg.exists;
}

c_long
v_entryDurableGroupCount(
    v_entry _this)
{
    return _this->nvgCount;
}

struct groupActionHelper {
    c_action action;
    c_voidp  arg;
};

static c_bool
groupAction(
    c_object o,
    c_voidp arg)
{
    v_proxy p = v_proxy(o);
    struct groupActionHelper *helper = (struct groupActionHelper *)arg;
    v_group group;
    c_bool result = FALSE;

    assert(helper);

    if (v_handleClaim(p->source, (v_object*)(&group)) == V_HANDLE_OK) {
        assert(helper->action);
        result = helper->action(group, helper->arg);
        (void)v_handleRelease(p->source);
    }
    return result;
}

c_bool
v_entryWalkGroups(
    v_entry _this,
    c_action action,
    c_voidp actionArg)
{
    struct groupActionHelper helper;
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_entry));

    helper.action = action;
    helper.arg = actionArg;

    return c_tableWalk(_this->groups, groupAction, &helper);
}

c_iter
v_entryGetGroups(
    v_entry _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_entry));

    return ospl_c_select(_this->groups, 0);
}

