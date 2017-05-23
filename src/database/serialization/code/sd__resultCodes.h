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
/** \file services/serialization/code/sd__resultCodes.h
 *  \brief Generic macro definitions used for deserialization validation
 */


#ifndef SD__RESULTCODES_H
#define SD__RESULTCODES_H

/* Macros for checking error codes etc */

#define SD_ERRNO(errorType)   SD_ERRNO_##errorType
#define SD_MESSAGE(errorType) SD_MESSAGE_##errorType

#define SD_VALIDATION_SET_ERROR(errorInfo, errorType, name, location)          \
    *errorInfo = sd_errorInfoNew(SD_ERRNO(errorType), name,                    \
                                 SD_MESSAGE(errorType), location)

#define SD_VALIDATION_ERROR(errorInfo)                                         \
    (*(errorInfo) != NULL)
#define SD_VALIDATION_RETURN_ON_ERROR(errorInfo)                               \
    if (SD_VALIDATION_ERROR(errorInfo)) {                                      \
        return FALSE;                                                          \
    }
#define SD_VALIDATION_ERROR_SET_NAME(errorInfo,name)                           \
    if (SD_VALIDATION_ERROR(errorInfo)) {                                      \
        if (!sd_errorInfoGetName(*errorInfo)) {                                \
            sd_errorInfoSetName(*errorInfo, name);                             \
        }                                                                      \
    }

/* Internal error numbers which might occur for all serializers */

#define SD_SUCCESS                      0U

#define SD_ERRNO_INVALID_SWITCHVALUE    1U
#define SD_MESSAGE_INVALID_SWITCHVALUE  "Encountered switchvalue with no "     \
                                        "corresponding case label"

#define SD_ERRNO_OUT_OF_MEMORY          2U
#define SD_MESSAGE_OUT_OF_MEMORY        "Out of memory"

#define SD_ERRNO_ERROR                  3U
#define SD_MESSAGE_ERROR                "Unspecified error"


#endif /* SD__RESULTCODES_H */
