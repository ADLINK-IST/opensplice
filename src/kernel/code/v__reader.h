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

#ifndef V__READER_H
#define V__READER_H

#include "v_reader.h"
#include "v_entity.h"

void
v_readerInit(
    v_reader _this,
    const c_char *name,
    v_subscriber s,
    v_readerQos qos,
    v_statistics rs,
    c_bool enable);

void
v_readerDeinit(
    v_reader _this);

void
v_readerFree(
    v_reader _this);

c_bool
v_readerSubscribe(
    v_reader _this,
    v_partition d);

c_bool
v_readerUnSubscribe(
    v_reader _this,
    v_partition d);

c_bool
v_readerSubscribeGroup (
    v_reader _this,
    v_group group);

c_bool
v_readerUnSubscribeGroup (
    v_reader _this,
    v_group g);

v_result
v_readerSetQos (
    v_reader _this,
    v_readerQos qos);

v_entry
v_readerAddEntry(
    v_reader _this,
    v_entry e);

v_entry
v_readerRemoveEntry(
    v_reader _this,
    v_entry e);

#endif
typedef int aap;
