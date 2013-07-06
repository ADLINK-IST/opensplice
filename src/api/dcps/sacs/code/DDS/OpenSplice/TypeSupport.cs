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
using System.Runtime.InteropServices;
using DDS;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    /**
     * The TypeSupportFactory is a helper class for TypeSupport, to which it can delegate
     * the creation of typed DataWriters and DataReaders. 
     * This functionality is split across 2 different classes to avoid cyclic dependencies:
     * the original TypeSupport can only be deleted by the garbage collector, so gapi cannot
     * store a normal backRef to it since that would introduce a memory leak. However, a weak
     * backRef would allow the original TypeSupport object to be garbage collected when it is
     * needed to insantiate a typed Reader/Writer. 
     * By splitting it up into 2 separate classes, the original TypeSupport may be garbage 
     * collected at any time, but the gapi may hold on to its TypeSupportFactory for as 
     * long as it wants. 
     * NOTE: To make the TypeSupportFactory available to gapi, the backRef pointer is not
     * used to point to the original TypeSupport, but rather to its TypeSupportFactory.  
     */
    public abstract class TypeSupportFactory
    {
        private IntPtr handleSelf = IntPtr.Zero;
        private DeleteEntityActionDelegate deleteEntityAction;
       
        public abstract DataWriter CreateDataWriter(IntPtr gapiPtr);

        public abstract DataReader CreateDataReader(IntPtr gapiPtr);
        
        public TypeSupportFactory()
        {
           deleteEntityAction = DeleteEntityAction;
        }
        
        internal void SetPeer(IntPtr gapiPtr)
        {
            GCHandleType backRefType;

            backRefType = GCHandleType.Normal;
            
            // Get the GCHandle for the current C# object and cache the IntPtr of the GCHandle
            // in the UserData field of its gapi pointer.
            GCHandle tmpGCHandle = GCHandle.Alloc(this, backRefType);
            handleSelf = GCHandle.ToIntPtr(tmpGCHandle);
            Gapi.Entity.set_user_data(gapiPtr, handleSelf, deleteEntityAction, IntPtr.Zero);
        }
        
        internal static TypeSupportFactory fromUserData(IntPtr gapiPtr)
        {
            TypeSupportFactory tsFactory = null;

            // Check whether the gapiPtr contains a valid pointer.
            if (gapiPtr != IntPtr.Zero)
            {
                // get the user data out of the gapi object
                IntPtr managedIntPtr = Gapi.Entity.get_user_data(gapiPtr);

                // Check whether the userData contains a valid pointer.
                if (managedIntPtr != IntPtr.Zero)
                {
                    // If so, get the relevant C# Object.
                    GCHandle tmpGCHandle = GCHandle.FromIntPtr(managedIntPtr);
                    tsFactory = tmpGCHandle.Target as TypeSupportFactory;
                }
            }

            return tsFactory;
        }

        
        internal void DeleteEntityAction(IntPtr entityData, IntPtr arg)
        {
            GCHandle tmpGCHandle = GCHandle.FromIntPtr(handleSelf);
            tmpGCHandle.Free();
            handleSelf = IntPtr.Zero;
        }
    }
    
    public abstract class TypeSupport : ITypeSupport
    {
        public abstract string TypeName { get; }
        public abstract string KeyList { get; }
        public abstract string[] Description { get; }
        public Type TypeSpec
        {
            get
            {
                return dataType;
            }
        }

        protected Type dataType = null;

        private IntPtr gapiPeer = IntPtr.Zero;
        public IntPtr GapiPeer { get { return gapiPeer; } }
        
        protected delegate void DummyOperationDelegate();
        protected DummyOperationDelegate dummyOperationDelegate;

        // HACK: This is a Mono workaround. Currently you cannot marshal a delegate
        // to a function pointer if the parameters include an "object" type. So we
        // will pass a fake delegate, and then internally use the real copyOut, this
        // may give better performance anyways, since we won't have to convert the
        // IntPtr to a Delegate for every ReaderCopy invocation.
        public void DummyOperation()
        { }

        /*
         * Constructor for a TypeSupport that uses the specified (custom) Marshaler.
         */
        public TypeSupport(Type dataType, TypeSupportFactory tsFactory)
        {
            this.dataType = dataType;
            
            System.Text.StringBuilder descriptor = new System.Text.StringBuilder();
            foreach (string s in Description) {
               descriptor.Append(s);
            }

            this.dummyOperationDelegate = new DummyOperationDelegate(DummyOperation);
            this.gapiPeer = Gapi.FooTypeSupport.alloc(
               TypeName,                    // Original IDL type name
               KeyList,
               descriptor.ToString(),
               IntPtr.Zero,                 // type_load
               dummyOperationDelegate,      // copyIn: delay initialization until marshaler present
               dummyOperationDelegate,      // copyOut: delay initialization until marshaler present
               0,                           // alloc_size
               IntPtr.Zero,                 // alloc buffer
               IntPtr.Zero,                 // writer copy
               dummyOperationDelegate);     // reader copy: delay initialization until marshaler present

            // Base class handles everything.
            if (GapiPeer != IntPtr.Zero)
            {
                tsFactory.SetPeer(GapiPeer);
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
