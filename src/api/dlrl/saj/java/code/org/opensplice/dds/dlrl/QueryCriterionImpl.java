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
package org.opensplice.dds.dlrl;

/** 
 * Prismtech implementation of the {@link DDS.QueryCriterion} interface. 
 */ 
public final class QueryCriterionImpl implements DDS.QueryCriterion { 

    private long admin;

    /* see DDS.QueryCriterion for javadoc */ 
    public final String expression() throws DDS.AlreadyDeleted{
        return jniExpression();
    }

    /* see DDS.QueryCriterion for javadoc */ 
    public final String[] parameters() throws DDS.AlreadyDeleted{
        return jniParameters();
    }

    /* see DDS.QueryCriterion for javadoc */ 
    public final void set_query(String expression, String[] parameters) throws DDS.AlreadyDeleted, DDS.SQLError{
        jniSetQuery(expression, parameters);
    }

    /* see DDS.QueryCriterion for javadoc */ 
    public final void set_parameters(String[] parameters) throws DDS.AlreadyDeleted, DDS.SQLError{
        jniSetParameters(parameters);
    }

    public final DDS.CriterionKind kind (){
        return DDS.CriterionKind.QUERY;
    }

    private native String jniExpression() throws DDS.AlreadyDeleted;

    private native String[] jniParameters() throws DDS.AlreadyDeleted;

    private native void jniSetQuery(String expression, String[] parameters) throws DDS.AlreadyDeleted, DDS.SQLError;

    private native void jniSetParameters(String[] parameters) throws DDS.AlreadyDeleted, DDS.SQLError;

}