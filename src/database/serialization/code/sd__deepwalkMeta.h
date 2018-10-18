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
/** \file services/serialization/code/sd__deepwalkMeta.h
 *  \brief Prototypes for the deepwalkMeta functionality, to be used by
 *         \b serializer descendants which need extended meta information.
 */

#ifndef SD__DEEPWALKMETA_H
#define SD__DEEPWALKMETA_H

#include "c_typebase.h"
#include "c_metabase.h"
#include "sd_deepwalkMeta.h"

/* A helper class for storing context information concerning the metaDeepwalk */


sd_errorInfo           sd_errorInfoNew(
                           c_ulong errorNumber,
                           const c_char *name,
                           const c_char *message,
                           c_char *location);

void                   sd_errorInfoFree(
                           sd_errorInfo errorInfo);

c_char *               sd_errorInfoGetName(
                           sd_errorInfo errorInfo);

void                   sd_errorInfoSetName(
                           sd_errorInfo errorInfo,
                           const c_char *name);

c_bool                 sd_errorInfoGet(
                           sd_errorInfo errorInfo,
                           c_ulong *errorNumber,
                           c_char **name,
                           c_char **message,
                           c_char **location);


#endif  /* SD__DEEPWALKMETA_H */
