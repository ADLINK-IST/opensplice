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
package org.opensplice.config.swing;

import javax.swing.JMenuItem;

import org.opensplice.config.data.DataNode;
import org.opensplice.config.meta.MetaNode;

public class DataNodeMenuItem extends JMenuItem {
    private static final long serialVersionUID = -5909316541978936911L;
    private DataNode data;
    private MetaNode childMeta;
    private DataNodePopupSupport source;
    
    public DataNodeMenuItem(String name, DataNode data, MetaNode childMeta){
        super(name);
        this.data = data;
        this.childMeta = childMeta;
        this.source = null;
    }
    
    public DataNodeMenuItem(String name, DataNode data, MetaNode childMeta, DataNodePopupSupport source){
        super(name);
        this.data = data;
        this.childMeta = childMeta;
        this.source = source;
    }
    
    public DataNode getData(){
        return this.data;
    }
    
    public MetaNode getChildMeta(){
        return this.childMeta;
    }
    
    public DataNodePopupSupport getSource(){
        return this.source;
    }
}
