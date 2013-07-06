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
#ifndef V__STATISTICS_H
#define V__STATISTICS_H

#include "v_kernel.h"
#include "v_statistics.h"
#include "v__statisticsInterface.h"

void
v_statisticsInit (
    v_statistics _this);

c_bool
v_statisticsResetField(
    v_statistics _this,
    const c_char * fieldName);

#endif /* V__STATISTICS_H */
