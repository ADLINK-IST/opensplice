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
package DDS;

public final class ViewKeyQosPolicy {

    public boolean use_key_list;
    public java.lang.String[] key_list = new java.lang.String[0];

    public ViewKeyQosPolicy() {
    }

    public ViewKeyQosPolicy(boolean _use_key_list, java.lang.String[] _key_list)
    {
        use_key_list = _use_key_list;
        key_list = _key_list;
    }

}
