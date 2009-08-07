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

namespace DDS.OpenSplice
{
    public abstract class TypeSupport : SacsSuperClass, ITypeSupport
    {
        // cache these types for performance
        //protected static Type copyInType = typeof(SampleCopyInDelegate);
        //protected static Type copyOutType = typeof(SampleCopyOutDelegate);
        //protected static Type readerCopyType = typeof(SampleReaderCopyDelegate);
        //protected static Type allocType = typeof(SampleReaderAllocDelegate);

        public abstract string TypeName     { get; }
        public abstract string KeyList      { get; }
        public abstract string Description  { get; }
        protected Type dataType = null;
        protected BaseMarshaler marshaler = null;
        protected IMarshalerTypeGenerator generator = null;
        
        protected delegate void DummyOperationDelegate();
        protected DummyOperationDelegate dummyOperationDelegate;
        //protected FakeSampleCopyOutDelegate fakeCopyOutDelegate;
        //protected SampleCopyInDelegate copyInDelegate;
        //protected SampleCopyOutDelegate copyOutDelegate;
        //protected SampleReaderCopyDelegate readerCopyDelegate;
        //protected SampleReaderAllocDelegate readerAllocDelegate;

    
        // HACK: This is a Mono workaround. Currently you cannot marshal a delegate
        // to a function pointer if the parameters include an "object" type. So we
        // will pass a fake delegate, and then internally use the real copyOut, this
        // may give better performance anyways, since we won't have to convert the
        // IntPtr to a Delegate for every ReaderCopy invocation.
        public void DummyOperation()//)IntPtr from, IntPtr to, int offset)
        { }

        public TypeSupport(
            Type dataType,
            BaseMarshaler marshaler)
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
            
            if (marshaler == null && generator == null) 
            {
                // TODO: Write error in message log.
                return result;
            }

            // Keeps these delegates alive and away from the GC
            //this.copyInDelegate = copyInDelegate;
            //this.copyOutDelegate = copyOutDelegate;
            //this.readerAllocDelegate = readerAllocDelegate;
            //this.readerCopyDelegate = new SampleReaderCopyDelegate(ReaderCopy);
            IntPtr domainObj = (participant as DomainParticipant).GapiPeer;
            //this.fakeCopyOutDelegate = new FakeSampleCopyOutDelegate(FakeCopyOut);
            
            result = Gapi.FooTypeSupport.register_type(
                GapiPeer,
                domainObj,
                typeName);
                        
            
            if (marshaler == null)
            {
                // Get the attribute names and offsets of this datatype.
                IntPtr metaData = Gapi.DomainParticipant.get_type_metadescription(domainObj, typeName);
                 
                // Generate a new marshaller using the available generator.
                marshaler = BaseMarshaler.Create(domainObj, metaData, dataType, generator);
                
                // Attach the functions in the generated marshaler to the current TypeSupport.
                result = AttachMarshalerDelegates(this.GapiPeer, marshaler);
                if (result == ReturnCode.Ok)
                {
                    // Attach the same functions to the copy of the TypeSupport that was already 
                    // registered in the Domain.
                    IntPtr registeredTS = Gapi.DomainParticipant.get_typesupport(domainObj, typeName);
                    result = AttachMarshalerDelegates(registeredTS, marshaler);
                }
            }
            


            // TODO: Is this Type_Name actually going to be different after 
            // register_type is called?
            //InitType(domainObj.GapiPeer);

            return result;
        }
        
        private ReturnCode AttachMarshalerDelegates(IntPtr ts, BaseMarshaler marshaler)
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
        

        /*public IntPtr GetMetaPtr(IDomainParticipant participant, string typeName)
        {
            DomainParticipant domainObj = (DomainParticipant)participant;
            return domainObj.GetTypeMetaDescription(typeName);
        }*/

        //protected abstract void InitType(IntPtr participant);

        public abstract DataWriter CreateDataWriter(IntPtr gapiPtr);

        public abstract DataReader CreateDataReader(IntPtr gapiPtr);

//		public static IntPtr GetArrayType(IntPtr basePtr, string arrayType, string subType, int maxLength)
//		{
			//subtype0 = c_type(c_metaResolve(c_metaObject(base), "c_long"));
			//type0 = c_metaArrayTypeNew(c_metaObject(base), "C_SEQUENCE<c_long>", subtype0, 0);
			//c_free(subtype0);

//			IntPtr subPtr = Gapi.DDSDatabase.metaResolve(basePtr, subType);
//			IntPtr arrayPtr = Gapi.DDSDatabase.metaArrayTypeNew(basePtr, arrayType, subPtr, maxLength);
//			Gapi.DDSDatabase.free(subPtr);

//			return arrayPtr;
//		}

//		public static IntPtr NewArray(IntPtr dataType, int size)
//		{
//			return Gapi.DDSDatabase.newArray(dataType, size);
//		}

//		public static int ArraySize(IntPtr arrayPtr)
//		{
//			return Gapi.DDSDatabase.arraySize(arrayPtr);
//		}

//        protected static Type CreateReaderType(Type type)
//        {
//            return ReaderWriterTypeGenerator.CreateReaderType(type);
//        }

//        protected static Type CreateWriterType(Type type)
//        {
//            return ReaderWriterTypeGenerator.CreateWriterType(type);
//        }
    }
}
