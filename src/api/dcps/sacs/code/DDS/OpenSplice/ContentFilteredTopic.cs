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
    /// ContentFilteredTopic is a specialization of TopicDescription that allows for content based subscriptions. 
    /// ContentFilteredTopic describes a more sophisticated subscription that indicates the Subscriber 
    /// does not necessarily want to see all values of each instance published under the Topic. 
    /// Rather, it only wants to see the values whose contents satisfy certain criteria. 
    /// Therefore this class must be used to request content-based subscriptions. The selection 
    /// of the content is done using the SQL based filter with parameters to adapt the filter clause.
    /// </summary>
    internal class ContentFilteredTopic : TopicDescription, IContentFilteredTopic
    {
        internal ContentFilteredTopic(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
        }

        /// <summary>
        /// This operation returns the filter_expression associated with the ContentFilteredTopic.
        /// </summary>
        /// <returns>The string that holds the filter expression associated with the ContentFilteredTopic.</returns>
        public string GetFilterExpression()
        {
            IntPtr ptr = Gapi.ContentFilteredTopic.get_filter_expression(GapiPeer);
            string result = Marshal.PtrToStringAnsi(ptr);
            Gapi.GenericAllocRelease.Free(ptr);

            return result;
        }

        /// <summary>
        /// This operation obtains the expression parameters associated with the ContentFilteredTopic.
        /// </summary>
        /// <param name="expressionParameters">A reference to a sequence of strings that will be used 
        /// to store the parameters used in the SQL expression.</param>
        /// <returns>The possible return codes are: Ok,Error,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode GetExpressionParameters(ref string[] expressionParameters)
        {
            ReturnCode result;
            
            using (SequenceStringMarshaler marshaler = new SequenceStringMarshaler())
            {
                result = Gapi.ContentFilteredTopic.get_expression_parameters(
                        GapiPeer,
                        marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref expressionParameters);
                }
                else
                {
                    expressionParameters = new string[0];
                }
            }

			return result;
        }

        /// <summary>
        /// This operation changes the expression parameters associated with the ContentFilteredTopic.
        /// </summary>
        /// <param name="expressionParameters">A sequence of strings with the parameters used in the SQL expression.
        /// The number of values in expressionParameters must be equal or greater than the highest referenced 
        /// %n token in the subscriptionExpression.</param>
        /// <returns>The possible return codes are: Ok, Error, BadParameter,AlreadyDeleted or OutOfResources.</returns>
        public ReturnCode SetExpressionParameters(params string[] expressionParameters)
        {
            ReturnCode result;
            
            using (SequenceStringMarshaler marshaler = new SequenceStringMarshaler())
            {
                result = marshaler.CopyIn(expressionParameters);
                if (result == DDS.ReturnCode.Ok)
                {
                    result = Gapi.ContentFilteredTopic.set_expression_parameters(
                            GapiPeer,
                            marshaler.GapiPtr);
                }
            }

            return result;
        }

        /// <summary>
        /// This property returns the related topic to the filter.
        /// </summary>
        public ITopic RelatedTopic
        {
            get
            {
                IntPtr gapiPtr = Gapi.ContentFilteredTopic.get_related_topic(GapiPeer);
                ITopic topic = SacsSuperClass.fromUserData(gapiPtr) as ITopic;
                return topic;
            }
        }
    }
}
