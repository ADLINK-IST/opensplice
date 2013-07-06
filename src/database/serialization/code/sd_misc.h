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
/** \file services/serialization/code/sd_misc.h
 *  \brief Declaration of functions for common use by all serializer descendants
 */

#ifndef SD_MISC_H
#define SD_MISC_H

#include "c_metabase.h"

c_unionCase  sd_unionDetermineActiveCase(c_union v_union, c_object object);
c_char *     sd_getScopedTypeName(c_type type, const c_char *moduleSep);
char *       sd_stringDup(const char *string);
c_bool       sd_stringToLong(const c_char *str, c_long *retval);
c_bool       sd_stringToLongLong(const c_char *str, c_longlong *retval);
c_bool       sd_stringToBoolean(const c_char *str, c_bool *retval);
c_bool       sd_stringToAddress(const c_char *str, c_address *retval);

#endif /* SD_MISC_H */
