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
 * <P>The IntMap is an abstract class that represents a Map that can store elements identified 
 * by an Integer key type. The operations to store and retrieve elements in the map are 
 * implemented in a specialized sub-class.</P>
 * <P>The Map offers operations that can produce the keys of all elements contained in the
 * Map, the keys of all elements that have been added in the last update round, the keys of all
 * elements that were modified during the last update round, and the keys of all elements that
 * were removed during the last update round. Changes to the map as a result of manipulation by
 * a local application cannot be retrieved this way.</P> 
 * <P>It is possible for an application to remove an element from the map by means of the key 
 * that identifies it. This is only possible for Collections that are located in a writeable 
 * CacheAccess.</P>
 */
public abstract class IntMap implements DDS.Collection{

    private long admin = 0;

    protected IntMap(){}

    public final int length() throws DDS.AlreadyDeleted{
        return jniLength();
    }

    /* Clears the contents of the collection, does not affect results of the added/modified/removed values methods.
     *
     * A PreconditionNotMet is raised if any of the following preconditions is violated:   
     * <ul><li>The Map is not located in a (writeable) CacheAccess;</li>
     * <li>The Map belongs to an ObjectRoot which is not yet registered.</li></ul>
     *
     * @throws DDS.AlreadyDeleted if the map is already deleted.
     * @throws DDS.PreconditionNotMet If one of the preconditions was not met.
     */
	//NOT IN DESIGN
	public final void clear() throws DDS.AlreadyDeleted, DDS.PreconditionNotMet{
		jniClear();
	}

    /**
     * Returns the keys of all elements that were added during the last update round.
     * Changes to the map as a result of manipulation by a local application are not 
     * taken into account. It is recommended to use the keys() operation instead of this
     * operation when dealing with a collection belonging to an ObjectRoot with read_state
     * OBJECT_NEW. In this case both lists will be equal and the keys() operation will give
     * better performance. But only in the described case, in other situations it's 
     * recommended to use this operation. When this collection belongs to an ObjectRoot with 
     * read_state VOID then this operation will always return a zero length array.
     * 
     * @return the keys of all elements that were added during the last update round.
     * @throws DDS.AlreadyDeleted if the map is already deleted.
     */
    public final int[] added_elements() throws DDS.AlreadyDeleted{
        return jniAddedElements();
    }

    /**
     * Returns the keys of all elements that were modified during the last update round.
     * Changes to the map as a result of manipulation by a local application are not 
     * taken into account. When this collection belongs to an ObjectRoot with read_state
     * VOID or OBJECT_NEW then this operation will always return a zero length array.
     * 
     * @return the keys of all elements that were modified during the last update round.
     * @throws DDS.AlreadyDeleted if the map is already deleted.
     */
    public final int[] modified_elements() throws DDS.AlreadyDeleted{
        return jniModifiedElements();
    }

    /**
     * Returns the keys of all elements that were removed during the last update round.
     * Changes to the map as a result of manipulation by a local application are not 
     * taken into account. When this collection belongs to an ObjectRoot with read_state
     * VOID or OBJECT_NEW then this operation will always return a zero length array.
     * 
     * @return the keys of all elements that were removed during the last update round.
     * @throws DDS.AlreadyDeleted if the map is already deleted.
     */
    public final int[] removed_elements() throws DDS.AlreadyDeleted{
        return jniRemovedElements();
    }

    /**
     * Returns the keys of all elements that are contained in the map.
     * 
     * @return the keys of all elements that are contained in the map.
     * @throws DDS.AlreadyDeleted if the map is already deleted.
     */
    public final int[] keys() throws DDS.AlreadyDeleted{
        return jniGetKeys();
    }

    /**
     * Removes the element that is identified by the specified key. 
     * 
     * A PreconditionNotMet is raised if any of the following preconditions is violated:   
     * <ul><li>The Map is not located in a (writeable) CacheAccess;</li>
     * <li>The Map belongs to an ObjectRoot which is not yet registered.</li></ul>
     *
     * @param key the key that identifies the element that is to be removed.
     * @throws DDS.AlreadyDeleted if the map is already deleted.
     * @throws PreconditionNotMet if any of the preconditions was not met.
     */
    public final void remove(int key) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet{
        jniRemove(key);
    }

    protected final void finalize(){
	    jniDeleteIntMap();
    }

    private native int jniLength() throws DDS.AlreadyDeleted;
	//NOT IN DESIGN
	private native void jniClear() throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;
    private native int[] jniAddedElements() throws DDS.AlreadyDeleted;
    private native int[] jniModifiedElements() throws DDS.AlreadyDeleted;
    private native int[] jniGetKeys() throws DDS.AlreadyDeleted;
    private native void jniRemove(int key) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;
    private native void jniDeleteIntMap();
    private native int[] jniRemovedElements() throws DDS.AlreadyDeleted;
    protected native DDS.ObjectRoot[] jniGetValues()throws DDS.AlreadyDeleted;
    protected native DDS.ObjectRoot jniGet(int key) throws DDS.AlreadyDeleted, DDS.NoSuchElement;
    protected native void jniPut(int key, DDS.ObjectRoot value) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;

} // class IntMap
