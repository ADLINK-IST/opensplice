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
 * Prismtech implementation of the {@link DDS.Selection} interface.
 */
public abstract class SelectionImpl implements DDS.Selection {

    private long admin;

    protected SelectionImpl(){}

    /* see DDS.Selection for javadoc */
    public final boolean auto_refresh () throws DDS.AlreadyDeleted {
        return jniAutoRefresh();
    }

    /* see DDS.Selection for javadoc */
    public final boolean concerns_contained () throws DDS.AlreadyDeleted {
        return jniConcernsContained();
    }

    /* see DDS.Selection for javadoc */
    public final void refresh () throws DDS.AlreadyDeleted {
        jniRefresh();
    }

    /* see DDS.Selection for javadoc */
	public final DDS.SelectionCriterion criterion() throws DDS.AlreadyDeleted {
		return jniCriterion();
	}

    private native boolean jniAutoRefresh() throws DDS.AlreadyDeleted;
    private native boolean jniConcernsContained() throws DDS.AlreadyDeleted;
    private native void jniRefresh() throws DDS.AlreadyDeleted;
    private native DDS.SelectionCriterion jniCriterion() throws DDS.AlreadyDeleted;
    protected native DDS.ObjectRoot[] jniMembers() throws DDS.AlreadyDeleted;
    private native void jniDeleteSelection();
    protected abstract int[] checkObjects(DDS.FilterCriterion filter, DDS.ObjectRoot[] objects);
    protected native DDS.SelectionListener jniSetListener(DDS.SelectionListener listener) throws DDS.AlreadyDeleted;
    protected native DDS.SelectionListener jniListener() throws DDS.AlreadyDeleted;
    protected native DDS.ObjectRoot[] jniInsertedMembers() throws DDS.AlreadyDeleted;
    protected native DDS.ObjectRoot[] jniModifiedMembers() throws DDS.AlreadyDeleted;
    protected native DDS.ObjectRoot[] jniRemovedMembers() throws DDS.AlreadyDeleted;

    protected final void finalize(){
	    jniDeleteSelection();
    }
}