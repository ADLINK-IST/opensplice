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
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void DeleteEntityActionDelegate(IntPtr objPtr, IntPtr arg);

    /**
     * This interface is implemented by all DDS defined classes and holds the 
     * adress of the equivalent <code>gapi</code> object. The adress is stored as 
     * a (platform-dependent) <code>int</code> because the <code>gapi</code> can 
     * be compiled for any platform using up to 64 bit adressing. 
     */
    public class SacsSuperClass : IDisposable
    {
        private bool WeakRef;
        private DeleteEntityActionDelegate deleteEntityAction;

        /**
         * The adress of the equivalent object in the <code>gapi</code> 
         */
        private IntPtr gapiPeer = IntPtr.Zero;
        public IntPtr GapiPeer { get { return gapiPeer; } }

        private IntPtr handleSelf = IntPtr.Zero;
        protected IntPtr HandleSelf { get { return handleSelf; } }


        /* This constructor is only used by all DDS classes that have a 
         * default constructor (i.e. TypeSupport, WaitSet, GuardCondition).
         * It is due to the fact that the Gapi.<Entity>.alloc() needs to
         * be called in order to get a gapi handle, but this cannot be done
         * in the constructor in this stage. 
         * The caller must call SetPeer and pass the gapi handle as soon as
         * he has obtained it.
         */
        internal SacsSuperClass()
        {
            this.deleteEntityAction = null;
        }

        /* This is the default constructor that is to be used by all DDS 
         * classes that have a corresponding Gapi.Create<Entity> operation.
         * The constuctor of these classes should only be invoked AFTER the
         * gapi has created the corresponding gapi handles. 
         */
        protected SacsSuperClass(IntPtr gapiPtr)
        {
            this.deleteEntityAction = DeleteEntityAction;
            this.SetPeer(gapiPtr, false);
        }



        /* Only to be invoked when the default (empty) constructor has been used.
         * Use this operation to register a gapi handle that has not yet 
         * passed to the constructor before. The GCHandle created this way
         * will be a weak handle, since the lifecycle of the C# object will
         * need to be fully determined by the application: the gapi may not
         * keep unreferenced C# objects alive. 
         */
        internal void SetPeer(IntPtr gapiPtr, bool isWeak)
        {
            GCHandleType backRefType;
            this.WeakRef = isWeak;

            // Determine the GCHandleType to be used.
            if (isWeak)
            {
                backRefType = GCHandleType.Weak;
            }
            else
            {
                backRefType = GCHandleType.Normal;
            }
            
            // Get the GCHandle for the current C# object and cache the IntPtr of the GCHandle
            // in the UserData field of its gapi pointer.
            GCHandle tmpGCHandle = GCHandle.Alloc(this, backRefType);
            handleSelf = GCHandle.ToIntPtr(tmpGCHandle);
            Gapi.Entity.set_user_data(gapiPtr, handleSelf, deleteEntityAction, IntPtr.Zero);

            // Store the gapi handle in the object itself.
            gapiPeer = gapiPtr;
        }

        // Using the Dispose/Finalize pattern from: 
        //   http://msdn.microsoft.com/en-us/library/b1yfkh5e(vs.71).aspx

        //Implement IDisposable.
        public void Dispose()
        {
            // Weak references are slaved to their wrappers lifecycle. 
            // So if the wrapper is disposed, so should the corresponding
            // gapi handle.
            if (WeakRef)
            {
                Gapi.GenericAllocRelease.Free(gapiPeer);
            }

            GCHandle tmpGCHandle = GCHandle.FromIntPtr(handleSelf);
            tmpGCHandle.Free();
            handleSelf = IntPtr.Zero;

            // Reset the gapi handles stored in this object.
            gapiPeer = IntPtr.Zero;

            GC.SuppressFinalize(this);
        }

        // Use C# destructor syntax for finalization code.
        ~SacsSuperClass()
        {
            // Check if the wrapper already disposed its gapi entity before.
            if (handleSelf != IntPtr.Zero)
            {
                // If not, simply call Dispose.
                Dispose();
            }
        }

        internal static SacsSuperClass fromUserData(IntPtr gapiPtr)
        {
            SacsSuperClass entity = null;

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
                    entity = tmpGCHandle.Target as SacsSuperClass;
                }
            }

            return entity;
        }

        internal void DeleteEntityAction(IntPtr entityData, IntPtr arg)
        {
            Dispose();
        }

    }
}
