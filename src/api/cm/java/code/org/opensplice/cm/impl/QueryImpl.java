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
package org.opensplice.cm.impl;

import org.opensplice.cm.CMException;
import org.opensplice.cm.CMFactory;
import org.opensplice.cm.Query;
import org.opensplice.cm.ReaderSnapshot;
import org.opensplice.cm.com.CommunicationException;
import org.opensplice.cm.com.Communicator;
import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.status.Status;
import org.opensplice.cm.data.State;
/**
 * Implementation of the Query interface.
 * 
 * @date May 18, 2005 
 */
public class QueryImpl extends ReaderImpl implements Query{
    private String expression;
    private String params;
    private State instanceState;
    private State sampleState;
    private State viewState;
    
    /** 
     * Constructs a new Query from the supplied arguments. This function
     * is for internal use only and should not be used by API users.
     * 
     * @param _index The index of the handle of the kernel entity that is
     *               associated with this entity.
     * @param _serial The serial of the handle of the kernel entity that is
     *                associated with this entity.
     * @param _pointer The address of the user layer entity that is associated
     *                 with this entity.
     * @param _name The name of the kernel entity that is associated with this
     *              entity.
     * @param _expression The query expression that is used to create the 
     *                    query.
     */
    public QueryImpl(Communicator communicator, long _index, long _serial, String _pointer, String _name,
            String _expression, String _params, State _instanceState, 
            State _sampleState, State _viewState) {
        super(communicator, _index, _serial, _pointer, _name);
        expression = _expression;
        params = _params;
        instanceState = _instanceState;
        sampleState = _sampleState;
        viewState = _viewState;
        
    }
    
    /**
     * Creates a new Query from the supplied arguments. The Query is owned by
     * the caller of this constructor.
     *
     * @param source The Reader to attach the Query to.
     * @param name The name of the Query.
     * @param expression The query expression.
     * @throws CMException Thrown when Query could not be created.
     */
    public QueryImpl(ReaderImpl source, String name, String expression) throws CMException{
        super(source.getCommunicator(), 0, 0, "", "");
        owner = true;
        QueryImpl q;
        try {
            q = (QueryImpl)getCommunicator().queryNew(source, name, expression);
        } catch (CommunicationException e) {
            throw new CMException(e.getMessage());
        }
        if(q == null){
            throw new CMException("Query could not be created.");
        }
        this.index = q.index;
        this.serial = q.serial;
        this.name = q.name;
        this.pointer = q.pointer;
        this.expression = q.expression;
        this.enabled = q.enabled;
        this.params = null;
        this.instanceState = new State(0);
        this.sampleState = new State(0);
        this.viewState = new State(0);
        q.freed = true;
    }
    
    /**
     * Provides access to the query expression.
     * 
     * @return The query expression.
     */
    public String getExpression(){
        return expression;
    }
    
    public String getExpressionParams(){
        return params;
    }
    
    public State getInstanceState(){
        return instanceState;
    }
    
    public State getSampleState(){
        return sampleState;
    }
    
    public State getViewState(){
        return viewState;
    }
    
    public Status getStatus() throws CMException{
        throw new CMException("Entity type has no status.");
    }
    
    public QoS getQoS() throws CMException{
        throw new CMException("Entity type has no QoS.");
    }
    
    public ReaderSnapshot makeSnapshot() throws CMException{
        throw new CMException("Snapshot of Query temporarily not supported (see BUCS_PL_CR_546).");
    }
}
