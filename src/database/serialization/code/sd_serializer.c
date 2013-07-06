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
/** \file services/serialization/code/sd_serializer.c
 *  \brief Implementation of the \b serializedData class
 *         and the abstract \b serializer class.
 *
 *  \b serializedData is a class encapsulating serialized data.
 *  It is a protected class used by sd_serializer and its descendants
 *  only.
 *
 *  \b serializer is the abstract baseclass for all serializer classes.
 *  Its functions are protected or virtual. The protected methods are to
 *  be called by the descendants. The virtual methods are part of the
 *  interface.
 *
 *  Since both \b serializedData and \b serializer contain protected parts,
 *  their prototypes are declared in the protected headerfile sd__serializer.h.
 */

/* interface */
#include "sd_serializer.h"
#include "sd__serializer.h"

/* implementation */
#include "os.h"
#include "c_base.h"
#include "sd__confidence.h"
#include "sd__resultCodes.h"

/* -------------------------------- serializedData ------------------------- */

/* ------------------------------ protected functions ---------------------- */

/** \brief Macro for calculating the header size of some serialized data
 *
 *  Serialized data consists of a header and the data itself. The header
 *  contains version information and the size of the data. This macro
 *  calculates the size of the header in bytes.
 */

#define SD_HEADER_SIZE ((os_uint32)sizeof(struct sd_serializedData_s) - 1U)

/** \brief Protected constructor for serialized data.
 *
 *  This constructor is to be called by any serializer. The serializers
 *  know the ID and the version of the format they produce, as well as the
 *  size of the data.
 *
 *  \param formatID The ID of the serialization method used. This will be
 *                  translated into a big endian in the header of the
 *                  serialized data.
 *  \param formatVersion The version number of the serialization used. This
 *                  will be translated into big endian in the header of the 
 *                  serialized data.
 *  \param dataSize The size of the data when serialized. This amount of bytes
 *                  will be allocated by the constructor.
 *  \return The newly created serializedData object, to be released using
 *          \b serializedDataFree.
 */

sd_serializedData
sd_serializedDataNew(
    c_ushort formatID,
    c_ushort formatVersion,
    c_ulong dataSize)
{
    sd_serializedData result;
    c_ulong calcSize = dataSize;

    result = (sd_serializedData)os_malloc(dataSize + SD_HEADER_SIZE);

    if (result) {
       result->formatID[0] = (c_octet)((c_ulong)formatID >> 8);
       result->formatID[1] = (c_octet)((c_ulong)formatID & 0xFFU);
       result->formatVersion[0] = (c_octet)((c_ulong)formatVersion >> 8);
       result->formatVersion[1] = (c_octet)((c_ulong)formatVersion & 0xFFU);
       result->dataSize[3] = (c_octet)((c_ulong)calcSize & 0xFFU);
       calcSize >>= 8;
       result->dataSize[2] = (c_octet)(calcSize & 0xFFU);
       calcSize >>= 8;
       result->dataSize[1] = (c_octet)(calcSize & 0xFFU);
       calcSize >>= 8;
       result->dataSize[0] = (c_octet)(calcSize & 0xFFU);
    }

    return result;
}

/** \brief Get the ID of the serialization method used to serialize the data.
 *
 *  Every serializer is supposed to have its own serialization ID. This ID can
 *  be used in order to check the format of the serialized data. This avoids
 *  data being deserialized by an incompatible serializer. The combination of
 *  the formatID and version number uniquely identifies the serialization
 *  format.
 *
 *  This function is supposed to be used by serializer objects to do
 *  format checking.
 *
 *  \param serData The serializedData object (self).
 *  \return The formatID of the serializer used to serialize the data.
 */

c_ushort
sd_serializedDataGetFormatID(
    sd_serializedData serData)
{
    c_ushort result;

    result = serData->formatID[0];
    result = (c_ushort)((int)result << 8);
    result =  (c_ushort)((int)result + (int)serData->formatID[1]);

    return result;
}


/** \brief Get the version number of the serialization method used to
 *         serialize the data.
 *
 *  In order to support different versions of a serialization method, every
 *  serializer has to provide a version number. The combination of
 *  the formatID and version number uniquely identifies the serialization
 *  format.
 *
 *  This function is supposed to be used by serializer objects to do
 *  format checking.
 *
 *  \param serData The serializedData object (self).
 *  \return The version number of the serializer used to serialize the data.
 */

