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
package org.opensplice.config.meta;

public class MetaException extends Exception {
    private static final long serialVersionUID = 4459748108068852410L;
    private MetaExceptionType type;
    
    public MetaException(String message, MetaExceptionType type) {
        super(message);
        this.type = type;
    }
    
    public MetaException(String message) {
        super(message);
        this.type = MetaExceptionType.META_ERROR;
    }
    
    public MetaExceptionType getType() {
        return this.type;
    }
}
