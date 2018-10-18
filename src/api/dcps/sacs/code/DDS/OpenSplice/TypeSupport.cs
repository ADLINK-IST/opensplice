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
using System.Collections.Generic;
using System.Runtime.InteropServices;
using DDS;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    public abstract class TypeSupport : ITypeSupport
    {
        private string typeDescriptor;
        private string typeName;
        private string internalTypeName;
        private string keyList;
        private Type dataType;

        public Type TypeSpec
        {
            get
            {
                return dataType;
            }
        }

        public string TypeName
        {
            get
            {
                return typeName;
            }
        }

        public string InternalTypeName
        {
            get
            {
                return internalTypeName;
            }
        }

        public string TypeDescriptor
        {
            get
            {
                return typeDescriptor;
            }
        }


        public string KeyList
        {
            get
            {
                return keyList;
            }
        }


        /*
         * Constructor for a TypeSupport that uses the specified (custom) Marshaler.
         */
        public TypeSupport(Type dataType, string[] descriptorArr, string typeName, string internalTypeName, string keyList)
        {
            this.dataType = dataType;
            this.typeName = typeName;
            this.internalTypeName = (internalTypeName.Length > 0) ? internalTypeName : typeName;
            this.keyList = keyList;

            System.Text.StringBuilder descriptor = new System.Text.StringBuilder();
            foreach (string s in descriptorArr)
            {
                descriptor.Append(s);
            }
            typeDescriptor = descriptor.ToString();
        }

        public abstract DataWriter CreateDataWriter(DatabaseMarshaler marshaler);
        public abstract DataReader CreateDataReader(DatabaseMarshaler marshaler);

        // The RegisterType operation should be implemented in its type specific specialization.
        public abstract ReturnCode RegisterType(
                IDomainParticipant participant,
                string typeName);

        public virtual ReturnCode RegisterType (
                IDomainParticipant participant,
                string typeName,
                DatabaseMarshaler marshaler)
        {
            ReturnCode result = ReturnCode.BadParameter;
            DomainParticipant dp;

            ReportStack.Start ();
            if (participant == null) {
                ReportStack.Report (result, "domain '<NULL>' is invalid.");
            } else if (marshaler == null) {
                ReportStack.Report (result, "marshaler '<NULL>' is invalid.");
            } else {
                dp = participant as DomainParticipant;
                if (dp == null) {
                    ReportStack.Report (result, "domain is invalid, not of type " +
                        "DDS::OpenSplice::DomainParticipant");
                } else {
                    if (typeName == null) {
                        typeName = this.typeName;
                    }
                    result = dp.nlReq_LoadTypeSupport (this, typeName);
                    if (result == ReturnCode.AlreadyDeleted) {
                        result = ReturnCode.BadParameter;
                    } else {
                        DatabaseMarshaler.Add (dp, dataType, marshaler);
                        marshaler.InitEmbeddedMarshalers (dp);
                    }
                }
            }
            ReportStack.Flush(null, result != ReturnCode.Ok);
            return result;
        }
    }
}
