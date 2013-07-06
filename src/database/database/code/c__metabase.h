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

#ifndef C__METABASE_H
#define C__METABASE_H

#include "c_metabase.h"

#define C_META_OFFSET_(t,f) \
        ((c_address)(&((t)0)->f))

#define C_META_ATTRIBUTE_(c,o,n,t) \
        c_metaAttributeNew(o,#n,t,C_META_OFFSET_(c,n))

#define C_META_TYPEINIT_(o,t) \
        (c_metaTypeInit(o,C_SIZEOF(t),C_ALIGNMENT(C_STRUCT(t))))

#if defined (__cplusplus)
extern "C" {
#endif

c_bool
c_objectIs (
    c_baseObject _this,
    c_metaKind kind);

c_bool
c_objectIsType (
    c_baseObject _this);

void
c_metaAttributeNew (
    c_metaObject scope,
    const c_char *name,
    c_type type,
    c_address offset);

void
c_metaTypeInit (
    c_object _this,
    c_long size,
    c_long alignment);

void
c_metaCopy (
    c_metaObject src,
    c_metaObject dst);

c_result
c__metaFinalize(
    c_metaObject o,
    c_bool normalize);

c_bool
c_isBaseObjectType(
    c_type type);

c_bool
c_isBaseObject(
    c_object obj);

#if defined (__cplusplus)
}
#endif

#endif
