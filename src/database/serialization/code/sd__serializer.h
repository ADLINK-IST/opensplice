/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
/** \file services/serialization/code/sd__serializer.h
 *  \brief Protected method prototypes of the
 *         \b serializedData and \b serializer classes.
 *
 *  This private header file is to be included by \b serializer
 *  descendants only in order to simulate protected access.
 */

#ifndef SD__SERIALIZER_H
#define SD__SERIALIZER_H

#include "sd_serializer.h"
#include "c_typebase.h"
#include "c_metabase.h"

/** \brief Platform independent representation of the serializer format ID */
typedef c_octet sd_formatID[2];

/** \brief Platform independent representation of the serializer format version
 */
typedef c_octet sd_formatVersion[2];

/** \brief Platform independent representation of size of the serialized data,
 *         in bytes
 */
typedef c_octet sd_dataSize[4];

/** \brief Shortcut for a stretchy array type */
typedef c_octet sd_data[1 /* ...stretchy... */ ];

/** \brief definition of the \b sd_serializedData class as given in
 *         sd_serializer.h
 */
C_STRUCT(sd_serializedData) {
    sd_formatID      formatID;
    sd_formatVersion formatVersion;
    sd_dataSize      dataSize;
    sd_data          data;
};

sd_serializedData
sd_serializedDataNew(c_ushort formatID,
    c_ushort formatVersion,
    os_size_t dataSize);

c_ushort
sd_serializedDataGetFormatID(
    sd_serializedData _this);

c_ushort
sd_serializedDataGetFormatVersion(
    sd_serializedData _this);

c_ulong
sd_serializedDataGetDataSize(
    sd_serializedData _this);


/** \brief Prototype for the virtual method serializer->serialize */
typedef sd_serializedData (*sd_serialFunc)(sd_serializer serializer, c_object object) __nonnull_all__ __attribute_warn_unused_result__;

/** \brief Prototype for the virtual method serializer->deserialize */
typedef c_object          (*sd_deserialFunc)(sd_serializer serializer, sd_serializedData serData) __nonnull_all__ __attribute_warn_unused_result__;

/** \brief Prototype for the virtual method serializer->deserializeInto */
typedef c_bool            (*sd_deserialIntoFunc)(sd_serializer serializer, sd_serializedData serData, c_object object) __nonnull_all__ __attribute_warn_unused_result__;

/** \brief Prototype for the virtual method serializer->toString */
typedef c_char *          (*sd_toStringFunc)(sd_serializer serializer, sd_serializedData serData) __nonnull_all__ __attribute_warn_unused_result__;

/** \brief Prototype for the virtual method serializer->fromString */
typedef sd_serializedData (*sd_fromStringFunc)(sd_serializer serializer, const c_char *str) __nonnull_all__ __attribute_warn_unused_result__;


/** \brief \b sd_validationInfo is a class encapsulating the data concerning
 *         validation results
 */
C_CLASS(sd_validationInfo);

/** \brief Declaration of the \b sd_validation info class */
C_STRUCT(sd_validationInfo) {
    /* Results from validation */
    c_ulong  errorNumber;
    c_char * message;
    c_char * location;
};

/** \brief Struct containing function pointers, like a virtual method table. */
struct sd_serializerVMT {
    sd_serialFunc       serialize;
    sd_deserialFunc     deserialize;
    sd_deserialIntoFunc deserializeInto;
    sd_toStringFunc     toString;
    sd_fromStringFunc   fromString;
};

/** \brief Declaration of the \b sd_serializer class */
C_STRUCT(sd_serializer) {
    c_ushort            formatID;
    c_ushort            formatVersion;
    c_base              base;
    c_type              type;
    /* Information needed for validating input
     * Note: this member makes the object an object with state */
    C_STRUCT(sd_validationInfo) validationInfo;
    /* Virtual functions */
    struct sd_serializerVMT VMT;
};

/* Private constructor, to be used only by descendants which do not
 * add any members */
 
sd_serializer
sd_serializerNew(
    c_ushort formatID,
    c_ushort formatVersion,
    c_base base,
    c_type type,
    struct sd_serializerVMT VMT) __nonnull((3)) __attribute_warn_unused_result__;

void
sd_serializerInitialize(
    sd_serializer _this,
    c_ushort formatID,
    c_ushort formatVersion,
    c_base base,
    c_type type,
    struct sd_serializerVMT VMT) __nonnull((1,4));
                               
void
sd_serializerResetValidationState(
    sd_serializer _this) __nonnull_all__;
                  
void
sd_serializerSetValidationInfo(
    sd_serializer _this,
    c_ulong errorNumber,
    c_char *message,
    c_char *location) __nonnull((1));

void
sd_serializerSetOutOfMemory(
    sd_serializer serializer) __nonnull_all__;

#endif  /* SD__SERIALIZER_H */
