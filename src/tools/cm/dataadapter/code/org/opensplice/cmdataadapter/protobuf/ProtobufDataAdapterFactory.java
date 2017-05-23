/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
 *
 */
package org.opensplice.cmdataadapter.protobuf;

/**
 * This class is the entrypoint for all calls to protobuf related classes and
 * methods. 
 * 
 * IMPORTANT: Due to the optional inclusion of the protobuf feature in Tester,
 * users of the Protobuf data adapter component must acquire the instance of the
 * ProtobufDataAdapter class through this factory, and check the presence of
 * protobuf feature through the {@link ProtobufDataAdapter#isEnabled()} method
 * before calling any other protobuf data adapter method.
 */
public class ProtobufDataAdapterFactory {

    private static ProtobufDataAdapter instance;

    public static ProtobufDataAdapter getInstance() {
        if (instance == null) {
            instance = new ProtobufDataAdapterImpl();
        }
        return instance;
    }
}
