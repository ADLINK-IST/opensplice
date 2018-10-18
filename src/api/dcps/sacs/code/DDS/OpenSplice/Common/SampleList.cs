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
 */

using System;
using System.Runtime.InteropServices;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice.Common
{
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
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void sampleListCopyFn(IntPtr sample, IntPtr info, IntPtr copyArg);

    /** \brief The cmn_samplesList module.
     *
     * This will be used to identify the individual sampleLists.
     */
    [StructLayout(LayoutKind.Sequential)]
    public class SampleList
    {
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
        [DllImport("ddskernel", EntryPoint = "cmn_samplesList_new", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New (int isView);

        /** \brief The class destructor.
         *
         * The constructor will destroy the given sampleList. It will remove all
         * samples from the list when there are some.
         *
         * \param _this    The sampleList to destroy.
         *
         * \return         void
         */
        [DllImport("ddskernel", EntryPoint = "cmn_samplesList_free", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Free (IntPtr _this);

        /** \brief Prepare sampleList.
         *
         * This will remove possible previous samples and will prepare the sampleList
         * for new samples.
         *
         * \param _this       The sampleList.
         * \param maxSamples  The maximum of samples that the sampleList will take.
         *
         * \return             void
         */
        [DllImport("ddskernel", EntryPoint = "cmn_samplesList_reset", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Reset(IntPtr _this, int maxSamples);

        /** \brief Insert read sample into the sampleList.
         *
         * This will insert the given sample into the sampleList.
         *
         * \param _this     The sampleList.
         * \param sample    The read sample to insert.
         *
         * \return          void
         */
        [DllImport("ddskernel", EntryPoint = "cmn_samplesList_insert", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Insert(IntPtr _this, IntPtr sample);

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
        [DllImport("ddskernel", EntryPoint = "cmn_samplesList_finalize", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Finalize(IntPtr _this);

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
        [DllImport("ddskernel", EntryPoint = "cmn_samplesList_read", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Read(IntPtr _this, uint index, sampleListCopyFn copyAction, IntPtr copyArg);

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
        [DllImport("ddskernel", EntryPoint = "cmn_samplesList_flush", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Flush(IntPtr _this, sampleListCopyFn copy_action, IntPtr copyArg);

        /** \brief Check if the list is full.
         *
         * Check if the given sampleList is full.
         *
         * \param _this       The sampleList.
         *
         * \return os_boolean TRUE = full, FALSE = not full.
         */
        [DllImport("ddskernel", EntryPoint = "cmn_samplesList_full", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Full(IntPtr _this);

        /** \brief Check if the list is empty.
         *
         * Check if the given sampleList is empty.
         *
         * \param _this       The sampleList.
         *
         * \return os_boolean TRUE = empty, FALSE = not empty.
         */
        [DllImport("ddskernel", EntryPoint = "cmn_samplesList_empty", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Empty(IntPtr _this);

        /** \brief Return the list length.
         *
         * Return the length (aka the number of inserted samples) of the
         * given sampleList.
         *
         * \param _this       The sampleList.
         *
         * \return os_uint32  The list length (nr of inserted samples).
         */
        [DllImport("ddskernel", EntryPoint = "cmn_samplesList_length", CallingConvention = CallingConvention.Cdecl)]
        public static extern uint Length(IntPtr _this);

        [DllImport("ddskernel", EntryPoint = "cmn_reader_action", CallingConvention = CallingConvention.Cdecl)]
        public static extern uint ReaderAction(IntPtr o, IntPtr arg);
    }
}

