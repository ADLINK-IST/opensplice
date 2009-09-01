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
    internal class QueryCondition : ReadCondition, IQueryCondition
    {
        internal QueryCondition(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
        }

        public string GetQueryExpression()
        {
            IntPtr ptr = Gapi.QueryCondition.get_query_expression(GapiPeer);
            string result = Marshal.PtrToStringAnsi(ptr);
            Gapi.GenericAllocRelease.Free(ptr);

            return result;
        }

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
                    marshaler.CopyOut(out queryParameters);
                }
            }

            return result;
        }

        public ReturnCode SetQueryParameters(params string[] queryParameters)
        {
            ReturnCode result = ReturnCode.Error;
            using (SequenceStringMarshaler marshaler = new SequenceStringMarshaler(queryParameters))
            {
                result = Gapi.QueryCondition.set_query_parameters(
                GapiPeer,
                marshaler.GapiPtr);
            }

            return result;
        }
    }
}
