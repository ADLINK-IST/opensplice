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
using System.Collections.Generic;
using DDS;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    public abstract class TypeSupport : SacsSuperClass, ITypeSupport
    {
        public abstract string TypeName { get; }
        public abstract string KeyList { get; }
        public abstract string Description { get; }
        public Type TypeSpec
        {
            get
            {
                return dataType;
            }
        }

        public abstract DataWriter CreateDataWriter(IntPtr gapiPtr);

        public abstract DataReader CreateDataReader(IntPtr gapiPtr);

        protected Type dataType = null;

        protected delegate void DummyOperationDelegate();
        protected DummyOperationDelegate dummyOperationDelegate;

        // HACK: This is a Mono workaround. Currently you cannot marshal a delegate
        // to a function pointer if the parameters include an "object" type. So we
        // will pass a fake delegate, and then internally use the real copyOut, this
        // may give better performance anyways, since we won't have to convert the
        // IntPtr to a Delegate for every ReaderCopy invocation.
        public void DummyOperation()//)IntPtr from, IntPtr to, int offset)
        { }

        /*
         * Constructor for a TypeSupport that uses the specified (custom) Marshaler.
         */
        public TypeSupport(Type dataType)
        {
            this.dataType = dataType;

            this.dummyOperationDelegate = new DummyOperationDelegate(DummyOperation);
            IntPtr ptr = Gapi.FooTypeSupport.alloc(
               TypeName,                    // Original IDL type name
               KeyList,
               Description,
               IntPtr.Zero,                 // type_load
               dummyOperationDelegate,      // copyIn: delay initialization until marshaler present
               dummyOperationDelegate,      // copyOut: delay initialization until marshaler present
               0,                           // alloc_size
               IntPtr.Zero,                 // alloc buffer
               IntPtr.Zero,                 // writer copy
               dummyOperationDelegate);     // reader copy: delay initialization until marshaler present

            // Base class handles everything.
            if (ptr != IntPtr.Zero)
            {
                SetPeer(ptr, true);
            }
            else
            {
                // Gapi already logged that the TypeSupport has not been created
                // successfully. Now create a deliberate null pointer exception
                // to let the current constructor fail.
                throw new System.NullReferenceException("gapi_fooTypeSupport__alloc returned a NULL pointer.");
            }
        }

        // The RegisterType operation should be implemented in its type specific specialization.
        public abstract ReturnCode RegisterType(
                IDomainParticipant participant,
                string typeName);

        // TODO: This operation is currently not thread-safe.
        public virtual ReturnCode RegisterType(
                IDomainParticipant participant,
                string typeName,
                DatabaseMarshaler marshaler)
        {
            ReturnCode result = ReturnCode.BadParameter;

            if (participant != null && marshaler != null)
            {
                // Now that a marshaler is present,
                result = AttachMarshalerDelegates(GapiPeer, marshaler);
                if (result == ReturnCode.Ok)
                {
                    IntPtr domainObj = (participant as DomainParticipant).GapiPeer;
                    result = Gapi.FooTypeSupport.register_type(
                            GapiPeer,
                            domainObj,
                            typeName);

                    if (result == ReturnCode.Ok)
                    {
                        DatabaseMarshaler.Add(participant, dataType, marshaler);
                        marshaler.InitEmbeddedMarshalers(participant);
                    }
                }
            }

            return result;
        }

        private ReturnCode AttachMarshalerDelegates(IntPtr ts, DatabaseMarshaler marshaler)
        {
            Gapi._TypeSupport typeSupport = null;
            IntPtr tsPtr = IntPtr.Zero;
            ReturnCode result = ReturnCode.Ok;

            typeSupport = Gapi.TypeSupport.Claim(ts, out tsPtr, ref result);
            if (result == ReturnCode.Ok)
            {
                typeSupport.copy_in = marshaler.CopyInDelegate;

                // HACK: This is a Mono workaround. Currently you cannot marshal a delegate
                // to a function pointer if the parameters include an "object" type. So we
                // will pass a fake delegate, and then internally use the real copyOut, this
                // may give better performance anyways, since we won't have to convert the
                // IntPtr to a Delegate for every ReaderCopy invocation.
                typeSupport.copy_out = marshaler.CopyOutDelegate;

                typeSupport.reader_copy = marshaler.ReaderCopyDelegate;
                Gapi.TypeSupport.Release(typeSupport, tsPtr);
            }

            return result;
        }
    }
}
