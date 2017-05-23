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
using DDS;
using DDS.OpenSplice.Kernel;

namespace DDS.OpenSplice
{
    /// <summary>
    /// The DataReader objects can create a set of ReadCondition (and
    /// StatusCondition) objects which provide support (in conjunction with WaitSet
    /// objects) for an alternative communication style between the Data Distribution
    /// Service and the application (i.e., state-based rather than event-based).
    /// ReadCondition objects allow an DataReader to specify the data samples it is
    /// interested in (by specifying the desired sample-states, view-states, and
    /// instance-states); see the parameter definitions for DataReader's
    /// create_readcondition operation. This allows the Data Distribution Service to
    /// trigger the condition only when suitable information is available. ReadCondition
    /// objects are to be used in conjunction with a WaitSet. More than one
    /// ReadCondition may be attached to the same DataReader.
    /// </summary>
    public class ReadCondition : Condition, IReadCondition
    {
        private DataReader dataReader;
        private SampleStateKind sampleState;
        private ViewStateKind viewState;
        private InstanceStateKind instanceState;

        internal ReadCondition(
                DataReader dataReader,
                SampleStateKind sampleState,
                ViewStateKind viewState,
                InstanceStateKind instanceState)
        {
            this.dataReader = dataReader;
            this.sampleState = sampleState;
            this.viewState = viewState;
            this.instanceState = instanceState;
            this.MyDomainId = dataReader.MyDomainId;
        }

        internal virtual ReturnCode init(IntPtr query)
        {
            return base.init(query, false);
        }

        /// <summary>
        /// This operation returns the set of sample_states that are taken into account to
        /// determine the trigger_value of the ReadCondition.
        /// </summary>
        /// <returns>The sample_states specified when the ReadCondition was created.</returns>
        public SampleStateKind GetSampleStateMask()
        {
            bool isAlive;
            SampleStateKind ssk = 0;

            ReportStack.Start();
            lock(this)
            {
                isAlive = this.rlReq_isAlive;
                if (isAlive)
                {
                    ssk = sampleState;
                }
            }
            ReportStack.Flush(this, !isAlive);

            return ssk;
        }

        /// <summary>
        /// This operation returns the set of view_states that are taken into account to
        /// determine the trigger_value of the ReadCondition.
        /// </summary>
        /// <returns>The view_states specified when the ReadCondition was created.</returns>
        public ViewStateKind GetViewStateMask()
        {
            bool isAlive;
            ViewStateKind vsk = 0;

            ReportStack.Start();
            lock(this)
            {
                isAlive = this.rlReq_isAlive;
                if (isAlive)
                {
                    vsk = viewState;
                }
            }
            ReportStack.Flush(this, !isAlive);

            return vsk;
        }

        /// <summary>
        /// This operation returns the set of instance_states that are taken into account to
        /// determine the trigger_value of the ReadCondition.
        /// </summary>
        /// <returns>The instance_states specified when the ReadCondition was created.</returns>
        public InstanceStateKind GetInstanceStateMask()
        {
            bool isAlive;
            InstanceStateKind isk = 0;

            ReportStack.Start();
            lock(this)
            {
                isAlive = this.rlReq_isAlive;
                if (isAlive)
                {
                    isk = instanceState;
                }
            }
            ReportStack.Flush(this, !isAlive);

            return isk;
        }

        /// <summary>
        /// This operation returns the DataReader associated with the ReadCondition.
        /// </summary>
        /// <returns>The DataReader associated with the ReadCondition.</returns>
        public IDataReader GetDataReader()
        {
            bool isAlive;
            IDataReader dr = null;

            ReportStack.Start();
            lock(this)
            {
                isAlive = this.rlReq_isAlive;
                if (isAlive)
                {
                    dr = dataReader;
                }
            }
            ReportStack.Flush(this, !isAlive);

            return dr;
        }

        internal static byte AlwaysTrue(IntPtr o, IntPtr arg)
        {
            return 1;
        }

        public override bool GetTriggerValue()
        {
            bool triggerValue = false;
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    byte result = User.Query.Test(rlReq_UserPeer, AlwaysTrue, IntPtr.Zero);
                    triggerValue = (result != 0);
                }
            }
            return triggerValue;
        }

        internal virtual ReturnCode Read(IntPtr sampleList)
        {
            uint mask = DataReader.StateMask(sampleState, viewState, instanceState);
            return SacsSuperClass.uResultToReturnCode(
                User.Reader.Read(
                    dataReader.rlReq_UserPeer,
                    mask,
                    Common.SampleList.ReaderAction,
                    sampleList, Duration.Zero.OsDuration));
        }

        internal virtual ReturnCode Take(IntPtr sampleList)
        {
            uint mask = DataReader.StateMask(sampleState, viewState, instanceState);
            return SacsSuperClass.uResultToReturnCode(
                    User.Reader.Take(
                            dataReader.rlReq_UserPeer,
                            mask,
                            Common.SampleList.ReaderAction,
                            sampleList, DDS.Duration.Zero.OsDuration));
        }

        internal virtual ReturnCode ReadNextInstance(InstanceHandle handle, IntPtr sampleList)
        {
            uint mask = DataReader.StateMask(sampleState, viewState, instanceState);
            return SacsSuperClass.uResultToReturnCode(
                    User.Reader.ReadNextInstance(
                            dataReader.rlReq_UserPeer,
                            handle,
                            mask,
                            Common.SampleList.ReaderAction,
                            sampleList, DDS.Duration.Zero.OsDuration));
        }

        internal virtual ReturnCode TakeNextInstance(InstanceHandle handle, IntPtr sampleList)
        {
            uint mask = DataReader.StateMask(sampleState, viewState, instanceState);
            return SacsSuperClass.uResultToReturnCode(
                    User.Reader.TakeNextInstance(
                            dataReader.rlReq_UserPeer,
                            handle,
                            mask,
                            Common.SampleList.ReaderAction,
                            sampleList, DDS.Duration.Zero.OsDuration));
        }
    }
}