c_ushort
sd_serializedDataGetFormatVersion(
    sd_serializedData serData)
{
    c_ushort result;

    result = serData->formatVersion[0];
    result = (c_ushort)((int)result << 8);
    result =  (c_ushort)((int)result + (int)serData->formatVersion[1]);

    return result;
}

/** \brief Get the size of the serialized data in bytes
 *
 *  The size of the serialized data is stored in the header of the
 *  serializedData object. This protected function extracts this size.
 *
 *  \param serData The serializedData object (self).
 *  \return The size of the serialized data in bytes.
 */

c_ulong
sd_serializedDataGetDataSize(
    sd_serializedData serData)
{
    c_ulong result;

    result = serData->dataSize[0];
    result <<= 8;
    result += (c_ulong)serData->dataSize[1];
    result <<= 8;
    result += (c_ulong)serData->dataSize[2];
    result <<= 8;
    result += (c_ulong)serData->dataSize[3];

    return result;
}


/* -------------------------- public functions -------------------------- */

/** \brief Get the total number of bytes allocated for a serializedData object
 *
 *  A serializedData object can have any size. This function returns the
 *  size, this is also the number of bytes allocated for this object.
 *
 *  \param serData The serializedData object (self).
 *  \return The size of the complete serializedData object in bytes.
 */

c_ulong
sd_serializedDataGetTotalSize(
    sd_serializedData serData)
{
    return (sd_serializedDataGetDataSize(serData) + SD_HEADER_SIZE);
}

#undef SD_HEADER_SIZE


/** \brief Free the memory in use for a serializedData object.
 *
 *  \param serData The serializedData object (self).
 */

void
sd_serializedDataFree(
    sd_serializedData serData)
{
    os_free(serData);
}


/* ----------------------------- validation information -------------------- */

/* The validation class contains validation information about deserialization
 * performed */

/** \brief Private constructor for the validation class */
static sd_validationInfo
sd_validationInfoNew(
    void)
{
    sd_validationInfo result;
    
    result = (sd_validationInfo)os_malloc((os_uint32)sizeof(*result));
    result->message = NULL;
    
    return result;
}

/** \brief Private function for clearing the validation state */
static void
sd_validationInfoReset(
    sd_validationInfo validationInfo)
{
    if (validationInfo) {
        validationInfo->errorNumber = SD_SUCCESS;
        if (validationInfo->message) {
            os_free(validationInfo->message);
            validationInfo->message = NULL;
        }
        validationInfo->location = NULL;
    }
}

/** \brief Private destructor for freeing a validation object */
static void
sd_validationInfoFree(
    sd_validationInfo validationInfo)
{
    if (validationInfo) {
        if (validationInfo->message) {
            os_free(validationInfo->message);
        }
        if (validationInfo->location) {
            os_free(validationInfo->location);
        }

        os_free(validationInfo);
    }
}    

/* --------------------------------- serializer  --------------------------- */


/* ------------------------------- private functions ----------------------- */

#ifndef NDEBUG

/** \brief Check both the format ID and version of the serialized data.
 *
 *  Before deserializing data, the serializer is supposed to check for
 *  compatibility of the serialized data. This routine offers such a check.
 *
 *  \param serializer The serializer object (self).
 *  \param serData The serializedData object to be checked.
 *  \return TRUE if the format ID and version of the serializer correspond
 *          to those of the serializedData; FALSE otherwise.
 */

static c_bool
sd_serializerCheckDataFormat(
    sd_serializer serializer,
    sd_serializedData serData)
{
    c_bool idOK, versionOK;
    c_bool result;
    
    idOK = (c_bool)((int)serializer->formatID == (int)sd_serializedDataGetFormatID(serData));
    versionOK = (c_bool)((int)serializer->formatVersion == (int)sd_serializedDataGetFormatVersion(serData));
    result = idOK && versionOK;
    
    return result;
}

/** \brief Check if an object resides in the correct database for serializing.
 *
 *  At creation, the serializer is assigned a database to operate on. Only
 *  objects residing in this database are to be serialized. This routine
 *  offers a check for this condition.
 *
 *  \param serializer The serializer object (self).
 *  \param object The object to be checked for its database.
 *  \return TRUE if the object resides in the database expected by the
 *          serializer; FALSE otherwise.
 */

