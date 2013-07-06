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
#include "rs_reportHandler.h"

#include "os_heap.h"
#include "os_mutex.h"
#include "os_stdlib.h"
#include "c_typebase.h"
#include "c_iterator.h"

C_STRUCT(rs_reportHandler) {
    c_char *handlerId;
    c_ushort port;
};

rs_reportHandler
rs_reportHandlerNew (
    c_char *handlerId,
    c_ushort port)
{
    rs_reportHandler reportHandler;

    assert (handlerId);

    reportHandler = os_malloc (C_SIZEOF(rs_reportHandler));
    reportHandler->handlerId = os_strdup(handlerId);
    reportHandler->port = port;
    return reportHandler;
}

void
rs_reportHandlerFree (
    rs_reportHandler reportHandler)
{
    assert (reportHandler);
    assert (reportHandler->handlerId);

    os_free (reportHandler->handlerId);
    os_free (reportHandler);
}

void
rs_reportHandlerReport (
    rs_reportHandler reportHandler,
    rs_reportMsg message)
{
    assert (reportHandler);
    assert (message);

    rs_reportMsgReport (message);
}
