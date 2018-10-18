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
 *
 */
package org.opensplice.cm.qos;

public class SharePolicy {
    public String name;
    public boolean enable;
    public static final SharePolicy DEFAULT = new SharePolicy(null, false);
    
    public SharePolicy(String _name, boolean _enable){
        name = _name;
        enable = _enable;
    }
    
    public SharePolicy copy(){
        String str;
        if(this.name != null){
            str = new String(this.name);
        } else {
            str = null;
        }
        return new SharePolicy(str, this.enable);
    }
}
