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
/**@file api/cm/xml/code/cmx__snapshot.h
 * 
 * Offers internal routines on a snapshot.
 */
#ifndef CMX__SNAPSHOT_H
#define CMX__SNAPSHOT_H

#include "c_typebase.h"
#include "c_iterator.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_snapshot.h"

/**
 * Frees all current reader and writer snapshots.
 */
void cmx_snapshotFreeAll();

/**
 * Resolves the snapshot kind.
 * 
 * @param snapshot The snapshot where to resolve the kind of.
 * @return READERSNAPSHOT if it is a readerSnapshot or WRITERSNAPSHOT if it is
 *         a writerSnapshot.
 */
const c_char* cmx_snapshotKind(const c_char* snapshot);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__SNAPSHOT_H */
