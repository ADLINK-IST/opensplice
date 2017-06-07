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
/**@file api/cm/xml/include/cmx_factory.h
 * @brief Offers facilities to initialise and detach the XML Control and
 *        Monitoring API.
 */
#ifndef CMX_FACTORY_H
#define CMX_FACTORY_H

#include "c_typebase.h"
#include "u_entity.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CMXML
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * @brief Initialises the XML C&M API.
 *
 * This is realized by initialising the user layer. This function is NOT
 * threadsafe.
 *
 * @return Whether or not the initialisation succeeded. When succeeded:
 * @verbatim<result>OK</result>@endverbatim is returned,
 * @verbatim<result>FAILED</result>@endverbatim otherwise.
 */
OS_API const c_char*   cmx_initialise(void);

/**
 * @brief Detaches the XML C&M API.
 *
 * This is realized by detaching the user layer. This function is NOT
 * threadsafe.
 *
 * @return Whether or not the initialisation succeeded. When succeeded:
 * @verbatim<result>OK</result>@endverbatim is returned,
 * @verbatim<result>FAILED</result>@endverbatim otherwise.
 */
OS_API const c_char*   cmx_detach();

/**
 * @brief Checks whether the C&M XML API is currently initialized.
 *
 * @return TRUE if initialized, FALSE otherwise.
 */
OS_API os_boolean      cmx_isInitialized(void);

/**
 * Internal routine that deregisters and frees all registered entities. This
 * implicates that entities which are still referenced by the user of the API
 * are not valid after calling this routine.
 */
OS_API void            cmx_deregisterAllEntities(void);
/**
 * @brief return the current version of the CM API.
 *
 * @return the version of the CM API:
 */
OS_API  c_char*        cmx_getVersion();

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_FACTORY_H */
