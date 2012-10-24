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
#ifndef NW_WRITERSYNC_H
#define NW_WRITERSYNC_H

#include "nw_writer.h"

/* This class is derived from the abstract baseclass nw_writer */

NW_CLASS(nw_writerSync);

nw_writer
nw_writerSyncNew(
    void);

#endif /*NW_WRITERSYNC_H*/
