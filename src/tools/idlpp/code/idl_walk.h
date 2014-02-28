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
#ifndef IDL_WALK_H
#define IDL_WALK_H

#include "c_base.h"
#include "c_typebase.h"
#include "c_iterator.h"

#include "idl_program.h"
#include "idl_tmplExp.h"

void
idl_walkPresetFile(
    const char *fileName);

void
idl_walkPresetModule(
    const char *moduleName);

void
idl_walk(
    c_base base,
    const char *fileName,
    c_iter objects,
    c_bool traceWalk,
    idl_program program);

idl_typeSpec
idl_makeTypeCollection(
    c_collectionType type);

idl_scope
idl_buildScope (
    c_metaObject o);

#endif /* IDL_WALK_H */
