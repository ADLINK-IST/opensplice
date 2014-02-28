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

#include "v_instance.h"

#include "v_time.h"
#include "v__dataReaderInstance.h"

#include "os.h"
#include "os_report.h"

void
v_instanceInit(
    v_instance instance)
{
    assert(C_TYPECHECK(instance, v_instance));

    /* Public part is initialised at reader or writer */

    instance->next = instance;
    instance->prev = instance;
    instance->lastDeadlineResetTime = C_TIME_MIN_INFINITE;
}

void
v_instanceDeinit(
    v_instance instance)
{
    assert(C_TYPECHECK(instance, v_instance));
    /* possible since next and prev are void pointers,
     * so c_free does not crash on this */
    v_instanceRemove(instance);
}

void
v_instanceInsert(
    v_instance instance,
    v_instance prev)
{
    assert(C_TYPECHECK(instance, v_instance));
    assert(C_TYPECHECK(prev, v_instance));
    assert(v_instanceAlone(prev));

    v_instance(instance->prev)->next = prev;
    prev->prev = instance->prev;
    instance->prev = prev;
    prev->next = instance;
}

void
v_instanceAppend(
    v_instance instance,
    v_instance next)
{
    assert(C_TYPECHECK(instance, v_instance));
    assert(C_TYPECHECK(next, v_instance));
    assert(v_instanceAlone(next));

    v_instance(instance->next)->prev = next;
    next->next = instance->next;
    instance->next = next;
    next->prev = instance;
}

void
v_instanceRemove(
    v_instance instance)
{
    assert(C_TYPECHECK(instance, v_instance));

    v_instance(instance->next)->prev = instance->prev;
    v_instance(instance->prev)->next = instance->next;
    instance->next = instance;
    instance->prev = instance;
}

c_bool
v_instanceAlone(
    v_instance instance)
{
    assert(C_TYPECHECK(instance, v_instance));

    return (instance->next == instance);
}

void
v_instanceUpdate(
    v_instance instance,
    c_time timestamp)
{
    assert(C_TYPECHECK(instance, v_instance));

    instance->lastDeadlineResetTime = timestamp;
}

v_writeResult
v_instanceWrite(
    v_instance instance,
    v_message message)
{
    c_char *metaName;

    assert(C_TYPECHECK(instance, v_instance));

    switch (v_objectKind(instance)) {
    case K_DATAREADERINSTANCE:
        return v_dataReaderInstanceWrite(v_dataReaderInstance(instance),message);
    default:
        metaName = c_metaName(c_metaObject(c_getType(instance)));
        OS_REPORT_1(OS_ERROR,
                    "v_instanceWrite",0,
                    "Unknown instance type <%s>",
                    metaName);
        c_free(metaName);
        return V_WRITE_PRE_NOT_MET;
    }
}

void
v_instanceUnregister (
    v_instance instance,
    v_registration registration,
    c_time timestamp)
{
    c_char* metaName;

    assert(C_TYPECHECK(instance, v_instance));

    switch (v_objectKind(instance)) {
    case K_DATAREADERINSTANCE:
        v_dataReaderInstanceUnregister(v_dataReaderInstance(instance),
                registration, timestamp);
    break;
    default:
        metaName = c_metaName(c_metaObject(c_getType(instance)));
        OS_REPORT_1(OS_ERROR,
                    "v_instanceUnregister",0,
                    "Unknown instance type <%s>",
                    metaName);
        c_free(metaName);
    break;
    }
}

