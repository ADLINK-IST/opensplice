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
/** \file services/serialization/code/sd_deepwalk.h
 *  \brief Prototypes for the deepwalk functionality, to be used by
 *         \b serializer descendants.
 */

#ifndef SD_DEEPWALK_H
#define SD_DEEPWALK_H

#include "c_typebase.h"
#include "c_metabase.h"

typedef c_bool sd_deepwalkFunc(c_type type, c_object *object, void *arg) __nonnull((1, 2)) __attribute_warn_unused_result__;

c_bool sd_deepwalk(c_type type, c_object *objectPtr, sd_deepwalkFunc action, void *actionArg) __nonnull((1, 2, 3)) __attribute_warn_unused_result__;

#endif  /* SD_DEEPWALK_H */
