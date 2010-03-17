// The OpenSplice DDS Community Edition project.
//
// Copyright (C) 2006 to 2009 PrismTech Limited and its licensees.
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
    /// MultiTopic is a specialization of TopicDescription that allows subscriptions
    /// to combine, filter and/or rearrange data coming from several Topics.
    /// MultiTopic allows a more sophisticated subscription that can select and combine
    /// data received from multiple Topics into a single data type (specified by the
    /// inherited type_name). The data will then be filtered (selection) and possibly
    /// re-arranged (aggregation and/or projection) according to an SQL expression with
    /// parameters to adapt the filter clause.
    /// </summary>
    internal class MultiTopic : TopicDescription, IMultiTopic
    {
        internal MultiTopic(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        public string SubscriptionExpression
        {
            get
            {
                IntPtr ptr = Gapi.MultiTopic.get_subscription_expression(GapiPeer);
                string result = Marshal.PtrToStringAnsi(ptr);
                Gapi.GenericAllocRelease.Free(ptr);

                return result;
            }
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="expressionParameters"></param>
        /// <returns></returns>
        public ReturnCode GetExpressionParameters(ref string[] expressionParameters)
        {
            ReturnCode result;
            
            using (SequenceStringMarshaler marshaler = new SequenceStringMarshaler())
            {
				result = Gapi.MultiTopic.get_expression_parameters(
                        GapiPeer,
                        marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref expressionParameters);
					return result;
                }
                else 
                {
                    expressionParameters = new string[0];
                }
            }

			return result;
        }

        /// <summary>
        /// This operation is not yet implemented. It is scheduled for a future release.
        /// </summary>
        /// <param name="expressionParameters"></param>
        /// <returns></returns>
        public ReturnCode SetExpressionParameters(params string[] expressionParameters)
        {
            ReturnCode result;
            
            using (SequenceStringMarshaler marshaler = new SequenceStringMarshaler())
            {
                result = marshaler.CopyIn(expressionParameters);
                if (result == DDS.ReturnCode.Ok)
                {
                    result = Gapi.MultiTopic.set_expression_parameters(
                            GapiPeer,
                            marshaler.GapiPtr);
                }
            }

            return result;
        }
    }
}
