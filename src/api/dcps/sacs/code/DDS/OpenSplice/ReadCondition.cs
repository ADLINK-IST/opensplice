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
using DDS;

namespace DDS.OpenSplice
{
    /// <summary>
    /// The DataReader objects can create a set of ReadCondition (and
    /// StatusCondition) objects which provide support (in conjunction with WaitSet
    /// objects) for an alternative communication style between the Data Distribution
    /// Service and the application (i.e., state-based rather than event-based).
    /// ReadCondition objects allow an DataReader to specify the data samples it is
    /// interested in (by specifying the desired sample-states, view-states, and
    /// instance-states); see the parameter definitions for DataReader's
    /// create_readcondition operation. This allows the Data Distribution Service to
    /// trigger the condition only when suitable information is available. ReadCondition
    /// objects are to be used in conjunction with a WaitSet. More than one
    /// ReadCondition may be attached to the same DataReader.
    /// </summary>
    public class ReadCondition : Condition, IReadCondition
    {
        internal ReadCondition(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
        }

        /// <summary>
        /// This operation returns the set of sample_states that are taken into account to
        /// determine the trigger_value of the ReadCondition.
        /// </summary>
        /// <returns>The sample_states specified when the ReadCondition was created.</returns>
        public SampleStateKind GetSampleStateMask()
        {
            return Gapi.ReadCondition.get_sample_state_mask(GapiPeer);
        }

        /// <summary>
        /// This operation returns the set of view_states that are taken into account to
        /// determine the trigger_value of the ReadCondition.
        /// </summary>
        /// <returns>The view_states specified when the ReadCondition was created.</returns>
        public ViewStateKind GetViewStateMask()
        {
            return Gapi.ReadCondition.get_view_state_mask(GapiPeer);
        }

        /// <summary>
        /// This operation returns the set of instance_states that are taken into account to
        /// determine the trigger_value of the ReadCondition.
        /// </summary>
        /// <returns>The instance_states specified when the ReadCondition was created.</returns>
        public InstanceStateKind GetInstanceStateMask()
        {
            return Gapi.ReadCondition.get_instance_state_mask(GapiPeer);
        }

        /// <summary>
        /// This operation returns the DataReader associated with the ReadCondition.
        /// </summary>
        /// <returns>The DataReader associated with the ReadCondition.</returns>
        public IDataReader GetDataReader()
        {
            IntPtr gapiPtr = Gapi.ReadCondition.get_datareader(GapiPeer);
            IDataReader dataReader = SacsSuperClass.fromUserData(gapiPtr) as IDataReader;
            return dataReader;
        }
    }
}
