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
 */

using System;
using System.Runtime.InteropServices;

namespace DDS.OpenSplice.User
{

    static internal class TopicQos
    {
        /*
         *     u_topicQos
         *     u_topicQosNew(
         *         u_topicQos _template);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicQosNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New(
            IntPtr template);

        /*
         *     void
         *     u_topicQosFree(
         *         u_topicQos _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_topicQosFree", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Free(
            IntPtr _this);
    }

    static internal class SubscriberQos
    {
        /*
         *     u_subscriberQos
         *     u_subscriberQosNew(
         *         u_subscriberQos _template);
         */
        [DllImport("ddskernel", EntryPoint = "u_subscriberQosNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New(
            IntPtr template);

        /*
         *     void
         *     u_subscriberQosFree(
         *         u_subscriberQos _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_subscriberQosFree", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Free(
            IntPtr _this);
    }

    static internal class ReaderQos
    {
        /*
         *     u_readerQos
         *     u_readerQosNew(
         *         u_readerQos _template);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerQosNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New(
            IntPtr template);

        /*
         *     void
         *     u_readerQosFree(
         *         u_readerQos _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_readerQosFree", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Free(
            IntPtr _this);
    }

    static internal class ParticipantQos
    {
        /*
         *     u_participantQos
         *     u_participantQosNew(
         *         u_participantQos _template);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantQosNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New(
            IntPtr template);

        /*
         *     void
         *     u_participantQosFree(
         *         u_participantQos _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_participantQosFree", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Free(
            IntPtr _this);
    }

    static internal class WriterQos
    {
        /*
         *     u_writerQos
         *     u_writerQosNew(
         *         u_writerQos _template);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerQosNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New(
            IntPtr template);

        /*
         *     void
         *     u_writerQosFree(
         *         u_writerQos _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_writerQosFree", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Free(
            IntPtr _this);
    }

    static internal class PublisherQos
    {
        /*
         *     u_publisherQos
         *     u_publisherQosNew(
         *         u_publisherQos _template);
         */
        [DllImport("ddskernel", EntryPoint = "u_publisherQosNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New(
            IntPtr template);

        /*
         *     void
         *     u_publisherQosFree(
         *         u_publisherQos _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_publisherQosFree", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Free(
            IntPtr _this);
    }
}