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
/**@file api/cm/xml/code/cmx__readerSnapshot.h
 * 
 * Offers internal routines on a reader snapshot.
 */
#ifndef CMX__READERSNAPSHOT_H
#define CMX__READERSNAPSHOT_H

#include "c_typebase.h"
#include "c_iterator.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_readerSnapshot.h"

C_CLASS(cmx_readerSnapshot);

C_STRUCT(cmx_readerSnapshot){
    c_iter samples;
};

struct cmx_readerSnapshotArg{
    cmx_readerSnapshot snapshot;
    c_bool success;
};

#define cmx_readerSnapshot(s) ((cmx_readerSnapshot)s)

/**
 * Action routine to create a snapshot of a reader database.
 * 
 * @param e The reader, which database must be copied.
 * @param args Must be of type struct cmx_readerSnapshotArg. This will be filled
 *             with the snapshot during the execution of this function.
 */
void cmx_readerSnapshotNewAction(v_entity e, c_voidp args);

/**
 * Frees all snapshots of reader databases.
 */
void cmx_readerSnapshotFreeAll();

/**
 * Resolves a reader snapshot according to its XML representation.
 * 
 * @return The resolved snapshot if available, NULL otherwise.
 */
cmx_readerSnapshot cmx_readerSnapshotLookup(const c_char* snapshot);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__READERSNAPSHOT_H */
