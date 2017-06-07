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
#ifndef V_TYPEREPRESENTATION_H_
#define V_TYPEREPRESENTATION_H_

#include "kernelModule.h"

/**
 * \brief The <code>v_typeRepresentation</code> cast method.
 *
 * This method casts an object to a <code>v_typeRepresentation</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_typeRepresentation</code> or
 * one of its subclasses.
 */
#define v_typeRepresentation(o) (C_CAST(o,v_typeRepresentation))

v_typeRepresentation
v_typeRepresentationNew(
    v_participant participant,
    const os_char *typeName,
    v_dataRepresentationId_t dataRepresentationId,
    const v_typeHash typeHash,
    const os_uchar *metaData,
    os_uint32 metaDataLength,
    const os_uchar *extentions,
    os_uint32 extentionsLength);

#endif /* V_TYPEREPRESENTATION_H_ */
