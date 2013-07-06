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

#ifndef V__LIFESPANADMIN_H
#define V__LIFESPANADMIN_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_kernel.h"
#include "v_lifespanSample.h"

#define v_lifespanAdmin(o) (C_CAST((o),v_lifespanAdmin))

v_lifespanAdmin
v_lifespanAdminNew(
    v_kernel kernel);

void
v_lifespanAdminInsert(
    v_lifespanAdmin _this,
    v_lifespanSample sample);

void
v_lifespanAdminRemove(
    v_lifespanAdmin _this,
    v_lifespanSample sample);
void
v_lifespanAdminTakeExpired(
    v_lifespanAdmin _this,
    v_lifespanSampleAction action,
    c_voidp arg);

c_long
v_lifespanAdminSampleCount(
    v_lifespanAdmin _this);

#if defined (__cplusplus)
}
#endif

#endif /* V__LIFESPANADMIN_H */
