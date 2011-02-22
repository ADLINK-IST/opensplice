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
using DDS.OpenSplice;

namespace DDS
{
    public static class ErrorInfo
    {
        private readonly static SacsSuperClass super;
        
        static ErrorInfo()
        {
            IntPtr ptr = OpenSplice.Gapi.ErrorInfo.alloc();
            super = new SacsSuperClass();
            if (ptr != IntPtr.Zero)
            {
                super.SetPeer(ptr, true);
            }
            else
            {
                // Gapi already logged that the ErrorInfo has not been created 
                // successfully. Now create a deliberate null pointer exception
                // to let the current constructor fail.
                throw new System.NullReferenceException("gapi_errorInfo__alloc returned a NULL pointer.");
            }
        }

        public static ReturnCode Update()
        {
            return OpenSplice.Gapi.ErrorInfo.update(super.GapiPeer);
        }

        public static ReturnCode GetCode(out ErrorCode code)
        {
            return OpenSplice.Gapi.ErrorInfo.get_code(
                super.GapiPeer,
                out code);
        }

        public static ReturnCode GetMessage(out string message)
        {
            return OpenSplice.Gapi.ErrorInfo.get_message(
                super.GapiPeer,
                out message);
        }

        public static ReturnCode GetLocation(out string location)
        {
            return OpenSplice.Gapi.ErrorInfo.get_location(
                super.GapiPeer,
                out location);
        }

        public static ReturnCode GetSourceLine(out string sourceLine)
        {
            return OpenSplice.Gapi.ErrorInfo.get_source_line(
                super.GapiPeer,
                out sourceLine);
        }

        public static ReturnCode GetStackTrace(out string stackTrace)
        {
            return OpenSplice.Gapi.ErrorInfo.get_stack_trace(
                super.GapiPeer,
                out stackTrace);
        }
    }
}
