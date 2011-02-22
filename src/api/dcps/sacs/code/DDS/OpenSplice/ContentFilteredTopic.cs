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
    internal class ContentFilteredTopic : TopicDescription, IContentFilteredTopic
    {
        internal ContentFilteredTopic(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
        }
        
        public string GetFilterExpression()
        {
            IntPtr ptr = Gapi.ContentFilteredTopic.get_filter_expression(GapiPeer);
            string result = Marshal.PtrToStringAnsi(ptr);
            Gapi.GenericAllocRelease.Free(ptr);

            return result;
        }
        
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
