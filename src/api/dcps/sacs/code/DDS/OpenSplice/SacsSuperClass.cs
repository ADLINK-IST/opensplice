/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

using System;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Threading;
using DDS.OpenSplice.Kernel;

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
        private bool WeakRef;
        private bool deinitialized = false;

        /**
         * The adress of the equivalent object in the <code>gapi</code>
         */
        private IntPtr userPeer = IntPtr.Zero;
        public IntPtr rlReq_UserPeer { get { return userPeer; } }

        private IntPtr handleSelf = IntPtr.Zero;
        internal IntPtr rlReq_HandleSelf { get { return handleSelf; } }


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
        }

        /*
         * Use C# destructor syntax for finalization code.
         * Note that this finalizer is only invoked when not yet suppressed by
         * an invocation of GC.SuppressFinalize().
         */
        ~SacsSuperClass()
        {
            if (userPeer != IntPtr.Zero)
            {
                ReportStack.Start();

                ReturnCode result = uResultToReturnCode(
                        DDS.OpenSplice.User.Object.Free(userPeer));
                Debug.Assert(result == DDS.ReturnCode.Ok);

                ReportStack.Flush(this, result != ReturnCode.Ok);
            }
        }

        /* This operation assumes it is invoked by the init() operations
         * of each of its children. Since an object is not yet accessable
         * before finalization of its init(), no lock is required to set
         * the address of the UserLayer object which it represents.
         * Weak Objects are objects that do not leave a pinned backref to
         * their C# wrappers in the UserLayer administration, since that
         * would prevent an object from becoming garbage collected when
         * there is no explicit API call to do so. Examples of objects
         * that need a weakRef are the WaitSet and the GuardCondition,
         * since the only way to get rid of those is to drop all references
         * to them and wait until the Garbage Collector comes to clean it up.
         */
        internal virtual ReturnCode init(IntPtr userPtr, bool isWeak)
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

            // Store the handle in the object itself.
            userPeer = userPtr;

            return DDS.ReturnCode.Ok;
        }

        /**
         * deinit() takes care of all deinitilization by invoking a virtual wlReq_deinit() call
         * that will be overridden by each child and that will be recursively traversed upward.
         */
        internal ReturnCode deinit()
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            lock(this)
            {
                if (this.rlReq_isAlive) {
                    result = wlReq_deinit();
                }
            }

            return result;
        }

        internal virtual ReturnCode wlReq_deinit()
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            // Weak references are slaved to their wrappers lifecycle.
            // So if the wrapper is disposed, so should the corresponding
            // UserLayer handle.
            deinitialized = true;
            if (userPeer != IntPtr.Zero)
            {
                result = uResultToReturnCode(
                        DDS.OpenSplice.User.Object.Close(userPeer));

                GCHandle tmpGCHandle = GCHandle.FromIntPtr(handleSelf);
                tmpGCHandle.Free();
                handleSelf = IntPtr.Zero;
            }

            // Make sure that Dispose() is not invoked a 2nd time by the destructor/finalizer.
            GC.SuppressFinalize(this);

            return result;
        }

        /* Using the Dispose/Finalize pattern from:
         * http://msdn.microsoft.com/en-us/library/b1yfkh5e(vs.71).aspx
         */
        public void Dispose()
        {
            deinit();
        }

        private int _MyDomainId = -1;
	public int MyDomainId
        {
            get { return _MyDomainId; }
            internal set { _MyDomainId = value; }
	}


        internal bool rlReq_isAlive
        {
            get
            {
                bool result = true;

                if (deinitialized) {
                    result = false;
                    ReportStack.Report(
                            DDS.ReturnCode.AlreadyDeleted,
                            "Trying to invoke an operation on an already deleted OpenSplice object.");
                }
                return result;
            }
        }

        internal static SacsSuperClass fromUserData(IntPtr userPtr)
        {
            SacsSuperClass entity = null;

            // Check whether the userPtr contains a valid pointer.
            if (userPtr != IntPtr.Zero)
            {
                // get the user data out of the user object
                IntPtr managedIntPtr = User.Observable.GetUserData(userPtr);

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

        internal static DDS.ReturnCode uResultToReturnCode(V_RESULT uResult)
        {
            DDS.ReturnCode result;

            switch (uResult) {
                case V_RESULT.OK:
                    result = DDS.ReturnCode.Ok;
                    break;
                case V_RESULT.OUT_OF_MEMORY:
                    result = DDS.ReturnCode.OutOfResources;
                    break;
                case V_RESULT.ILL_PARAM:
                    result = DDS.ReturnCode.BadParameter;
                    break;
                case V_RESULT.CLASS_MISMATCH:
                    result = DDS.ReturnCode.PreconditionNotMet;
                    break;
                case V_RESULT.DETACHING:
                    result = DDS.ReturnCode.AlreadyDeleted;
                    break;
                case V_RESULT.TIMEOUT:
                    result = DDS.ReturnCode.Timeout;
                    break;
                case V_RESULT.OUT_OF_RESOURCES:
                    result = DDS.ReturnCode.OutOfResources;
                    break;
                case V_RESULT.INCONSISTENT_QOS:
                    result = DDS.ReturnCode.InconsistentPolicy;
                    break;
                case V_RESULT.IMMUTABLE_POLICY:
                    result = DDS.ReturnCode.ImmutablePolicy;
                    break;
                case V_RESULT.PRECONDITION_NOT_MET:
                    result = DDS.ReturnCode.PreconditionNotMet;
                    break;
                case V_RESULT.ALREADY_DELETED:
                    result = DDS.ReturnCode.AlreadyDeleted;
                    break;
                case V_RESULT.HANDLE_EXPIRED:
                    result = DDS.ReturnCode.BadParameter;
                    break;
                case V_RESULT.UNSUPPORTED:
                    result = DDS.ReturnCode.Unsupported;
                    break;
                case V_RESULT.NO_DATA:
                    result = DDS.ReturnCode.NoData;
                    break;
                case V_RESULT.NOT_ENABLED:
                    result = DDS.ReturnCode.NotEnabled;
                    break;
                case V_RESULT.UNDEFINED:
                case V_RESULT.INTERRUPTED:
                case V_RESULT.INTERNAL_ERROR:
                default:
                    result = DDS.ReturnCode.Error;
                    break;
                }
            return result;
        }
    }
}
