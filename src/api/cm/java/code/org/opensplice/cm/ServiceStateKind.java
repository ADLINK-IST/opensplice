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
package org.opensplice.cm;



/**
 * Represents the kind of a ServiceState in SPLICE-DDS. 
 */
public class ServiceStateKind {
    private static final int _NONE                       = 0;
    private static final int _INITIALISING               = 1;
    private static final int _OPERATIONAL                = 2;
    private static final int _INCOMPATIBLE_CONFIGURATION = 3;
    private static final int _TERMINATING                = 4;
    private static final int _TERMINATED                 = 5;
    private static final int _DIED                       = 6;

    public static final ServiceStateKind NONE                       = new ServiceStateKind(_NONE);
    public static final ServiceStateKind INITIALISING               = new ServiceStateKind(_INITIALISING);
    public static final ServiceStateKind OPERATIONAL                = new ServiceStateKind(_OPERATIONAL);
    public static final ServiceStateKind INCOMPATIBLE_CONFIGURATION = new ServiceStateKind(_INCOMPATIBLE_CONFIGURATION);
    public static final ServiceStateKind TERMINATING                = new ServiceStateKind(_TERMINATING);
    public static final ServiceStateKind TERMINATED                 = new ServiceStateKind(_TERMINATED);
    public static final ServiceStateKind DIED                       = new ServiceStateKind(_DIED);
    
    private ServiceStateKind(int kind){}
    
    /**
     * Creates a String representation of the supplied kind.
     * 
     * @param sk The kind to convert to a String representation. 
     * @return The String representation of the supplied kind.
     */
    public static String getString(ServiceStateKind sk){
        String result = null;
        
        if(sk.equals(NONE))                            {   result = "NONE";            }
        else if(sk.equals(INITIALISING))               {   result = "INITIALISING";    }
        else if(sk.equals(OPERATIONAL))                {   result = "OPERATIONAL";     }
        else if(sk.equals(INCOMPATIBLE_CONFIGURATION)) {   result = "INCOMPATIBLE_CONFIGURATION";     }
        else if(sk.equals(TERMINATING))                {   result = "TERMINATING";     }
        else if(sk.equals(TERMINATED))                 {   result = "TERMINATED";      }
        else if(sk.equals(DIED))                       {   result = "DIED";            }
        else{   
            result = "NONE";
            assert false: "Unknown ServiceStateKind in ServiceStateKind.getString()"; 
        }
        return result;
    }
}
