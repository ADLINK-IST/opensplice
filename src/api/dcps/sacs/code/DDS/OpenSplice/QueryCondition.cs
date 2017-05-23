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
using DDS;
using DDS.OpenSplice.CustomMarshalers;
using DDS.OpenSplice.Kernel;

namespace DDS.OpenSplice
{
    /// <summary>
    /// QueryCondition objects are specialized ReadCondition objects that allow the
    /// application to specify a filter on the locally available data. The DataReader objects
    /// accept a set of QueryCondition objects for the DataReader and provide support
    /// (in conjunction with WaitSet objects) for an alternative communication style
    /// between the Data Distribution Service and the application (i.e., state-based rather
    /// than event-based).
    /// </summary>
    internal class QueryCondition : ReadCondition, IQueryCondition
    {
        private string queryExpression;
        private string[] queryParameters;
    
        internal QueryCondition(
                DataReader dataReader,
                SampleStateKind sampleState, 
                ViewStateKind viewState, 
                InstanceStateKind instanceState, 
                string queryExpression, 
                string[] queryParameters)
            : base(dataReader, sampleState, viewState, instanceState)
        {
            this.queryExpression = queryExpression;
            this.queryParameters = queryParameters;
        }

        internal override ReturnCode init(IntPtr query)
        {
            return base.init(query);
        }
        
        /// <summary>
        /// This operation returns the query expression associated with the QueryCondition.
        /// </summary>
        /// <returns>The query expression associated with the QueryCondition.</returns>
        public string GetQueryExpression()
        {
            bool isAlive;
            string qexpr = null;

            ReportStack.Start();
            lock(this)
            {
                isAlive = this.rlReq_isAlive;
                if (isAlive)
                {
                    qexpr = queryExpression;
                }
            }
            ReportStack.Flush(this, !isAlive);

            return qexpr;
        }

        /// <summary>
        /// This operation obtains the queryParameters associated with the QueryCondition
        /// </summary>
        /// <param name="queryParameters">A reference to a sequence of strings that will be 
        /// used to store the parameters used in the SQL expression</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetQueryParameters(ref string[] queryParameters)
        {
            queryParameters = null;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    queryParameters = this.queryParameters;
                    result = DDS.ReturnCode.Ok;
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        /// <summary>
        /// This operation changes the query parameters associated with the QueryCondition.
        /// </summary>
        /// <param name="queryParameters">A sequence of strings which are the parameters used in the SQL query string
        /// (i.e., the “%n” tokens in the expression).</param>
        /// <returns>Return codes are:Ok,Error,BadParameter,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode SetQueryParameters(params string[] queryParameters)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    using (SequenceStringToArrMarshaler marshaler = new SequenceStringToArrMarshaler())
                    {
                        result = marshaler.CopyIn(queryParameters);
                        if (result == ReturnCode.Ok)
                        {
                            uint length = queryParameters == null ? 0 : (uint) queryParameters.Length;
                            result = uResultToReturnCode(
                                    User.Query.Set(rlReq_UserPeer, marshaler.UserPtr, length));
                            if (result == ReturnCode.Ok)
                            {
                                marshaler.CopyOut(ref this.queryParameters); // Make deep copy.
                            } else {
                                ReportStack.Report(result, "Could not copy out query_paramters.");
                            }
                        } else {
                            ReportStack.Report(result, "Could not copy int query_paramters.");
                        }
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }
        
        internal override ReturnCode Read(IntPtr sampleList)
        {
            return uResultToReturnCode(
                    User.Query.Read(rlReq_UserPeer, Common.SampleList.ReaderAction, sampleList, DDS.Duration.Zero.OsDuration));
        }
         
        internal override ReturnCode Take(IntPtr sampleList)
        {
            return uResultToReturnCode(
                    User.Query.Take(rlReq_UserPeer, Common.SampleList.ReaderAction, sampleList, DDS.Duration.Zero.OsDuration));
        }
         
        internal override ReturnCode ReadNextInstance(InstanceHandle handle, IntPtr sampleList)
        {
            return uResultToReturnCode(
                    User.Query.ReadNextInstance(rlReq_UserPeer, handle, Common.SampleList.ReaderAction, sampleList, DDS.Duration.Zero.OsDuration));
        }
         
        internal override ReturnCode TakeNextInstance(InstanceHandle handle, IntPtr sampleList)
        {
            return uResultToReturnCode(
                    User.Query.TakeNextInstance(rlReq_UserPeer, handle, Common.SampleList.ReaderAction, sampleList, DDS.Duration.Zero.OsDuration));
        }
    }
}
