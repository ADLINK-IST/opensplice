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
/** \file services/serialization/code/sd__resultCodesXMLMetadata.h
 *  \brief Macro definitions for XML deserialization validation error codes
 */

#ifndef SD__RESULTCODESXMLMETADATA_H
#define SD__RESULTCODESXMLMETADATA_H

#include "sd__resultCodesXML.h"

/* Internal error numbers which might occur for the XMLMetadata serializer */

#define SD_ERRNO_UNMATCHING_TYPE             200U
#define SD_MESSAGE_UNMATCHING_TYPE           "Types do not match"

#endif /* SD__RESULTCODESXMLMETADATA_H */
