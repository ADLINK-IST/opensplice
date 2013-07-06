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
 * <P>A SelectionCriterion is the interface that must be implemented by all criterion 
 * types (Filters and Queries). It has an operation to return the specific kind of 
 * the criterion.</P>
 */
public interface SelectionCriterion {

    /**
     * This operation returns the kind of this criterion.
     *
     * @return an enumeration class to indicate what kind of criterion this is.
     */
	DDS.CriterionKind kind ();
} // interface Criterion
