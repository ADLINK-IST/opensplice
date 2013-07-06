/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
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
