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
/**
 * @file api/cm/xml/include/cmx_readerSnapshot.h
 * 
 * Represents a snapshot of a reader database in XML format.
 */
#ifndef CMX_READERSNAPSHOT_H
#define CMX_READERSNAPSHOT_H

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
 * Makes a snapshot of the database of the supplied reader by serializing the
 * complete database to XML format and storing it in heap memory.
 * 
 * An XML reader snapshot looks like:
   @verbatim
   <readerSnapshot>
        <id>...</id>
   </readerSnapshot>
   @endverbatim
 * The id represents the heap address of the snapshot.
 * 
 * @param reader The XML representation of the reader, which database needs to 
 *               be serialized.
 * @return The XML representation of the reader snapshot.
 */
OS_API c_char* cmx_readerSnapshotNew       (const c_char* reader);

/**
 * Frees the supplied snapshot.
 * 
 * @param snapshot The snapshot to free.
 */
OS_API void    cmx_readerSnapshotFree      (c_char* snapshot);

/**
 * Reads one sample from the supplied snapshot. The result format depends on the
 * data type of the samples in the snapshot.
 * 
 * @param snapshot The snapshot to read a sample from.
 * @return The XML representation of the read sample or NULL if no sample was
 *         available.
 */
OS_API c_char* cmx_readerSnapshotRead      (const c_char* snapshot);

/**
 * Takes one sample from the supplied snapshot. The result format depends on the
 * data type of the samples in the snapshot.
 * 
 * @param snapshot The snapshot to take a sample from.
 * @return The XML representation of the taken sample or NULL if no sample was
 *         available.
 */
OS_API c_char* cmx_readerSnapshotTake      (const c_char* snapshot);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_READERSNAPSHOT*/
