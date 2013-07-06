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
package DDS;

/**
 * <P>The Set is an abstract class that represents a Set that can store any number of elements 
 * in an un-ordered way. The operations to store and retrieve elements in the set are 
 * implemented in a specialized sub-class.</P>
 */
public abstract class Set implements DDS.Collection{
   
    private long admin = 0;

    protected Set(){}

    public final int length() throws DDS.AlreadyDeleted{
        return jniLength();
    }

	
    /* Clears the contents of the collection, does not affect results of the added/removed values methods.
     *
     * A PreconditionNotMet is raised if any of the following preconditions is violated:   
     * <ul><li>The Set is not located in a (writeable) CacheAccess;</li>
     * <li>The Set belongs to an ObjectRoot which is not yet registered.</li></ul>
     *
     * @throws DDS.AlreadyDeleted if the map is already deleted.
     * @throws DDS.PreconditionNotMet If one of the preconditions was not met.
     */
	//NOT IN DESIGN
    public final void clear() throws DDS.AlreadyDeleted, DDS.PreconditionNotMet{
		jniClear();
	}

    protected final void finalize(){        
	    jniDeleteSet();
    }

    protected native void jniAdd(DDS.ObjectRoot value) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;
    protected native void jniRemove(DDS.ObjectRoot value) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;
    protected native boolean jniContains(DDS.ObjectRoot value) throws DDS.AlreadyDeleted;
    private native int jniLength() throws DDS.AlreadyDeleted;
	//NOT IN DESIGN
	private native void jniClear() throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;
    private native void jniDeleteSet();
    protected native DDS.ObjectRoot[] jniAddedElements() throws DDS.AlreadyDeleted;
    protected native DDS.ObjectRoot[] jniRemovedElements() throws DDS.AlreadyDeleted;
    protected native DDS.ObjectRoot[] jniGetValues()throws DDS.AlreadyDeleted;

} // class Set
