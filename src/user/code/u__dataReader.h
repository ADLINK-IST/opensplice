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

#ifndef U__DATAREADER_H
#define U__DATAREADER_H

#include "u_dataReader.h"

u_result
u_dataReaderInit (
    u_dataReader _this);

u_result
u_dataReaderDeinit (
    u_dataReader _this);

u_result
u_dataReaderClaim(
    u_dataReader _this,
    v_dataReader *dataReader);

u_result
u_dataReaderRelease(
    u_dataReader _this);

#endif
