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

OS_API c_ulong sd_serializedDataGetTotalSize(sd_serializedData serData);
OS_API void    sd_serializedDataFree(sd_serializedData serData);

/** \brief \b sd_serializer is the abstract baseclass for all serializer
 *            classes. It defines the serialization interface and offers
 *            support for serialization validation.
 */
C_CLASS(sd_serializer);

OS_API sd_serializedData
sd_serializerSerialize(
    sd_serializer _this,
    c_object object);
    
OS_API c_object
sd_serializerDeserialize(
    sd_serializer _this,
    sd_serializedData serData);
    
OS_API c_object
sd_serializerDeserializeValidated(
    sd_serializer _this,
    sd_serializedData serData);
    
OS_API void
sd_serializerDeserializeInto(
    sd_serializer _this,
    sd_serializedData serData,
    c_object object);
    
OS_API void
sd_serializerDeserializeIntoValidated(
    sd_serializer _this,
    sd_serializedData serData,
    c_object object);

OS_API c_char *
sd_serializerToString(
    sd_serializer _this,
    sd_serializedData serData);

OS_API sd_serializedData
sd_serializerFromString(
    sd_serializer _this,
    const c_char *str);
    
OS_API void
sd_serializerFree(
    sd_serializer _this);

/* Validation results */

/** \brief Enumeration indicating the result of validation
 * 
 *  The meaning of the values is as follows:
 *  NOT_DONE: no validation was done, so no result available
 *  SUCCESS: validation was done, no errors were encountered
 *  ERROR: validation was done and errors were encountered
 */
typedef enum sd_validationResult_e {
   SD_VAL_NOT_DONE,
   SD_VAL_SUCCESS,
   SD_VAL_ERROR
} sd_validationResult;

OS_API sd_validationResult
sd_serializerLastValidationResult(
    sd_serializer _this);

OS_API c_char *
sd_serializerLastValidationMessage(
    sd_serializer _this);

OS_API c_char *
sd_serializerLastValidationLocation(
    sd_serializer _this);

#undef OS_API

#endif  /* SD_SERIALIZER_H */
