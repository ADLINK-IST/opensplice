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
#ifndef NW_WRITER_H
#define NW_WRITER_H

#include "nw_commonTypes.h"
#include "v_kernel.h"

/* nw_writer is the abstract base class for all
 * network writers. Therefore, it does not have
 * a constructor. */

/**
* @class nw_writer
*/
NW_CLASS(nw_writer);

/* These are the public methods */

nw_bool
nw_writerWriteMessage(
    nw_writer writer,
    v_networkReaderEntry entry,
    v_message message,
    c_ulong messageId,
    v_gid sender,
    c_bool sendTo,
    v_gid receiver);

void
nw_writerFree(
    nw_writer writer);

#endif /*NW_WRITER_H*/
