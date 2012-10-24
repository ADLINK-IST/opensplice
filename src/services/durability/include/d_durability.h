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


#ifndef D_DURABILITY_H
#define D_DURABILITY_H

#include "u_user.h"
#include "d__types.h"
#include "v_durabilityStatistics.h"
#include "v_maxValue.h"
#include "v_minValue.h"
#include "v_statistics.h"
#include "v_statisticsHelpers.h"
#include "v_statisticsInterface.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_durability(d) ((d_durability)(d))

typedef void (*d_durabilityStatisticsCallback)(v_durabilityStatistics statistics, c_voidp userData);

int                 main                                (int argc, char* argv[]);

u_service           d_durabilityGetService              (d_durability durability);

d_configuration     d_durabilityGetConfiguration        (d_durability durability);

c_bool              d_durabilityWaitForAttachToGroup    (d_durability durability,
                                                         v_group group);

d_serviceState      d_durabilityGetState                (d_durability durability);

void                d_durabilitySetState                (d_durability durability,
                                                         d_serviceState state);

c_bool              d_durabilityMustTerminate           (d_durability durability);

void                d_durabilityTerminate               (d_durability durability);

void                d_durabilityUpdateStatistics        (d_durability durability,
                                                         d_durabilityStatisticsCallback callback,
                                                         c_voidp userData);
u_result
d_durabilityTakePersistentSnapshot(
    d_durability durability,
    c_char* partitionExpr,
    c_char* topicExpr,
    c_char* uri);
#if defined (__cplusplus)
}
#endif

#endif /* D_DURABILITY_H */
