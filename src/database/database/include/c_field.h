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
#ifndef C_FIELD_H
#define C_FIELD_H

/** \file c_base.h
 *  \brief This file specifies the database c_field class interface.
 *
 * The c_field class implements a helper object to access the field value
 * of a structured type.
 */

#include "c_base.h"
#include "os_if.h"

#ifdef OSPL_BUILD_DB
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Meta data definitions:
 */
C_CLASS(c_field);

OS_API c_type
c_field_t(
    c_base base);

/**
 * c_field constructors:
 */
OS_API c_field
c_fieldNew(
    c_type type,
    const c_char *fieldName);

OS_API c_field
c_fieldConcat(
    c_field head,
    c_field tail);

/**
 * c_field methods:
 */
OS_API c_string
c_fieldName(
    c_field _this);

OS_API c_array
c_fieldPath(
    c_field _this);

OS_API c_type
c_fieldType(
    c_field _this);

OS_API c_valueKind
c_fieldValueKind(
    c_field _this);

OS_API void
c_fieldFreeRef (
    c_field field,
    c_object o);

OS_API void
c_fieldAssign(
    c_field _this,
    c_object o,
    c_value value);

OS_API c_value
c_fieldValue(
    c_field _this,
    c_object o);

OS_API void
c_fieldCopy(
    c_field srcfield,
    c_object src,
    c_field dstfield,
    c_object dst);

OS_API void
c_fieldClone(
    c_field srcfield,
    c_object src,
    c_field dstfield,
    c_object dst);

OS_API c_equality
c_fieldCompare (
    c_field field1,
    c_object src1,
    c_field field2,
    c_object src2);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
