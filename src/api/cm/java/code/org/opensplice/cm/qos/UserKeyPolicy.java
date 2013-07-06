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

public class UserKeyPolicy {
    public boolean enable;
    public String expression;
    
    public static final UserKeyPolicy DEFAULT = new UserKeyPolicy(false, null);
    
    /**
     * Constructs a new UserKeyPolicy.  
     */
    public UserKeyPolicy(boolean _enable, String _expression){
        enable = _enable;
        expression = _expression;
    }
    
    public UserKeyPolicy copy(){
        String str;
        if(this.expression != null){
            str = new String(this.expression);
        } else {
            str = null;
        }
        return new UserKeyPolicy(this.enable, str);
    }
}
