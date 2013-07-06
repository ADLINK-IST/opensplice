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

#ifndef U__DATAVIEW_H
#define U__DATAVIEW_H

#include "u_dataView.h"

u_result
u_dataViewInit (
    u_dataView view,
    u_dataReader reader);

u_result
u_dataViewDeinit (
    u_dataView view);

#endif
