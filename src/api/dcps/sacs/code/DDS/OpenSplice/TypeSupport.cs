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

        public abstract DataWriter CreateDataWriter(IntPtr gapiPtr);

        public abstract DataReader CreateDataReader(IntPtr gapiPtr);

        protected Type dataType = null;
        protected DatabaseMarshaler marshaler = null;
        protected IMarshalerTypeGenerator generator = null;

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
        public TypeSupport(
            Type dataType,
            DatabaseMarshaler marshaler)
        {
            this.dataType = dataType;
            this.marshaler = marshaler;

            this.dummyOperationDelegate = new DummyOperationDelegate(DummyOperation);
            IntPtr ptr = Gapi.FooTypeSupport.alloc(
               TypeName,                    /* Original IDL type name */
               KeyList,
               Description,
               IntPtr.Zero,                 /* type_load */
               marshaler.CopyInDelegate,    /* copyIn: copy from C# types */
               dummyOperationDelegate,      /* copyOut: copy to C# types */
               0,                           /* alloc_size */
               IntPtr.Zero,                 /* alloc buffer */
               IntPtr.Zero,                 /* writer copy */
               marshaler.ReaderCopyDelegate,/* reader copy */
               IntPtr.Zero,                 /* create datawriter */
               IntPtr.Zero);                /* create datareader */

            // Base class handles everything.
            base.SetPeer(ptr);
        }

        /*
         * Constructor for a TypeSupport that generates a Marshaler using
         * the specified Marshaler generator.
         */
        public TypeSupport(
            Type dataType,
            IMarshalerTypeGenerator generator)
        {
            this.dataType = dataType;
            this.generator = generator;

            //this.fakeCopyOutDelegate = new FakeSampleCopyOutDelegate(FakeCopyOut);
            this.dummyOperationDelegate = new DummyOperationDelegate(DummyOperation);
            IntPtr ptr = Gapi.FooTypeSupport.alloc(
               TypeName,                    /* Original IDL type name */
               KeyList,
               Description,
               IntPtr.Zero,                 /* type_load */
               dummyOperationDelegate,      /* copyIn: copy from C# types */
               dummyOperationDelegate,      /* copyOut: copy to C# types */
               0,                           /* alloc_size */
               IntPtr.Zero,                 /* alloc buffer */
               IntPtr.Zero,                 /* writer copy */
               dummyOperationDelegate,      /* reader copy */
               IntPtr.Zero,                 /* create datawriter */
               IntPtr.Zero);                /* create datareader */

            // Base class handles everything.
            base.SetPeer(ptr);
        }

        // TODO: This operation is currently not thread-safe. 
        public virtual ReturnCode RegisterType(IDomainParticipant participant, string typeName)
        {
            ReturnCode result = ReturnCode.Error;

            if (participant == null)
            {
                return result;
            }

            if (marshaler == null && generator == null)
            {
                // TODO: Write error in message log.
                return result;
            }

            IntPtr domainObj = (participant as DomainParticipant).GapiPeer;
            result = Gapi.FooTypeSupport.register_type(
                GapiPeer,
                domainObj,
                typeName);


            // If there is no explicit marshaler, then generate one.
            if (marshaler == null)
            {
                // Get the attribute names and offsets of this datatype.
                // This meta-data is looked up by using the IDL type name.
                IntPtr metaData = Gapi.DomainParticipant.get_type_metadescription(domainObj, TypeName);

                // Generate a new marshaller using the available generator.
                marshaler = DatabaseMarshaler.Create(domainObj, metaData, dataType, generator);

                // Attach the functions in the generated marshaler to the current TypeSupport.
                result = AttachMarshalerDelegates(this.GapiPeer, marshaler);
                if (result == ReturnCode.Ok)
                {
                    // Attach the same functions to the copy of the TypeSupport that was already 
                    // registered in the Domain.
                    IntPtr registeredTS;
                    
                    //if typeName is null use TypeName instead - 
                    //this case applies only here to avoid a NullReferenceException
                    
                    if (typeName == null)
                    {
                        registeredTS = Gapi.DomainParticipant.get_typesupport(domainObj, TypeName);
                    }
                    else 
                    { 
                        registeredTS = Gapi.DomainParticipant.get_typesupport(domainObj, typeName); 
                    }
                    result = AttachMarshalerDelegates(registeredTS, marshaler);
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
            typeSupport.copy_in = marshaler.CopyInDelegate;

            // HACK: This is a Mono workaround. Currently you cannot marshal a delegate
            // to a function pointer if the parameters include an "object" type. So we
            // will pass a fake delegate, and then internally use the real copyOut, this
            // may give better performance anyways, since we won't have to convert the
            // IntPtr to a Delegate for every ReaderCopy invocation.
            typeSupport.copy_out = dummyOperationDelegate;

            typeSupport.reader_copy = marshaler.ReaderCopyDelegate;
            Gapi.TypeSupport.Release(typeSupport, tsPtr);
            return result;
        }
    }
}