static c_bool
sd_serializerCheckBase(
    sd_serializer serializer,
    c_object object)
{
    /* Can not check null references... */
    return ((object == NULL) || (serializer->base == c_getBase(object)));
}

#endif

/* ----------------------------- protected functions ----------------------- */


/** \brief Create a new serializer and initialize its private classes.
 *
 *  \b sd_serializer is an abstract baseclass for several serializer classes.
 *  Its descendants should call this baseclass constructor before initializing
 *  their own members. Therfore, it is protected.
 *
 *  \param formatID Each serializer identifies the format of the data it
 *                  produces by an ID. This ID identifies the serializer type
 *                  and can be used for compatibility checking.
 *  \param formatVersion Each serializer might support different versions of
 *                  the same format. This version identifies the format
 *                  number and can be used for version checking.
 *  \param base Each serializer is bound to a database at creation time. This
 *                  allows database-specific optimizations. Only objects from
 *                  this database can be deserialized and serialization is
 *                  done into this database.
 *  \param type Pointer to the type this serializer will be able to handle. Use
 *                  NULL for any type.
 *  \return The newly created serializer baseclass, to be released using
 *          \b serializerFree.
 */

sd_serializer
sd_serializerNew(
    c_ushort formatID,
    c_ushort formatVersion,
    c_base base,
    c_type type,
    struct sd_serializerVMT VMT)
{
    sd_serializer result;

    SD_CONFIDENCE((type == NULL) || (base == c_getBase(type)));

    result = (sd_serializer)os_malloc((os_uint32)sizeof(*result));

    if (result) {
        sd_serializerInitialize(result, formatID, formatVersion,
            base, type, VMT);
    }

    return result;
}


void
sd_serializerInitialize(
    sd_serializer serializer,
    c_ushort formatID,
    c_ushort formatVersion,
    c_base base,
    c_type type,
    struct sd_serializerVMT VMT)
{
     if (serializer) {
        serializer->formatID = formatID;
        serializer->formatVersion = formatVersion;
        serializer->base = base;
        serializer->type = type;
        serializer->validationInfo = NULL;
        serializer->VMT = VMT;
     }
}    
                               

/** \brief Protected function used by \b sd_serializer descendants for
 *  indicating if they want to do validation during deserialization
 *
 *  Some descendants of \b sd_serializer implement algorithms for checking
 *  deserialization input. In order to support this validation, \b sd_serializer
 *  stores some internal information. This protected function offers the
 *  possibility for descendants to indicate that they indeed want to use the
 *  internal information for validation.
 * 
 *  \param  serializer   The serializer object (self)
 *  \param  doValidation Boolean value indicating whether validation information
 *                       is to be maintained or not
 */
void
sd_serializerSetValidationState(
    sd_serializer serializer,
    c_bool doValidation)
{
    if (doValidation) {
        if (!serializer->validationInfo) {
            serializer->validationInfo = sd_validationInfoNew();
        }
        sd_validationInfoReset(serializer->validationInfo);
    } else {
        sd_validationInfoFree(serializer->validationInfo);
    }
}    

/** \brief Protected function used by \b sd_serializer descendants for setting
 *  validation information in case of an error.
 *
 *  Some descendants of \b sd_serializer implement algorithms for checking
 *  deserialization input. This protected function is a setter interface to the
 *  encapsulated validation state. The error numbers used are supposed to be
 *  unique over all serializing classes and are to be interpreted by the class
 *  itself only.
 * 
 *  \param  serializer  The serializer object (self)
 *  \param  errorNumber Numeric representation of the error encountered
 *  \param  message     Readable message explaining the error encountered
 *  \param  location    Pointer into the datastring indicating where the error
 *                      was encountered
 */
void
sd_serializerSetValidationInfo(
    sd_serializer serializer,
    c_ulong errorNumber,
    c_char *message,
    c_char *location)
{
    SD_CONFIDENCE(serializer);
    SD_CONFIDENCE(serializer->validationInfo);
    
    if (serializer && serializer->validationInfo) {
        serializer->validationInfo->errorNumber = errorNumber;
        serializer->validationInfo->message = message;
        serializer->validationInfo->location = location;
    }
}

