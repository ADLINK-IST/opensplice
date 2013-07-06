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

#ifndef V__ENTRY_H
#define V__ENTRY_H

#include "v_entry.h"

#define v_entryReader(_this) \
        v_reader(v_entry(_this)->reader)

#define v_entryReaderQos(_this) \
        (v_entryReader(_this)->qos)

void
v_entryInit (
    v_entry _this,
    v_reader r);

void
v_entryFree (
    v_entry _this);

/**
 * Adds the group to the entry. Returns TRUE if the entry was not yet connected
 * to the group and the group has thus been added. Returns FALSE otherwise.
 *
 * @param   _this   The entry to add the group to
 * @param   g       The group to be added to the entry
 * @return          TRUE if the group was added, FALSE if the group was already
 *                  in the entry
 */
c_bool
v_entryAddGroup (
    v_entry _this,
    v_group g);

void
v_entryRemoveGroup (
    v_entry _this,
    v_group g);

c_bool
v_entryGroupExists(
    v_entry entry,
    v_group group);

#endif
