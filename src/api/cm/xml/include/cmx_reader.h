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
/**@file api/cm/xml/include/cmx_reader.h
 * 
 * Represents a reader in the Splice kernel in XML format.
 */
#ifndef CMX_READER_H
#define CMX_READER_H

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
 * Resolves the data type of the data in the database of the supplied reader.
 * The resolved type actually is the type of the userData in the samples in the
 * database.
 * 
 * @param reader The XML representation of a reader.
 * @return The XML representation of the data type of the data in the database
 *         of the supplied reader.
 */
OS_API c_char* cmx_readerDataType  (const c_char* reader);

/**
 * Reads one sample from the database of the supplied reader. The format of the
 * XML depends on the type of the sample.
 * 
 * @param reader The XML representation of the reader to read from.
 * @return The XML representation of the read sample, or NULL if no sample was
 *         available.
 */
OS_API c_char* cmx_readerRead      (const c_char* reader);

/**
 * Takes one sample from the database of the supplied reader. The format of the
 * XML depends on the type of the sample.
 * 
 * @param reader The XML representation of the reader to take from.
 * @return The XML representation of the taken sample, or NULL if no sample was
 *         available.
 */
OS_API c_char* cmx_readerTake      (const c_char* reader);

/**
 * Reads the next instance from the database of the supplied reader. The format
 * of the XML depends on the type of the sample.
 * 
 * @param reader The XML representation of the reader to read data from.
 * @param localId The local id of the previous instance.
 * @param extId The extended id of the previous instance.
 * @return The XML representation of the read sample or NULL if no sample was
 *         available.
 */
OS_API c_char* cmx_readerReadNext  (const c_char* reader,
                                    const c_char* localId,
                                    const c_char* extId);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_READER*/
