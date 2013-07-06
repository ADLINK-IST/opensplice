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

import org.opensplice.cm.data.State;

/**
 * Represents a Query in SPLICE-DDS. 
 */
public interface Query extends Reader {
    /**
     * Provides access to the query expression.
     * 
     * @return The query expression.
     */
    public String getExpression();
    
    public String getExpressionParams();
    
    public State getInstanceState();
    
    public State getSampleState();
    
    public State getViewState();
}
