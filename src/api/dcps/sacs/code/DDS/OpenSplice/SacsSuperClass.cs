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

namespace DDS.OpenSplice
{
    /**
     * This interface is implemented by all DDS defined classes and holds the 
     * adress of the equivalent <code>gapi</code> object. The adress is stored as 
     * a (platform-dependent) <code>int</code> because the <code>gapi</code> can 
     * be compiled for any platform using up to 64 bit adressing. 
     */
    public class SacsSuperClass : IDisposable
    {
        /**
         * The adress of the equivalent object in the <code>gapi</code> 
         */
        private bool weak;

        private IntPtr gapiPeer = IntPtr.Zero;
        public IntPtr GapiPeer { get { return gapiPeer; } }

        private IntPtr handleSelf = IntPtr.Zero;
        protected IntPtr HandleSelf { get { return handleSelf; } }


        // This constructor is only used by the TypeSupport class. It is due
        // to the fact that the FooTypeSupport.alloc() is called and returns 
        // the peer that we need. The caller must call SetPeer in its ctor.
        protected SacsSuperClass()
        {/*Subclass...call SetPeer() */}

        protected void SetPeer(IntPtr gapiPtr)
        {
            if (gapiPeer == IntPtr.Zero)
            {
                // Get the GCHandle for the current C# object and cache the IntPtr of the GCHandle
                // in the UserData field of its gapi pointer.
                GCHandle tmpGCHandle = GCHandle.Alloc(this, GCHandleType.Normal);
                handleSelf = GCHandle.ToIntPtr(tmpGCHandle);
                Gapi.Entity.set_user_data(gapiPtr, handleSelf);

                // Store the gapi handle in the object itself.
                gapiPeer = gapiPtr;
            }
        }

        internal SacsSuperClass(IntPtr gapiPtr)
            : this(gapiPtr, false)
        { }

        protected SacsSuperClass(IntPtr gapiPtr, bool weak)
        {
            this.weak = weak;
            // Get the GCHandle for the current C# object and cache the IntPtr of the GCHandle
            // in the UserData field of its gapi pointer.
            GCHandle tmpGCHandle = GCHandle.Alloc(this, weak ? GCHandleType.Weak : GCHandleType.Normal);
            handleSelf = GCHandle.ToIntPtr(tmpGCHandle);
            Gapi.Entity.set_user_data(gapiPtr, handleSelf);

            // Store the gapi handle in the object itself.
            gapiPeer = gapiPtr;
        }

        // Using the Dispose/Finalize pattern from: 
        //   http://msdn.microsoft.com/en-us/library/b1yfkh5e(vs.71).aspx

        //Implement IDisposable.
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                // Free other state (managed objects).
            }
            // Free your own state (unmanaged objects).

            GCHandle tmpGCHandle = GCHandle.FromIntPtr(handleSelf);
            tmpGCHandle.Free();
            handleSelf = IntPtr.Zero;

            // TODO: Verify when we need to release unmanaged memory
            if (weak)
            {
                try
                {
                    Gapi.GenericAllocRelease.Free(gapiPeer);
                }
                catch (AccessViolationException)
                {
                    // TODO: JLS - We should probably figure out why this is happening.
                }
            }

            // Reset the gapi handles stored in this object.
            gapiPeer = IntPtr.Zero;

            // Set large fields to null.
        }

        // Use C# destructor syntax for finalization code.
        ~SacsSuperClass()
        {
            // Simply call Dispose(false).
            Dispose(false);
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
    }
}
