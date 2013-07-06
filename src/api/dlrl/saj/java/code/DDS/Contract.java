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
 * <P>The current implementation does not yet support the notion of a 
 * Contract, so no operations in this interface are supported.</P>
 */
public interface Contract {

	DDS.ObjectRoot contracted_object();

	int depth();

	DDS.ObjectScope scope();

	void set_depth(int depth);

	void set_scope(DDS.ObjectScope scope) ;

} // interface Contract