/* ------------------ public (de)serialization functions ----------------------- */

/** \brief Serialize a database object
 *
 *  By default, this virtual function returns NULL. In general, descending
 *  classes will implement it.
 *
 *  Before serialization, the database the object resides in is checked.
 *  This has to correspond with the serializer database.
 *
 *  \param serializer The serializer object (self).
 *  \param object The object to be serialized
 *  \return A serializedData instance containing a serialized version
 *          of object. To be released using \b serializedDataFree.
 */

sd_serializedData
sd_serializerSerialize(
    sd_serializer serializer,
    c_object object)
{
    sd_serializedData result = NULL;
    
    SD_CONFIDENCE(sd_serializerCheckBase(serializer, object));
    
    if (serializer->VMT.serialize) {
        result = serializer->VMT.serialize(serializer, object);
    }
    
    return result;
}

/** \brief Deserialize into newly created database object
 *
 *  By default, this virtual function returns NULL. In general, descending
 *  classes will implement it.
 *
 *  Before deserialization, the serialized data is checked for format ID
 *  and version. This has to correspond with the serializer format ID and
 *  version.
 *
 *  \param serializer The serializer object (self).
 *  \param serData The serialized data to be deserialized
 *  \return A object resulting from the deserialization. This object
 *          resides in the database as given at construction of the
 *          serializer. To be released using \b c_free.
 */

c_object
sd_serializerDeserialize(
   sd_serializer serializer,
   sd_serializedData serData)
{
    c_object result = NULL;
    
    SD_CONFIDENCE(sd_serializerCheckDataFormat(serializer, serData));
    
    if (serializer->VMT.deserialize) {
        result = serializer->VMT.deserialize(serializer, serData, FALSE);
    }
    
    return result;
}

/** \brief Deserialize into newly created database object, validating the input
 *  during the deserialization process.
 *
 *  By default, this virtual function returns NULL. In general, descending
 *  classes will implement it.
 *
 *  Before deserialization, the serialized data is checked for format ID
 *  and version. This has to correspond with the serializer format ID and
 *  version.
 * 
 *  Use the serializer's lastValidationResult function in order to check on
 *  encountered errors.
 *
 *  \param serializer The serializer object (self).
 *  \param serData The serialized data to be deserialized
 *  \return An object resulting from the deserialization. This object
 *          resides in the database as given at construction of the
 *          serializer. To be released using \b c_free.
 */

c_object
sd_serializerDeserializeValidated(
   sd_serializer serializer,
   sd_serializedData serData)
{
    c_object result = NULL;
    
    SD_CONFIDENCE(sd_serializerCheckDataFormat(serializer, serData));
    
    if (serializer->VMT.deserialize) {
        result = serializer->VMT.deserialize(serializer, serData, TRUE);
    }
    
    return result;
}

/** \brief Deserialize into an existing database object.
 *
 *  By default, this virtual function does nothing. In general, descending
 *  classes will implement it.
 *
 *  Before deserialization, the serialized data is checked for format ID
 *  and version. This has to correspond with the serializer format ID and
 *  version.
 * 
 *  This is function serves the special requirement for processes which have
 *  to serialize into an already created object. Note that this is possible
 *  only for types which have a fixed base size. For example, stretchy arrays
 *  are not allowed, but structs containing a stretchy array are.
 *
 *  \param serializer The serializer object (self).
 *  \param serData The serialized data to be deserialized
 *  \param object The object to deserialize the data into.
 */

void
sd_serializerDeserializeInto(
    sd_serializer serializer,
    sd_serializedData serData,
    c_object object)
{
    SD_CONFIDENCE(sd_serializerCheckDataFormat(serializer, serData));
    
    if (serializer->VMT.deserializeInto) {
        serializer->VMT.deserializeInto(serializer, serData, object, FALSE);
    }
}
    
/** \brief Deserialize into an existing database object, validating the input
 *  during the deserialization process.
 *
 *  By default, this virtual function does nothing. In general, descending
 *  classes will implement it.
 *
 *  Before deserialization, the serialized data is checked for format ID
 *  and version. This has to correspond with the serializer format ID and
 *  version.
 * 
 *  Use the serializer's lastValidationResult function in order to check on
 *  encountered errors.
 *
 *  This is function serves the special requirement for processes which have
 *  to serialize into an already created object. Note that this is possible
 *  only for types which have a fixed base size. For example, stretchy arrays
 *  are not allowed, but structs containing a stretchy array are.
 *
 *  \param serializer The serializer object (self).
 *  \param serData The serialized data to be deserialized
 *  \param object The object to deserialize the data into.
 */
