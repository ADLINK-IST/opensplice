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
#ifndef CF_DATA_H
#define CF_DATA_H

#include "cf_node.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define CF_DATANAME "#text"

#define       cf_data(o)    ((cf_data)(o))

C_CLASS(cf_data);
C_STRUCT(cf_data) {
    C_EXTENDS(cf_node);
    c_value value;
};

OS_API cf_data
cf_dataNew (
    c_value value);

OS_API void
cf_dataInit (
    cf_data data,
    c_value value);

OS_API void
cf_dataDeinit (
    cf_data data);

OS_API c_value
cf_dataValue(
    cf_data data);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CF_DATA_H */
