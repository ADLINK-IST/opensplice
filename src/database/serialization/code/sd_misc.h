/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
char *       sd_stringDup(const char *string) __attribute_warn_unused_result__;
c_bool       sd_stringToLong(const c_char *str, c_long *retval);
c_bool       sd_stringToLongLong(const c_char *str, c_longlong *retval);
c_bool       sd_stringToBoolean(const c_char *str, c_bool *retval);
c_bool       sd_stringToAddress(const c_char *str, c_address *retval);

#endif /* SD_MISC_H */
