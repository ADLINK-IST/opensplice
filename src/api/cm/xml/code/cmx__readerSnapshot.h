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
void cmx_readerSnapshotNewAction(v_public p, c_voidp args);

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
