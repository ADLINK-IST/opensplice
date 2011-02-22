// The OpenSplice DDS Community Edition project.
//
// Copyright (C) 2006 to 2011 PrismTech Limited and its licensees.
// Copyright (C) 2009  L-3 Communications / IS
// 
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License Version 3 dated 29 June 2007, as published by the
//  Free Software Foundation.
// 
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
// 
//  You should have received a copy of the GNU Lesser General Public
//  License along with OpenSplice DDS Community Edition; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

using System;
using System.Runtime.InteropServices;
using DDS;
using DDS.OpenSplice.CustomMarshalers;

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
        internal QueryCondition(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
        }

        /// <summary>
        /// This operation returns the query expression associated with the QueryCondition.
        /// </summary>
        /// <returns>The query expression associated with the QueryCondition.</returns>
        public string GetQueryExpression()
        {
            IntPtr ptr = Gapi.QueryCondition.get_query_expression(GapiPeer);
            string result = Marshal.PtrToStringAnsi(ptr);
            Gapi.GenericAllocRelease.Free(ptr);

            return result;
        }

        /// <summary>
        /// This operation obtains the queryParameters associated with the QueryCondition
        /// </summary>
        /// <param name="queryParameters">A reference to a sequence of strings that will be 
        /// used to store the parameters used in the SQL expression</param>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetQueryParameters(ref string[] queryParameters)
        {
            ReturnCode result = ReturnCode.Error;
            using (SequenceStringMarshaler marshaler = new SequenceStringMarshaler())
            {
                result = Gapi.QueryCondition.get_query_parameters(
                        GapiPeer,
                        marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref queryParameters);
                }
            }

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
            ReturnCode result = ReturnCode.Error;
            using (SequenceStringMarshaler marshaler = new SequenceStringMarshaler())
            {
                if (marshaler.CopyIn(queryParameters) == ReturnCode.Ok)
                {
                    result = Gapi.QueryCondition.set_query_parameters(
                            GapiPeer,
                            marshaler.GapiPtr);
                }
            }

            return result;
        }
    }
}
