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
/**@file api/cm/xml/include/cmx_storage.h
 * @brief Represents a storage. It offers facilities to read/append data from/
 * to a storage.
 */
#ifndef CMX_STORAGE_H
#define CMX_STORAGE_H

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

OS_API c_char* cmx_storageOpen (const c_char* attrs);
OS_API c_char* cmx_storageClose (const c_char* storage);
OS_API c_char* cmx_storageAppend (const c_char* storage, const c_char* metadata, const c_char* data);
OS_API c_char* cmx_storageRead (const c_char* storage);
OS_API c_char* cmx_storageGetType (const c_char* storage, const c_char* typeName);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_STORAGE_H */
