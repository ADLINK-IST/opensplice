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
#include "v__entry.h"
#include "v_entity.h"
#include "v_partition.h"
#include "v_topic.h"
#include "v_reader.h"
#include "v_writer.h"
#include "v_kernel.h"
#include "v_handle.h"
#include "v_dataReaderEntry.h"
#include "v_networkReaderEntry.h"
#include "v_deliveryServiceEntry.h"
#include "v_writerInstance.h"
#include "v_public.h"
#include "v_group.h"
#include "v_proxy.h"
#include "os_report.h"

/* protected */
void
v_entryInit(
    v_entry e,
    v_reader r)
{
    v_kernel kernel;

    assert(C_TYPECHECK(e,v_entry));
    assert(C_TYPECHECK(r,v_reader));

    kernel = v_objectKernel(r);
    e->reader = r;
    e->groups = c_tableNew(v_kernelType(kernel,K_PROXY),
                          "source.index,source.server");
    assert(e->groups != NULL);
}

void
v_entryFree(
    v_entry entry)
{
    c_iter proxies;
    v_proxy proxy;
    v_group group;

    assert(C_TYPECHECK(entry,v_entry));

    proxies = ospl_c_select(entry->groups, 0);
    proxy = c_iterTakeFirst(proxies);
    while (proxy != NULL) {
        group = v_group(v_proxyClaim(proxy));
        if (group) {
            v_groupRemoveEntry(group, entry);
            v_proxyRelease(proxy);
        }
        c_free(proxy);
        proxy = c_iterTakeFirst(proxies);
    }
    c_iterFree(proxies);

    /* No parent to call Free on */
}


v_writeResult
v_entryWrite(
    v_entry e,
    v_message o,
    v_networkId writingNetworkId,
    v_instance *instance)
{
    v_writeResult writeResult;

    assert(C_TYPECHECK(e,v_entry));
    assert(C_TYPECHECK(o,v_message));

    switch(v_objectKind(e->reader)) {
    case K_DATAREADER:
        writeResult = v_dataReaderEntryWrite(v_dataReaderEntry(e),
                                             o,
                                             instance);
    break;
    case K_NETWORKREADER:
        writeResult = v_networkReaderEntryWrite(v_networkReaderEntry(e),
                                                o, writingNetworkId);
    break;
    case K_DELIVERYSERVICE:
        writeResult = v_deliveryServiceEntryWrite(v_deliveryServiceEntry(e),o,instance);
    break;
    default:
        OS_REPORT_1(OS_ERROR,
                    "v_entryWrite failed",0,
                    "illegal reader kind (%d) specified",
                    v_objectKind(e->reader));
        assert(FALSE);
        return V_WRITE_UNDEFINED;
    }
    return writeResult;
}


v_writeResult
v_entryResend(
    v_entry e,
    v_message o)
{
    v_writeResult writeResult;

    OS_UNUSED_ARG(o);
    assert(C_TYPECHECK(e,v_entry));
    assert(C_TYPECHECK(o,v_message));

    switch(v_objectKind(e->reader)) {
    case K_NETWORKREADER:
        writeResult = V_WRITE_SUCCESS;
    break;
    case K_DATAREADER:
    default:
        OS_REPORT_1(OS_ERROR,
                    "v_entryWrite failed",0,
                    "illegal reader kind (%d) specified",
                    v_objectKind(e->reader));
        assert(FALSE);
        return V_WRITE_UNDEFINED;
    }

    return writeResult;
}

c_bool
v_entryAddGroup(
    v_entry entry,
    v_group group)
{
    v_proxy proxy;
    v_proxy found;
    c_bool result;

    assert(C_TYPECHECK(entry,v_entry));
    assert(C_TYPECHECK(group,v_group));

    proxy = v_proxyNew(v_objectKernel(group),
                       v_publicHandle(v_public(group)), NULL);
    found = c_insert(entry->groups, proxy);
    if(found != proxy){
        /* The group was already available in the groupset. This can happen if
         * the reader gets notified of the group it has just created. In that
         * case the administration should not be updated. */
        result = FALSE;
    } else {
        result = TRUE;
    }
    c_free(proxy);

    return result;
}

void
v_entryRemoveGroup(
    v_entry entry,
    v_group group)
{
    c_query query;
    q_expr qExpr;
    c_value params[2];
    c_iter groups;
    v_proxy proxy, proxy2;
    v_handle handle;

    assert(entry != NULL);
    assert(C_TYPECHECK(entry,v_entry));
    assert(group != NULL);
    assert(C_TYPECHECK(group,v_group));

    handle = v_publicHandle(v_public(group));
    qExpr = (q_expr)q_parse("source.index = %0 and source.server = %1");
    params[0] = c_longValue(handle.index);
    params[1] = c_addressValue(handle.server);

    query = c_queryNew(entry->groups, qExpr, params);
    q_dispose(qExpr);
    groups = ospl_c_select(query, 0);
    c_free(query);
    assert(c_iterLength(groups) <= 1);
    proxy = v_proxy(c_iterTakeFirst(groups));
    proxy2 = c_remove(entry->groups, proxy, NULL, NULL);
    c_iterFree(groups);
    c_free(proxy);
    c_free(proxy2);
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
    v_entry entry,
    v_group group)
{
    c_bool r;
    struct groupExistsArg arg;

    assert(entry != NULL);
    assert(C_TYPECHECK(entry,v_entry));
    assert(group != NULL);
    assert(C_TYPECHECK(group,v_group));

    arg.exists = FALSE;
    arg.proxy = v_proxyNew(v_objectKernel(group),
                       v_publicHandle(v_public(group)), NULL);
    r = c_tableWalk(entry->groups, groupExists, &arg);
    assert(r != (arg.exists));
    c_free(arg.proxy);
    return arg.exists;
}
