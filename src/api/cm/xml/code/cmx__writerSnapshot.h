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
/**@file api/cm/xml/code/cmx__writerSnapshot.h
 *
 * Offers internal routines on a writer snapshot.
 */
#ifndef CMX__WRITERSNAPSHOT_H
#define CMX__WRITERSNAPSHOT_H

#include "c_typebase.h"
#include "c_iterator.h"
#include "c_collection.h"
#include "v_kernel.h"
#include "sd_serializer.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_writerSnapshot.h"

C_CLASS(cmx_writerSnapshot);

C_STRUCT(cmx_writerSnapshot){
    c_iter samples;
};

struct cmx_writerSnapshotArg{
    cmx_writerSnapshot snapshot;
    c_bool success;
    sd_serializer serializer;
};

#define cmx_writerSnapshot(s) ((cmx_writerSnapshot)s)

/**
 * Action routine to create a snapshot of writer history.
 *
 * @param e The writer, which history must be copied.
 * @param args Must be of type struct cmx_writerSnapshotArg. This will be filled
 *             with the snapshot during the execution of this function.
 */
void                cmx_writerSnapshotNewAction (v_public p, c_voidp args);

/**
 * Frees all snapshots of writer history.
 */
void                cmx_writerSnapshotFreeAll   ();

/**
 * Resolves a writer snapshot according to its XML representation.
 *
 * @return The resolved snapshot if available, NULL otherwise.
 */
cmx_writerSnapshot  cmx_writerSnapshotLookup    (const c_char* snapshot);

/**
 * Copy routine for copying the history of a writer.
 *
 * @param sample The sample to copy.
 * @param args Must be of type struct cmx_writerSnapshotArg. It will be filled
 *             with the XML representation of samples in the history during
 *             the execution of this function.
 */
c_bool              cmx_writerHistoryCopy       (c_object sample,
                                                 c_voidp args);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__WRITERSNAPSHOT_H */
