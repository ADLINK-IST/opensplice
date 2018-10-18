/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#ifndef CMN_SAMPLESLIST_H
#define CMN_SAMPLESLIST_H

/** \file cmn_sampleList.h
 *  \brief The sampleList can be used by all language bindings to cache read samples.
 *
 * All language bindings need to cache read samples during the read. The copying of
 * these samples into the language bindings types is done after the read, out of the
 * user layers'/kernels' context, which makes the read more efficient.
 */

/** Supported methods:
 *
 *  cmn_samplesList_new();
 *  cmn_samplesList_free();
 *
 *  cmn_samplesList_reset();
 *  cmn_samplesList_insert();
 *  cmn_samplesList_finalize();
 *
 *  cmn_samplesList_read();
 *  cmn_samplesList_flush();
 *
 *  cmn_samplesList_full();
 *  cmn_samplesList_empty();
 *  cmn_samplesList_length();
 */


#include "v_kernel.h"
#include "u_user.h"
#include "vortex_os.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */


/*
 * The following defines have to have the same value as the
 * related DDS values within the language bindings.
 */
#define DDS_READ_SAMPLE_STATE                       1U
#define DDS_NOT_READ_SAMPLE_STATE                   2U

#define DDS_NEW_VIEW_STATE                          1U
#define DDS_NOT_NEW_VIEW_STATE                      2U

#define DDS_ALIVE_INSTANCE_STATE                    1U
#define DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE       2U
#define DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE     4U



/** \brief The cmn_samplesList cast method.
 *
 * In comparison with kernel classes this cast method will not perform
 * runtime type checking due to the lack of type information.
 */
#define cmn_samplesList(o) ((cmn_samplesList)(o))



/** \brief The cmn_samplesList module.
 *
 * This will be used to identify the individual sampleLists.
 */
C_CLASS(cmn_samplesList);

/** \brief The cmn_sampleInfo structure.
 *
 * This will contain information about the related stored read sample.
 * It will be used when cmn_samplesList_read() or cmn_samplesList_flush()
 * is used.
 */
C_CLASS(cmn_sampleInfo);
C_STRUCT(cmn_sampleInfo){
    os_uint32  sample_state;
    os_uint32  view_state;
    os_uint32  instance_state;
    os_int32   disposed_generation_count;
    os_int32   no_writers_generation_count;
    os_int32   sample_rank;
    os_int32   generation_rank;
    os_int32   absolute_generation_rank;
    os_timeW   source_timestamp;
    os_int64   instance_handle;
    os_int64   publication_handle;
    os_boolean valid_data;
    os_timeW   reception_timestamp;
};



/** \brief Callback function to copy samples into the language binding.
 *
 * When there are samples within the list, then this callback will be called
 * by cmn_samplesList_read() or cmn_samplesList_flush(). The language binding
 * can use this call to copy the provided sample into its' own types.
 *
 * \param _this    The sampleList.
 * \param sample   The sample data.
 * \param info     The sample info.
 * \param copy_arg The copy_arg provided with the read() or flush() call.
 *
 * \return         void
 */
typedef void
(*cmn_sampleList_copy_func)(
        void *sample,
        cmn_sampleInfo info,
        void *copy_arg);



/** \brief The class constructor.
 *
 * The constructor will create a sampleList.
 *
 * \param isView   Indication if the sampleList is related to a reader or
 *                 readerView.
 *
 * \return         The created sampleList on a succesful operation or NULL
 *                 if the operation failed.
 */
OS_API cmn_samplesList
cmn_samplesList_new (
    os_boolean isView);

/** \brief The class destructor.
 *
 * The constructor will destroy the given sampleList. It will remove all
 * samples from the list when there are some.
 *
 * \param _this    The sampleList to destroy.
 *
 * \return         void
 */
OS_API void
cmn_samplesList_free (
    cmn_samplesList _this);


/** \brief Prepare sampleList.
 *
 * This will remove possible previous samples and will prepare the sampleList
 * for new samples.
 *
 * \param _this       The sampleList.
 * \param max_samples The maximum of samples that the sampleList will take.
 *
 * \return             void
 */
OS_API void
cmn_samplesList_reset (
    cmn_samplesList _this,
    const os_int32 max_samples);

/** \brief Insert read sample into the sampleList.
 *
 * This will insert the given sample into the sampleList.
 *
 * \param _this     The sampleList.
 * \param sample    The read sample to insert.
 *
 * \return          void
 */
OS_API os_boolean
cmn_samplesList_insert(
    cmn_samplesList _this,
    v_readerSample sample);

/** \brief Finalize the sampleList.
 *
 * Before the inserted samples can be properly extracted by means of the read()
 * and flush() functions, they should be finalized to add the proper information.
 * This function will do that for every sample within the list.
 *
 * \param _this     The sampleList.
 *
 * \return          void
 */
OS_API void
cmn_samplesList_finalize (
    cmn_samplesList _this);


/** \brief Read a single sample.
 *
 * Read a single sample from the sampleList on position 'index'. The related
 * sample will be provided by a callback to 'copy_action()'.
 *
 * \param _this       The sampleList.
 * \param index       Position of the wanted sample within the list.
 * \param copy_action The copy callback function.
 * \param copy_arg    The argument passed along with the copy_action call.
 *
 * \return os_int32   The number of read samples (0 or 1).
 */
OS_API os_int32
cmn_samplesList_read(
    cmn_samplesList _this,
    os_uint32 index,
    cmn_sampleList_copy_func copy_action,
    void *copy_arg);

/** \brief Read and remove all single samples.
 *
 * Read all data from the sampleList. The related samples will be provided
 * by a callback to 'copy_action()'. The samples will be removed from the list.
 *
 * \param _this       The sampleList.
 * \param copy_action The copy callback function.
 * \param copy_arg    The argument passed along with the copy_action calls.
 *
 * \return os_int32   The number of read samples.
 */
OS_API os_int32
cmn_samplesList_flush(
    cmn_samplesList _this,
    cmn_sampleList_copy_func copy_action,
    void *copy_arg);

/** \brief Read and remove all single samples.
 *
 * Read all samples from the sampleList. The related samples will be provided
 * by a callback to 'copy_action()'. The samples will be removed from the list.
 *
 * \param _this       The sampleList.
 * \param copy_action The copy callback function.
 * \param copy_arg    The argument passed along with the copy_action calls.
 *
 * \return os_int32   The number of read samples.
 */
OS_API os_int32
cmn_samplesList_flush2(
    cmn_samplesList _this,
    cmn_sampleList_copy_func copy_action,
    void *copy_arg);

/** \brief Check if the list is full.
 *
 * Check if the given sampleList is full.
 *
 * \param _this       The sampleList.
 *
 * \return os_boolean TRUE = full, FALSE = not full.
 */
OS_API os_boolean
cmn_samplesList_full (
    cmn_samplesList _this);

/** \brief Check if the list is empty.
 *
 * Check if the given sampleList is empty.
 *
 * \param _this       The sampleList.
 *
 * \return os_boolean TRUE = empty, FALSE = not empty.
 */
OS_API os_boolean
cmn_samplesList_empty (
    cmn_samplesList _this);

/** \brief Return the list length.
 *
 * Return the length (aka the number of inserted samples) of the
 * given sampleList.
 *
 * \param _this       The sampleList.
 *
 * \return os_uint32  The list length (nr of inserted samples).
 */
OS_API os_uint32
cmn_samplesList_length(
    cmn_samplesList _this);



#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMN_SAMPLESLIST_H */
