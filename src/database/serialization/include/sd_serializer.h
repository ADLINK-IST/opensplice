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
/** \file services/serialization/include/sd_serializer.h
 *  \brief Declaration of the \b serializer class.
 */

#ifndef SD_SERIALIZER_H
#define SD_SERIALIZER_H

#include "c_typebase.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief \b sd_serializedData is a class encapsulating the serialized data
 *            resulting from the serialization funtion. It is used by
 *            \b sd_serializer.
 */
C_CLASS(sd_serializedData);

OS_API c_ulong sd_serializedDataGetTotalSize(sd_serializedData serData) __nonnull_all__;
OS_API void    sd_serializedDataFree(sd_serializedData serData) __nonnull_all__;

/** \brief \b sd_serializer is the abstract baseclass for all serializer
 *            classes. It defines the serialization interface and offers
 *            support for serialization validation.
 */
C_CLASS(sd_serializer);

OS_API sd_serializedData
sd_serializerSerialize(
    sd_serializer _this,
    c_object object) __nonnull_all__ __attribute_warn_unused_result__;
    
OS_API c_object
sd_serializerDeserialize(
    sd_serializer _this,
    sd_serializedData serData) __nonnull_all__ __attribute_warn_unused_result__;
    
OS_API c_bool
sd_serializerDeserializeInto(
    sd_serializer _this,
    sd_serializedData serData,
    c_object object) __nonnull_all__ __attribute_warn_unused_result__;
    
OS_API c_char *
sd_serializerToString(
    sd_serializer _this,
    sd_serializedData serData) __nonnull_all__ __attribute_warn_unused_result__;

OS_API sd_serializedData
sd_serializerFromString(
    sd_serializer _this,
    const c_char *str) __nonnull_all__ __attribute_warn_unused_result__;
    
OS_API void
sd_serializerFree(
    sd_serializer _this) __nonnull_all__;

/* Validation results */

OS_API c_char *
sd_serializerLastValidationMessage(
    sd_serializer _this) __nonnull_all__;

OS_API c_char *
sd_serializerLastValidationLocation(
    sd_serializer _this) __nonnull_all__;

#undef OS_API

#endif  /* SD_SERIALIZER_H */
