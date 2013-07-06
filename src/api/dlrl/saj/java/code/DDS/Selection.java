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
 * <P>A Selection provides the means to designate a subset of DLRL object
 * instances of a specific type. A selection determines the correct subset
 * by means of the provided selection criterion. Selections can be created
 * and deleted by the corresponding typed ObjectHome. </P>
 *
 * <P>The current implementation of the selection does not support QueryCondition 
 * as a valid SelectionCriterion. By default every selection will be created with
 * auto_refresh = <code>false</code>. The value of concerns_contained is ignored.
 */
public interface Selection {

    /**
     * <P>This operation returns if the selection can be manually
     * refreshed using the refresh operation of the selection
     * (<code>false</code>) or if the selection is automatically
     * refreshed whenever the related {@link DDS.Cache} is refreshed
     * (<code>true</code>). Take note that it does not matter if the
     * related cache is in enabled or disabled update mode.</P>
     *
     * <P>The current implementation will always return false.</P>
     *
     * @throws DDS.AlreadyDeleted if the Selection is already deleted.
     * @return <code>true</code> if the Selection will be automatically
     * refreshed and <code>false</code> if it will not.
     */
	boolean auto_refresh () throws DDS.AlreadyDeleted;

    /**
     * <P>This operation returns true if the Selection considers
     * change to the contained relations of it's member objects
     * as a modification to itself and false if it only takes 
     * changes to it's member objects into account. This feature is 
     * only usefull when using the selection in combination with 
     * a selection listener, which is currently unsupported</P>
     *
     * <P>The current implementation will always return false.</P>
     *
     * @throws DDS.AlreadyDeleted if the Selection is already deleted.
     * @return <code>true</code> if the Selection sees modifications
     * to its contained members as modifications to itself and 
     * <code>false</code> otherwise.
     */
	boolean concerns_contained () throws DDS.AlreadyDeleted;

    /**
     * <P>This operation updates the membership of the selection. Any 
     * objects that no longer pass the criterion are removed and 
     * objects that now match the criterion are added. If this operation
     * is called the {@link DDS.Selection#auto_refresh} returns <code>true</code>
     * then this operation is considered a no-op.</P>
     *
     * @throws DDS.AlreadyDeleted if the Selection is already deleted.
     */
	void refresh () throws DDS.AlreadyDeleted;

    /**
     * <P>This operation returns the {@link DDS.SelectionCriterion} class
     * that belongs to the Selection. </P>
     *
     * @throws DDS.AlreadyDeleted if the Selection is already deleted.
     * @return the criterion that belongs to the selection
     */
	DDS.SelectionCriterion criterion() throws DDS.AlreadyDeleted;
} // interface Selection
