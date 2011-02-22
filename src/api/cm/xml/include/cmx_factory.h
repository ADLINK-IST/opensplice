/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/**@file api/cm/xml/include/cmx_factory.h
 * @brief Offers facilities to initialise and detach the XML Control and 
 *        Monitoring API. 
 */
#ifndef CMX_FACTORY_H
#define CMX_FACTORY_H

#include "c_typebase.h"

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
OS_API c_bool          cmx_isInitialized(void);

/**
 * Internal routine that deregisters and frees all registered entities. This
 * implicates that entities which are still referenced by the user of the API
 * are not valid after calling this routine.
 */
OS_API void            cmx_deregisterAllEntities(void);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_FACTORY_H */
