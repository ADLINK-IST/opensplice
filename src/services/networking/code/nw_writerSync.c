/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
/* Interface */
#include "nw_writerSync.h"

/* Base class */
#include "nw__writer.h"

/* Implementation */
#include "os.h"
#include "v_networkReaderEntry.h"

/**
* Derived class that is exactly the same as the base class
* @extends nw_writer_s
*/
NW_STRUCT(nw_writerSync) {
    NW_EXTENDS(nw_writer);
    /* No members in derived class */
};

static nw_bool nw_writerSyncWriteMessage(nw_writer writer,
    v_networkReaderEntry entry, v_message message, c_ulong messageId,
    v_gid   sender, c_bool  sendTo, v_gid   receiver);


nw_writer
nw_writerSyncNew(
    void)
{
    nw_writer result = NULL;
    nw_writerSync writerSync;

    writerSync = (nw_writerSync)os_malloc(sizeof(*writerSync));

    if (writerSync) {
        result = (nw_writer)writerSync;
        nw_writerInitialize(result, nw_writerSyncWriteMessage, NULL);
    }

    return result;
}


static nw_bool
nw_writerSyncWriteMessage(
    nw_writer writer,
    v_networkReaderEntry entry,
    v_message message,
    c_ulong messageId,
    v_gid   sender,
    c_bool  sendTo,
    v_gid   receiver)
{
    v_writeResult write_res;
    /* Passive component, just forward to the entry */
    write_res = v_networkReaderEntryReceive(entry, message, messageId, sender, sendTo, receiver);
    return (write_res == V_WRITE_SUCCESS);
}
