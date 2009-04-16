/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef V__GROUPSETOBSERVER_H
#define V__GROUPSETOBSERVER_H

v_groupSetObserver
v_groupSetObserverNew(
    v_groupSet set);

void
v_groupSetObserverSetNewGroupStatus(
    v_groupSetObserver _this,
    v_group group);
                       
#endif
