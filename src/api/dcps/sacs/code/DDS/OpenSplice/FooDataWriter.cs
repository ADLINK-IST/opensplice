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

namespace DDS.OpenSplice
{
    public static class FooDataWriter // : DataWriter
    {
        public static InstanceHandle RegisterInstance(
                DataWriter writer, 
                object instanceData)
        {
            GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
            InstanceHandle result = Gapi.FooDataWriter.register_instance(
                    writer.GapiPeer,
                    GCHandle.ToIntPtr(tmpGCHandle));
            tmpGCHandle.Free();

            return result;
        }

        public static InstanceHandle RegisterInstanceWithTimestamp(
                DataWriter writer, 
                object instanceData, 
                Time sourceTimestamp)
        {
            GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
            InstanceHandle result = Gapi.FooDataWriter.register_instance_w_timestamp(
                    writer.GapiPeer,
                    GCHandle.ToIntPtr(tmpGCHandle),
                    ref sourceTimestamp);
            tmpGCHandle.Free();

            return result;
        }

        public static ReturnCode UnregisterInstance(
                DataWriter writer, 
                object instanceData, 
                InstanceHandle instanceHandle)
        {
            GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
            ReturnCode result = Gapi.FooDataWriter.unregister_instance(
                    writer.GapiPeer,
                    GCHandle.ToIntPtr(tmpGCHandle),
                    instanceHandle);
            tmpGCHandle.Free();

            return result;
        }

        public static ReturnCode UnregisterInstanceWithTimestamp(
                DataWriter writer, 
                object instanceData,
                InstanceHandle instanceHandle, 
                Time sourceTimestamp)
        {
            GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
            ReturnCode result = Gapi.FooDataWriter.unregister_instance_w_timestamp(
                    writer.GapiPeer,
                    GCHandle.ToIntPtr(tmpGCHandle),
                    instanceHandle,
                    ref sourceTimestamp);
            tmpGCHandle.Free();

            return result;
        }

        public static ReturnCode Write(
                DataWriter writer, 
                object instanceData, 
                InstanceHandle instanceHandle)
        {
            GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
            ReturnCode result = Gapi.FooDataWriter.write(
                    writer.GapiPeer,
                    GCHandle.ToIntPtr(tmpGCHandle),
                    instanceHandle);
            tmpGCHandle.Free();

            return result;
        }

        public static ReturnCode WriteWithTimestamp(
                DataWriter writer, 
                object instanceData, 
                InstanceHandle instanceHandle, 
                Time sourceTimestamp)
        {
            GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
            ReturnCode result = Gapi.FooDataWriter.write_w_timestamp(
                    writer.GapiPeer,
                    GCHandle.ToIntPtr(tmpGCHandle),
                    instanceHandle,
                    ref sourceTimestamp);
            tmpGCHandle.Free();

            return result;
        }

        public static ReturnCode Dispose(
                DataWriter writer, 
                object instanceData, 
                InstanceHandle instanceHandle)
        {
            GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
            ReturnCode result = Gapi.FooDataWriter.dispose(
                    writer.GapiPeer,
                    GCHandle.ToIntPtr(tmpGCHandle),
                    instanceHandle);
            tmpGCHandle.Free();

            return result;
        }

        public static ReturnCode DisposeWithTimestamp(
                DataWriter writer, 
                object instanceData,
                InstanceHandle instanceHandle, 
                Time sourceTimestamp)
        {
            GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
            ReturnCode result = Gapi.FooDataWriter.dispose_w_timestamp(
                    writer.GapiPeer,
                    GCHandle.ToIntPtr(tmpGCHandle),
                    instanceHandle,
                    ref sourceTimestamp);
            tmpGCHandle.Free();

            return result;
        }

        public static ReturnCode WriteDispose(
                DataWriter writer, 
                object instanceData, 
                InstanceHandle instanceHandle)
        {
            GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
            ReturnCode result = Gapi.FooDataWriter.writedispose(
                    writer.GapiPeer,
                    GCHandle.ToIntPtr(tmpGCHandle),
                    instanceHandle);
            tmpGCHandle.Free();

            return result;
        }

        public static ReturnCode WriteDisposeWithTimestamp(
                DataWriter writer, 
                object instanceData,
                InstanceHandle instanceHandle, 
                Time sourceTimestamp)
        {
            GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
            ReturnCode result = Gapi.FooDataWriter.writedispose_w_timestamp(
                    writer.GapiPeer,
                    GCHandle.ToIntPtr(tmpGCHandle),
                    instanceHandle,
                    ref sourceTimestamp);
            tmpGCHandle.Free();

            return result;
        }

        public static ReturnCode GetKeyValue(
                DataWriter writer, 
                ref object key, 
                InstanceHandle instanceHandle)
        {
            GCHandle tmpGCHandle = GCHandle.Alloc(key, GCHandleType.Normal);
            ReturnCode result = Gapi.FooDataWriter.get_key_value(
                    writer.GapiPeer,
                    GCHandle.ToIntPtr(tmpGCHandle),
                    instanceHandle);
            tmpGCHandle.Free();

            return result;
        }

        public static InstanceHandle LookupInstance(
                DataWriter writer, 
                object instanceData)
        {
            GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
            InstanceHandle result = Gapi.FooDataWriter.lookup_instance(
                    writer.GapiPeer,
                    GCHandle.ToIntPtr(tmpGCHandle));
            tmpGCHandle.Free();

            return result;
        }
    }
}
