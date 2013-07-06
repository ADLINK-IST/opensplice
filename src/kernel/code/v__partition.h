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

#ifndef V__PARTITION_H
#define V__PARTITION_H

#include "v_partition.h"

#define v_partitionInterest(o) (C_CAST(o,v_partitionInterest))

c_bool
v_partitionExpressionIsAbsolute(
    const c_char* expression);

c_bool
v_partitionStringMatchesExpression(
    const c_char* string,
    const c_char* expression);

v_partitionInterest
v_partitionInterestNew (
    v_kernel kernel,
    const char *partitionExpression);

#endif
