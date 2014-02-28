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

namespace DDS.OpenSplice.Gapi
{
    enum _ObjectKind {
        OBJECT_KIND_UNDEFINED                 = 0x00000000,
        OBJECT_KIND_ENTITY                    = 0x00000001,
        OBJECT_KIND_DOMAINENTITY              = 0x00000003,  
        OBJECT_KIND_DOMAINPARTICIPANT         = 0x00000005,
        OBJECT_KIND_TYPESUPPORT               = 0x00000008,
        OBJECT_KIND_TOPICDESCRIPTION          = 0x00000010,
        OBJECT_KIND_TOPIC                     = 0x00000033,
        OBJECT_KIND_CONTENTFILTEREDTOPIC      = 0x00000050,
        OBJECT_KIND_MULTITOPIC                = 0x00000090,
        OBJECT_KIND_PUBLISHER                 = 0x00000103,
        OBJECT_KIND_SUBSCRIBER                = 0x00000203,
        OBJECT_KIND_DATAWRITER                = 0x00000403,
        OBJECT_KIND_DATAREADER                = 0x00000803,
        OBJECT_KIND_FOOTYPESUPPORT            = 0x00001008,
        OBJECT_KIND_FOODATAWRITER             = 0x00002403,
        OBJECT_KIND_FOODATAREADER             = 0x00004803,
        OBJECT_KIND_CONDITION                 = 0x00008000,
        OBJECT_KIND_GUARDCONDITION            = 0x00018000,
        OBJECT_KIND_STATUSCONDITION           = 0x00028000,
        OBJECT_KIND_READCONDITION             = 0x00048000,
        OBJECT_KIND_QUERYCONDITION            = 0x000C8000,
        OBJECT_KIND_WAITSET                   = 0x00100000,
        OBJECT_KIND_STATUS                    = 0x00200000,
        OBJECT_KIND_PARTICIPANT_STATUS        = 0x00600000,
        OBJECT_KIND_TOPIC_STATUS              = 0x00A00000,
        OBJECT_KIND_PUBLISHER_STATUS          = 0x01200000,
        OBJECT_KIND_SUBSCRIBER_STATUS         = 0x02200000,
        OBJECT_KIND_WRITER_STATUS             = 0x04200000,
        OBJECT_KIND_READER_STATUS             = 0x08200000,
        OBJECT_KIND_DATAVIEW                  = 0x10000001,
        OBJECT_KIND_FOODATAVIEW               = 0x30000001,
        OBJECT_KIND_DOMAINPARTICIPANTFACTORY  = 0x40000001,
        OBJECT_KIND_ERRORINFO                 = 0x60000000
    }
        
    class Object
    {
    
        //OS_API _Object
		//gapi_objectClaim (
		//    gapi_object handle,
		//    _ObjectKind kind,
		//    gapi_returnCode_t *result);
        [DllImport("ddskernel", EntryPoint = "gapi_objectClaim")]
        public static extern IntPtr Claim (
            IntPtr handle,
            _ObjectKind kind,
            ref ReturnCode result);
        
        //OS_API gapi_object
		//_ObjectRelease (
		//    _Object object);
        [DllImport("ddskernel", EntryPoint = "_ObjectRelease")]
        public static extern IntPtr Release (
            IntPtr _object);
    }
}
    
