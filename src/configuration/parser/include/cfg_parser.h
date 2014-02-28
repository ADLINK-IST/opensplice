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
#ifndef CFG_PARSER_H
#define CFG_PARSER_H

#include "cf_element.h"

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

typedef enum cfgprs_status_enum {
    CFGPRS_OK,
    CFGPRS_NO_INPUT,
    CFGPRS_ERROR
} cfgprs_status;

OS_API cfgprs_status
cfg_parse_ospl (
    const char *uri,
    cf_element *spliceElement);

OS_API cfgprs_status
cfg_parse_str (
    const char *str,
    cf_element *spliceElement);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CFG_PARSER_H */