void
sd_serializerDeserializeIntoValidated(
    sd_serializer serializer,
    sd_serializedData serData,
    c_object object)
{
    SD_CONFIDENCE(sd_serializerCheckDataFormat(serializer, serData));
    
    if (serializer->VMT.deserializeInto) {
        serializer->VMT.deserializeInto(serializer, serData, object, TRUE);
    }
}    

/** \brief Convert serialized data to a string representation.
 *
 *  By default, this virtual function returns NULL. In general, descending
 *  classes will implement it.
 *
 *  Before conversion, the serialized data is checked for format ID
 *  and version. This has to correspond with the serializer format ID and
 *  version.
 *
 *  \param serializer The serializer object (self).
 *  \param serData The serialized data to be converted.
 *  \return A string representation of the serialized data.
 *          Use \b os_free to release the result.
 */

c_char *
sd_serializerToString(
    sd_serializer serializer,
    sd_serializedData serData)
{
    c_char *result = NULL;
    
    SD_CONFIDENCE(sd_serializerCheckDataFormat(serializer, serData));
    
    if (serializer->VMT.toString) {
        result =  serializer->VMT.toString(serializer, serData);
    }
    
    return result;
}

/** \brief Create serialized data from a string representation.
 *
 *  By default, this virtual function returns NULL. In general, descending
 *  classes will implement it.
 *
 *  \param serializer The serializer object (self).
 *  \param str The string representation of the serialized data.
 *  \return A serialized data object as represented by str, to be
 *          released using \b serializedDataFree.
 */

sd_serializedData
sd_serializerFromString(
    sd_serializer serializer,
    const c_char *str)
{
    sd_serializedData result = NULL;
    
    if (serializer->VMT.fromString) {
        result = serializer->VMT.fromString(serializer, str);
    }
    
    return result;
}


/** \brief Free the memory in use for a serializer object.
 *
 *  \param serializer The serializer object (self).
 */

void
sd_serializerFree(
    sd_serializer serializer)
{
    if (serializer) {
        sd_validationInfoFree(serializer->validationInfo);
        os_free(serializer);
    }
}

/* --------------- public validation-related functions ----------------------- */


/** \brief Query the result of the last executed deserialization
 *
 *  Some \b sd_serializer descendants implement validation of the data they
 *  deserialize. The result of this validation can be queried using this
 *  function.
 *
 *  \param serializer The serializer object (self).
 */
sd_validationResult
sd_serializerLastValidationResult(
    sd_serializer serializer)
{
    sd_validationResult result = SD_VAL_NOT_DONE;
    
    if (serializer && serializer->validationInfo) {
        if (serializer->validationInfo->errorNumber == SD_SUCCESS) {
           result = SD_VAL_SUCCESS;
        } else {
           result = SD_VAL_ERROR;
        }
    }
    
    return result;
}
   
    
/** \brief Query the error message generated by the last executed
 *         deserialization
 *
 *  Some \b sd_serializer descendants implement validation of the data they
 *  deserialize. Any error message resulting from this can be queried using this
 *  function.
 *
 *  \param serializer The serializer object (self).
 */
c_char *
sd_serializerLastValidationMessage(
    sd_serializer serializer)
{
    c_char *result = NULL;
    
    if (serializer && serializer->validationInfo) {
         result = serializer->validationInfo->message;
    }
    
    return result;
}
   
    
/** \brief Query a character pointer to the first error encountered during
 *         deserialization.
 *
 *  Some \b sd_serializer descendants implement validation of the data they
 *  deserialize. The location of the first error encountered in the
 *  deserialization stream can be queried using this function.
 *
 *  \param serializer The serializer object (self).
 */
c_char *
sd_serializerLastValidationLocation(
    sd_serializer serializer)
{
    c_char *result = NULL;
    
    if (serializer && serializer->validationInfo) {
         result = serializer->validationInfo->location;
    }
    
    return result;
}
