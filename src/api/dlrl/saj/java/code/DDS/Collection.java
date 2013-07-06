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
 * <P>A Collection is the interface that must be implemented by all collection 
 * types (Lists, Maps and Sets). It has an operation to return the number of 
 * contained elements.</P>
 */
public interface Collection{

	/**
	 * Returns the number of elements contained in the collection.
	 * 
	 * @return the number of contained elements.
	 * @throws DDS.AlreadyDeleted if the collection is already deleted.
	 */
	public int length () throws DDS.AlreadyDeleted;

	/**
	 * Clears all elements from the collection.
	 *
	 * @throws DDS.PreconditionNotMet if this collection is not contained within a writeable cache access.
	 * @throws DDS.AlreadyDeleted if the collection is already deleted.
	 */
	//NOT IN DESIGN
	public void clear() throws DDS.PreconditionNotMet, DDS.AlreadyDeleted;

} // interface Collection
