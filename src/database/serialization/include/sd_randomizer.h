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
/** \file services/serialization/include/sd_randomizer.h
 *  \brief Declaration of the \b randomizer class for creating random instances
 *         of database objects.
 */

#ifndef SD_RANDOMIZER_H
#define SD_RANDOMIZER_H

#include "c_typebase.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(sd_randomizer);

OS_API sd_randomizer
sd_randomizerNew(
    c_base base) __nonnull_all__;
    
OS_API void
sd_randomizerInit(
    sd_randomizer randomizer,
    c_ulong seed) __nonnull_all__;
    
OS_API c_object
sd_randomizerRandomInstance(
    sd_randomizer randomizer,
    const c_char *typeName) __nonnull_all__ __attribute_warn_unused_result__;
    
OS_API void
sd_randomizerFree(
    sd_randomizer randomizer) __nonnull_all__;

#undef OS_API

#endif  /* SD_RANDOMIZER_H */
