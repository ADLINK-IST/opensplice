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

#ifndef OSPL_PROC_H
#define OSPL_PROC_H

void
kill_descendents (
    pid_t pid,
    int signal
    );

#endif /* OSPL_PROC_H */
