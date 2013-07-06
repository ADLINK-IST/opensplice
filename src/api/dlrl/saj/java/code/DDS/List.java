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
//NOT IN DESIGN - entire class

package DDS;

/**
 * The List class is not supported.
 *
 * <P>The List is an abstract class that represents a linked list that can store elements identified 
 * by an Integer index type. The operations to store and retrieve elements in the list are 
 * implemented in a specialized sub-class.</P>
 */
public abstract class List implements DDS.Collection{

    private long admin = 0;

    protected List(){}

    public final int length() throws DDS.AlreadyDeleted{
        return jniLength();
    }

    /**
     * This operation is not supported.
     *
     * Clears the contents of the collection, does not affect results of the added/modified/removed values methods.
     *
     * A PreconditionNotMet is raised if any of the following preconditions is violated:   
     * <ul><li>The List is not located in a (writeable) CacheAccess;</li>
     * <li>The List belongs to an ObjectRoot which is not yet registered.</li></ul>
     *
     * @throws DDS.AlreadyDeleted if the list is already deleted.
     * @throws DDS.PreconditionNotMet If one of the preconditions was not met.
     */
	public final void clear() throws DDS.PreconditionNotMet, DDS.AlreadyDeleted{
		jniClear();
	}

    /**
     * This operation is not supported.
     *
     * Returns the indexes of all elements that were added during the last update round.
     * Changes to the list as a result of manipulation by a local application are not 
     * taken into account. When this collection belongs to an ObjectRoot with 
     * read_state VOID then this operation will always return a zero length array.
     * 
     * @return the indexes of all elements that were added during the last update round.
     * @throws DDS.AlreadyDeleted if the list is already deleted.
     */
    public final int[] added_elements() throws DDS.AlreadyDeleted{
        return jniAddedElements();
    }

    /**
     * This operation is not supported.
     *
     * Returns the indexes of all elements that were modified during the last update round.
     * Changes to the list as a result of manipulation by a local application are not 
     * taken into account. When this collection belongs to an ObjectRoot with read_state
     * VOID or OBJECT_NEW then this operation will always return a zero length array.
     * 
     * @return the indexes of all elements that were modified during the last update round.
     * @throws DDS.AlreadyDeleted if the list is already deleted.
     */
    public final int[] modified_elements() throws DDS.AlreadyDeleted{
        return jniModifiedElements();
    }

    /**
     * This operation is not supported.
     *
     * Returns the indexes of all elements that were removed during the last update round.
     * Changes to the list as a result of manipulation by a local application are not 
     * taken into account. When this collection belongs to an ObjectRoot with read_state
     * VOID or OBJECT_NEW then this operation will always return a zero length array.
     * 
     * @return the indexes of all elements that were removed during the last update round.
     * @throws DDS.AlreadyDeleted if the list is already deleted.
     */
    public final int[] removed_elements() throws DDS.AlreadyDeleted{
        return jniRemovedElements();
    }

    /**
     * This operation is not supported.
     *
     * Removes the element with the highest index (i.e. last element) from the list. 
     * 
     * A PreconditionNotMet is raised if any of the following preconditions is violated:   
     * <ul><li>The List is not located in a (writeable) CacheAccess;</li>
     * <li>The List belongs to an ObjectRoot which is not yet registered.</li></ul>
     *
     * @throws DDS.AlreadyDeleted if the list is already deleted.
     * @throws PreconditionNotMet if any of the preconditions was not met
     */
    public final void remove() throws DDS.AlreadyDeleted, DDS.PreconditionNotMet{
        jniRemove();
    }

    protected final void finalize(){
	    jniDeleteList();
    }

    private native int jniLength() throws DDS.AlreadyDeleted;	
    private native int[] jniAddedElements() throws DDS.AlreadyDeleted;
    private native int[] jniModifiedElements() throws DDS.AlreadyDeleted;
    private native int[] jniRemovedElements() throws DDS.AlreadyDeleted;
    private native void jniRemove() throws DDS.PreconditionNotMet, DDS.AlreadyDeleted;
    private native void jniClear() throws DDS.PreconditionNotMet, DDS.AlreadyDeleted;
    private native void jniDeleteList();    
    protected native DDS.ObjectRoot[] jniGetValues()throws DDS.AlreadyDeleted;
    protected native void jniAdd(DDS.ObjectRoot value) throws DDS.PreconditionNotMet, DDS.AlreadyDeleted;
    protected native void jniPut(int key, DDS.ObjectRoot value) throws DDS.PreconditionNotMet, DDS.AlreadyDeleted;
    protected native DDS.ObjectRoot jniGet(int key) throws DDS.AlreadyDeleted, DDS.NoSuchElement;


} // class IntMap
